/* New Deposit / Withdrawal Dlg */
#define CLEAR_AFTER         -10

HWND hNewTrnsDlg;
char catsel[MAXCAT];

char *fill2zeros(int);

LRESULT CALLBACK odValEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_CHAR) {
        if ((wParam >= 'a' && wParam <= 'z') || (wParam >= 'A' && wParam <= 'Z') || wParam == '-' || wParam == '$' || (ispunct(wParam) && wParam != '.')) {
            MessageBeep(0);
            return 0;
        } else if (wParam == '.') {
            SetFocus(GetDlgItem(hNewTrnsDlg, IDC_VAL_CENTS));
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
BOOL CALLBACK NewTrnsDlg(HWND hdlg, UINT m, WPARAM wparam, LPARAM lparam)
{
    #define IDC_SWITCHMOD       6002
    static int mode;
    static bool inOKandNew = false;

    //SetWindowTheme(GetDlgItem(hdlg, IDOK), " ", " ");
    switch (m) {
    case WM_INITDIALOG:{
        hNewTrnsDlg = hdlg;
        mode = (int) lparam;
        SetWindowSubclass(GetDlgItem(hdlg, IDC_VAL), odValEdit, 0, 0);
        SetWindowSubclass(GetDlgItem(hdlg, IDC_VAL_CENTS), NumEditPrc, 0, 0);

        Edit_LimitText(GetDlgItem(hdlg, IDC_VAL_CENTS), 2);
        Edit_LimitText(GetDlgItem(hdlg, IDC_DESCR), MAXNAME);

        loadcategory(GetDlgItem(hdlg, IDC_CAT));
        seticon(hdlg, mode);

        switch (mode) {
        case DLGMODE_DEP:
            initDeposit(hdlg);
            break;
        case DLGMODE_WD:
            initWd(hdlg);
            break;
        default:
            initEdit(hdlg, mode);
        }
        return TRUE;}
    case WM_SYSCOMMAND:
        switch (wparam) {
        case SC_CONTEXTHELP:
            if (mode == DLGMODE_DEP || mode == DLGMODE_WD)
                showhelp("hlpAdd.html");
            else
                showhelp("hlpEdit.html");
            break;
        default:
            return DefWindowProcA(hdlg, m, wparam, lparam);
        }
        return TRUE;
    case WM_COMMAND:
        int pos;
        char txt[MAXCAT];

        if (HIWORD(wparam) == CBN_SELCHANGE) {
            pos = ComboBox_GetCurSel(GetDlgItem(hdlg, IDC_CAT));
            ComboBox_GetLBText(GetDlgItem(hdlg, IDC_CAT), pos, txt);
        }
        switch (LOWORD(wparam)) {
        case IDOK: {
            char *err = new char[32];
            xaction next;

            if ((err = getfields(hdlg, &next)) == NULL) {
                g_arrtrns[(mode >= 0) ? mode : g_len] = next;

                if (mode >= 0)
                    setneedsave(true);
                else
                    setneedsave(true, g_len++);
            } else {
                errmsg(err, "");
                break;
            }
            if ((int) lparam == CLEAR_AFTER) {
                clear();
                break;
            }
            delete err;
            EndDialog(hdlg, IDOK);
            break;}
        case IDC_QTY: {
            qtydlg qd;
            if (qd.show() == IDOK)
                setamt(hdlg, qd.unprc * qd.qty);
            break;}
        case IDC_WD_OKNEW:
            SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM) CLEAR_AFTER);
            inOKandNew = true;
            break;
        case IDCLOSE:
        case IDCANCEL:
            if (!inOKandNew) {
                g_needSave = false;
                changePartStatus("Saved", SBPOS_ISSAVED);
            }
            EndDialog(hdlg, IDCANCEL);
            break;
        }

        return TRUE;
    }
    return FALSE;
}

void initDeposit(HWND hDlg)
{
    SetWindowText(hDlg, "Deposit");
}
void initWd(HWND hDlg)
{
    SetWindowText(hDlg, "Withdrawal");
}
void initEdit(HWND hDlg, int pos)
{
    SetWindowText(GetDlgItem(hDlg, IDC_DESCR), g_arrtrns[pos].descr);

    SetWindowText(GetDlgItem(hDlg, IDC_CAT), g_arrtrns[pos].cat);


    int whole, cents;
    currency n = fabs(g_arrtrns[pos].amt);
    moneysplit(n, &whole, &cents);

    if (SetDlgItemInt(hDlg, IDC_VAL, whole, FALSE) == FALSE)
        fmtmsgbox(MB_OK, "Error: %s", fmtmsg());
    if (SetDlgItemInt(hDlg, IDC_VAL_CENTS, cents, FALSE) == FALSE)
        fmtmsgbox(MB_OK, "Error: %s", fmtmsg());


    SYSTEMTIME st = g_arrtrns[pos].d.toWinTime();
    DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATE), GDT_VALID, &st);

    SetWindowLong(GetDlgItem(hDlg, IDC_WD_OKNEW), GWL_STYLE, WS_CHILD);
}
void loadcategory(HWND hcb)
{
    ComboBox_AddString(hcb, "Food");
    ComboBox_AddString(hcb, "Salary");
    ComboBox_AddString(hcb, "Bills");
    ComboBox_AddString(hcb, "Recreation");
    ComboBox_AddString(hcb, "Car");
    ComboBox_AddString(hcb, "Gift");
    ComboBox_AddString(hcb, "Profits");
}
char *getfields(HWND hDlg, xaction *target)
{
    int sign;

    HWND hDescr = GetDlgItem(hDlg, IDC_DESCR);

    sign = (gettype(hDlg) == DLGMODE_DEP) ? 1 : -1;

    GetWindowTextA(hDescr, target->descr, MAXNAME);

    GetWindowTextA(GetDlgItem(hDlg, IDC_CAT), target->cat, MAXCAT);


    if ((target->amt = getamt(GetDlgItem(hDlg, IDC_VAL), GetDlgItem(hDlg, IDC_VAL_CENTS)) * sign) == 0)
        return "Amount is zero";

    SYSTEMTIME xdate;
    HWND hDP = GetDlgItem(hDlg, IDC_DATE);
    if (DateTime_GetSystemtime(hDP, &xdate) == GDT_VALID) {
        target->d.day = xdate.wDay;
        target->d.yr = xdate.wYear;
        target->d.month = xdate.wMonth;
    } else return (char *) "Invalid date";

    /*if (target->d > today())
        return ((char *) "Are you from the future?");*/

    return NULL;
}
void clear()
{
    SYSTEMTIME st;

    SetWindowText(GetDlgItem(hNewTrnsDlg, IDC_DESCR), "");
    SetWindowText(GetDlgItem(hNewTrnsDlg, IDC_VAL), "");
    SetWindowText(GetDlgItem(hNewTrnsDlg, IDC_VAL_CENTS), "");
    SetWindowText(GetDlgItem(hNewTrnsDlg, IDC_CAT), "");

    st = today().toWinTime();
    DateTime_SetSystemtime(GetDlgItem(hNewTrnsDlg, IDC_DATE), GDT_VALID, &st);
}
int gettype(HWND hDlg)
{
    char cap[64];
    GetWindowText(hDlg, cap, 64);

    if (strstr(cap, "Deposit"))
        return DLGMODE_DEP;
    return DLGMODE_WD;
}
void setamt(HWND hdlg, int whol, int cnts)
{
    setWndTextInt(GetDlgItem(hdlg, IDC_VAL), whol);
    setWndTextInt(GetDlgItem(hdlg, IDC_VAL_CENTS), cnts);
}
void setamt(HWND hdlg, double m)
{
    char buf[32];
    int cnts, whol;

    sprintf(buf, "%lf", m);
    sscanf(buf, "%d.%d", &whol, &cnts);
    setamt(hdlg, whol, cnts);
}
currency getamt(HWND hVal, HWND hValCents)
{
    char whole[8], cents[3], buf[11];
    xaction x;

    GetWindowText(hVal, whole, 8);
    GetWindowText(hValCents, cents, 3);

    strcpy(buf, whole);
    strcat(buf, ".");
    strcat(buf, cents);

    x.setAmtStr(buf);
    return x.amt;
}
void seticon(HWND hdlg, int what)
{
    HICON hi;

    switch (what) {
    case DLGMODE_DEP:
    case DLGMODE_WD:
        hi = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_NEWTRANS));
        break;
    default:
        hi = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_EDITTRANS));
    }
    SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM) hi);
}
