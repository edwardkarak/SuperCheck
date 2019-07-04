static BOOL CALLBACK OpenLstDlg(HWND, UINT, WPARAM, LPARAM);   // dialog procedure for the open list dlg
static void lbset(HWND);                       // loads directory listing to list box
static void desel(HWND);                       // deselects anything, if selected
static LRESULT CALLBACK ListPrc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);  // subclasses listbox to allow double-click to open
static BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);     // About Dlg procedure
static BOOL CALLBACK PasswordDlg(HWND, UINT, WPARAM, LPARAM);      // Password Dlg
static currency settotbal(HWND hdlg);
void loadlog(HWND hStatic);

/* Dialog Procedures */
static BOOL CALLBACK OpenLstDlg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HWND hLB = GetDlgItem(hwnd, IDC_OPENLIST);
    static RECT dlgrect;
    static char sel[64];

    switch (msg) {
        case WM_INITDIALOG: {
            SetWindowSubclass(GetDlgItem(hwnd, IDC_OPENLIST), ListPrc, 0, 0);

            lbset(hwnd);
            desel(hwnd);

            for (int i = 0; i < ListBox_GetCount(GetDlgItem(hwnd, IDC_OPENLIST)); ++i) {
                char buf[MAX_PATH];
                ListBox_GetText(GetDlgItem(hwnd, IDC_OPENLIST), i, buf);
                if (isfreever(buf)) {
                    free2new(buf);
                }
            }
            lbset(hwnd);
            desel(hwnd);

            if (ListBox_GetCount(hLB) == 0) {
                enablecmd(ID_FILE_OPEN, false);
                DestroyWindow(hwnd);
            }

            EnableWindow(GetDlgItem(hwnd, ID_DEL), FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_RENAME), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);

            ListBox_DeleteString(hLB, ListBox_FindString(hLB, -1, ftitle(g_filename)));

            GetWindowRect(hwnd, &dlgrect);
            settotbal(hwnd);
            return TRUE;}
        case WM_GETMINMAXINFO:
            ((MINMAXINFO *)(lp))->ptMinTrackSize.x = dlgrect.right - dlgrect.left;
            ((MINMAXINFO *)(lp))->ptMinTrackSize.y = dlgrect.bottom - dlgrect.top;

            SetWindowLongPtr(hwnd, DWL_MSGRESULT, 0);
            return TRUE;
        case WM_COMMAND:
            if (LOWORD(wp) == IDC_OPENLIST && HIWORD(wp) == LBN_SELCHANGE) {
                int pos;
                pos = ListBox_GetCurSel(GetDlgItem(hwnd, IDC_OPENLIST));
                ListBox_GetText(GetDlgItem(hwnd, IDC_OPENLIST), pos, sel);
                if (empty(sel))
                    break;
                EnableWindow(GetDlgItem(hwnd, ID_DEL), TRUE);
                EnableWindow(GetDlgItem(hwnd, ID_RENAME), TRUE);
                EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
            } else if (LOWORD(wp) == IDC_OPENLIST && HIWORD(wp) == LBN_DBLCLK) {
                int pos = ListBox_GetCurSel(GetDlgItem(hwnd, IDC_OPENLIST));
                ListBox_GetText(GetDlgItem(hwnd, IDC_OPENLIST), pos, sel);
                if (empty(sel))
                    break;
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM) GetDlgItem(hwnd, IDC_OPENLIST));
            }

            switch (LOWORD(wp)) {
            case ID_RENAME:
                strcpy(g_filename, envcat(ENV_SAVE, sel));
                renm();
                lbset(hwnd);
                break;
            case ID_DEL:
                if (MessageBox(hwnd, cat(3, "Are you sure you want to remove ", sel, "?"), "Confirm Removal", MB_YESNO) == IDNO)
                    return TRUE;
                closeall();
                if (remove(envcat(ENV_SAVE, sel)) != 0) {
                    char msg[32];
                    strcpy(msg, cat(4, "Couldn't remove file ", sel, ": ", strerror(errno)));
                    errmsg(msg, "");
                }
                lbset(hwnd);
                settotbal(hwnd);
                desel(hwnd);
                break;
            case IDOK:  // "open" button
                if (isopen(sel)) {
                    errmsg("This file is already open", "");
                    break;
                }

                if (safecpy(g_filename, envcat(ENV_SAVE, sel), MAX_PATH) > MAX_PATH || strlen(ftitle(g_filename)) > MAX_FN) {
                    errmsg("File name too long", "");
                    break;
                }
                load();
                switch (askpasswd()) {
                    case CANCELED:
                        return TRUE;
                    case MATCH:
                    case NOPASSWORD:
                        SetTimer(g_hwndMain, IDT_AUTOSAVE, g_progsettings.autosaveMins * 60 * 1000, (TIMERPROC) NULL);

                        labelWnd(g_filename);
                        if (iscorr())
                            errmsg(cat(2, g_filename, " is not a valid Transactions file"), "Could not open file");

                        enableItms();
                        lvupd();
                        break;
                    }

            case IDCANCEL:
                DestroyWindow(hwnd);
                break;
            return TRUE;
        }
    }
    return FALSE;
}
static currency settotbal(HWND hdlg)
{
    HWND hLB = GetDlgItem(hdlg, IDC_OPENLIST);
    int i;
    char nam[MAX_PATH];
    headmeta hm;
    currency amt;

    for (i = amt = 0; i < ListBox_GetCount(hLB); ++i) {
        ListBox_GetText(hLB, i, nam);
        header(nam, &hm);
        if (hm.len == 0)
            continue;
        amt += hm.endbal;
    }
    fmtwrite(hdlg, ID_TOTBAL, "Total Balance: $%.2f", (amt));

    return amt;
}
static void desel(HWND hdlg)
{
    ListBox_SetCurSel(GetDlgItem(hdlg, IDC_OPENLIST), -1);
    EnableWindow(GetDlgItem(hdlg, IDOK), FALSE);
    EnableWindow(GetDlgItem(hdlg, ID_DEL), FALSE);
    EnableWindow(GetDlgItem(hdlg, ID_RENAME), FALSE);
}
static void lbset(HWND hOwner)
{
    DlgDirListA(hOwner, envcat(ENV_SAVE, "\\*.such"), IDC_OPENLIST, 0, DDL_ARCHIVE | DDL_SYSTEM);
}
static LRESULT CALLBACK ListPrc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(1, LBN_DBLCLK), (LPARAM) hWnd);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
void loadlog(HWND hStatic)
{
    HBITMAP hBmp = (HBITMAP) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDB_LOGO), IMAGE_BITMAP,245, 60,LR_DEFAULTCOLOR);

    SendMessage(hStatic, STM_SETIMAGE,  (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
}
static BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        HICON x = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICO), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) x);

        loadlog(GetDlgItem(hwnd, IDC_ABT_LOG));

        return TRUE;}
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) EndDialog(hwnd, IDOK);
        return TRUE;
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;
    default:
        return FALSE;
    }
}
HWND hQtyDlg;
LRESULT CALLBACK QtyOdValEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_CHAR) {
        if ((wParam >= 'a' && wParam <= 'z') || (wParam >= 'A' && wParam <= 'Z') || wParam == '-' || wParam == '$' || (ispunct(wParam) && wParam != '.')) {
            MessageBeep(0);
            return 0;
        } else if (wParam == '.') {
            SetFocus(GetDlgItem(hQtyDlg, IDC_QTY_UNPRC_CNTS));
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK qtydlg::QtyDlg(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
    static qtydlg *pThis;
    switch (wm) {
    case WM_INITDIALOG:
        pThis = (qtydlg *) lp;
        hQtyDlg = hwnd;
        SetWindowSubclass(GetDlgItem(hwnd, IDC_QTY_UNPRC), QtyOdValEdit, 0, 0);
        SetWindowSubclass(GetDlgItem(hwnd, IDC_QTY_UNPRC_CNTS), NumEditPrc, 0, 0);

        SetWindowLongPtr(GetDlgItem(hwnd, IDC_QTY_RES), GWL_STYLE, WS_CHILD);   // hide "result will be" for now

        Edit_LimitText(GetDlgItem(hwnd, IDC_QTY_UNPRC_CNTS), 2);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK) {
            BOOL success;

            pThis->unprc = (getamt(GetDlgItem(hwnd, IDC_QTY_UNPRC), GetDlgItem(hwnd, IDC_QTY_UNPRC_CNTS)));
            pThis->qty = (int) GetDlgItemInt(hwnd, IDC_QTY, &success, FALSE);
            if (!success)
                errmsg("Could not read qty box", "");


            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        else if (LOWORD(wp) == IDCANCEL) {
            memset(pThis, 0, sizeof pThis);
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}
int qtydlg::show()
{
    return DialogBoxParamA(GetModuleHandle(NULL), MAKEINTRESOURCEA(QTYDLG), g_hwndMain, qtydlg::QtyDlg, (LPARAM) this);
}
BOOL CALLBACK TutDlg(HWND h, UINT wm, WPARAM wp, LPARAM lp)
{
    switch (wm) {
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_TUT_SHOWHLP:
            showhelp("hlpIntro.html");
            break;
        case ID_TUT_MODCONFIG:
            confgdlg();
            break;
        case ID_TUT_EXIT:
            EndDialog(h, ID_TUT_EXIT);
            break;
        }
        g_progsettings.tut = false;
        g_progsettings.saveSettings();
        return TRUE;
    }
    return FALSE;
}

#include "newtrns.h"
BOOL CALLBACK inpbx::InputDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static inpbx *cur;

    HWND hEdit = GetDlgItem(hwnd, IDC_INPUT_USER);
    HWND hDescr = GetDlgItem(hwnd, IDC_INPUT_DESCR);
    switch (msg) {
    case WM_INITDIALOG: {
        cur = (inpbx *) lParam;
        if (cur->flags & IBF_PASSWORD)
            SendMessageW(hEdit, EM_SETPASSWORDCHAR, (WPARAM)(UINT) 0x25cf , 0);

        if (cur->flags & IBF_FILENAM) {
            cur->maxuser = MAX_FN;
        }

        SetWindowTextA(hDescr, cur->descr);
        SetWindowText(hwnd, cur->title);
        if (strcmp(cur->deftext, "") != 0)
            SetWindowText(hEdit,  cur->deftext);

        SendMessage(hEdit, EM_SETLIMITTEXT, cur->maxuser, 0);
        SendMessage(hEdit, EM_SETSEL, 0, -1);   // select all text

        if (empty(cur->thirdbtn))
            SetWindowLongPtr(GetDlgItem(hwnd, IDC_INPUT_THIRD), GWL_STYLE, WS_CHILD);
        else
            SetWindowTextA(GetDlgItem(hwnd, IDC_INPUT_THIRD), cur->thirdbtn);
        return TRUE;}
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_THIRD:
            EndDialog(hwnd, IDC_INPUT_THIRD);
            break;
        case IDOK:
            GetWindowText(hEdit, cur->input, MAXINPUT);
            if (cur->flags & IBF_FILENAM) {
                if (!isvalidfn(cur->input)) {
                    MessageBox(NULL, "The filename is invalid", "", MB_ICONSTOP);
                    break;
                }
            }


            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            memset(cur->input, 0, sizeof (cur->input));
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    default:
        return FALSE;
    }
}
BOOL CALLBACK segfaultdlgproc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_COMMAND:
        EndDialog(hdlg, LOWORD(wp));
        return TRUE;
    }
    return FALSE;
}
int inpbx::show()
{
    return (this->btn = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCEA(INPUTDLG), g_hwndMain, InputDlgProc, (LPARAM) this));
}
int inpbx::show(char *user, size_t nuser)
{
    this->btn = show();
    safecpy(user, this->input, nuser);

    return this->btn;
}

