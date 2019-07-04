#ifndef PASSWD_H_INCLUDED
#define PASSWD_H_INCLUDED
const int MAXTRY    = 5;

static bool restrictpass(char *);                                  // restricts the password
/* Password encryption/decryption algorithms */

void encryptPass(const char *uncrypt, int ndest, char *destCrypt)
{
    char cpy[ndest + 1];
    strcpy(cpy, uncrypt);

    reverse(cpy);
    addone(cpy);
    toggcase(cpy, 1);
    strtohex(cpy, destCrypt);
}
void decryptPass(const char *encrypt, int ndest, char *destDecrypt)
{
    hextostr(encrypt, destDecrypt);
    toggcase(destDecrypt, 1);
    minusone(destDecrypt);
    reverse(destDecrypt);
}


void strtohex(char *str, char bufHex[])
{
    char buf[32];
    int i, iChar;

    strcpy(bufHex, "");

    for (i = 0; str[i]; ++i) {
        iChar = str[i];
        sprintf(buf, "%x", iChar);
        strcat(bufHex, buf);
    }
}
void hextostr(const char *hex, char bufText[])
{
    int c, i;
    for (i = 0; hex[0] && hex[1] && sscanf(hex, "%2x", &c); hex += 2, ++i)
        bufText[i] = c;
    bufText[i] = 0;
}


void addone(char *s)
{
    int i;
    for (i = 0; s[i]; ++i)
        (s[i])++;
}
void minusone(char *s)
{
    int i;
    for (i = 0; s[i]; ++i)
        (s[i])--;
}


void toggcase(char *s, int freq)
{
    int i;

    i = (freq % 2 == 0) ? 0 : 1;

    for (; s[i]; i += 2) {
        if (isupper(s[i]))
            s[i] = tolower(s[i]);
        else
            s[i] = toupper(s[i]);
    }
}
passwdstat askpasswd()
{
    char attempt[MAXPASS], cryptAttempt[MAXPASS];

    if (!empty(g_cryptPassword)) {
        do {
            if (prompt(attempt, MAXPASS, "Enter the password", "Password", IBF_PASSWORD) == IDCANCEL)
                return CANCELED;
            else {
                encryptPass(attempt, MAXPASS, cryptAttempt);
                if (isequal(g_cryptPassword, cryptAttempt))
                    return MATCH;
            }
        } while (!isequal(g_cryptPassword, cryptAttempt));
    } else
        return NOPASSWORD;

    return (passwdstat) -1;      // suppress compiler warn
}
BOOL CALLBACK PasswordDlg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hPass = GetDlgItem(hwnd, IDC_PASS);
    HWND hPass1 = GetDlgItem(hwnd, IDC_PASS1);
    switch (msg) {
    case WM_INITDIALOG:
        Edit_LimitText(hPass, MAXPASS - 1);
        Edit_LimitText(hPass1, MAXPASS - 1);
        if (!empty(g_cryptPassword)) {
            MessageBox(hwnd, "In order to change the password, you will need to enter the password again.\r\nPress OK to continue.", "", MB_OK);
            switch (askpasswd()) {
            case CANCELED:
                return TRUE;
            case MATCH:
                break;
            default:
                break;      // suppress -Wswitch warning
            }
        } else
            EnableWindow(GetDlgItem(hwnd, IDC_REMPASS), FALSE);
        return TRUE;
    case WM_SYSCOMMAND:
        if (LOWORD(wParam) == SC_CONTEXTHELP)
            showhelp("hlpPasswd.html");
        else return DefWindowProcA(hwnd, msg, wParam, lParam);
        return TRUE;
    case WM_COMMAND:
        char pass[MAXPASS], pass1[MAXPASS];

        switch (LOWORD(wParam)) {
        case IDC_REMPASS:
            memset(g_cryptPassword, 0, MAXPASS);
            MessageBox(g_hwndMain, "Password Cleared", "", MB_OK | MB_ICONASTERISK);
            setneedsave(true);
            EndDialog(hwnd, IDC_REMPASS);
            break;
        case IDOK:
            GetWindowText(hPass, pass, MAXPASS);
            GetWindowText(hPass1, pass1, MAXPASS);

            if (strcmp(pass, pass1) == 0)
                encryptPass(pass, MAXPASS, g_cryptPassword);
            else {
                MessageBox(hwnd, "The boxes are not the same!", "", MB_OK);
                break;
            }
            if (!restrictpass(pass))
                break;

            setneedsave(true);
            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE; // suppress compiler warning
}
static bool restrictpass(char *pass)
{
    if (pass[1] == 0) {
        MessageBox(g_hwndMain, "The password can't be only 1 character", "", MB_OK | MB_ICONERROR);
        return false;
    }
    if (*pass == 0) {
        MessageBox(g_hwndMain, "You didn't type anything", "", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}
/*void song()
{
    #define G4      392

    #define A4      440

    #define B4      494

    #define C5      523

    #define D5      587

    #define Eb5     622
    #define E5      659

    #define F5      698

    #define G5      784

    #define A5      880

    #define B5      988

    #define C6      1047

    #define D6      1175

    #define E6      1319

    #define F6      1397

    #define G6      1568



    #define WHOLE   1000

    #define DHALF   750
    #define HALF    500

    #define DQUART  375
    #define QUART   250

    #define DEIGHTH 188
    #define EIGHTH  125

    #define DSXTNTH 95
    #define SXTNTH  63

    Beep(E5, 500);
    Beep(C5, 500);
    Beep(E5, 500);
    Beep(C5, 500);

    Beep(F5, 500);
    Beep(E5, 500);
    Beep(D5, 500);

    Sleep(250);

    Beep(G4, 500);
    Beep(G4, 500);
    Beep(G4, 500);

    Beep(A4, 250);
    Beep(B4, 250);

    Beep(C5, 500);
    Beep(C5, 500);
    Beep(C5, 500);
}*/

#endif // PASSWD_H_INCLUDED
