//
// Created by Henri on 2017-12-18.
//

#ifndef CONMAN_LOADSAVE_H
#define CONMAN_LOADSAVE_H

#include "utils.h"


file* readFile(FILE *f, directory *parent, file *lastSibling);
directory* loadData(char* codec, directory *parent, FILE* _f, directory *lastSibling);

// writes all the data from a single file
void writeFile(FILE* f, file *data);
void saveData(directory *parent, char* codec, FILE* _f);

#endif //CONMAN_LOADSAVE_H
