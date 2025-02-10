#include "core/files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/str.h>

File *readFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *)malloc(fileSize + 1);
    if (!content)
    {
        fclose(file);
        return 0;
    }

    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';

    File *result = (File *)malloc(sizeof(File));
    if (!result)
    {
        free(content);
        fclose(file);
        return 0;
    }

    u32 path_len = strlen(path);
    result->path = (char *)malloc(sizeof(char) * (path_len + 1));
    memcpy(result->path, path, path_len);
    result->path[path_len] = '\0';
    result->content = content;
    result->size = (u32)fileSize;

    fclose(file);
    return result;
}

void freeFile(File *file)
{
    if(file == 0) return;
    free(file->path);
    free(file->content);
    free(file);
}

static size_t _find_last_separator(String path) {
    size_t slash_idx = str_rfind_char(path, '/');
    size_t bslash_idx = str_rfind_char(path, '\\');
    return (bslash_idx != STR_NPOS && bslash_idx > slash_idx) ? bslash_idx : slash_idx;
}

String file_name(const char* path) {
    String path_str = str_new(path);
    if (path_str.len == 0) return path_str;

    // Extract basename
    size_t sep_idx = _find_last_separator(path_str);
    size_t basename_start = (sep_idx != STR_NPOS) ? sep_idx + 1 : 0;
    String basename = str_slice(path_str, basename_start, path_str.len);
    str_free(&path_str);

    // Split name and extension
    size_t dot_idx = str_rfind_char(basename, '.');
    if (dot_idx == STR_NPOS) return basename; // No extension
    
    String name = str_slice(basename, 0, dot_idx);
    str_free(&basename);
    return name;
}

String file_extension(const char* path) {
    String path_str = str_new(path);
    if (path_str.len == 0) return path_str;

    // Find last dot after the last separator
    size_t sep_idx = _find_last_separator(path_str);
    size_t dot_idx = str_rfind_char(path_str, '.');
    
    if (dot_idx != STR_NPOS && (sep_idx == STR_NPOS || dot_idx > sep_idx)) {
        String ext = str_slice(path_str, dot_idx + 1, path_str.len);
        str_free(&path_str);
        return ext;
    }
    
    str_free(&path_str);
    return EMPTY_STRING;
}