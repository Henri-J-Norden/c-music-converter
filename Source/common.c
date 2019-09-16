#include "common.h"

#define MAX_THREADS 8
#define UMAX_PATH 32768 // windows max unicode path length

int ENCODING = 2;
char* CUSTOM_ENCODING; // used if ENCODING == -1
const char* ENCODINGS[] =  {"MP3", "FLAC", "OPUS"};
const char* EXTENSIONS[] = {".mp3", ".flac", ".opus"};
char* ENCODE[100] = {".mp3", ".flac", ".opus", ".m4a", ".wma", ".wav", ".aiff", ".aac", ".alac", ".cda", ".mp2", ".ac3", ".ogg", ".mid", ".wmv", ".mpc", ".mpp", ".psid"};


// Settings (default values)
int LOAD = 1; // bool
int SAVE = 1; // bool
int CONV = 1; // bool
int LOG = 50; // 0 ... 5
int THREADS = 1;
STARTUPINFO* SI;
PROCESS_INFORMATION* PI;
char INPATH[UMAX_PATH] = ".\\";
char OUTPATH[UMAX_PATH] = ".\\OUT\\"; // it is ensured that user inputted in and out paths are terminated with "\\"
char EXTENSION[UMAX_PATH/4] = ":";
char EXECUTABLE[UMAX_PATH] = "C:\\Program Files\\dBpoweramp\\CoreConverter.exe";
char ARGS[1000];
char BITRATE[100];

// Global counters
int DIR_COUNT = 0;
int FILE_COUNT = 0;
int MAX_FILE_COUNT = 0;
int PROCESSING_DEPTH = 0;


const int STR_LEN = 255;
char** MESSAGES;
int MSG_COUNT = -1;
int _MAX_MSG_COUNT = 100;
int DATA_LOADED = 1; // bool

const struct file file_DEFAULT = {
        NULL, NULL, NULL, NULL, 0, 0, 0
};

const struct directory directory_DEFAULT = {
        NULL, NULL, NULL, NULL, NULL, NULL, 0, 0
};


int parseArgs(int c, char *args[]) {
    for (int i = 1; i < c; i++) {
        if (strcmp(args[i], "-nosave") == 0) {
            SAVE = 0;
        } else if (strcmp(args[i], "-noload") == 0) {
            LOAD = 0;
        } else if (strcmp(args[i], "-noconv") == 0) {
            CONV = 0;
        } else if (strcmp(args[i], "-help") == 0 || strcmp(args[i], "-h") == 0 || strcmp(args[i], "-?") == 0) {
            printf("%s%s%s%s%s%s%s%s%s%s%s",
                   "\"-noload\"\t - Ignore any existing database file.\n\n",
                   "\"-nosave\"\t - Do not create or write to any database files.\n\n",
                   "\"-noconv\"\t - Do not convert any files.\n\n",
                   "\"-log <int>\"\t - Set the log level (default: 20)\n\t\t    0 - No log\n\t\t   10 - Only log removed files\n\t\t   20 - Log removed files and file operations\n\t\t   30 - Log operations, removed and changed files\n\t\t   35 - Log operations, removed and changed files and file reading\n\t\t   40 - Log file reading, changes, operations and data loading\n\t\t   50 - Log everything (changes, operations, data loading and saving)\n\n",
                   "\"-i <path>\"\t - Use to specify the input path (default: \".\").\n\n",
                   "\"-o <path>\"\t - Use to specify the output path:\n\t\t   * a path starting with \".\" is interpreted as relative to the current directory,\n\t\t   * a path starting with a drive letter + \":\" is interpreted as absolute,\n\t\t   * any other path is interpreted as relative to the input path.\n\t\t      For example, the default path is relative to the input. (default: \"OUT\")\n\n",
                   "\"-e <enc>\"\t - The encoding to convert to. Officially supported: \"MP3\", \"FLAC\", \"OPUS\",\n\t\t   however any dBpoweramp encoding should work (default: \"OPUS\").\n\n",
                   "\"-b <bitrate>\"\t - Encoding bitrate or quality (e.g \"128\" or \"V0\" for MP3).\n\t\t   NB: this option in only available for supported encodings\n\t\t   (and the encoding must be set before this option)!\n\t\t   Unsupported encodings must use \"-a\" to set the quality!\n\n",
                   "\"-args <str>\"\t - A complete string of valid custom arguments for the specified encoding.\n\t\t   Can be used to set advanced options.\n\n",
                   "\"-oext <ext>\"\t - Set a custom extension for output files (including the first \".\").\n\t\t   Correct defaults are available for supported encodings.\n\t\t   For unsupported encodings the default is \"\".\n\n",
                   "\"-exe <path>\"\t - Path to the dBpoweramp converter (default: \"C:\\Program Files\\dBpoweramp\\CoreConverter.exe\").\n\n");
            return 1;
        } else if (strcmp(args[i], "-log") == 0) {
            LOG = atoi(args[++i]);
        } else if (strcmp(args[i], "-i") == 0) {
            strcpy(INPATH, args[++i]);
            if (INPATH[strlen(INPATH) - 2] != '\\') strcat(INPATH, "\\");
        } else if (strcmp(args[i], "-o") == 0) {
            strcpy(OUTPATH, args[++i]);
            if (OUTPATH[strlen(OUTPATH) - 2] != '\\') strcat(OUTPATH, "\\");
        } else if (strcmp(args[i], "-e") == 0) {
            char encs[1000] = "";
            int found = 0;
            i++;
            for (int j = 0; j < sizeof(ENCODINGS) / sizeof(ENCODINGS[0]); j++) {
                if (j != 0) strcat(encs, ", ");
                strcat(encs, ENCODINGS[j]);
                if (strcmp(ENCODINGS[j], args[i]) == 0) {
                    ENCODING = j;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("NOTICE: Using custom (unsupported) encoding: %s.\nIf this was not intentional please select one of the supported encodings instead: %s.\n\n", args[i], encs);
                ENCODING = -1;
                CUSTOM_ENCODING = args[i];
            }
        } else if (strcmp(args[i], "-b") == 0) {
            strcpy(BITRATE, args[++i]);
        } else if (strcmp(args[i], "-args") == 0) {
            strcpy(ARGS, args[++i]);
        } else if (strcmp(args[i], "-oext") == 0) {
            strcpy(EXTENSION, args[++i]);
        } else if (strcmp(args[i], "-exe") == 0) {
            strcpy(EXECUTABLE, args[++i]);
        } else {
            printf("Unknown argument \"%s\"! (Use \"-help\" or \"-h\" to view all options)", args[i]);
            return 1;
        }
    }
    return 0;
}

