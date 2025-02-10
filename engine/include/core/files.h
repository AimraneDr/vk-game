#ifndef FILES_H
#define FILES_H

#include "data_types.h"
#include "engine_defines.h"

#include <string/str.h>

typedef struct File_t
{
    char *path;
    
    char *content;
    u32 size;
} File;

API File *readFile(const char *path);
API void freeFile(File *file);

API String file_name(const char *path);
API String file_extension(const char *path);

#endif //FILES_H