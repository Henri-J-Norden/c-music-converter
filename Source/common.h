#ifndef CONMAN_COMMON_H
#define CONMAN_COMMON_H

#include <windows.h>
#include <stdio.h>

#define UMAX_PATH 32768 // windows max unicode path length

extern int ENCODING;
extern char* CUSTOM_ENCODING; // used if ENCODING == -1
extern const char* ENCODINGS[];
extern const char* EXTENSIONS[];
extern char* ENCODE[100];

typedef struct _THREAD_DATA {
    char* cmd;
} _THREAD_DATA, *P_THREAD_DATA;

extern int LOAD; // bool
extern int SAVE; // bool
extern int CONV; // bool
extern int LOG; // 0 ... 5
extern int THREADS;
extern STARTUPINFO* SI;
extern PROCESS_INFORMATION* PI;
extern char INPATH[UMAX_PATH];
extern char OUTPATH[UMAX_PATH]; // it is ensured that user inputted in and out paths are terminated with "\\"
extern char EXTENSION[UMAX_PATH/4];
extern char EXECUTABLE[UMAX_PATH];
extern char ARGS[1000];
extern char BITRATE[100];

// Global counters
extern int DIR_COUNT;
extern int FILE_COUNT;
extern int MAX_FILE_COUNT;
extern int PROCESSING_DEPTH;

extern const int STR_LEN;
extern char** MESSAGES;
extern int MSG_COUNT;
extern int _MAX_MSG_COUNT;
extern int DATA_LOADED; // bool

int parseArgs(int c, char *args[]);

typedef struct file {
    struct file *next; //sibling
    struct file *prev; //sibling
    struct directory *parent;

    char* name;
    long long size;
    FILETIME lastWriteTime;
    FILETIME creationTime;
} file;

extern const struct file file_DEFAULT;

typedef struct directory {
    struct directory *next; //sibling
    struct directory *prev; //sibling
    struct directory *parent;
    struct directory *firstSubdir;
    struct file *firstFile;

    char* fullPath;
    int FILE_COUNT;
    int DIR_COUNT;
} directory;

extern const struct directory directory_DEFAULT;

#endif //CONMAN_COMMON_H
