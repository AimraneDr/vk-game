#include "core/files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        return NULL;
    }

    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';

    File *result = (File *)malloc(sizeof(File));
    if (!result)
    {
        free(content);
        fclose(file);
        return NULL;
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
    free(file->path);
    free(file->content);
    free(file);
}