long fsiz(char *, char *);
int ls(const char *fullpathdir, const char *ext, dirent *arrEnt, unsigned flags)
{
    DIR *dir;
    struct dirent *ent;
    int i;
    char buf[MAX_PATH];

    if ((flags & LSF_NOEXT) && ext == 0)
        return LSE_ARGINVAL;
    if (flags < 0)
        return LSE_ARGINVAL;
    if ((flags & LSF_TOARR) && arrEnt == NULL)
        return LSE_ARGINVAL;

    if ((dir = opendir(fullpathdir))) {
        for (i = 0; (ent = readdir(dir)); ) {
            if ((flags & LSF_NODIR) && isdir(fullpathdir, ent->d_name))
                continue;
            if ((flags & LSF_MATCHEXT) && !isExt(ent->d_name, ext))
                continue;
            if ((flags & LSF_NOPARENT) && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0))
                continue;

            strcpy(buf, ent->d_name);

            if (flags & LSF_NOEXT) {
                char *p;

                if ((p = strrchr(buf, '.')))
                    buf[p - buf] = 0;
            }
            if ((flags & LSF_TOARR)) {
                if (i < MAXLISTING)
                    arrEnt[i] = *ent;
                else
                    return LSE_NOMEM;
            }
            else
                printf("%s\n", buf);
            ++i;
        }
        closedir(dir);
        return i;
    } else
        return LSE_NODIR;
    return LSE_OTHER;       // suppress compiler warning
}
int ls(const char *fullpathdir, const char *ext, unsigned flags)
{
    return ls(fullpathdir, ext, NULL, flags);
}
int ls(const char *fullpathdir, unsigned flags)
{
    return ls(fullpathdir, NULL, NULL, flags);
}
bool isdir(const char *pth, const char *fn)
{
    char full[MAX_PATH];
    strcpy(full, pth);
    strcat(full, "\\");
    strcat(full, fn);
    if (GetFileAttributes(full) == FILE_ATTRIBUTE_DIRECTORY || strcmp(fn, "..") == 0 || strcmp(fn, ".") == 0)
        return true;
    else
        return false;
}
long fsiz(char *path, char *fn)
{
    FILE *fp;
    long s;
    char buf[MAX_PATH];

    sprintf(buf, "%s\\%s", path, fn);

    if ((fp = fopen(buf, "rb"))) {
        fseek(fp, 0, SEEK_END);
        s = ftell(fp);

        fclose(fp);
        return s;
    }
    return -1;
}
