#ifndef CONMAN_PROCESSOR_H
#define CONMAN_PROCESSOR_H

#include "loadsave.h"

// string fullPath must be terminated with "\" (and must point to a directory)
void processDir(char* fullPath, directory* parent);

// Sets the file's data in the DB to the current filesystem data
// Returns 1 if changes were made to the DB file, otherwise returns 0
int setFileData(file *f, WIN32_FIND_DATA *fData, int compareFirst);

void removeFile(file* f);
void removeDir(directory* d);
void removeDeletedFiles(char* fullPath, directory* curD);

#endif //CONMAN_PROCESSOR_H
