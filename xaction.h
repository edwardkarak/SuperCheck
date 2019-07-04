#ifndef XACTION_H_INCLUDED
#define XACTION_H_INCLUDED
/* Deals with the xaction class and files */
#include "date.h"


xaction::xaction(char *ca, char *n, currency c, date d_) : amt(c), d(d_)
{
    strcpy(cat, ca);
    strcpy(descr, n);
}
xaction::xaction() : amt(0)
{
    memset(descr, 0, sizeof descr);
    memset(cat, 0, MAXCAT);
}

Status xaction::fileappend()
{
    int ret;
    FILE *fp = fopen(g_filename, "ab");
    binrec r;

    if (fp) {
        ret = OK;

        this->exportbin(&r);

         if (fwrite(&r, sizeof (binrec), 1, fp) != 1)
            ret = FAIL;
    } else
        ret = FAIL;

    fclose(fp);
    return ret;
}
int xaction::find()
{
    for (int i = 0; i < g_len; ++i)
        if (strcmp(g_arrtrns[i].descr, this->descr) == 0)
            return i;
    return -1;
}
int xaction::findr()
{
    for (int i = g_len; i > 0; --i)
        if (strcmp(g_arrtrns[i].descr, this->descr) == 0)
            return i;
    return -1;
}
char *xaction::getAmtStr(char *whol, char *cnts)
{
    int iwhol, icnts;
    static char buf[32];

    sprintf(buf, "$%.2lf", TODOLLARS(amt));

    sscanf(buf, "$%d.%d", &iwhol, &icnts);

    ints(iwhol, whol, 8);
    ints(icnts, cnts, 3);

    return buf;
}
char *xaction::getAmtStr()
{
    char x[8], y[3];
    return getAmtStr(x, y);
}
Status xaction::setAmtStr(char *bamt)
{
    if (sscanf(bamt, "%lf", &this->amt) == 1 || sscanf(bamt, "$%lf", &this->amt) == 1)
        return OK;
    return FAIL;
}
Status xaction::setCurbalStr(char *s)
{
    float f;
    Status sRet;
    if (sscanf(s, "%f", &f) == 0)
        sRet = FAIL;
    else sRet = OK;

    this->curbal = TOCENTS(f);

    return sRet;
}
char *xaction::getCurbalStr()
{
    static char b[64];
    floats(this->curbal, b, 64);

    return b;
}
void xaction::exportbin(binrec *r)
{
    encryptPass(this->descr, sizeof r->descr, r->descr);
    r->amt = this->amt;
    r->day = this->d.day;
    r->month = this->d.month;
    r->yr = this->d.yr;
    r->curbal = 0;     // calculated later
    encryptPass(this->cat, sizeof r->cat, r->cat);
}
void xaction::importbin(const binrec &r)
{
    decryptPass(r.descr, sizeof this->descr, this->descr);
    this->amt = r.amt;
    this->d.month = r.month;
    this->d.day = r.day;
    this->d.yr = r.yr;
    this->curbal = 0;       // calculated later
    decryptPass(r.cat, sizeof this->cat, this->cat);
}

void xaction::setstr(char *s, bool isClpbrd)
{
    char *ptr = s;
    char datbuf[8], amtbuf[16], curbalbuf[16];
    int i;

    if (isClpbrd)
        while (*ptr++ != '>') ;

    for (i = 0; *ptr != '\t'; ++ptr)
        this->cat[i++] = *ptr;
    this->cat[i] = 0;
    ptr++;

    for (i = 0; *ptr != '\t'; ++ptr)
        this->descr[i++] = *ptr;
    this->descr[i] = 0;
    ptr++;

    for (i = 0; *ptr != '\t'; ++ptr)
        datbuf[i++] = *ptr;
    datbuf[i] = 0;
    ptr++;

    for (i = 0; *ptr != '\t'; ++ptr)
        amtbuf[i++] = *ptr;
    amtbuf[i] = 0;
    ptr++;

    if (!isClpbrd) {
        for (i = 0; *ptr; ++ptr)
            curbalbuf[i++] = *ptr;
        curbalbuf[i] = 0;

        this->setCurbalStr(curbalbuf);
    }

    if (this->d.mkFmt(datbuf) == FAIL)
        fmtmsgbox(0, "Invalid transaction");
    this->setAmtStr(amtbuf);
}
void xaction::getstr(char *dest, size_t ndest, bool isClpbrd)
{
    char curbalstr[64];
    floats(this->curbal, curbalstr, 64);
    snprintf(dest, ndest, "%s>%s\t%s\t%s\t%s\t%s", isClpbrd ? "trnsv1" : "", this->cat, this->descr, this->d.getfmt(), this->getAmtStr(), isClpbrd ? curbalstr : "");
}
// end xaction functions
Status read()
{
    FILE *fp;
    int i;

    binrec r;
    headmeta hm;

    readmeta(&hm);

    g_len = hm.len;
    strcpy(g_cryptPassword, hm.passwd);

    g_arrtrns = (xaction *) malloc(sizeof (xaction) * (lim = multhi(REALLOC_STEP, g_len)));

    if ((fp = fopen(g_filename, "rb")) != NULL) {
        fseek(fp, metasize + 1, SEEK_SET);

        for (i = 0; i < g_len; ++i) {
            fread(&r, sizeof (binrec), 1, fp);
            g_arrtrns[i].importbin(r);
        }
        return OK;
    }
    fclose(fp);
    return FAIL;
}
Status writemeta(const headmeta *hm)
{
    FILE *fp;

    if ((fp = fopen(g_filename, "wb"))) {
        fwrite(&metasize, sizeof (unsigned char), 1, fp);
        fwrite(hm, sizeof (headmeta), 1, fp);

        fclose(fp);
        return OK;
    }
    return FAIL;
}
Status readmeta(headmeta *hm)
{
    FILE *fp;

    if ((fp = fopen(g_filename, "rb"))) {
        fread(&metasize, sizeof (unsigned char), 1, fp);

        fread(hm, metasize, 1, fp);

        fclose(fp);
        return OK;
    }
    return FAIL;
}
long filesiz(const char *path)
{
    long siz;
    FILE *fp;

    if ((fp = fopen(path, "rb"))) {
        fseek(fp, 0, SEEK_END);
        siz = ftell(fp);

        fclose(fp);
        return siz;
    }
    return -1;
}
bool isfreever(char *fn)
{
    FILE *fp;

    if ((fp = fopen(fn, "rb"))) {
        if (getc(fp) == FREESTART && isequal(ftitle(fn), "data.such"))
            return true;
        fclose(fp);
    }
    return false;
}
inline bool iscorr()
{
    headmeta m;
    bool ismultiple = ((filesiz(g_filename) - metasize - 1) % sizeof (binrec) == 0);
    Status ismeta = (readmeta(&m));

    if (ismultiple && ismeta == OK)
        return false;
    return true;
}
currency calcBalance(bool addTax)
{
    currency bal = 0;
    int i = 0;

    for (i = 0; i < g_len; ++i)
        bal += g_arrtrns[i].amt;
    return bal;
}
currency calcBalance(int zbUpTo, bool addTax)
{
    currency bal = 0;
    int i;

    for (i = 0; i < zbUpTo; ++i)
        bal += g_arrtrns[i].amt;
    return bal;
}

Status creatFile()
{
    FILE *data = fopen(g_filename, "w");
    if (data == NULL)
        return FAIL;
    fclose(data);
    return OK;
}
void load()
{
    if (!read()) {
        creatFile();
        g_len = 0;
    }
}
#endif // XACTION_H_INCLUDED
