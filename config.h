/* Deals with the Settings dlg */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void settings::getvalue(const char *settingName, char value[])
{
    int i;
    char line[MAXLINE];
    char key[MAXKEY], val[MAXVAL];
    FILE *fp;

    if ((fp = fopen(envcat(ENV_SAVE, "settings.txt"), "r"))) {
        for (i = 0; fgets(line, MAXLINE - 1, fp); ++i) {
            if (sscanf(line, "%s = %s", key, val) == 2) {
                if (strcmp(key, settingName) == 0) {
                    strcpy(value, val);
                    break;
                }
            }
        }
        fclose(fp);
    }
}
void settings::saveSettings()
{
    FILE *fp;

    if ((fp = fopen(envcat(ENV_SAVE, "settings.txt"), "w"))) {
        fprintf(fp, "%s = %d\n", all_keys[0], winHeight);
        fprintf(fp, "%s = %d\n", all_keys[1], winWidth);
        fprintf(fp, "%s = %d\n", all_keys[2], winX);
        fprintf(fp, "%s = %d\n", all_keys[3], winY);

        fprintf(fp, "%s = %lf\n", all_keys[4], taxrate);
        fprintf(fp, "%s = %d\n", all_keys[5], colNameWidth);
        fprintf(fp, "%s = %d\n", all_keys[6], colDateWidth);
        fprintf(fp, "%s = %d\n", all_keys[7], colAmtWidth);
        fprintf(fp, "%s = %d\n", all_keys[8], colTaxWidth);
        fprintf(fp, "%s = %d\n", all_keys[9], colBalWidth);
        fprintf(fp, "%s = %d\n", all_keys[10], autosaveMins);
        fprintf(fp, "%s = %lf\n", all_keys[11], yrSalary);
        fprintf(fp, "%s = %d\n", all_keys[12], colCatWid);
        fprintf(fp, "%s = %s\n", all_keys[13], PRINTBOOL(tut));
        fclose(fp);
    } else
        errmsg(cat(2, "Could not read settings file: ", strerror(errno)), "");
}
Status settings::readSettings()
{
    int i;
    Status ret = OK;
    char all_values[MAXVAL][MAXSETTINGS];
    float tr, ys;

    for (i = 0; i < MAXSETTINGS; ++i)
        getvalue(all_keys[i], all_values[i]);

    if (getint(all_values[0], &winHeight) == FAIL
        || getint(all_values[1], &winWidth) == FAIL
        || getint(all_values[2], &winX) == FAIL
        || getint(all_values[3], &winY) == FAIL
        || getfloat(all_values[4], &tr) == FAIL
        || getint(all_values[5], &colNameWidth) == FAIL
        || getint(all_values[6], &colDateWidth) == FAIL
        || getint(all_values[7], &colAmtWidth) == FAIL
        || getint(all_values[8], &colTaxWidth) == FAIL
        || getint(all_values[9], &colBalWidth) == FAIL
        || getint(all_values[10], &autosaveMins) == FAIL
        || getfloat(all_values[11], &ys) == FAIL
        || getint(all_values[12], &colCatWid) == FAIL
        || getbool(all_values[13], &tut) == FAIL)
                ret = FAIL;

    taxrate = tr;
    yrSalary = ys;

    return ret;
}
void settings::setDefaults()
{
    winHeight = 450;
    winWidth = 650;
    winX = winY = 0;
    taxrate = 8.875;
    colCatWid = 74;
    colNameWidth = 200;
    colDateWidth = 100;
    colAmtWidth = 100;
    colTaxWidth = 75;
    colBalWidth = 75;
    yrSalary = 50000;
    tut = true;
    this->autosaveMins = 5;

    saveSettings();
}
Status getint(char *buf, int *val)
{
    if (sscanf(buf, "%d", val) == 0) {
        *val = 0;
        return FAIL;
    }
    else
        return OK;
}

enum { SUCCESS, ENDGARBAGE, NODIGIT, UNDFLOW, OVFLOW};
int getlong(const char *str, long *val, int bas = 10)
{
    char *end;

    errno = 0;
    *val = strtol(str, &end, bas);

    if (*end != '\0' && end != str)
        return ENDGARBAGE;          // garbage on end ex: "65garbage"
    if (end == str || *val == 0)
        return NODIGIT;             // no digits at all ex: "garbage"
    if (*val == LONG_MIN)
        return UNDFLOW;
    if (*val == LONG_MAX)
        return OVFLOW;
    return SUCCESS;
}

Status getfloat(char *buf, float *val)
{
    if (sscanf(buf, "%f", val) == 0) {
        *val = 0;
        return FAIL;
    }
    else
        return OK;
}
Status getbool(char *buf, bool *val)
{
    if (*buf == 't') *val = true;
    else if (*buf == 'f') *val = false;
    else return FAIL;
    return OK;
}
int confgdlg()
{
    PROPSHEETPAGE psp[1];

    PROPSHEETHEADER psh;

    psp[0].dwSize      = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags     = PSP_USEICONID | PSP_USETITLE | PSP_HASHELP;
    psp[0].hInstance   = g_hinst;
    psp[0].pszTemplate = MAKEINTRESOURCE(PREFDLG);
    psp[0].pszIcon     = 0;//MAKEINTRESOURCE(IDI_BORDER);
    psp[0].pfnDlgProc  = PrefDlg;
    psp[0].pszTitle    = "Preferences";
    psp[0].lParam      = 0;
    psp[0].pfnCallback = NULL;

    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP | PSH_HASHELP;
    psh.hwndParent  = g_hwndMain;
    psh.hInstance   = g_hinst;
    psh.pszIcon     = 0;//MAKEINTRESOURCE(IDI_CELL_PROPERTIES);
    psh.pszCaption  = (LPSTR) "Settings";
    psh.nPages      = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage  = 0;
    psh.ppsp        = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

    if (PropertySheet(&psh) >= 1)
        return IDOK;
    return IDCANCEL;
}
inline void confhelp()
{
    showhelp("hlpConfigdlg.html");
}
BOOL CALLBACK PrefDlg(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
    case WM_INITDIALOG:
        Button_SetCheck(GetDlgItem(h, IDC_PREF_ISAUTOSV), BST_UNCHECKED);
        EnableWindow(GetDlgItem(h, IDC_PREF_AUTOSV), FALSE);

        SendMessage(GetDlgItem(h, IDC_PREF_AUTOSV_SB), UDM_SETRANGE, 2, 15);

        SetWindowSubclass(GetDlgItem(h, IDC_PREF_AUTOSV), NumEditPrc, 0, 0);


        if (g_progsettings.autosaveMins == 0) {   // no autosave
            Button_SetCheck(GetDlgItem(h, IDC_PREF_ISAUTOSV), BST_UNCHECKED);
            EnableWindow(GetDlgItem(h, IDC_PREF_AUTOSV), FALSE);
            EnableWindow(GetDlgItem(h, IDC_PREF_ISAUTOSV), TRUE);
            Button_SetCheck(GetDlgItem(h, IDC_PREF_ISAUTOSV), BST_UNCHECKED);
            fmtwrite(h, IDC_PREF_AUTOSV, "");
        } else {
            fmtwrite(h, IDC_PREF_AUTOSV, "%d", g_progsettings.autosaveMins);
            Button_SetCheck(GetDlgItem(h, IDC_PREF_ISAUTOSV), BST_CHECKED);
            EnableWindow(GetDlgItem(h, IDC_PREF_AUTOSV), TRUE);
            EnableWindow(GetDlgItem(h, IDC_PREF_ISAUTOSV), TRUE);
        }
        fmtwrite(h, IDC_PREF_TAX, "%g%%", g_progsettings.taxrate);
        return TRUE;
        case WM_NOTIFY:
            switch (((PSHNOTIFY *) l)->hdr.code) {
            case PSN_APPLY:
                fmtread(h, IDC_PREF_TAX, "%g", &g_progsettings.taxrate);
                fmtread(h, IDC_PREF_AUTOSV, "%d", &g_progsettings.autosaveMins);
                break;
            case PSN_HELP:
                confhelp();
                break;
            }
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(w)) {
            case IDC_PREF_ISAUTOSV:
                if (Button_GetCheck(GetDlgItem(h, IDC_PREF_ISAUTOSV)) == BST_CHECKED)
                    EnableWindow(GetDlgItem(h, IDC_PREF_AUTOSV), TRUE);
                else
                    EnableWindow(GetDlgItem(h, IDC_PREF_AUTOSV), FALSE);
                break;
            case IDC_PREF_FONT:
                CHOOSEFONTA cf;
                LOGFONT lf;

                cf.lStructSize = sizeof (CHOOSEFONT);
                cf.hwndOwner = g_hwndMain;
                cf.lpLogFont = &lf;
                cf.nSizeMin = 7;
                cf.nSizeMax = 20;
                cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE | CF_NOVERTFONTS | CF_NOSIZESEL;

                Status savefnt(const LOGFONT *);
                if (ChooseFontA(&cf) == TRUE) {
                    if (savefnt(&lf) == FAIL)
                        errmsg("Could not save font", strerror(errno));
                    SendMessage(g_hwndTable, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);
                }
                break;
            }
        return TRUE;
    default:
        return FALSE;
    }
}
BOOL CALLBACK WndPosDlg(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
    case WM_INITDIALOG:
        SetWindowSubclass(GetDlgItem(h, IDC_WP_HEIGHT), NumEditPrc, 0, 0);
        SetWindowSubclass(GetDlgItem(h, IDC_WP_WID), NumEditPrc, 0, 0);
        SetWindowSubclass(GetDlgItem(h, IDC_WP_XPOS), NumEditPrc, 0, 0);
        SetWindowSubclass(GetDlgItem(h, IDC_WP_YPOS), NumEditPrc, 0, 0);


        fmtwrite(h, IDC_WP_HEIGHT, "%d", g_progsettings.winHeight);
        fmtwrite(h, IDC_WP_WID, "%d", g_progsettings.winWidth);

        if (g_progsettings.winX == -1 && g_progsettings.winY == -1) {       // center the window
            Button_SetCheck(GetDlgItem(h, IDC_WP_CNTR), BST_CHECKED);
            EnableWindow(GetDlgItem(h, IDC_WP_XPOS), FALSE);
            EnableWindow(GetDlgItem(h, IDC_WP_YPOS), FALSE);
        } else {
            Button_SetCheck(GetDlgItem(h, IDC_WP_CNTR), BST_UNCHECKED);

            fmtwrite(h, IDC_WP_XPOS, "%d", g_progsettings.winX);
            fmtwrite(h, IDC_WP_YPOS, "%d", g_progsettings.winY);
        }
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(w)) {
        case IDC_WP_CNTR:
            if (Button_GetCheck(GetDlgItem(h, IDC_WP_CNTR)) == BST_CHECKED) {
                EnableWindow(GetDlgItem(h, IDC_WP_XPOS), FALSE);
                EnableWindow(GetDlgItem(h, IDC_WP_YPOS), FALSE);
                g_progsettings.winX = g_progsettings.winY = -1;     // indicates that window will be centered
            } else {
                EnableWindow(GetDlgItem(h, IDC_WP_YPOS), TRUE);
                EnableWindow(GetDlgItem(h, IDC_WP_XPOS), TRUE);
            }
            break;
        }
        break;
    case WM_NOTIFY:
        switch (((PSHNOTIFY *) l)->hdr.code) {
        case PSN_APPLY:
            g_progsettings.winHeight = atoi(wndtxt(h, IDC_WP_HEIGHT));
            g_progsettings.winWidth = atoi(wndtxt(h, IDC_WP_WID));

            if (IsWindowEnabled(GetDlgItem(h, IDC_WP_XPOS)) &&
                IsWindowEnabled(GetDlgItem(h, IDC_WP_YPOS)))
            {
                g_progsettings.winX = atoi(wndtxt(h, IDC_WP_XPOS));
                g_progsettings.winY = atoi(wndtxt(h, IDC_WP_YPOS));
            }
            break;
        case PSN_HELP:
            confhelp();
        }
        return TRUE;
    }
    return FALSE;
}
BOOL CALLBACK ColWidthPrc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m)
    {
        case WM_INITDIALOG:
            char buf[16];
            SendMessage(GetDlgItem(h, IDC_AMT_SB), UDM_SETRANGE, 30, 75);
            SendMessage(GetDlgItem(h, IDC_BAL_SB), UDM_SETRANGE, 30, 75);
            SendMessage(GetDlgItem(h, IDC_CAT_SB), UDM_SETRANGE, 60, 100);
            SendMessage(GetDlgItem(h, IDC_DESCR_SB), UDM_SETRANGE, 50, 200);
            SendMessage(GetDlgItem(h, IDC_DAT_SB), UDM_SETRANGE, 60, 100);

            SetWindowSubclass(GetDlgItem(h, IDC_CATWID), NumEditPrc, 0, 0);
            SetWindowSubclass(GetDlgItem(h, IDC_AMTWID), NumEditPrc, 0, 0);
            SetWindowSubclass(GetDlgItem(h, IDC_DATWID), NumEditPrc, 0, 0);
            SetWindowSubclass(GetDlgItem(h, IDC_BALWID), NumEditPrc, 0, 0);
            SetWindowSubclass(GetDlgItem(h, IDC_DESCRWID), NumEditPrc, 0, 0);


            ints(g_progsettings.colAmtWidth, buf, 16);
            SetDlgItemTextA(h, IDC_AMTWID, buf);

            ints(g_progsettings.colBalWidth, buf, 16);
            SetDlgItemTextA(h, IDC_BALWID, buf);

            ints(g_progsettings.colDateWidth, buf, 16);
            SetDlgItemTextA(h, IDC_DATWID, buf);

            ints(g_progsettings.colNameWidth, buf, 16);
            SetDlgItemTextA(h, IDC_DESCRWID, buf);

            ints(g_progsettings.colCatWid, buf, 16);
            SetDlgItemTextA(h, IDC_CATWID, buf);
            return TRUE;
        case WM_NOTIFY:
            switch (((PSHNOTIFY *) l)->hdr.code) {
            case PSN_APPLY:
                g_progsettings.colAmtWidth = atoi(wndtxt(h, IDC_AMTWID));
                g_progsettings.colDateWidth = atoi(wndtxt(h, IDC_DATWID));
                g_progsettings.colBalWidth = atoi(wndtxt(h, IDC_BALWID));
                g_progsettings.colNameWidth = atoi(wndtxt(h, IDC_DESCRWID));
                g_progsettings.colCatWid = atoi(wndtxt(h, IDC_CATWID));
                break;
            case PSN_HELP:
                confhelp();
                break;
            }
            return TRUE;
        default:
            return FALSE;
    }
}
