#include "core/files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string/str.h>

FileHandle *file_load(const char *path)
{
    FILE *file; 
    fopen_s(&file, path, "rb");
    
    if (!file) return 0;

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

    FileHandle *result = (FileHandle *)malloc(sizeof(FileHandle));
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
    result->cursor = 0;

    //lines count
    //max line size
    result->line_count = result->size > 0 ? 1 : 0;
    result->max_line_size = 0;
    if(result->size > 0){
        u32 current_line_size = 0;
        for(u32 i=0; i < result->size; i++, current_line_size++){
            if(content[i] == '\n'){
                if(current_line_size > result->max_line_size){
                    result->max_line_size = current_line_size;
                }
                current_line_size = 0;
                result->line_count++;
            }
        }
    }

    fclose(file);
    return result;
}


const char* file_next_line(FileHandle *file){
    if(!file) return 0;
    const char* line_ptr = &file->content[file->cursor];
    while(file->content[file->cursor++] != '\n'){
        //if the cursor reaches the end of the file go back to the start
        if(file->cursor > file->size){
            file->cursor = 0;
            //getout
            break;
        }
    }
    return line_ptr;
}

String file_line_toString(FileHandle *file){
    String content = str_new(&file->content[file->cursor]);
    //find next new line
    u32 cursor = 0;
    while(content.val[cursor] != '\n' && content.val[cursor] != '\0'){
        if(cursor > content.len)break;
        cursor++;
    }
    if(cursor == content.len-1) return content;
    String new = str_slice(content, 0,cursor);
    str_free(&content);
    return new;
}

void file_unload(FileHandle *file)
{
    if(file == 0) return;
    free(file->path);
    free(file->content);
    free(file);
}

size_t _find_last_separator(String path) {
    size_t slash_idx = str_find_char_last(path, '/');
    size_t bslash_idx = str_find_char_last(path, '\\');
    return (bslash_idx != -1 && bslash_idx > slash_idx) ? bslash_idx : slash_idx;
}

String path_get_file_name(const char* path) {
    String path_str = str_new(path);
    if (path_str.len == 0) return path_str;

    // Extract basename
    size_t sep_idx = _find_last_separator(path_str);
    size_t basename_start = (sep_idx != -1) ? sep_idx + 1 : 0;
    String basename = str_slice(path_str, basename_start, path_str.len);
    str_free(&path_str);

    // Split name and extension
    size_t dot_idx = str_find_char_last(basename, '.');
    if (dot_idx == -1) return basename; // No extension
    
    String name = str_slice(basename, 0, dot_idx);
    str_free(&basename);
    return name;
}

String path_get_file_dir(const char* path) {
    String path_str = str_new(path);
    if (path_str.len == 0) return path_str;

    // Extract directory
    size_t sep_idx = _find_last_separator(path_str);
    if (sep_idx == -1) return EMPTY_STRING; // No directory

    String dir = str_slice(path_str, 0, sep_idx + 1); // Include the separator
    str_free(&path_str);
    return dir;
}

String path_get_file_extension(const char* path) {
    String path_str = str_new(path);
    if (path_str.len == 0) return path_str;

    // Find last dot after the last separator
    size_t sep_idx = _find_last_separator(path_str);
    size_t dot_idx = str_find_char_last(path_str, '.');
    
    if (dot_idx != -1 && (sep_idx == -1 || dot_idx > sep_idx)) {
        String ext = str_slice(path_str, dot_idx + 1, path_str.len);
        str_free(&path_str);
        return ext;
    }
    
    str_free(&path_str);
    return EMPTY_STRING;
}