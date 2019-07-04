/* The main file */
#define  STRICT
#define  _WIN32_WINNT 0x0603
#define  _WIN32_IE 0x0900

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <commctrl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <mapi.h>
#include "resource.h"

#define IDC_MAIN_STATUS                         40004
#define IDC_TABLE                               40005
#define ID_SHOWDETS                             40006
#define IDC_MAIN_TB                             40007

#define DLGMODE_WD              (-1)
#define DLGMODE_DEP             (-2)

#define UNTITLD     "Untitled"
#define ENV_SAVE    (char *) "APPDATA"
#define OK      true
#define FAIL    false


#define IDT_AUTOSAVE 8000
#define FREESTART       '\6'        // the start byte of files created by Free edition

#define DBG

#define MAX_FN      32

HWND g_hwndTable;
HWND g_hwndMain;
HWND g_hOpenLst = NULL;
HINSTANCE g_hinst;

const int REALLOC_STEP = 32;
const int MAXPASS = 32;
int lim = REALLOC_STEP;       // initial limit is 32

char g_filename[MAX_PATH];
char g_cryptPassword[MAXPASS];
bool g_needSave = false;

char g_classname[] = "srchk";
char g_progname[] = "SuperCheck";

#include "proto.h"
#include "dlgproc.h"
#include "xaction.h"
#include "config.h"
#include "resource.h"
#include "util.h"
#include "passwd.h"
#include "ls.h"
#include "find.h"
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    MSG msg;
    HACCEL hAccel;
    BOOL msgRet;
    INITCOMMONCONTROLSEX icex;

    if (signal(SIGSEGV, onSegfault) == SIG_ERR) {
        errmsg("signal: Couldn't register handler for SIGSEGV", "");
        return EXIT_FAILURE;
    }

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
    if (InitCommonControlsEx(&icex) == FALSE) {
        fmtmsgbox(MB_ICONERROR, "Could not initialize extended common controls: %s", fmtmsg());
        return EXIT_FAILURE;
    }

    if (winregister(hInst) == FAIL)
        errmsg("Could not register window class", "");


    memset(&g_progsettings, 0, sizeof (settings));
    if (g_progsettings.readSettings() == FAIL)
        g_progsettings.setDefaults();

    mainwindow(hInst, nCmdShow);
    hAccel = LoadAcceleratorsA(hInst, MAKEINTRESOURCEA(IDR_ACCEL));

    onCreat();

    while ((msgRet = GetMessageA(&msg, NULL, 0, 0))) {
        if (msgRet == -1) {
            errmsg("GetMessage -1", fmtmsg());
            exit(EXIT_FAILURE);
        }
        if (!g_fd.isdlgmsg(&msg)) {
            if (!TranslateAcceleratorA(g_hwndMain, hAccel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }
    return 0;
}
Status winregister(HINSTANCE hThisInstance)
{
    WNDCLASSEXA wc = { };

    wc.hInstance = hThisInstance;
    wc.lpszClassName = g_classname;
    wc.lpfnWndProc = WndPrc;
    wc.style = 0;
    wc.cbSize = sizeof (WNDCLASSEXA);
    wc.hIcon = (HICON) LoadImage(GetModuleHandle(NULL),
                                 MAKEINTRESOURCE(IDI_APPICO), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
    wc.hIconSm = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICO),
                                IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = MAKEINTRESOURCEA(IDR_MENU);
    wc.cbClsExtra = 0;                      /* no extra bytes after the window class */
    wc.cbWndExtra = 0;                      /* structure or the window instance */
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE); /*(HBRUSH) GetStockObject(WHITE_PEN);*/

    if (!RegisterClassExA(&wc))
        return FAIL;
    return OK;
}
LRESULT CALLBACK WndPrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == g_fd.findmsg)
        g_fd.onFindNext();
    if (message == g_fd.hlpmsg)
        g_fd.onhlp();
    switch (message) {
        HANDLE_MSG(hwnd, WM_CLOSE, onClose);
        HANDLE_MSG(hwnd, WM_DESTROY, onClose);
        HANDLE_MSG(hwnd, WM_SIZE, onSize);
        HANDLE_MSG(hwnd, WM_COMMAND, onCmd);
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, setminsiz);
        case WM_SYSCOMMAND:
            onSysCmd((int) LOWORD(wParam));
            break;
        case WM_NOTIFY:
            onNotify(wParam, lParam);
            break;
        case WM_TIMER:
            onTimer(hwnd, wParam);
            break;
        case WM_EXITMENULOOP:
            char text[32];
            if (calcBalance() < 0)
                sprintf(text, "End Balance: -$%s", comma(-(calcBalance())));
            else
                sprintf(text, "End Balance: $%s", comma((calcBalance())));
            changePartStatus(text, 2);
            break;
        case WM_MENUSELECT:
            char *helpstr;
            helpstr = hlpstr(wParam);
            changePartStatus(helpstr, 2);
            break;
        case WM_ERASEBKGND:
            return 1;           // don't paint the background twice
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
char *hlpstr(WPARAM wParam)
{
    char *helpstr = "";
    if (HIWORD(wParam) & MF_HILITE) {
        switch (LOWORD(wParam)) {
        case SC_CLOSE:
        case ID_FILE_EXIT:
            helpstr = "Exits SuperCheck.";
            break;
        case ID_FILE_EXPORTEXCEL:
            helpstr = "Exports the current file into an Excel(xls) file.";
            break;
        case ID_FILE_NEW:
            helpstr = "Creates a new SuperCheck file.";
            break;
        case ID_FILE_OPEN:
            helpstr = "Opens a SuperCheck file.";
            break;
        case ID_FILE_PASS:
            helpstr = "Modify the password associated with this file.";
            break;
        case ID_FILE_REN:
            helpstr = "Renames this file.";
            break;
        case ID_FILE_REMOVE:
            helpstr = "Removes this file.";
            break;
        case ID_FILE_SAVE:
            helpstr = "Saves changes to this file, if any.";
            break;
        case ID_FILE_SETTINGS:
            helpstr = "Change program settings.";
            break;
        case ID_OPS_CPY:
            helpstr = "Copies the selected transaction to clipboard.";
            break;
        case ID_OPS_PASTE:
            helpstr = "Adds the clipboard contents to the list.";
            break;
        case ID_NEGAT:
            helpstr = "Negates the selected transaction.";
            break;
        case ID_GOTO:
            helpstr = "Goes to a transaction position.";
            break;
        case ID_OPS_EDIT:
            helpstr = "Edit the selected transaction.";
            break;
        case ID_OPS_NEWDEP:
            helpstr = "Creates a new deposit.";
            break;
        case ID_OPS_NEWWITH:
            helpstr = "Creates a new withdrawal.";
            break;
        case ID_OPS_REM:
            helpstr = "Removes the selected transaction.";
            break;
        case ID_OPS_STARTBAL:
            helpstr = "Modifies the starting balance.";
            break;
        case ID_SYS_ABT:
        case ID_HELP_ABT:
            helpstr = "Shows the About dialog.";
            break;
        case ID_HELP_HLP:
            helpstr = "Shows help about this application.";
            break;
        case ID_SYS_CASC:
            helpstr = "Cascades all SuperCheck windows.";
            break;
        case ID_SYS_TILE:
            helpstr = "Tiles all SuperCheck windows.";
            break;
        case ID_FULLSCREEN:
            helpstr = "Makes this window occupy the entire screen.";
            break;
        case SC_MAXIMIZE:
            helpstr = "Maximizes this window.";
            break;
        case SC_MINIMIZE:
            helpstr = "Minimizes this window.";
            break;
        case SC_SIZE:
            helpstr = "Resizes this window.";
            break;
        case SC_MOVE:
            helpstr = "Moves this window.";
            break;
        case SC_RESTORE:
            helpstr = "Restores this window.";
            break;
        case ID_FIND_CAT:
            helpstr = "Finds by the Category column.";
            break;
        case ID_FIND_DESCR:
            helpstr = "Finds by the Description column.";
            break;
        }
    }
    return helpstr;
}
int setminsiz(HWND h, MINMAXINFO *mmi)
{
    mmi->ptMinTrackSize.x = 300;     // width
    mmi->ptMinTrackSize.y = 400;     // height
    return 0;
}
void onCreat()
{
    RECT rc;                                                                // |

    strcpy(g_filename, UNTITLD);
    strcpy(g_cryptPassword, "");
    g_needSave = false;

    g_arrtrns = (xaction *) malloc(lim * sizeof (xaction));

    makeTB();
    makeStatus();
    appendSysMnu();
    g_hwndTable = makeLV();
    SetWindowPos(g_hwndTable, NULL, 0, getClientHeight(GetDlgItem(g_hwndMain, IDC_MAIN_TB)),
                                getClientWidth(g_hwndMain), getClientWidth(g_hwndTable), SWP_NOZORDER);


    GetWindowRect(g_hwndTable, &rc);
    SendMessage(g_hwndMain, WM_SIZE, 0, 0);

    HICON hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_SAVE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_FILE_SAVE, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_DEL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_OPS_REM, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_NEW), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_FILE_NEW, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_OPEN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_FILE_OPEN, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_NEWTRANS), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_OPS_NEWWITH, hi);
    setico(g_hwndMain, ID_OPS_NEWDEP, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_EDITTRANS), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_OPS_EDIT, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_EXCEL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_FILE_EXPORTEXCEL, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_CPY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_OPS_CPY, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_PST), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_OPS_PASTE, hi);
    hi = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_CUT), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    setico(g_hwndMain, ID_OPS_CUT, hi);

    if (g_progsettings.tut)
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCEA(TUTDLG), g_hwndMain, TutDlg);

    if (isequal(g_filename, UNTITLD))
        enableItms(false);
    else
        enableItms();

    labelWnd(UNTITLD);
    opendlg();
    if (!isequal(g_filename, UNTITLD))
        extcat(g_filename);

    LOGFONT lf;
    Status readfnt(LOGFONT *);
    if (readfnt(&lf) == FAIL)
        return;
    else {
        HFONT hf = CreateFontIndirect(&lf);
        SendMessage(g_hwndTable, WM_SETFONT, (WPARAM) hf, 0);
    }
}
HWND makeballoontt(const char *text, HWND hParent)
{
    HWND hwndTooltip = CreateWindow(TOOLTIPS_CLASS, NULL,
                            WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                            0, 0, 0, 0, NULL, NULL, g_hinst, NULL);

    if (hwndTooltip) {
        TOOLINFO ti;

        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_TRANSPARENT | TTF_CENTERTIP;
        ti.hwnd     = hParent;
        ti.uId      = 0;
        ti.hinst    = NULL;
        ti.lpszText = const_cast<char *>(text);

        GetClientRect(g_hwndMain, &ti.rect);

        SendMessage(hwndTooltip, TTM_ADDTOOL, 0, (LPARAM) &ti);
    }
    return hwndTooltip;
}
HWND makeStatus()
{
    int statWidths[] = {100, 200, -1};
    HWND hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                                "Saved", g_hwndMain,
                                                IDC_MAIN_STATUS);
    SendMessage(hStatus, SB_SETPARTS, (sizeof statWidths / sizeof (int)),
                (LPARAM) &statWidths);
    updStatusBar();

    return hStatus;
}
HWND makeTB()
{
    const int TBBSIZE = 8;
    HWND hTool;
    TBBUTTON tbb[TBBSIZE];
    // create toolbar

    InitCommonControls();

    hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE
                        | TBSTYLE_TOOLTIPS, 0, 0, 0, 0,
                         g_hwndMain, (HMENU) IDC_MAIN_TB,
                         GetModuleHandle(NULL), NULL);

     if (hTool == NULL)
        return NULL;

     SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

     makeTBImgList(hTool);

     ZeroMemory(tbb, sizeof tbb);

     tbb[0].iBitmap = 0;
     tbb[0].fsState = TBSTATE_ENABLED;
     tbb[0].fsStyle = TBSTYLE_BUTTON;
     tbb[0].idCommand = ID_FILE_NEW;
     tbb[0].iString = (INT_PTR) "New (Ctrl + N)";

     tbb[1].iBitmap = 1;
     tbb[1].fsState = TBSTATE_ENABLED;
     tbb[1].fsStyle = TBSTYLE_BUTTON;
     tbb[1].idCommand = ID_FILE_OPEN;
     tbb[1].iString = (INT_PTR) "Open (Ctrl + O)";

     tbb[2].iBitmap = 2;
     tbb[2].fsState = TBSTATE_ENABLED;    // disabled
     tbb[2].fsStyle = TBSTYLE_BUTTON;
     tbb[2].idCommand = ID_FILE_SAVE;
     tbb[2].iString = (INT_PTR) "Save (Ctrl + S)";

     tbb[3].iBitmap = 0;
     tbb[3].fsState = TBSTATE_ENABLED;
     tbb[3].fsStyle = BTNS_SEP;
     tbb[3].idCommand = 0;

     tbb[4].iBitmap = 3;
     tbb[4].fsState = TBSTATE_ENABLED;    // disabled
     tbb[4].fsStyle = BTNS_WHOLEDROPDOWN;
     tbb[4].idCommand = ID_OPS_NEWWITH;
     tbb[4].iString = (INT_PTR) "New Deposit/Withdrawal";

     tbb[5].iBitmap = 4;
     tbb[5].fsState = TBSTATE_ENABLED;    // disabled
     tbb[5].fsStyle = TBSTYLE_BUTTON;
     tbb[5].idCommand = ID_OPS_EDIT;
     tbb[5].iString = (INT_PTR) "Edit Transaction (Ctrl + E)";

     tbb[6].iBitmap = 5;
     tbb[6].fsState = TBSTATE_ENABLED;    // disabled
     tbb[6].fsStyle = TBSTYLE_BUTTON;
     tbb[6].idCommand = ID_OPS_REM;
     tbb[6].iString = (INT_PTR) "Remove Transaction (Del)";

     tbb[7].iBitmap = 6;
     tbb[7].fsState = TBSTATE_ENABLED;
     tbb[7].fsStyle = BTNS_WHOLEDROPDOWN;
     tbb[7].idCommand = ID_FIND_DESCR;
     tbb[7].iString = (INT_PTR) "Find By";

     SendMessage(hTool, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON),
                 (LPARAM) &tbb);

     SendMessage(hTool, TB_SETMAXTEXTROWS, 0, 0);
     SendMessage(hTool, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DOUBLEBUFFER);

     return hTool;
}
HIMAGELIST makeTBImgList(HWND hTool)
{
     HIMAGELIST imgList;
     HICON hIcon;
     imgList = ImageList_Create(16, 16, ILC_COLOR32, 2, 8);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_NEW));
     ImageList_AddIcon(imgList, hIcon);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_OPEN));
     ImageList_AddIcon(imgList, hIcon);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_SAVE));
     ImageList_AddIcon(imgList, hIcon);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_NEWTRANS));
     ImageList_AddIcon(imgList, hIcon);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_EDITTRANS));
     ImageList_AddIcon(imgList, hIcon);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_DEL));
     ImageList_AddIcon(imgList, hIcon);

     hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_TB_FIND));
     ImageList_AddIcon(imgList, hIcon);

     SendMessage(hTool, TB_SETIMAGELIST, (WPARAM) 0, (LPARAM) imgList);

     return imgList;
}

HWND makeLV()
{
    const int N_COLS = 5;
	int index;
	LV_COLUMN lvC;
	char szText[MAX_PATH];
	int statusHeight, tbHeight, lvHeight;

    statusHeight = getClientHeight(GetDlgItem(g_hwndMain, IDC_MAIN_STATUS));
    tbHeight = getClientHeight(GetDlgItem(g_hwndMain, IDC_MAIN_TB));

    lvHeight = getClientHeight(g_hwndMain) - statusHeight - tbHeight;


	g_hwndTable = CreateWindow(WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD |
                            WS_BORDER|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS
                            , 0, 0, lvHeight, getClientWidth(g_hwndMain),
                            g_hwndMain, (HMENU) IDC_TABLE,
		GetModuleHandle(NULL), NULL);

    ListView_SetExtendedListViewStyleEx(g_hwndTable, LVS_EX_FULLROWSELECT,
                                        LVS_EX_FULLROWSELECT);

	if (!g_hwndTable) {
        errmsg(cat(2, "Couldn't make listview", fmtmsg()), "");
		return NULL;
	}

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.pszText = szText;
    int i;
    for (i = IDS_CAT_COL; i < IDS_CAT_COL + N_COLS; ++i) {
        lvC.iSubItem = i;
        lvC.pszText = szText;
        switch (i) {
            case IDS_CAT_COL:
                lvC.pszText = (char *)"Category";
                lvC.cx = g_progsettings.colCatWid;
                break;
            case IDS_DESCR_COL:
                lvC.cx = g_progsettings.colNameWidth;
                lvC.pszText = (char *)"Description";
                break;
            case IDS_DATE_COL:
                lvC.cx = g_progsettings.colDateWidth;
                lvC.pszText = (char *) "Date";
                break;
            case IDS_AMT_COL:
                lvC.cx = g_progsettings.colAmtWidth;
                lvC.pszText = (char *) "Amount";
                break;
            case IDS_BAL_COL:
                lvC.cx = g_progsettings.colBalWidth;
                lvC.pszText = (char *) "Balance";
                break;
        }
        if (ListView_InsertColumn(g_hwndTable, i, &lvC) == -1)
            return NULL;
    }
	for (index = 0; index < g_len; ++index)
        lvappend(index, g_hwndTable);

    return g_hwndTable;
}
void onSize(HWND hwnd, UINT state, int cx, int cy)
{
    HWND hStatus = GetDlgItem(g_hwndMain, IDC_MAIN_STATUS);
    HWND hTB = GetDlgItem(g_hwndMain, IDC_MAIN_TB);
    int statusHeight, tbHeight, lvHeight;

    statusHeight = getClientHeight(hStatus);
    tbHeight = getClientHeight(hTB);

    SendMessage(hStatus, WM_SIZE, 0, 0);

    SendMessage(hTB, TB_AUTOSIZE, 0, 0);

    lvHeight = getClientHeight(g_hwndMain) - tbHeight - statusHeight;

    RECT rc;
    GetClientRect(g_hwndMain, &rc);

    SetWindowPos(g_hwndTable, NULL, 0, tbHeight,rc.right,lvHeight,SWP_NOZORDER);
}
void onSysCmd(int id)
{
    if (id == ID_SYS_CASC)
        casc();
    else if (id == ID_SYS_ABT)
        DialogBoxA(g_hinst, MAKEINTRESOURCEA(ABOUTDLG),g_hwndMain,AboutDlgProc);
    else if (id == ID_SYS_TILE)
        tile();
    else if (id == ID_FULLSCREEN) {
        #define STYLES (WS_CAPTION|WS_SYSMENU \
                        |WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SIZEBOX)
        DWORD style, newstyle;
        HDC hdc;

        static int x, y, width, height;

        hdc = GetWindowDC(NULL);
        if (getClientWidth(g_hwndMain) == GetDeviceCaps(hdc, HORZRES) &&
                    !(GetWindowLong(g_hwndMain, GWL_STYLE) & WS_SIZEBOX)) {
            style = GetWindowLong(g_hwndMain, GWL_STYLE);
            style |= STYLES;
            SetWindowLong(g_hwndMain, GWL_STYLE, LONG(style));
            SetWindowPos(g_hwndMain, NULL, x, y, width,height,SWP_FRAMECHANGED);
        } else {
            // save the pos info
            RECT rc;
            GetWindowRect(g_hwndMain, &rc);

            width = abs(rc.right - rc.left);
            height = abs(rc.bottom - rc.top);
            x = rc.left;
            y = rc.top;

            style = GetWindowLong(g_hwndMain, GWL_STYLE);
            newstyle = style & ~STYLES;

            SetWindowLong(g_hwndMain, GWL_STYLE, LONG(newstyle));
            SetWindowPos(g_hwndMain, NULL, 0, 0, GetDeviceCaps(hdc, HORZRES),
                         GetDeviceCaps(hdc, VERTRES), SWP_FRAMECHANGED);
        }
    }
}
void appendSysMnu()
{
    HMENU hSysMnu = GetSystemMenu(g_hwndMain, FALSE);

    AppendMenu(hSysMnu, MF_SEPARATOR, 0, "");
    AppendMenu(hSysMnu, MF_STRING, ID_FULLSCREEN, "Full Screen\tF11");
    AppendMenu(hSysMnu, MF_STRING, ID_SYS_CASC, "Cascade Windows\tF6");
    AppendMenu(hSysMnu, MF_STRING, ID_SYS_TILE, "Tile Windows");
    AppendMenu(hSysMnu, MF_STRING, ID_SYS_ABT, "About");
}
void setico(HWND hMain, UINT idCmd, HICON hico)
{
    HMENU hm;
    MENUITEMINFOA mi;

    mi.cbSize = sizeof (MENUITEMINFOA);
    mi.fMask = MIIM_BITMAP;
    mi.hbmpItem = bmpTransparent(hicoTohbmp(hico));
    hm = GetMenu(hMain);
    SetMenuItemInfoA(hm, idCmd, FALSE, &mi);
}
void onNotify(WPARAM wParam, LPARAM lParam)
{
    switch (((LPNMHDR)lParam)->code) {
        case LVN_BEGINLABELEDIT:
            // dont allow label editing
            SendMessage(g_hwndTable, LVM_CANCELEDITLABEL, 0, 0);
            break;
        case LVN_GETDISPINFO:
            onNotifyDisplayLV(lParam);
            break;
        case NM_RCLICK:
            if ((((LPNMHDR) lParam)->hwndFrom) == g_hwndTable)
                ctxmenu((LPNMHDR) lParam);
            else if ((((LPNMHDR) lParam)->hwndFrom) ==
                     GetDlgItem(g_hwndMain, IDC_MAIN_STATUS)) {
            }
            break;
        case TBN_DROPDOWN: {
            #define lpnmTB ((LPNMTOOLBAR)lParam)
            RECT rc;
            HMENU hMenuLoaded=0, hPopupMenu;
            TPMPARAMS tpm;

            SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem,
                        (LPARAM)&rc);

            MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc,2);

            switch (lpnmTB->iItem) {
            case ID_OPS_NEWWITH:
                hMenuLoaded = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_SPLITMENU));
                break;
            case ID_FIND_DESCR:
                hMenuLoaded=LoadMenu(g_hinst,MAKEINTRESOURCEA(IDM_FINDSPLITMENU)
                                     );
                break;
            }

            hPopupMenu = GetSubMenu(hMenuLoaded, 0);

            tpm.cbSize    = sizeof(TPMPARAMS);
            tpm.rcExclude = rc;


            TrackPopupMenuEx(hPopupMenu,
                             TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
                             rc.left, rc.bottom, g_hwndMain, &tpm);

            DestroyMenu(hMenuLoaded);
            break;}
        case NM_DBLCLK:
            onEdit();
            break;
    }
}
void ctxmenu(NMHDR *nmhdr)
{
    POINT cursor;
    HMENU hPopup;
    int sel;

    GetCursorPos(&cursor);
    hPopup = CreatePopupMenu();
    if ((sel = SendMessage(g_hwndTable,LVM_GETNEXTITEM,-1,LVNI_SELECTED))!=-1) {   //a xaction is selected
        AppendMenu(hPopup, MF_STRING, ID_OPS_REM, "&Remove Transaction\tDel");
        AppendMenu(hPopup,MF_STRING,ID_OPS_EDIT,"&Edit Transaction...\tCtrl+E");
        AppendMenu(hPopup, MF_STRING, ID_NEGAT, "&Negate Transaction");
        AppendMenu(hPopup, MF_STRING, ID_OPS_CPY, "&Copy\tCtrl+C");
        AppendMenu(hPopup, MF_STRING, ID_OPS_CUT, "&Cut\tCtrl+X");
    } else {                                                                            //a xaction is not selected
        AppendMenu(hPopup,MF_STRING,ID_OPS_NEWWITH,"&New Withdrawal...\tCtrl+W"
                   );
        AppendMenu(hPopup, MF_STRING, ID_OPS_NEWDEP, "&New Deposit...\tCtrl+D");
        AppendMenu(hPopup, MF_STRING, ID_OPS_PASTE, "&Paste\tCtrl+V");
        AppendMenu(hPopup, MF_STRING, ID_GOTO, "&Go to...");
    }
    AppendMenu(hPopup, MF_SEPARATOR, 0, "");
    if (!g_needSave)
        AppendMenu(hPopup,MF_STRING | MF_GRAYED, ID_FILE_SAVE, "&Save\tCtrl+S");
    else
        AppendMenu(hPopup, MF_STRING, ID_FILE_SAVE, "&Save\tCtrl+S");
    TrackPopupMenu(hPopup,TPM_RIGHTBUTTON, cursor.x,cursor.y,0,g_hwndMain,NULL);
    DestroyMenu(hPopup);
}
void onNotifyDisplayLV(LPARAM lParam)
{
    char szText[256];
    LV_DISPINFOA *pLvdi = (LV_DISPINFO *) lParam;
    switch (pLvdi->item.iSubItem) {
        case 0: { // Category
            pLvdi->item.pszText = g_arrtrns[pLvdi->item.iItem].cat;
            break;}
        case 1:     // Name
            pLvdi->item.pszText = g_arrtrns[pLvdi->item.iItem].descr;
            break;
        case 2:{     // Date
            pLvdi->item.pszText=g_arrtrns[pLvdi->item.iItem].d.prn(PRINT_DOW_WRD
                                                                   );
            break;}
        case 3:{     // Amt
            currency amt = g_arrtrns[pLvdi->item.iItem].amt;
            if (amt < 0)
                sprintf(szText, "-$%s", comma(-(amt)));
            else
                sprintf(szText, "$%s", comma((amt)));
            pLvdi->item.pszText = szText;
            break;}
        case 4:{     // Balance
            currency bal = g_arrtrns[pLvdi->item.iItem].curbal;
            if (bal < 0)
                sprintf(szText, "-$%s", comma(-(bal)));
            else
                sprintf(szText, "$%s", comma((bal)));
            pLvdi->item.pszText = szText;
            break;}
    }
}
void onPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(g_hwndMain, &ps);

    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

    EndPaint(g_hwndMain, &ps);
}
void onTimer(HWND hwnd, UINT id)
{
    if (g_needSave && id == IDT_AUTOSAVE) {
        if (g_progsettings.autosaveMins == 0)
            return;
        save();
        changePartStatus((char *) "Autosaved", SBPOS_ISSAVED);
    }
}
void onClose(HWND hwnd)
{
    g_progsettings.winX = getwinx(g_hwndMain);
    g_progsettings.winY = getwiny(g_hwndMain);
    g_progsettings.winWidth = getClientWidth(g_hwndMain);
    g_progsettings.winHeight = getClientHeight(g_hwndMain);
    g_progsettings.colAmtWidth = ListView_GetColumnWidth(g_hwndTable, 3);
    g_progsettings.colCatWid = ListView_GetColumnWidth(g_hwndTable, 0);
    g_progsettings.colBalWidth = ListView_GetColumnWidth(g_hwndTable, 4);
    g_progsettings.colDateWidth = ListView_GetColumnWidth(g_hwndTable, 2);
    g_progsettings.colNameWidth = ListView_GetColumnWidth(g_hwndTable, 1);

    g_progsettings.saveSettings();

    if (!g_needSave) {
        PostQuitMessage(0);
        return;
    }
    switch ((isequal(UNTITLD, g_filename)) ? saveasdlg(g_filename) : asksave()){
    case IDYES:
        save();
                        // fall through
    case IDNO:
        free(g_arrtrns);
        KillTimer(g_hwndMain, IDT_AUTOSAVE);
        PostQuitMessage(0);
        break;
    case IDCANCEL:
        break;
    }
}
void onCmd(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    static int pos = -1;
    HGLOBAL hGlobal;
    char *pGlobal, *pTxt;

    switch (id) {
    case ID_GOTO: {
        int n = 0;
        char user[MAXINPUT];
        if(prompt(user,MAXINPUT, "Enter the line to jump to", "Go To...")==IDOK)
            ListView_SetItemState(g_hwndTable, (n = atoi(user) - 1),
                                  LVIS_FOCUSED | LVIS_SELECTED, -1);
        if (n > g_len || n < 0)
            errmsg("That transaction does not exist!", "No Such Transaction");
        break;}
    case ID_HELP_4PROG:
        fmtmsgbox(MB_OK, "Window Width: %d\r\nHeight: %d\r\nX: %d\r\nY: %d",
                  getClientWidth(g_hwndMain),
                  getClientHeight(g_hwndMain), getwinx(g_hwndMain),
                  getwiny(g_hwndMain));
        break;
    case ID_CTXMENU:
        NMHDR nmhdr;
        nmhdr.code = NM_RCLICK;
        nmhdr.hwndFrom = hwnd;
        nmhdr.idFrom = GetDlgCtrlID(hwnd);
        ctxmenu(&nmhdr);
        break;
    case ID_DN:
        if (pos == g_len - 1)
            break;

        lvsetsel(g_hwndTable, ++pos);
        break;
    case ID_UP:
        if (pos == -1)
            break;

        lvsetsel(g_hwndTable, --pos);
        break;
    case ID_FILE_EXIT:
        SendMessage(g_hwndMain, WM_CLOSE, 0, 0);
        break;
    case ID_FILE_REMOVE:
        if (MessageBox(g_hwndMain, "Are you sure you want"
                       "to remove this account?",
                       "Comfirm Removal", MB_YESNO) == IDYES) {
            closeall();
            remove(g_filename);
            reset();
        }
        break;
    case ID_FILE_EXPORTEXCEL:
        onExcelExprt();
        break;
    case ID_FILE_NEW:
        if (isequal(UNTITLD, g_filename) && g_len == 0)
            break;
        if (g_needSave)
            if (dosave(asksave()) == IDCANCEL)
                break;
        reset();
        break;
    case ID_FILE_OPEN:{
        opendlg();
        break;}
    case ID_FILE_PASS: {
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCEA(SETPASSDLG), g_hwndMain, PasswordDlg);
        break;}
    case ID_FILE_REN:
        if (isequal(g_filename, UNTITLD)) {
            errmsg("You can't rename an unsaved file, so\r\nsave this file first", "");
            break;
        }
        strcpy(g_filename, envcat(ENV_SAVE, renm()));
        labelWnd(g_filename);
        break;
    case ID_FILE_SAVE:
        char fn[MAX_PATH];
        if (!g_needSave)
            break;
        if (isequal(g_filename, UNTITLD)) {
            switch (saveasdlg(fn)) {
            case IDYES:
                enableItms(true);
                strcpy(g_filename, fn);
                labelWnd(g_filename);
                save();
                                // fall through
            case IDCANCEL:
                break;
            case IDNO:
                PostQuitMessage(0);
                break;
            }
        } else if (g_needSave)
            save();
        break;
    case ID_FILE_SETTINGS:
        if (confgdlg() == IDOK)
            g_progsettings.saveSettings();
        break;

                                // edit menu
    case ID_NEGAT:
        int pos;
        if ((pos = lvgetsel(g_hwndTable)) != -1) {
            g_arrtrns[pos].amt *= -1;
            setneedsave(true);
        }
        break;
    case ID_OPS_PASTE:
        OpenClipboard(hwnd);
        if (hGlobal = GetClipboardData(CF_TEXT)) {
            pGlobal = (char *) GlobalLock(hGlobal);
            if (pTxt) {
                free(pTxt);
                pTxt = NULL;
            }
            pTxt = (char *) malloc(GlobalSize(hGlobal));
            strcpy(pTxt, pGlobal);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        CloseClipboard();
        g_arrtrns[g_len].setstr(pTxt, true);
        setneedsave(true, g_len++);
        break;
    case ID_OPS_CPY:
    case ID_OPS_CUT:
        int sel;
        if ((sel = lvgetsel(g_hwndTable)) == -1)
            return;
        if (!pTxt)
            return;
        pTxt = (char *) malloc(1024);
        g_arrtrns[sel].getstr(pTxt, 1024, true);

        hGlobal = GlobalAlloc(GHND | GMEM_SHARE, strlen(pTxt) + 1);
        pGlobal = (char *) GlobalLock(hGlobal);
        strcpy(pGlobal, pTxt);
        GlobalUnlock(hGlobal);
        OpenClipboard(hwnd);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hGlobal);
        CloseClipboard();
        if (id == ID_OPS_CPY)
            return;
                                // fall through
    case ID_OPS_REM:
        onRemove();
        break;
    case ID_OPS_EDIT:
        onEdit();
        break;
    case ID_OPS_NEWDEP:
        DialogBoxParam(g_hinst, MAKEINTRESOURCEA(WDDLG), g_hwndMain, NewTrnsDlg, (LPARAM) DLGMODE_DEP);
        break;
    case ID_OPS_NEWWITH:
        DialogBoxParam(g_hinst, MAKEINTRESOURCEA(WDDLG), g_hwndMain, NewTrnsDlg, (LPARAM) DLGMODE_WD);
        break;
                        // help menu
    case ID_HELP_ABT:
        DialogBoxA(g_hinst, MAKEINTRESOURCEA(ABOUTDLG), g_hwndMain, AboutDlgProc);
        break;
    case ID_HELP_HLP:{
        showhelp();
        break;}

                    // "find" popup menu
    case ID_FIND_DESCR:
        g_fd.col = 1;
        g_fd.show(g_hwndMain, FR_MATCHCASE | FR_HIDEWHOLEWORD | FR_NOWHOLEWORD | FR_HIDEUPDOWN | FR_SHOWHELP, "Find by Description");
        break;
    case ID_FIND_CAT:
        g_fd.col = 0;
        g_fd.show(g_hwndMain, FR_MATCHCASE | FR_HIDEWHOLEWORD | FR_NOWHOLEWORD | FR_HIDEUPDOWN | FR_SHOWHELP, "Find by Category");
        break;
    }
}
Status savefnt(const LOGFONT *lf)
{
    FILE *fp;

    if (fp = fopen(envcat(ENV_SAVE, "fnt"), "wb")) {
        fwrite(lf, sizeof (LOGFONT), 1, fp);
        fclose(fp);

        return OK;
    }
    return FAIL;
}
Status readfnt(LOGFONT *lf)
{
    FILE *fp;

    if (fp = fopen(envcat(ENV_SAVE, "fnt"), "rb")) {
        fread(lf, sizeof (LOGFONT), 1, fp);
        fclose(fp);

        return OK;
    }
    return FAIL;
}
void onEdit()
{
    if (g_len == 0)
        return;
    int editSelection;
    if ((editSelection = ListView_GetNextItem(g_hwndTable, -1, LVNI_SELECTED | LVNI_FOCUSED)) != -1)
        DialogBoxParam(g_hinst, MAKEINTRESOURCEA(WDDLG), g_hwndMain, NewTrnsDlg, (LPARAM) editSelection);
}
void onRemove()
{
    int pos;
    if (g_len == 0)
        return;
    if ((pos = ListView_GetNextItem(g_hwndTable, -1, LVNI_SELECTED | LVNI_FOCUSED)) != -1) {
        switch (MessageBox(g_hwndMain, MSG_CONFIRMREM, "", MB_YESNO | MB_ICONQUESTION)) {
        case IDYES:
            rem();
            EnableMenuItem(GetMenu(g_hwndMain), ID_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);
            setneedsave(true);
            break;
        case IDNO:
            break;
        }
    }
}
void onExcelExprt()
{
    FILE *fp;
    char xlsPath[MAX_PATH];

    if (g_needSave)
        save();


    sprintf(xlsPath, "%s\\export%s.xls", getenv("USERPROFILE"), ftitle(g_filename));

    if (fexists(xlsPath)) {
        MessageBox(g_hwndMain, "The file already exists", "", MB_OK | MB_ICONASTERISK);
        return;
    }

    if ((fp = fopen(xlsPath, "w"))) {
        int i;

        fprintf(fp, "%s\t%s\t%s\t%s\n", "Category", "Descr", "Date", "Amount");
        for (i = 0; i < g_len; ++i) {
            fprintf(fp, "%s\t%s\t%s\t$%.2lf\n", g_arrtrns[i].cat, g_arrtrns[i].descr,
                            g_arrtrns[i].d.prn(PRINT_DOW_WRD), (g_arrtrns[i].amt));
        }
        fclose(fp);
        switch (fmtmsgbox(MB_OKCANCEL | MB_ICONINFORMATION, "You can find your Excel file in %s. Press OK to open it", xlsPath)) {
        case IDOK:
            ShellExecute(g_hwndMain, "open", xlsPath, NULL, NULL, SW_MAXIMIZE);
            break;
        }
    } else
        errmsg(cat(4, "Could not open file ", xlsPath, ": ", strerror(errno)), "");
}
void updStatusBar()
{
    char part[100];

    SendMessage(GetDlgItem(g_hwndMain, IDC_MAIN_STATUS), SB_SETTEXT, SBPOS_ISSAVED, (g_needSave) ? (LPARAM) "Not Saved" : (LPARAM) "Saved");

    sprintf(part, "Transactions: %d", (signed) g_len);
    SendMessage(GetDlgItem(g_hwndMain, IDC_MAIN_STATUS), SB_SETTEXT, SBPOS_NTRNS, (LPARAM) part);

    if (calcBalance() < 0)
        sprintf(part, "End Balance: -$%s", comma(-(calcBalance())));
    else
        sprintf(part, "End Balance: $%s", comma((calcBalance())));
    SendMessage(GetDlgItem(g_hwndMain, IDC_MAIN_STATUS), SB_SETTEXT, SBPOS_ENDBAL, (LPARAM) part);
}
void save()
{
    int i;
    headmeta hm;

    fclr(g_filename);       // clear file to avoid dups

    metasize = sizeof (headmeta);
    hm.len = g_len;
    hm.startbal = g_arrtrns[0].amt;
    hm.endbal = g_arrtrns[g_len - 1].curbal;
    strcpy(hm.passwd, g_cryptPassword);

    if (writemeta(&hm) == FAIL) {
        errmsg(cat(2, "Could not write file header: ", strerror(errno)), "Write error");
        return;
    }

    for (i = 0; i < g_len; ++i)
        g_arrtrns[i].fileappend();
    g_needSave = false;
    updStatusBar();
}
Status lvappend(int index, HWND hLV)
{
    const int NUM_COLUMNS = 6;
    LV_ITEM lvI;
    const int MAX_ITEMLEN = 64;
    int iSubItem;

	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_DI_SETITEM;
	lvI.state = 0;
	lvI.stateMask = 0;

    lvI.iItem = index;
    lvI.iSubItem = 0;
    lvI.pszText = LPSTR_TEXTCALLBACKA;       // callback to LVN_GETDISPINFO
    lvI.cchTextMax = MAX_ITEMLEN;
    lvI.iImage = index;
    lvI.lParam = (LPARAM) &g_arrtrns[index];

    if (ListView_InsertItem(hLV, &lvI) == -1)
        return FAIL;
    for (iSubItem = 1; iSubItem < NUM_COLUMNS; iSubItem++) {
        ListView_SetItemText(hLV,
            index,
            iSubItem,
            LPSTR_TEXTCALLBACK);
    }
    return OK;
}
void onSegfault(int signo)
{
    errmsg("Segfault", "");
    exit(EXIT_FAILURE);
}
