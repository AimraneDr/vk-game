#ifndef FILES_H
#define FILES_H

#include "data_types.h"
#include "engine_defines.h"

#include <string/str.h>

typedef struct FileHandle_t
{
    char *path;
    
    char *content;
    u32 size;
    u32 cursor;
} FileHandle;

/// @brief load the file into memory
/// @return a handle to the file 
API FileHandle *file_load(const char *path);

/// @return a ptr to the start of next line
API const char* file_next_line(FileHandle *file);

/// @return string of the current line the cursor points to
API String file_line_toString(FileHandle *file);

/// @brief free resources allocated by the file handle 
API void file_unload(FileHandle *file);

/// @return the file name part of the path 
API String path_get_file_name(const char *path);
/// @return the directory that the file lives in 
API String path_get_file_dir(const char *path);
/// @return the file extension part of the path 
API String path_get_file_extension(const char *path);

#endif //FILES_H