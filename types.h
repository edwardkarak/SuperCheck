/* Global Types */
typedef struct tagMeta {
    int           len;
    currency           startbal;
    currency           endbal;
    char          passwd[MAXPASS];
} headmeta;
class date {
    void init(int, int, int);
    int cmp(const date &);
public:
    int day, month, yr;

    date(int, int, int);
    date();

    int mkFmt(char *);
    char *prn(int = PRINT_WORDMONTH);
    void setprn(char [], int);
    char *getfmt();
    date operator=(const date &);
    bool operator>(const date &);
    bool operator>=(const date &);
    bool operator<(const date &);
    bool operator<=(const date &);
    bool operator==(const date &);
    void toTmStruct(struct tm *);
    SYSTEMTIME toWinTime();
    time_t toUnixTime();
};
typedef struct tagBinary {
    char descr[MAXNAME];
    char cat[MAXCAT];
    currency amt;
    int  day, month, yr;
    currency  curbal;
} binrec;

class xaction {
public:
    char        cat[MAXCAT];
    char        descr[MAXNAME];
    currency    amt;
    date        d;
    currency    curbal;

    xaction(char *, char *, currency,  date);
    xaction();

    Status      fileappend();
    int         find();
    int         findr();
    bool        isDeposit()    { return (amt > 0) ? true : false;    }
    bool        isWithdrawal() { return (amt < 0) ? true : false;    }

    void        importbin(const binrec &);
    void        exportbin(binrec *);

    char        *getTaxrateStr();
    Status      setTaxrateStr(char *);
    Status      setPayrollStr(char *);

    char        *getAmtStr(char *, char *);
    char        *getAmtStr();
    Status      setAmtStr(char *);

    Status      setCurbalStr(char *);
    char        *getCurbalStr();

    void        getstr(char *, size_t, bool);
    void        setstr(char *, bool);
};
typedef struct tagArry {
    size_t nelem;
    int    arr[64];
} Arry;
class settings
{
    void getvalue(const char *, char[]);
public:
    static const int MIN = 10;
    static const int MAX = 500;

    int winHeight, winWidth;
    int winX, winY;

    double taxrate;
    double yrSalary;

    int colNameWidth, colDateWidth, colAmtWidth, colTaxWidth, colBalWidth, colCatWid;

    int autosaveMins;

    bool tut;

    Status readSettings();
    void setDefaults();
    void saveSettings();
} g_progsettings;

#define MAXINPUT (64)
#define IBF_PASSWORD 1
#define IBF_FILENAM  2

class inpbx {
public:
    static BOOL CALLBACK InputDlgProc(HWND, UINT, WPARAM, LPARAM);
    char     descr[128];
    char     title[32];
    char     deftext[64];
    char     thirdbtn[16];
    unsigned flags;
    char     input[MAXINPUT];
    int      btn;
    size_t   maxuser;

    int show();
    int show(char *, size_t);

    inpbx(const char *adescr, const char *atitl="", size_t nuser=MAXINPUT, const char *adeftxt="", const char *athirdbtn="", unsigned aflag=0) :  flags(aflag) , btn(0), maxuser(nuser)
    {
        strcpy(this->descr, adescr);
        strcpy(this->title, atitl);
        strcpy(this->deftext, adeftxt);
        strcpy(this->thirdbtn, athirdbtn);
        strcpy(this->input, "");
    }
    inpbx() : flags(0) , btn(0), maxuser(0)
    {
        strcpy(this->descr, "");
        strcpy(this->title, "");
        strcpy(this->deftext, "");
        strcpy(this->thirdbtn, "");
        strcpy(this->input, "");
    }
};
class qtydlg {
    static BOOL CALLBACK QtyDlg(HWND, UINT, WPARAM, LPARAM);
public:
    double unprc;
    int qty;

    int show();
    qtydlg() : unprc(0), qty(0) { }
};
