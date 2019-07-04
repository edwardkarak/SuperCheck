#ifndef DATE_H_INCLUDED
#define DATE_H_INCLUDED

#include <time.h>

/* Deals with dates */

// private member functions:
void date::init(int m, int d, int y)
{
    this->day = d;
    month = m;
    yr = y;
}
int date::cmp(const date &x)
{
    if (this->yr > x.yr)
        return 1;
    else if (this->yr < x.yr)
        return -1;

    if (this->yr == x.yr) {
        if (this->month > x.month)
            return 1;
        else if (this->month < x.month)
            return -1;
        else if (this->day > x.day)
            return 1;
        else if (this->day < x.day)
            return -1;
        else
            return 0;
    }
    return 0;
}

// public member functions:
date::date(int m, int d, int y)
{
    init(m, d, y);
}
date::date()
{
    init(0, 0, 0);
}
int date::mkFmt(char *fmt)
{
    char *p = fmt;
    char m = *fmt;
    char d = fmt[1];
    int y;

    p += 2;
    sscanf(p, "%d", &y);

    if (m >= 'A' && m <= 'A' + 12 && d >= 'A' && m <= 'A' + 31) {
        init(int(m - 'A' + 1), int(d - 'A' + 1), y);
        return OK;
    }
    else return FAIL;
}
char *date::prn(int flag)
{
    static char buf[32];
    switch (flag)
    {
    case PRINT_WORDMONTH:
        sprintf(buf, "%s %d %d", MONTHS[this->month - 1], this->day, this->yr);
        break;
    case PRINT_NUM:
        sprintf(buf, "%d / %d / %d", this->month, this->day, this->yr);
        break;
    case PRINT_DOW_WRD:
        sprintf(buf, "%s %s %d %d", dowtostr(dow(*this)), MONTHS[this->month - 1], this->day, this->yr);
        break;
    }
    return buf;
}
void date::setprn(char str[], int flag)
{
    switch (flag) {
    case PRINT_WORDMONTH:
        char s[10];
        sscanf(str, "%s %d %d", s, &this->day, &this->yr);
        this->month = month2i(s);
        break;
    case PRINT_NUM:
        sscanf(str, "%d / %d / %d", &this->month, &this->day, &this->yr);
        break;
    case PRINT_DOW_WRD:
        NOIMPLEMENT;
        break;
    }
}
char *date::getfmt()
{
    char *buf = new char[7];
    sprintf(buf, "%c%c%d", this->month + 'A' - 1, this->day + 'A' - 1, this->yr);
    return buf;
}
date date::operator=(const date &dr)
{
    this->init(dr.month, dr.day, dr.yr);
    return *this;
}
bool date::operator>(const date &x)
{
    switch (this->cmp(x)) {
        case 0:
        case -1:
            return false;
        case 1:
            return true;
    }
    return false;
}
bool date::operator>=(const date &x)
{
    switch (this->cmp(x)) {
    case 0:
    case 1:
        return true;
    case -1:
        return false;
    }
    return false;
}
bool date::operator<(const date &x)
{
    switch (this->cmp(x)) {
    case 0:
    case 1:
        return false;
    case -1:
        return true;
    }
    return false;
}
bool date::operator<=(const date &x)
{
    switch (this->cmp(x)) {
    case 0:
    case -1:
        return true;
    case 1:
        return false;
    }
    return false;
}
bool date::operator==(const date &x)
{
    switch (this->cmp(x)) {
    case 0:
        return true;
    case -1:
    case 1:
        return false;
    }
    return false;
}
SYSTEMTIME date::toWinTime()
{
    SYSTEMTIME wtime;
    wtime.wDay = day;
    wtime.wMonth = month;
    wtime.wYear = yr;

    return wtime;
}
void date::toTmStruct(struct tm *tim)
{
    tim->tm_mday = this->day;
    tim->tm_mon = this->month - 1;
    tim->tm_year = this->yr - 1900;
}
time_t date::toUnixTime()
{
    struct tm posix;

    this->toTmStruct(&posix);
    return mktime(&posix);
}
// normal functions:
date today()
{
    date r;
    struct tm *tim;
    time_t raw;

    time(&raw);

    if ((tim = localtime(&raw)) != NULL) {
        r.day = tim->tm_mday;
        r.month = tim->tm_mon + 1;
        r.yr = tim->tm_year + 1900;
    } else
        r.day = r.month = r.yr = 0;

    return r;
}
int dow(date &d)
{
    static int tbl[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    d.yr -= d.month < 3;
    return (d.yr + d.yr / 4 - d.yr / 100 + d.yr / 400 + tbl[d.month - 1] + d.day) % 7;
}
char *dowtostr(int dowRet)
{
    switch (dowRet) {
    case 0:
        return (char *) "Sun";
    case 1:
        return (char *) "Mon";
    case 2:
        return (char *) "Tue";
    case 3:
        return (char *) "Wed";
    case 4:
        return (char *) "Thur";
    case 5:
        return (char *) "Fri";
    case 6:
        return (char *) "Sat";
    }
    return "GARBAGE";       // suppress compiler warn
}
bool isleap(int y)
{
    if (y % 400 == 0)
        return true;
    else if (y % 100 == 0)
        return false;
    else if (y % 4)
        return false;
    else
        return false;
}

int month2i(const char *month)
{
    int i;

    for (i = 0; i < 12; ++i)
        if (isequal(month, MONTHS[i], false))
            return (i + 1);     // adjust index

    return -1;                  // invalid month
}
date getcreat(char *), getmod(char *);
#endif // DATE_H_INCLUDED
