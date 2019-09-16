#include "processor.h"

void runCommand(char* cmd) {
    DWORD code = 0;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    while (1) {
        for (int i = 0; i < THREADS; i++) {
            if (PI[i].hProcess != NULL) {
                GetExitCodeProcess(PI[i].hProcess, &code);
                if ((int)code != 259) {
                    CloseHandle(PI[i].hProcess);
                    CloseHandle(PI[i].hThread);
                    PI[i].hProcess = NULL;
                }
            }
            if (PI[i].hProcess == NULL) {
                if (!CreateProcess( NULL,   // No module name (use command line)
                               cmd,        // Command line
                               NULL,           // Process handle not inheritable
                               NULL,           // Thread handle not inheritable
                               FALSE,          // Set handle inheritance to FALSE
                               CREATE_NO_WINDOW ,              // Disable output
                               NULL,           // Use parent's environment block
                               NULL,           // Use parent's starting directory
                               &(SI[i]),            // Pointer to STARTUPINFO structure
                               &(PI[i])           // Pointer to PROCESS_INFORMATION structure
                )) {
                    printf( "CreateProcess failed (%d).\n", (int)GetLastError() );
                    return;
                }
                return;
            }
        }
        Sleep(250);
    }
}

// string fullPath must be terminated with "\" (and must point to a directory)
void processDir(char* fullPath, directory* parent) {
    WIN32_FIND_DATA *fData = malloc(sizeof(WIN32_FIND_DATA));

    char* findDir = malloc((strlen(fullPath) + 10) * sizeof(char));
    strcpy(findDir, fullPath);
    strcat(findDir, "*");

    directory *curD = NULL;
    if (parent != NULL) { // check whether the given directory is already saved
        if (parent->firstSubdir != NULL) { // if the parent folder has subdirectories
            curD = parent->firstSubdir;
            while (curD->next != NULL && strcmp(curD->fullPath, fullPath) != 0) { // walk the subdirectory data
                curD = curD->next;
            }
        }
        if (curD == NULL || strcmp(curD->fullPath, fullPath) != 0) { // make new directory
            directory *newD = malloc(sizeof(directory));
            *newD = directory_DEFAULT;
            if (curD != NULL) {
                while (curD->next != NULL) {
                    curD = curD->next;
                }
                newD->prev = curD;
                curD->next = newD;
            }
            newD->fullPath = malloc((strlen(fullPath) + 2) * sizeof(char));
            strcpy(newD->fullPath, fullPath);
            newD->parent = parent;
            parent->DIR_COUNT++;
            curD = newD;
            DIR_COUNT++;
        }
    }
    if (parent->firstSubdir == NULL) {
        parent->firstSubdir = curD;
    }
    if (DATA_LOADED) removeDeletedFiles(fullPath, curD);

    // process files and subdirs in this directory
    if (LOG >= 33) printf("%s->%s\n", spaces(), curD->fullPath);
    PROCESSING_DEPTH++;
    HANDLE hFind = FindFirstFile(findDir, fData);
    char* spc = spaces();
    do {
        if (strcmp(fData->cFileName, ".") == 0 || strcmp(fData->cFileName, "..") == 0) continue;
        char* curPath = malloc((strlen(fullPath) + strlen(fData->cFileName) + 4) * sizeof(char));
        strcpy(curPath, fullPath);
        strcat(curPath, fData->cFileName);

        if (fData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // is directory
            strcat(curPath, "\\");
            processDir(curPath, curD);

        } else { // is file
            file *curF = NULL;
            // find the file obj
            if (curD->firstFile != NULL) {
                curF = curD->firstFile;
                while (curF->next != NULL && strcmp(curF->name, fData->cFileName) != 0) {
                    curF = curF->next;
                }
            }

            // create new file obj
            if (curF == NULL || strcmp(curF->name, fData->cFileName) != 0) {
                file *newF = malloc(sizeof(file));
                *newF = file_DEFAULT;
                if (curF != NULL) {
                    while (curF->next != NULL) {
                        curF = curF->next;
                    }
                    newF->prev = curF;
                    curF->next = newF;
                }
                newF->parent = curD;
                newF->name = malloc((strlen(fData->cFileName) + 8) * sizeof(char));
                strcpy(newF->name, fData->cFileName);
                curF = newF;
                FILE_COUNT++;
                curD->FILE_COUNT++;
            }
            if (curD->firstFile == NULL) {
                curD->firstFile = curF;
            }
            // if the current file is different from saved data
            if (setFileData(curF, fData, 1)) {
                if (LOG >= 30) printf("%sFile %s [CHANGED]\n", spc, fData->cFileName);
                if (CONV) {
                    char* ext = strlwr(getExtension(fData->cFileName));
                    int matched = 0;
                    for (int i = 0; i < sizeof(ENCODE)/sizeof(ENCODE[0]); i++) {
                        if (ENCODE[i] == NULL) break;
                        if (strcmp(ext, ENCODE[i]) == 0) {
                            matched = 1;
                            break;
                        }
                    }
                    if (!matched) {
                        if (LOG >= 21) printf("%s \"%s\" did not match a valid audio extension. Skipping...\n", spc, ext);
                    } else {
                        runCommand(buildCommand(curD->fullPath, curF->name));
                    }
                }
            } else if (LOG >= 34) {
                printf("%sFile %s\n", spc, fData->cFileName);
            }
        }
        free(curPath);
    } while (FindNextFile(hFind, fData));
    PROCESSING_DEPTH--;
    if (LOG >= 33) printf("%s<-%s\n", spaces(), curD->fullPath);
    free(spc);
    free(fData);
    free(findDir);
}


// Sets the file's data in the DB to the current filesystem data
// Returns 1 if changes were made to the DB file, otherwise returns 0
int setFileData(file *f, WIN32_FIND_DATA *fData, int compareFirst) {
    LARGE_INTEGER fSize;
    fSize.LowPart = fData->nFileSizeLow;
    fSize.HighPart = fData->nFileSizeHigh;

    if (!compareFirst || (
            f->size != fSize.QuadPart ||
            CompareFileTime(&(f->lastWriteTime), &(fData->ftLastWriteTime)) != 0 ||
            CompareFileTime(&(f->creationTime), &(fData->ftCreationTime)) != 0)) {
        f->lastWriteTime = fData->ftLastWriteTime;
        f->creationTime = fData->ftCreationTime;
        f->size = fSize.QuadPart;
        return 1;
    }
    return 0;
}


void removeFile(file* f) { // from DB
    if (f->prev == NULL) { // f is the first file
        f->parent->firstFile = f->next; // NULL if f is the only file
        if (f->next != NULL) f->next->prev = NULL;
    } else { // f is not the first file
        f->prev->next = f->next; // NULL if f is the last file
        if (f->next != NULL) f->next->prev = f->prev;
    }
    f->parent->FILE_COUNT--;
    free(f->name);
}


void removeDir(directory* d) { // from DB
    if (d->prev == NULL) { // f is the first file
        d->parent->firstSubdir = d->next; // NULL if f is the only file
        if (d->next != NULL) d->next->prev = NULL;
    } else { // f is not the first file
        d->prev->next = d->next; // NULL if f is the last file
        if (d->next != NULL) d->next->prev = d->prev;
    }
    d->parent->DIR_COUNT--;
    free(d->fullPath);
}


void removeDeletedFiles(char* fullPath, directory* curD) { //could be optimized
    WIN32_FIND_DATA *fData = malloc(sizeof(WIN32_FIND_DATA));
    char* findDir = malloc((strlen(fullPath) + 4) * sizeof(char));
    strcpy(findDir, fullPath);
    strcat(findDir, "*");
    char* s = malloc(UMAX_PATH*2+100);
    file* f = curD->firstFile;
    while (f != NULL) {
        int found = 0;
        HANDLE hFind = FindFirstFile(findDir, fData);
        do {
            if (!(fData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(fData->cFileName, f->name) == 0) {
                found = 1;
                break;
            }
        } while (FindNextFile(hFind, fData));
        if (!found) {
            removeFile(f);
            sprintf(s, "DELETED FILE: %s in %s", f->name, findDir);
            if (LOG >= 10) addMessage(s);
        }
        f = f->next;
    }
    directory* d = curD->firstSubdir;
    char* curPath = malloc(UMAX_PATH + 100);
    while (d != NULL) {
        int found = 0;
        HANDLE hFind = FindFirstFile(findDir, fData);
        do {
            strcpy(curPath, fullPath);
            strcat(curPath, fData->cFileName);
            strcat(curPath, "\\");
            if (fData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(curPath, d->fullPath) == 0) {
                found = 1;
                break;
            }
        } while (FindNextFile(hFind, fData));
        if (!found) {
            removeDir(d);
            sprintf(s, "DELETED DIR: %s", d->fullPath);
            if (LOG >= 10) addMessage(s);
        }
        d = d->next;
    }
    free(curPath);
    free(s);
    free(findDir);
    free(fData);
}