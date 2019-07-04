/* Prototypes and globals */
#define CAST (char*)
// suppress -Wreturntype warning
#define SUPPRESS(type)      return (type)(42)

typedef bool Status;

int g_len = 0;
const int MAXNAME = 32;
const int MAXCAT = 16;
const int MAXLINE = 100;

const char *MONTHS[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
enum { JAN = 1, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEPT, OCT, NOV, DEC };

enum passwdstat { NOPASSWORD, MATCH, CANCELED };


#define MSG_CONFIRMSAV        "Do you want to save the changes?"
#define MSG_CONFIRMREM         "Are you sure you want to remove this?"

#define SBPOS_ISSAVED   0
#define SBPOS_NTRNS     1
#define SBPOS_STARTBAL  3
#define SBPOS_ENDBAL    2

#define NOIMPLEMENT MessageBox(NULL, "This feature does not work yet!", "Coming Soon", MB_OK | MB_ICONASTERISK)


#define LSF_NOPARENT    (1 << 0)    /* does not show . or .. */
#define LSF_MATCHEXT    (1 << 1)    /* matches `extension` argument */
#define LSF_NODIR       (1 << 2)    /* does not list directories */
#define LSF_NOEXT       (1 << 3)    /* shows the filename, minus the extension */
#define LSF_TOARR       (1 << 4)    /* writes to the array of dirents */

#define LSE_NODIR       (-1)        /* no such directory */
#define LSE_ARGINVAL    (-2)        /* invalid arguments to `ls` */
#define LSE_NOMEM       (-3)        /* out of memory */
#define LSE_OTHER       (-4)        /* unknown error*/

#define MAXLISTING      16


enum { PRINT_WORDMONTH, PRINT_NUM, PRINT_DOW_WRD };

typedef double currency;
#define TODOLLARS(c) ((c))
#define TOCENTS(d)  ((d))
unsigned char metasize;

#include "types.h"
xaction *g_arrtrns;
// ~210 total functions (including WinMain)

// main.cpp
LRESULT CALLBACK WndPrc(HWND, UINT, WPARAM, LPARAM);    // window proc
void        onCreat();                                  // init app
void        onSize(HWND, UINT, int, int);               // WM_SIZE
void        onNotify(WPARAM, LPARAM);                   // WM_NOTIFY
void        onNotifyDisplayLV(LPARAM);                  // LVN_GETDISPINFO
void        onPaint(HWND);                              // WM_PAINT
void        onTimer(HWND, UINT);                        // WM_TIMER
void        onClose(HWND);                              // WM_CLOSE, WM_DESTROY
void        onCmd(HWND, int, HWND, UINT);               // WM_COMMAND
void        ctxmenu(NMHDR *);                           // shows context menu
void        onPaste();                                  // "Paste"
void        onCpy();                                    // "Copy"
void        onFileNew();                                // "File"->"New"
int         onSave(int);                                // "File"->"Save"
void        save();                                     // saves the current file
void        appendSysMnu();                             // appends some items to the system menu
void        onEdit();                                   // "Edit"->"Edit Transaction"
void        onSysCmd(int);                              // WM_SYSCOMMAND
void        onRemove();                                 // "Edit"->"Remove Transaction"
void        onExcelExprt();                             // Exports the file to an Excel file
void        onStartbal();                               // "Edit"->Change Start Bal
void        updStatusBar();                             // updates the status bar
char        *hlpstr(WPARAM);                         // sets the help string for the specified id
void        onSegfault(int);                            // activated when SIGSEGV is received
void        setico(HWND, UINT, HICON);                  // sets an icon for the menu item
Status      lvappend(int, HWND);                        // appends to a listview
Status      winregister(HINSTANCE);                     // registers the window class
HWND        makeTB();                                   // makes the toolbar...
HIMAGELIST  makeTBImgList(HWND hTool);                  // ...and the image list...
HWND        makeStatus();                               // ... and the statusbar...
HWND        makeLV();                                   // ... and the listview
HWND        makeballoontt(char *, HWND);                  // creates a balloon tooltip
int         setminsiz(HWND, MINMAXINFO *);              // sets the minimum size for the window

// util.h
int     varMsgbox(UINT, int, ...);          // messagebox with varargs
int     errmsg(const char *, const char *); // displays messagebox with MB_ICONSTOP
int     getClientHeight(HWND);                 // gets the height of a window
int     getClientWidth(HWND);                  // gets the width of a window
char    *envcat(char *, const char *);     // appends the specified environment var
bool    fexists(char *);                   // does the file exist?
int     asksave();                          // prompts user to save changes
void    getfilelocation(char *, char []);  // gets the file location ex: C:\Users
char    *changePartStatus(char *, int);    // sets the text of a status bar part. Returns the old part text
Status  ints(int, char *, int);          // writes the int into the buffer
Status  floats(float, char *, int);      // writes the float into the buffer
Status  doubs(double, char *, int);      // writes the double into the buffer
LRESULT CALLBACK NumEditPrc
        (HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);      // "edit control" window subclass to allow digits only
char    *trimends(char *);                 // trims any leading or trailing whitespace
void    lowerstr(char *);                  // converts the entire string to lowercase
char    *substr(char *, char *, bool);     // finds a string inside another string
char    *comma(double);                    // inserts commas every 3 digits
int     digits(int);                        // returns the number of digits
int     dosave(int);                        // saves if the user says so
char    *ext(char *);                      // returns the extension of a file
int     strcasecomp(const char *, const char *);    // case insensitive string compare
void    moneysplit(currency c, int *, int *);
void    reverse(char *);                   // reverses the string (in-place)
void    centerwin(HWND);                   // centers the window on the screen
inline bool empty(char *);              // is the string empty?
bool    isequal(const char *, const char *, bool = true);   // are the strings equal?
void    toClpbrd(char *, int);             // sends the text to the clipboard
char    *fromClpbrd(int);                  // retrieves text from the clipboard
bool    isvalidfn(char *);                 // is this a valid windows filename?
bool    contains(char *, int);             // determines if the string contains a character
int     multhi(int, int);                   // returns the nearest multiple up
void    extcat(char *);                    // appends the ".such" extension, if that has not been done yet
char    *getMainWndText();                 // get the text in the main window's title bar
bool    isvalidPaste();                    // checks if the clipboard text is valid transaction
char    *datastr(int, int);                // gets the string from the column and row
BOOL    CALLBACK EnumWndPrc(HWND hwnd, LPARAM lParam); // finds windows that have the same window class
char    *wndtxt(HWND);                     // returns the window text of the specifed window
char    *wndtxt(HWND, int);                // returns the window text of the specified window
void    mainwindow(HINSTANCE, int);        // creates the main window
Status free2new(const char *);              // converts to new file
Status  header(const char *, headmeta *);// gives the header for the named file
size_t  fmtmsg(char *);                  // gives an error message string from the last win32 error
char    *fmtmsg();                         // returns error msg string from last win32 error
void    setbal();                          // recalculates the balance
void    enablecmd(int, bool);              // enable the commands when user saves the "Untitled" file
char    *renm();                            // rename the file
int     fmtmsgbox(UINT, const char *, ...); // message box printf
int     fmtmsgbox(const char *, ...);       // message box printf
int     fmtmsgbox(size_t *, HWND, UINT, const char *, const char *, ...);   // message box printf
Status  fclr(const char *);              // clears the file
void    showhelp(const char *);            // loads the specified help html file
void    closeall();                        // close all open files, except stdin, stdout, and stderr
char    *cat(int, ...);                    // appends everything together
int     fmtwrite(HWND, int, const char *, ...);     // writes formatted text into a window
int     fmtwrite(HWND, const char *, ...);  // writes formatted text into a window
int     fmtread(HWND , int, const char *, ...);     // reads formatted text from a window
bool    isopen(char *);           // is this file open in another instance?
char    *ftitle(char *);                   // gets the filename Ex: ftitle("C:\Users\foo") returns "foo"
char    *ftitleExt(char *);                // ftitle, except it returns the extension too
char    *labelWnd(char *);                 // sets the main window text to the filename
void    rem();                             // removes a transaction
bool    isallsame(char *, char);           // checks if all characters of the string are equal
bool    containscntrl(char *);             // does the string contain any cntrl chars(not including null terminator)
bool    issubstr(char *, char *, bool b);  // does the string contain the specified sequence?
Status  reallocate();                    // reallocates the array containing the transactions
void    dbgmsg(const char *);              // if DBG macro is defined, prints the message
void    lvupd();                           // updates the listview
void    reset();                           // resets the count, listview, and statusbar
HBITMAP hicoTohbmp(HICON);              // convert hicon to hbitmap
void    enableItms(bool enabl = true);     // enables all the context-sensitive toolbar/menu items
DWORD   majver();                         // returns the major version number of the OS
DWORD   minver();                         // returns the minor version number of the OS
int     saveasdlg(char *);                  // displays the "save as" input box
void    opendlg();                         // displays the "Open dialog"
int     prompt(int*,char*,char*,unsigned,char*,char*);
void    tile();                            // tiles all instances of this prog
void    casc();                            // cascades all instances of this prog
bool    isstartbal();                      // does a starting balance exist?
HBITMAP bmpTransparent(HBITMAP);        // makes the bitmap transparent to the menu
void    getcreat
        (const char *, const char *, char *);   // gets the creation date string for the file
void    insert(int, xaction &);            // inserts the specified transaction
void    setWndTextDbl(HWND, double);       // sets window text to a double
void    setWndTextInt(HWND, int);          // sets window text to an int
char    *getCatName(int number);
void    setneedsave(bool);                 // sets g_needSave to true, and updates LV, SB, current balance
void    setneedsave(bool, int);            // sets g_needSave to true, and appends to the specified index
int     safecpy(char*,const char*,size_t);  // safe string copy function
TBBUTTON tbbtn(int,int,const char*,UINT,UINT); // make a tb button
TBBUTTON tbsep();                       // make a tb separator
TBBUTTON tbdd(int, int, const char *);  // make a tb drop down item
inline char*safecat(char*,char*,size_t);// safe strcat function
int     prompt(char*, size_t, const char*, const char* ="", unsigned=0, const char* ="", const char* ="");   // displays an input box (prompt)
int     prompt(currency*, char*, char*, unsigned, char*, char*);      // reads money from input box
int     prompt(double*, char*, char*, unsigned, char*, char*);     // reads double from input box

// xaction.h
Status readmeta(headmeta *);            // reads the metadata
Status writemeta(const headmeta *);     // writes the metadata
Status read();                          // reads the file
long filesiz(const char *);             // returns the size of the file
inline bool iscorr();                   // is the file valid?
currency calcBalance(bool = true);         // calculates currentbal
currency calcBalance(int, bool = true);    // calculates currentbal up till specified index
Status creatFile();                     // recreates the file
void load();                            // calls read
Status readTodayFile(date *);           // reads today's date from a file
Status writeToday();                    // write today's date into a file
bool isfreever(char *fn);
Status freetopaid(char *);

// passwd.h
void encryptPass(const char *, int, char *);  // encrypt password
void decryptPass(const char *, int, char *);  // decrypt password
void strtohex(char *, char *);          // converts text to hex
void hextostr(const char *, char *);          // converts hex to text
void addone(char *);                    // adds one to each character. Ex: 'a' becomes 'b'
void minusone(char *);                  // inverse of addone
void toggcase(char *, int);             // toggles the case. Ex: "Abc" becomes "aBC"
passwdstat askpasswd();                 // asks the password from the user
inline bool islockedout();              // is the file accessible
bool direxists(const char *dirnam);

//dlgproc.h
date getcreat(char *);                  // get created date of file
date getmod(char *);                    // get modified date of file


// newtrns.h
void initDeposit(HWND), initWd(HWND), initEdit(HWND, int), loadcategory(HWND),
            allowtax(HWND, HWND), disabletax(HWND, HWND), checktax(HWND, HWND), uncheckTax(HWND, HWND);
int gettype(HWND);                  // gets the type of transaction (Deposit or Withdrawal)
char *getfields(HWND, xaction *);   // retrieves the data in the dialog
void seticon(HWND, int);            // sets the icon of the dialog, depending on Deposit or Withdrawal
currency getamt(HWND, HWND);           // gets the amount of money, based on the two edit controls (one for cents, the other for whole dollars)
void clear();                       // clears everything in the dialog
LRESULT CALLBACK odValEdit(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);    // subclasses the whole dollars edit cntrl
LRESULT CALLBACK odValCentsEdit(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);   // subclasses the cents edit control
BOOL CALLBACK NewTrnsDlg(HWND, UINT, WPARAM, LPARAM);           // dialog procedure for New Deposit/Withdrawal Dlg
char *fill2zeros(int);              // fills with zeros. Ex: 7 becomes 70
void setamt(HWND, int, int);        // sets the amount edit cntrls
void setamt(HWND, double);          // sets the amount edit cntrls


// config.h
Status getint(char *, int *);       // reads an int from the string
int getlong(const char *, long *, int);     // reads a long from the string
Status getfloat(char *, float *);   // reads a float from the string
Status getbool(char *, bool *);     // reads a bool from the string
int confgdlg();                     // shows the settings dlg
BOOL CALLBACK ColWidthPrc(HWND, UINT, WPARAM, LPARAM);  // dialog procedure for column widths pg
BOOL CALLBACK WndPosDlg(HWND, UINT, WPARAM, LPARAM);    // dialog proc for window pos pg
BOOL CALLBACK PrefDlg(HWND, UINT, WPARAM, LPARAM);      // dialog proc for preferences pg

#define PRINTBOOL(b) b ? "t" : "f"

const int MAXKEY = 32;
const int MAXVAL = 64;
const int MAXSETTINGS = 14;
const char *all_keys[MAXSETTINGS] = {"height", "width", "x", "y", "s-tax", "col-name-width", "col-date-width", "col-amt-width", "col-tax-width",
                                    "col-bal-width", "autosave", "yr-salary", "col-cat-width", "tut"};

// date.h

date today();       // returns today's date
static int dow(date &);    // returns day of week. 0 = Sun, 1 = Mon, etc.
char *dowtostr(int);// converts the return value from dow into a string
bool isleap(int);   // is the year a leap year?
int month2i(const char *);      // converts month string to int

// ls.h
bool isdir(const char *, const char *);                 // is the file a directory?
int ls(const char *, const char *, dirent *, unsigned); // shows the directory listing
int ls(const char *, const char *, unsigned);           // shows the directory listing
int ls(const char *, unsigned);                         // shows the directory listing
bool isExt(const char *, const char *);                 // does the file name have this extension?
