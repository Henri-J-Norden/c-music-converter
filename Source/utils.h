#ifndef CONMAN_UTILS_H
#define CONMAN_UTILS_H

#include "common.h"

void addMessage(char* s);
char* getFilePath();
char* spaces();

char* readStr(FILE* f);
int readInt(FILE* f);
long long readLInt(FILE* f);
FILETIME longToTime(long long i);

void writeStr(FILE* f, char* s);
void writeInt(FILE* f, int i);
void writeLInt(FILE* f, long long i);
long long timeToLong(FILETIME t);

char* buildCommand(char* path, char* fileName);
char* getExtension(char* fileName);


#endif //CONMAN_UTILS_H