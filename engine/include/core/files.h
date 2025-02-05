#ifndef FILES_H
#define FILES_H

#include "data_types.h"

typedef struct File_t
{
    char *path;
    char *content;
    u32 size;
} File;

File *readFile(const char *path);
void freeFile(File *file);

#endif //FILES_H