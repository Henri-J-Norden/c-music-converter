#include "loadsave.h"

file* readFile(FILE *f, directory *parent, file *lastSibling) {
    file *d = malloc(sizeof(file));
    *d = file_DEFAULT;
    char* name = readStr(f);
    d->name = name;
    d->size = readLInt(f);
    d->creationTime = longToTime(readLInt(f));
    d->lastWriteTime = longToTime(readLInt(f));

    d->parent = parent;
    if (parent->firstFile == NULL) {
        parent->firstFile = d;
    } else {
        d->prev = lastSibling;
        lastSibling->next = d;
    }

    return d;
}


directory* loadData(char* codec, directory *parent, FILE* _f, directory *lastSibling) {
    FILE* f;
    if (_f == NULL) {
        char* fpath = getFilePath(codec);
        f = fopen(fpath, "r");
        if (f == NULL || LOAD == 0) {
            directory *curD = malloc(sizeof(directory));
            *curD = directory_DEFAULT;
            char* path = malloc((strlen(INPATH) + 1) * sizeof(char));
            strcpy(path, INPATH);
            curD->fullPath = malloc(strlen(path)*sizeof(char)+128);
            strcpy(curD->fullPath, path);
            DATA_LOADED = 0;
            return curD;
        }
        if (LOG > 0) printf("\nLoading data (%s)...\n", fpath);
    } else {
        f = _f;
    }

    directory *curD = malloc(sizeof(directory));
    *curD = directory_DEFAULT;

    char* dPath = readStr(f);
    if (LOG >= 36) printf("%s->%s\n", spaces(), dPath);
    curD->fullPath = malloc((strlen(dPath) + 2) * sizeof(char));
    strcpy(curD->fullPath, dPath);
    curD->DIR_COUNT = readInt(f);
    if (parent == NULL) {
        loadData(codec, curD, f, NULL);
        fclose(_f);
        return curD;
    }
    // set relations
    curD->parent = parent;
    if (parent->firstSubdir == NULL) {
        parent->firstSubdir = curD;
    } else {
        lastSibling->next = curD;
        curD->prev = lastSibling;
    }
    // process subdirectories
    directory *lastSubD = NULL;
    PROCESSING_DEPTH++;
    for (int i = 0; i < curD->DIR_COUNT; i++) {
        lastSubD = loadData(codec, curD, f, lastSubD);
    }

    // process files
    curD->FILE_COUNT = readInt(f);
    file *lastSubF = malloc(sizeof(file));
    char* spc;
    if (LOG >= 40) spc = spaces();
    for (int i = 0; i < curD->FILE_COUNT; i++) {
        lastSubF = readFile(f, curD, lastSubF);
        if (LOG >= 40) printf("%sFile %s\n", spc, lastSubF->name);
    }
    PROCESSING_DEPTH--;
    if (LOG >= 36) printf("%s<-%s\n", spaces(), dPath);

    if (_f == NULL) {
        fclose(_f);
        free(f);
        return parent;
    }
    return curD;
}



// writes all the data from a single file
void writeFile(FILE* f, file *data) {
    writeStr(f, data->name);
    writeLInt(f, data->size);
    writeLInt(f, timeToLong(data->creationTime));
    writeLInt(f, timeToLong(data->lastWriteTime));
}


void saveData(directory *parent, char* codec, FILE* _f) {
    FILE* f;
    if (_f == NULL) {
        f = fopen(getFilePath(codec), "w");
    } else {
        f = _f;
    }

    // save subdirectories
    if (LOG >= 50) printf("Saving %s\n", parent->fullPath);
    writeStr(f, parent->fullPath);
    writeInt(f, parent->DIR_COUNT);
    directory *subD = NULL;
    if (parent->DIR_COUNT > 0) {
        subD = parent->firstSubdir;
    }
    while (subD != NULL) {
        fflush(f);
        saveData(subD, codec, f);
        subD = subD->next;
    }

    // save files in current directory
    writeInt(f, parent->FILE_COUNT);
    file *subF = NULL;
    if (parent->FILE_COUNT > 0) {
        fflush(f);
        subF = parent->firstFile;
    }
    while (subF != NULL) {
        writeFile(f, subF);
        subF = subF->next;
        fflush(f);
    }

    if (_f == NULL) {
        fclose(f);
        fclose(_f);
    }
}