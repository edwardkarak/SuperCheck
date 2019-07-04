#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED
/* Utility Functions */

#define MAXWIN 8
HWND hwndArr[MAXWIN];
int nwin = 0;
int varMsgbox(UINT msgFlags, int n, ...)
{
    va_list vl;
    char buf[512];
    int i;

    memset(buf, 0, sizeof buf);

    va_start(vl, n);
    for (i = 0; i < n; ++i)
        strcat(buf, va_arg(vl, char *));
    va_end(vl);
    return MessageBox(NULL, buf, "", msgFlags);
}
int errmsg(const char *msg, const char *titl)
{
    return MessageBox(NULL, msg, titl, MB_OK | MB_ICONERROR);
}
void showhelp(const char *topic="hlpIntro.html")
{
    int n;
    char cwd[MAX_PATH];
    GetModuleFileName(g_hinst, cwd, MAX_PATH);

    char *p = strrchr(cwd, '\\');
    *p = 0;

    strcpy(cwd, cat(3, cwd, "\\help\\", topic));

    if ((n = (int)ShellExecute(g_hwndMain, "open", cwd, NULL, NULL, SW_SHOWNORMAL)) < 32)
        errmsg("ShellExecute fail", "");
}
size_t fmtmsg(char *msgbuf)
{
    DWORD err = GetLastError();
    size_t siz;

    if (!err) {
        msgbuf = NULL;
        return 0;
    }
    else {
        LPSTR lpMsg;
        siz = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsg, 0, NULL);
        LocalFree(msgbuf);
        return safecpy(msgbuf, lpMsg, siz);
    }
}
char *fmtmsg()
{
    static char buf[64];
    fmtmsg(buf);

    return buf;
}
int fmtwrite(HWND wnd, const char *fmt, ...)
{
    va_list ap;
    char text[512];
    int cchWrite;

    va_start(ap, fmt);

    cchWrite = vsprintf(text, fmt, ap);
    SetWindowTextA(wnd,  text);

    va_end(ap);
    return cchWrite;
}
int fmtwrite(HWND hOwner, int id, const char *fmt, ...)
{
    va_list ap;
    char text[512];
    int cchWrite;

    va_start(ap, fmt);

    cchWrite = vsprintf(text, fmt, ap);
    SetWindowTextA(GetDlgItem(hOwner, id), text);

    va_end(ap);
    return cchWrite;
}
int fmtmsgbox(size_t *nwritten, HWND hOwner, UINT flgs, const char *cap, const char *fmt, ...)
{
    const int MAXMSG = 1024;

    va_list ap;
    char buf[MAXMSG];

    va_start(ap, fmt);
    *nwritten = vsnprintf(buf, MAXMSG, fmt, ap);

    return MessageBox(hOwner, buf, cap, flgs);
}
int fmtmsgbox(const char *fmt, ...)
{
    const int MAXMSG = 1024;

    va_list ap;
    char buf[MAXMSG];

    va_start(ap, fmt);
    vsnprintf(buf, MAXMSG, fmt, ap);

    return MessageBox(NULL, buf, "", 0);
}
int fmtmsgbox(UINT flgs, const char *fmt, ...)
{
    const int MAXMSG = 1024;

    va_list ap;
    char buf[MAXMSG];

    va_start(ap, fmt);
    vsnprintf(buf, MAXMSG, fmt, ap);

    return MessageBox(NULL, buf, "", flgs);
}
int fmtread(HWND hOwner, int id, const char *fmt, ...)
{
    va_list ap;
    char text[512];
    int nread;

    va_start(ap, fmt);

    GetDlgItemTextA(hOwner, id, text, 512);
    nread = vsscanf(text, fmt, ap);

    va_end(ap);
    return nread;
}
void dbgmsg(const char *s)
{
    #ifdef DBG
        MessageBox(NULL, s, "", MB_OK);
    #endif // DBG
}
Status fclr(const char *fn)
{
    FILE *fp;
    if ((fp = fopen(g_filename, "w"))) {
        fclose(fp);
        return OK;
    }
    return FAIL;
}
void extcat(char *userinp)
{
    char *p = userinp;
    while (*p++ != '.' && *p) ;

    if (strcmp(p, "such") == 0)
        return;
    strcat(userinp, ".such");
}
int getClientHeight(HWND wnd)
{
    RECT rc;
    GetClientRect(wnd, &rc);
    return rc.bottom - rc.top;
}
int getClientWidth(HWND wnd)
{
    RECT rc;
    GetClientRect(wnd, &rc);
    return rc.right - rc.left;
}
void centerwin(HWND win)
{
    RECT rc;
    int x, y;

    GetWindowRect(win, &rc);

    x = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

    MoveWindow(win, x, y, getClientWidth(win), getClientHeight(win), TRUE);
}
char *envcat(char *env, const char *file)
{
    static char b[MAX_PATH];

    strcpy(b, getenv(env));
    if (*file != '\\')
        strcat(b, "\\");
    strcat(b, file);

    return b;
}
LRESULT CALLBACK NumEditPrc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_CHAR) {
        if ((!isdigit(wParam) && wParam != VK_BACK && wParam != VK_DELETE) || wParam == '.') {
            MessageBeep(0);
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
char *wndtxt(HWND h)
{
    static char buf[256];
    GetWindowText(h, buf, 256);

    return buf;
}
inline char *wndtxt(HWND h, int id)
{
    return wndtxt(GetDlgItem(h, id));
}
char *getCatName(int number)
{
    enum {OTHER, GIFT, PROFITS, CAR, RECR, BILLS, SALARY, FOOD};

    char *buf = new char[32];
    switch (number) {
    case FOOD:
        buf = (char *) "Food";
        break;
    case SALARY:
        buf = (char *) "Salary";
        break;
    case BILLS:
        buf = (char *) "Bills";
        break;
    case RECR:
        buf = (char *) "Recreation";
        break;
    case CAR:
        buf = (char *) "Car";
        break;
    case PROFITS:
        buf = (char *) "Profits";
        break;
	case GIFT:
		buf = (char *) "Gift";
		break;
    case OTHER:
        buf = (char *) "Other";
        break;
    default:
        buf = (char *) "Other";
    }
    return buf;
}
void getcreat(const char *path, const char *fn, char *str)
{
    struct stat stbuf;

    char buf[MAX_PATH];
    sprintf(buf, "%s\\%s", path, fn);

    stat(buf, &stbuf);

    strftime(str, 16, "%a %b %d %Y", localtime(&(stbuf.st_ctime)));
}
bool isExt(const char *fn, const char *ext)
{
    char *p;

    p = strrchr(fn, '.');

    if (p && strcmp(p, ext) == 0)
        return true;
    return false;
}
char *datastr(int colind, int rowind)
{
    char *text = NULL;
    switch (colind) {
    case 0: // cat
        text = g_arrtrns[rowind].cat;
        break;
    case 1: //descr
        text = g_arrtrns[rowind].descr;
        break;
    case 2: // date
        text = g_arrtrns[rowind].d.prn(PRINT_DOW_WRD);
        break;
    case 3:{ // amt
        currency amt = g_arrtrns[rowind].amt;
        if (amt < 0)
            sprintf(text, "-$%s", comma(-TODOLLARS(amt)));
        else
            sprintf(text, "$%s", comma(TODOLLARS(amt)));
        break;}
    case 5:{ // curbal
        currency amt = g_arrtrns[rowind].curbal;
        if (amt < 0)
            sprintf(text, "-$%s", comma(-TODOLLARS(amt)));
        else
            sprintf(text, "$%s", comma(TODOLLARS(amt)));
        break;}
    }
    return text;
}
bool isallsame(char *s, char c)
{
    int i;

    for (i = 0; s[i]; ++i)
        if (s[i] != c)
            return false;
    return true;
}
bool containscntrl(char *s)
{
    int n = strlen(s), i;

    // subtract one, or else null terminator will be considered a cntrl char
    for (i = 0; i < n - 1; ++i)
        if (s[i] < 32)
            return true;
    return false;
}
bool issubstr(char *str, char *seq, bool b = true)
{
    if (substr(str, seq, b) == NULL)
        return false;
    return true;
}
bool isvalidfn(char *fn)
{
    // invalid chars: < > : " / \ | ? *
    // invalid names: CON, PRN, AUX, NUL, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9

    char *titl = ftitleExt(fn);

    bool a = contains(fn, '>');
    bool b = contains(fn, '<');
    bool c = contains(fn, ':');
    bool d = contains(fn, '\"');
    bool e = contains(fn, '\\');
    bool f = contains(fn, '/');
    bool g = contains(fn, '|');
    bool h = contains(fn, '?');
    bool i = contains(fn, '*');
    bool j = containscntrl(fn);
    bool k = isequal(titl, "CON");
    bool l = isequal(titl, "PRN");
    bool m = isequal(titl, "AUX");
    bool n = isequal(titl, "NUL");
    bool o = issubstr(titl, "COM") && strlen(titl) == 4 && isdigit(titl[3]);
    bool p = issubstr(titl, "LPT") && strlen(titl) == 4 && isdigit(titl[3]);
    bool q = isallsame(titl, '.');
    bool r = (strlen(titl)) ? false : true;

    if (a || b || c || d || e || f || g || h || i || j || k || l || m || n || o || p || q || r)
        return false;
    return true;
}
bool fexists(char *fn)
{
    bool ret = true;
    FILE *fp = fopen(fn, "r");

    if (!fp) {
        if (errno == ENOENT)
            ret = false;
    } else
        ret = true;
    fclose(fp);
    return ret;
}
Status reallocate()
{
    xaction *tmp;

    lim += 32;
    tmp = (xaction *) realloc(g_arrtrns, sizeof (xaction) * lim);
    if (tmp != NULL) {
        g_arrtrns = (xaction *) tmp;
        return OK;
    }
    return FAIL;
}
int multhi(int mult, int n)
{
    int i;

    for (i = n + 1; ; ++i) {
        if (i % mult == 0)
            return i;
    }
}
void lvupd()
{
    int i;
    ListView_DeleteAllItems(g_hwndTable);
    for (i = 0; i < g_len; ++i)
        lvappend(i, g_hwndTable);

    setbal();
    updStatusBar();
}
void setneedsave(bool x)
{
    g_needSave = x;
    if (g_len >= lim)
        if (reallocate() == FAIL && errno == ENOMEM)
            errmsg("Could not move memory to new location!", "Reallocation error");
    lvupd();
}
void setneedsave(bool x, int ind)
{
    g_needSave = x;

    if (ind < 0)
        return;

    if (g_len >= lim)
        if (reallocate() == FAIL && errno == ENOMEM)
            errmsg("Could not move memory to new location!", "Reallocation error");

    lvappend(ind, g_hwndTable);
    updStatusBar();
    setbal();
}
int asksave()
{
    if (g_needSave) {
        char cap[64];
        GetWindowText(g_hwndMain, cap, 64);
        return MessageBox(g_hwndMain, MSG_CONFIRMSAV, cap, MB_YESNOCANCEL | MB_ICONWARNING);
    }
    return 0;       // supress compiler warning
}
int dosave(int askSave_ret)
{
    switch (askSave_ret) {
    case IDYES:
        save();
        break;
    }
    return askSave_ret;
}
void tile()
{
    EnumWindows(EnumWndPrc, 0);
    if (nwin == 1)
        return;
    if (TileWindows(NULL, MDITILE_VERTICAL, NULL, nwin + 1, hwndArr) == 0)
        errmsg("Could not tile windows", "TileWindows");
}
void casc()
{
    EnumWindows(EnumWndPrc, 0);
    if (nwin == 1)
        return;
    if (CascadeWindows(NULL, MDITILE_SKIPDISABLED, NULL, nwin + 1, hwndArr) == 0)
        errmsg("Could not cascade windows", "CascadeWindows");
}
void reset()
{
    g_len = 0;
    g_needSave = false;
    labelWnd(UNTITLD);
    strcpy(g_filename, UNTITLD);

    lvupd();
}
inline int lvgetsel(HWND hLV)
{
    return ListView_GetNextItem(hLV, -1, LVNI_SELECTED | LVNI_FOCUSED);
}
inline void lvsetsel(HWND hLV, int pos)
{
    ListView_SetItemState(hLV, pos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}
void getfilelocation(char *fn, char res[])
{
    char *pend;
    int i;

    pend = strrchr(fn, '\\');

    for (i = 0; fn < pend; ++fn, ++i)
       res[i] = *fn;
    res[i] = 0;
}
Status free2new(const char *fnOld)
{
    typedef struct tagBinary {
        char descr[MAXNAME];
        int cat;
        currency amt;
        int day, month, yr;
        currency curbal;
    } oldbinrec;

    FILE *fpOld, *fpNew;
    int i;
    oldbinrec brOld;
    binrec    brNew;
    headmeta  hmNew;
    if ((fpOld = fopen(fnOld, "rb")) && (fpNew = fopen("newdata.such", "wb"))) {
        getc(fpOld);   // skip start byte

        for (hmNew.startbal = hmNew.len = 0; fread(&brOld, sizeof (oldbinrec), 1, fpOld) == 1; ++(hmNew.len)) {
            if (hmNew.len == 0)
                hmNew.startbal = brOld.amt;
            hmNew.endbal = brOld.amt;
        }
        memset(hmNew.passwd, 0, MAXPASS);
        metasize = sizeof (headmeta);
        fwrite(&metasize, sizeof (unsigned char), 1, fpNew);
        fwrite(&hmNew, sizeof (headmeta), 1, fpNew);
        rewind(fpOld);
        getc(fpOld);
        for (i = 0; fread(&brOld, sizeof (oldbinrec), 1, fpOld) == 1; ++i) {
            strcpy(brNew.cat, getCatName(brOld.cat));
            strcpy(brNew.descr, brOld.descr);
            brNew.day = brOld.day;
            brNew.month = brOld.month;
            brNew.yr = brOld.yr;
            brNew.amt = brOld.amt;
            brNew.curbal = brOld.curbal;

            fwrite(&brNew, sizeof (binrec), 1, fpNew);
        }
        fclose(fpNew);
        fclose(fpOld);
        closeall();
        if (remove(fnOld) != 0)
            fmtmsgbox(MB_ICONERROR, "Could not remove file %s: %s", fnOld, strerror(errno));
        return OK;
    }
    return FAIL;
}
char *changePartStatus(char *newtext, int part)
{
    static char old[32];
    SendMessage(GetDlgItem(g_hwndMain, IDC_MAIN_STATUS), SB_GETTEXT, part, (LPARAM) old);
    SendMessage(GetDlgItem(g_hwndMain, IDC_MAIN_STATUS), SB_SETTEXT, part, (LPARAM) newtext);

    return old;
}
Status ints(int i, char *dest, int ndest)
{
    if (snprintf(dest, ndest, "%d", i) >= 0)
        return OK;
    return FAIL;
}
Status floats(float f, char *dest, int ndest)
{
    if (snprintf(dest, ndest, "%f", f) >= 0)
        return OK;
    return FAIL;
}
Status moneys(currency c, char *dest, int ndest)
{
    if (snprintf(dest, ndest, "$%.2lf", TODOLLARS(c)) >= 0)
        return OK;
    return FAIL;
}
Status doubs(double d, char *dest, int ndest)
{
    if (snprintf(dest, ndest, "%g", d) >= 0)
        return OK;
    return FAIL;
}
char *trimends(char *str)
{
    char *end;

    for (; isspace(*str); ++str) ;

    if (!(*str))
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    end[1] = 0;

    return str;
}
void lowerstr(char *s)
{
    int i;

    for (i = 0; s[i]; ++i)
        s[i] = tolower(s[i]);
}
Status header(const char *nam, headmeta *hm)
{
    char old[MAXNAME];
    Status ret;

    strcpy(old, g_filename);
    strcpy(g_filename, envcat(ENV_SAVE, nam));
    ret = readmeta(hm);
    strcpy(g_filename, old);

    return ret;
}
char *substr(char *str, char *seq, bool caseSensitive = true)
{
    char s[80];

    strcpy(s, str);
    if (!caseSensitive) {
        lowerstr(s);
        lowerstr(seq);
    }
    return strstr(s, seq);
}
char *comma(double val)
{
    int digits(int);
    static char result[64];
    char *ptr;
    int n;

    sprintf(result, "%.2f", val);

    for (ptr = result; *ptr && *ptr != '.'; ptr++)
        ;
    // Length of fractional part
    n = result + sizeof(result) - ptr;
    // Step backwards, inserting spaces
    do {
        ptr -= 3;
        if (ptr > result) {
            memmove(ptr + 1, ptr, n);
            *ptr = ',';
            n += 4; // 3 digits + separator
        } else
            break;
    } while (1);
    return result;
}
int digits(int x)
{
    char buf[32];
    sprintf(buf, "%d", x);
    return strlen(buf);
}
int strcasecomp(const char *s1, const char *s2)
{           // case insensitive string comp
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;

    for (; *p1; ++p1, ++p2) {
        if (*p2 == '\0') return  1;
        if (tolower(*p2) > tolower(*p1))   return -1;
        if (tolower(*p1) > tolower(*p2))   return  1;
    }

    if (*p2 != '\0') return -1;

    return 0;
}
void reverse(char *str)
{
    char *p = str;
    char *pend;

    for (pend = str + strlen(str) - 1; pend > p; --pend, ++p) {
        char tmp;
        tmp = *pend;
        *pend = *p;
        *p = tmp;
    }
}
inline bool empty(char *s)
{
    return (strcmp(s, "") == 0) ? true : false;
}
char *cat(int nargs, ...)
{
    int i;
    static char buf[128];
    va_list vl;

    memset(buf, 0, 128);
    va_start(vl, nargs);

    for (i = 0; i < nargs; ++i)
        strcat(buf, va_arg(vl, char *));
    va_end(vl);
    return buf;
}
bool isequal(const char *s, const char *t, bool caseSensitive)
{
    if (caseSensitive)
        return (strcmp(s, t) == 0) ? true : false;
    return (strcasecomp(s, t) == 0) ? true : false;
}
void closeall()
{
    int i;

    for (i = STDERR_FILENO + 1; i < FOPEN_MAX; ++i)
        fclose(&_iob[i]);
}
char *renm()
{
    static char input[MAXINPUT];
    if (prompt(input, MAXINPUT, "Enter the new name", "", IBF_FILENAM) == IDOK) {
        extcat(input);
        closeall();
        if (rename(g_filename, input) != 0) {
            char x[32];
            strcpy(x, cat(4, "Couldn't rename file ", g_filename, ": ", strerror(errno)));
            errmsg(x, "");
        }
    }
    return input;
}
void toClpbrd(char *s, int siz)
{
    HGLOBAL hg;
    char *ptr;

    OpenClipboard(g_hwndMain);
    EmptyClipboard();

    hg = GlobalAlloc(GHND | GMEM_SHARE, siz + 1);
    ptr = (char *) GlobalLock(hg);

    strcpy(ptr, s);

    GlobalUnlock(hg);

    OpenClipboard(g_hwndMain);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
}
char *fromClpbrd(int siz)
{
    HGLOBAL hGlobal;
    char *pGlobal;
    static char dest[256];

    OpenClipboard (g_hwndMain);
    if ((hGlobal = GetClipboardData(CF_TEXT))) {
        pGlobal = (char *) GlobalLock(hGlobal);
        strncpy(dest, pGlobal, (size_t) 256);
    }
    CloseClipboard();
    return dest;
}
void enablecmd(int id, bool bEnable = true)
{
    SendDlgItemMessageA(g_hwndMain, IDC_MAIN_TB, TB_ENABLEBUTTON, id, MAKELPARAM(
                                                                               bEnable ? TRUE : FALSE,
                                                                               0));
    EnableMenuItem(GetMenu(g_hwndMain), id,
                   bEnable ? MF_ENABLED : MF_DISABLED);
}
void enableItms(bool enabl)
{
    enablecmd(ID_FILE_NEW, enabl);
    enablecmd(ID_FILE_REN, enabl);
    enablecmd(ID_FILE_EXPORTEXCEL, enabl);
    enablecmd(ID_FILE_REN, enabl);
}
DWORD majver()
{
    DWORD v;
    DWORD maj;

    v = GetVersion();

    maj = (DWORD) LOBYTE(LOWORD(v));
    return maj;
}
DWORD minver()
{
    DWORD v;
    DWORD min;

    v = GetVersion();
    return (min = (DWORD) HIBYTE(LOWORD(v)));
}
char *getMainWndText()
{
    static char buf[64];
    GetWindowText(g_hwndMain, buf, 64);

    return buf;
}
bool isvalidPaste()
{
    char inp[256];

    strcpy(inp, fromClpbrd(256));

    if (strchr(inp, '\t') == NULL || strchr(inp, '>') == NULL)
        return false;
    return true;
}
void mainwindow(HINSTANCE hThisInstance, int nCmdShow)
{
    g_hwndMain = CreateWindow(g_classname, "SuperCheck",
                              WS_OVERLAPPEDWINDOW, g_progsettings.winX, g_progsettings.winY,
                              g_progsettings.winWidth, g_progsettings.winHeight,
                              0, 0, NULL, 0);

    if (!g_hwndMain) {
        MessageBox(NULL, "Couldn't make window", "CreateWindowEx", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    if (g_progsettings.winX == -1 && g_progsettings.winY == -1)
        centerwin(g_hwndMain);

    /* Make the window visible on the screen */
    ShowWindow(g_hwndMain, nCmdShow);
    UpdateWindow(g_hwndMain);

    if (g_progsettings.winX < 0 && g_progsettings.winY < 0)
        ShowWindow(g_hwndMain, SW_MAXIMIZE);
}
void rem()
{
    if (!g_len)
        setneedsave(false);
    int item;
    if ((item = lvgetsel(g_hwndTable)) != -1) {
        if (g_len > 0 && item < g_len && item > -1) {
           int i;
           int last_index = g_len - 1;
           for (i = item; i < last_index; i++)
              g_arrtrns[i] = g_arrtrns[i + 1];
           g_len--;
        }
        setneedsave(true);
    } else
        MessageBox(g_hwndMain, "You didn't select anything!", "", MB_OK | MB_ICONASTERISK);
}
void rem(int item)
{
    if (g_len > 0 && item < g_len && item > -1) {
       int i;
       int last_index = g_len - 1;
       for (i = item; i < last_index; i++)
          g_arrtrns[i] = g_arrtrns[i + 1];
       g_len--;
    }
    setneedsave(true);
}
bool isopen(char *ftitle)
{
    char cap[32];

    strcpy(cap, "Transactions - ");
    strcat(cap, ftitle);
    if (FindWindow(g_classname, cap) == NULL)
        return false;
    return true;
}
void insert(int pos, xaction &x)
{
    int end;
    for (end = g_len; end >= pos; --end)
        g_arrtrns[end + 1] = g_arrtrns[end];
    g_arrtrns[pos] = x;

    ++g_len;
}
void setbal()
{
    int i;
    currency bal = 0;

    for (i = 0; i < g_len; ++i) {
        bal += g_arrtrns[i].amt;
        g_arrtrns[i].curbal = bal;
    }
}
void getsysfnt(LOGFONT *lf)
{
    NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

    *lf = ncm.lfMessageFont;
}
HFONT getsysfnt()
{
    LOGFONT lf;

    getsysfnt(&lf);

    return CreateFontIndirect(&lf);
}
void setWndTextDbl(HWND h, double d)
{
    char buf[32];
    sprintf(buf, "%lf", d);
    SetWindowText(h, buf);
}
void setWndTextInt(HWND h, int i)
{
    char buf[32];
    sprintf(buf, "%d", i);
    SetWindowText(h, buf);
}
BOOL CALLBACK EnumWndPrc(HWND hwnd, LPARAM lParam)
{
    char cls[16];
    GetClassNameA(hwnd, cls, 16);

    if (nwin == MAXWIN - 1)
        return FALSE;

    if (isequal(cls, g_classname)) {
        hwndArr[nwin++] = hwnd;
    }
    return TRUE;
}
bool contains(char *s, int c)
{
    for (; *s; ++s)
        if (*s == c)
            return true;
    return false;
}
int saveasdlg(char *fnbuf)
{
    int iRet;
    char input[MAXINPUT];
    if ((iRet = prompt(input, MAXINPUT, "Enter the name of this account", "Save As", IBF_FILENAM, "", "Discard && Exit")) == IDOK) {  // exit and save
        if (!empty(input)) {
            extcat(input);
            strcpy(fnbuf, envcat(ENV_SAVE, input));
            return IDYES;
        }
    } else if (iRet == IDC_INPUT_THIRD)             // exit and discard changes
        return IDNO;
    else if (iRet == IDCANCEL)          // nevermind
        return IDCANCEL;
    SUPPRESS(int);
}
bool direxists(const char *dirnam)
{
    struct stat stbuf;

    return (stat(dirnam, &stbuf) == 0 && S_ISDIR(stbuf.st_mode));
}
void opendlg()
{
    if ((g_hOpenLst = CreateDialog(g_hinst, MAKEINTRESOURCEA(TRNSOPENDLG), g_hwndMain, OpenLstDlg)))
        ShowWindow(g_hOpenLst, SW_SHOWNOACTIVATE);
}
inline int randm(int lb, int ub)
{
    return (lb + rand() % ub);
}
inline void insstr(char *s, char c, int pos)
{
    memmove(s + pos + 1, s + pos, strlen(s + pos));
    s[pos] = c;
}
TBBUTTON tbbtn(int bmpInd, int id, const char *str="", UINT styl = TBSTYLE_BUTTON, UINT stat = TBSTATE_ENABLED)
{
    TBBUTTON btn;

    btn.iBitmap = bmpInd;
    btn.fsState = stat;
    btn.fsStyle = styl;
    btn.idCommand = id;
    btn.iString = (INT_PTR) str;

    return btn;
}
TBBUTTON tbsep()
{
    TBBUTTON btn;

    btn.iBitmap = 0;
    btn.fsState = TBSTATE_ENABLED;
    btn.fsStyle = TBSTYLE_SEP;
    btn.idCommand = 0;

    return btn;
}
TBBUTTON tbdd(int bmpind, int id, const char *str="")
{
    TBBUTTON btn;

    btn.iBitmap = bmpind;
    btn.idCommand = id;
    btn.iString = (INT_PTR) str;
    btn.fsState = TBSTATE_ENABLED;
    btn.fsStyle = BTNS_WHOLEDROPDOWN;

    return btn;
}
HBITMAP hicoTohbmp(HICON hi)
{
    ICONINFO ii;

    GetIconInfo(hi, &ii);

    return ii.hbmColor;
}
inline bool isalign(void *pv, int nalign)
{
    return (((int) pv) % nalign);
}
HBITMAP bmpTransparent(HBITMAP hbmSrc)
{
	HDC hdcSrc, hdcDst;
	HBITMAP hbmpNew = NULL;
	BITMAP bm;
	COLORREF clrTP, clrBk;

	if ((hdcSrc = CreateCompatibleDC(NULL))) {
		if ((hdcDst = CreateCompatibleDC(NULL))) {
			int row, col;
			GetObject(hbmSrc, sizeof(bm), &bm);
			SelectObject(hdcSrc, hbmSrc);
			hbmpNew = CreateBitmap(bm.bmWidth, bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel, NULL);
			SelectObject(hdcDst, hbmpNew);

			BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

			clrTP = GetPixel(hdcDst, 0, 0);     /* color of first pixel at origin */
			clrBk = GetSysColor(COLOR_MENU);    /* the current background color of the menu */

			for (row = 0; row < bm.bmHeight; row++)
				for (col = 0; col < bm.bmWidth; col++)
					if (GetPixel(hdcDst, col, row) == clrTP)
						SetPixel(hdcDst, col, row, clrBk);

			DeleteDC(hdcDst);
		}
		DeleteDC(hdcSrc);
	}
	return hbmpNew;
}
char *ext(char *fn)
{
    char *ptr = strrchr(fn, '.');

    return ptr;
}
char *ftitle(char *path)
{
    char *fn;
    if ((fn = strrchr(path, '\\')) != NULL)
        return ++fn;      // strrchr includes the initial '/'
    return path;
}
char *ftitleExt(char *path)
{
    char *p = strrchr(path, '.');
    if (p == NULL)
        return path;

    *(path + (p - path)) = 0;

    return path;
}
char *labelWnd(char *nam)
{
    static char cap[32];
    snprintf(cap, 31, "SuperCheck-%s", ftitle(g_filename));

    SetWindowText(g_hwndMain, cap);
    return cap;
}
int safecpy(char *s, const char *t, size_t siz)
{
	size_t i;

	for (i = 1; (*s = *t) && i < siz; ++i, s++, t++)
		;
	s[i] = 0;

	return i;
}
void moneysplit(currency c, int *whole, int *cents)
{
    char buf[32];

    sprintf(buf, "%.2lf", c);
    sscanf(buf, "%d.%d", whole, cents);
}
inline char *safecat(char *dest, const char *src, size_t siz)
{
    return strncat(dest, src, siz);
}
int prompt(char *user, size_t nuser, const char *descr, const char *titl, unsigned flg, const char *deftxt, const char *third)
{
    inpbx ib(descr, titl, nuser, deftxt, third, flg);
    return ib.show(user, nuser);
}
int prompt(int *userint, char *descr, char *titl, unsigned flg, char *deftxt, char *third)
{
    char user[MAXINPUT];
    int btn;

    inpbx ib(descr, titl, MAXINPUT, deftxt, third, flg);
    btn = ib.show(user, MAXINPUT);

    if (sscanf(user, "%d", userint) == 0) {
        *userint = 0;
        return -1;  // invalid input
    }

    return btn;
}
int prompt(double *userdbl, char *descr, char *titl, unsigned flg, char *deftxt, char *third)
{
    char user[MAXINPUT];
    int btn;

    inpbx ib(descr, titl, MAXINPUT, deftxt, third, flg);
    btn = ib.show(user, MAXINPUT);

    if (sscanf(user, "%lf", userdbl) == 0) {
        *userdbl = 0;
        return -1;  // invalid input
    }

    return btn;
}
LONG getwinx(HWND h)
{
    RECT rc;

    GetWindowRect(h, &rc);

    return rc.left;
}
LONG getwiny(HWND h)
{
    RECT rc;

    GetWindowRect(h, &rc);

    return rc.top;
}
#endif // UTIL_H_INCLUDED
