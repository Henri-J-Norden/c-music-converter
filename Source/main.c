#include "processor.h"


int main(int argc, char *argv[], char **envp) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    THREADS = (int)sysinfo.dwNumberOfProcessors;
    SI = calloc((size_t)THREADS, sizeof(STARTUPINFO));
    PI = calloc((size_t)THREADS, sizeof(PROCESS_INFORMATION));

    if (parseArgs(argc, argv)) return 1;

    directory *curParent = loadData((strcmp(EXTENSION, ":") ? EXTENSION  : (char*)EXTENSIONS[ENCODING]), NULL, NULL, NULL);

    if (LOG > 0) printf("\nProcessing filesystem...\n");
    processDir(INPATH, curParent);

    if (SAVE) {
        if (LOG > 0) printf("\nSaving (%s)...\n", getFilePath());
        saveData(curParent, (strcmp(EXTENSION, ":") ? EXTENSION  : (char*)EXTENSIONS[ENCODING]), NULL);
    }

    if (LOG > 0) {
        if (MSG_COUNT != 0) printf("\n");
        for (int i = 0; i < MSG_COUNT; i++) {
            printf("%s\n", MESSAGES[i]);
        }
    }

    return 0;
}