#include "utils.h"

char* buildCommand(char* path, char* fileName) { // path "INPATH
    int l = 0;
    char* p = malloc(UMAX_PATH * 2);
    char* fullPath = malloc(UMAX_PATH);
    char* buf = malloc(UMAX_PATH);
    sprintf(buf, "%s%s", path, fileName); // write the possibly relative input path to the buffer
    _fullpath(fullPath, buf, UMAX_PATH);
    l += sprintf(p, "\"%s\" -infile=\"%s\" ", EXECUTABLE, fullPath);
    // write the output path to fullPath
        // write path inside INPATH to buf2
    char* buf2 = malloc(UMAX_PATH);
    sprintf(buf, "%s", INPATH);
    _fullpath(fullPath, buf, UMAX_PATH);
    sprintf(buf, "%s", path);
    _fullpath(buf2, buf, UMAX_PATH);
    int i, j=0;
    for (i = 0; i < strlen(buf2); i++){
        if (fullPath[i] != buf2[i]) {
            fullPath[i+1] = 0;
            break;
        }
    }
    for (i; i < strlen(buf2); i++) {
        buf2[j++] = buf2[i];
    }
    buf2[j++] = 0;
        // -
    if (OUTPATH[0] == '.' || OUTPATH[1] == ':') {
        sprintf(buf, "%s%s", OUTPATH, buf2);
    } else { //path relative to the input
        sprintf(buf, "%s%s%s", INPATH, OUTPATH, buf2);
    }
    j = 0;
    int bufl = (int)strlen(buf);
    int end;
    for (int i = (int)strlen(fileName) - 1; i >= 0; i--) {
        if (fileName[i] == '.') {
            end = i;
            break;
        }
    }

    sprintf(buf2, "if 1==1 mkdir \"%s\" 2> NUL", buf); // mkdir
    system(buf2);

    for (int i = 0; i < end; i++) {
        buf[bufl + j++] = fileName[i];
    }
    if (strcmp(EXTENSION, ":") == 0) {
        if (CUSTOM_ENCODING == NULL) strcat(buf, EXTENSIONS[ENCODING]);
    } else {
        strcat(buf, EXTENSION);
    }
    // ---
    l += sprintf(p + l, "-outfile=\"%s\" -convert_to=\"%s\" -priority=\"idle\" %s", buf, CUSTOM_ENCODING != NULL ? CUSTOM_ENCODING : ENCODINGS[ENCODING], ARGS);

    return p;
}


char* readStr(FILE* f) {
    char* buf = malloc(UMAX_PATH);
    char b;
    int i = 0;
    do {
        b = (char)fgetc(f);
        buf[i++] = b;
    } while (b != '\0' && b != -1);
    char* buf2 = malloc(sizeof(char) * (i + 2));
    strcpy(buf2, buf);
    free(buf);
    return buf2;
}


int readInt(FILE* f) {
    int i;
    char *end;
    i = (int)strtol(readStr(f), &end, 0);
    return i;
}


long long readLInt(FILE* f) {
    long long i;
    char *end;
    i = strtoll(readStr(f), &end, 0);
    return i;
}


FILETIME longToTime(long long i) {
    FILETIME t;
    LARGE_INTEGER li;
    li.QuadPart = i;
    t.dwLowDateTime = li.LowPart;
    t.dwHighDateTime = li.HighPart;
    return t;
}


char* spaces() {
    char *s = malloc(sizeof(char) * (2 * PROCESSING_DEPTH + 10));
    int i = 0;
    while (i < PROCESSING_DEPTH*2) {
        s[i++] = ' ';
        s[i++] = ' ';
    }
    s[i++] = 0;
    return s;
}


void writeStr(FILE* f, char* s) {
    fwrite(s, sizeof(char), strlen(s), f);
    fwrite("\0", sizeof(char), 1, f);
}


void writeInt(FILE* f, int i) {
    //fwrite(&i, 4, 1, f);
    fprintf(f, "%i", i);
    fwrite("\0", sizeof(char), 1, f);
}


void writeLInt(FILE* f, long long i) {
    fprintf(f, "%lli", i);
    fwrite("\0", sizeof(char), 1, f);
}


long long timeToLong(FILETIME t) {
    LARGE_INTEGER i;
    i.HighPart = t.dwHighDateTime;
    i.LowPart = t.dwLowDateTime;
    return i.QuadPart;
}


void addMessage(char* s) { // Messages stored with this function are printed at the end of the program
    if (MSG_COUNT == -1) {
        MSG_COUNT = 0;
        MESSAGES = malloc(_MAX_MSG_COUNT * sizeof(char*));
    }
    if (++MSG_COUNT > _MAX_MSG_COUNT) {
        _MAX_MSG_COUNT *= 2;
        realloc(MESSAGES, _MAX_MSG_COUNT * sizeof(char*));
    }
    MESSAGES[MSG_COUNT - 1] = malloc((strlen(s) + 2) * sizeof(char));
    strcpy(MESSAGES[MSG_COUNT - 1], s);
}


char* getFilePath() { // Returns the path for the database file
    const char* codec = CUSTOM_ENCODING != NULL ? CUSTOM_ENCODING : ENCODINGS[ENCODING];
    char* path = malloc(strlen(codec + 4) * sizeof(char));
    strcpy(path, codec);
    strcat(path, ".db");
    return path;
}


char* getExtension(char* fileName) {
    char* ext = malloc(sizeof(char) * (strlen(fileName) + 2));
    int j = 0, i;
    for (i = (int)strlen(fileName) - 1; i >= 0; i--) {
        if (fileName[i] == '.') break;
    }
    for (i; i < (int)strlen(fileName); i++) {
        ext[j++] = fileName[i];
    }
    ext[j++] = 0;
    return ext;
}