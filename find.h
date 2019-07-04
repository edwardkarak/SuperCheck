class finddlg {
    FINDREPLACE fr;
    HWND hFindDlg;
public:
    enum { MAXFIND = 80 };
    UINT findmsg, hlpmsg;
    int  col;
    char findbuf[MAXFIND];

    void show(HWND, UINT, const char * ="Find");
    void show(HWND, UINT, const char *, void (*)(HWND));
    bool isdlgmsg(MSG *);
    void onFindNext();
    void onhlp();
    int  findnxt(bool = false);
    finddlg() { memset(&fr, 0, sizeof (FINDREPLACE)); hFindDlg = NULL; }

} g_fd;
void finddlg::show(HWND hOwner, UINT flags, const char *cap)
{
    this->findmsg = RegisterWindowMessageA(FINDMSGSTRINGA);

    memset(&fr, 0, sizeof (FINDREPLACE));

    this->fr.lStructSize = sizeof (FINDREPLACE);
    this->fr.hwndOwner = hOwner;
    this->fr.lpstrFindWhat = findbuf;
    this->fr.wFindWhatLen = MAXFIND;
    this->fr.Flags = flags;

    if (flags & FR_SHOWHELP)
        this->hlpmsg = RegisterWindowMessage(HELPMSGSTRING);
    else
        this->hlpmsg = 0;

    if (this->hFindDlg == NULL)
        this->hFindDlg = FindTextA(&fr);

    SetWindowText(this->hFindDlg, cap);
}
void finddlg::show(HWND a, UINT b, const char *c, void (*draw)(HWND))
{
    this->show(a, b, c);
    draw(this->hFindDlg);
}
bool finddlg::isdlgmsg(MSG *lpMsg)
{
    return IsDialogMessageA(this->hFindDlg, lpMsg);
}
void finddlg::onFindNext()
{
    if (this->fr.Flags & FR_DIALOGTERM) {
        this->hFindDlg = NULL;
        this->findnxt(true);        // does not actually search, it just resets the
                                    // function's position indicator
    }

    if (this->fr.Flags & FR_FINDNEXT) {
       int pos;
       pos = this->findnxt();
       if (pos == -1) {
            varMsgbox(MB_OK, 3, "Can't find \"", this->fr.lpstrFindWhat, "\"");
            this->findnxt(true);
       }
       else
            ListView_SetItemState(g_hwndTable, pos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}
int finddlg::findnxt(bool callJustToReset)
{
    int i;
    char *text;
    static int lastfind = -1;
    if (callJustToReset) {
        i = 0;
        return (lastfind = -1);
    }

    for (i = lastfind + 1; i < g_len; ++i) {
        text = datastr(this->col, i);

        bool x =  (fr.Flags & FR_MATCHCASE);
        if (substr(text, fr.lpstrFindWhat, x))
            return (lastfind = i);
    }
    return -1;  // end of search
}
void finddlg::onhlp()
{
    showhelp("hlpFind.html");
}
