#pragma once

#include <data_types.h>

typedef enum MemoryType_e{
    MEM_TYPE_UNKNOWN,

    MEM_TYPE_HASHSET,
    
    MEM_TYPE_ECS,

    MEM_TYPE_TEXTURE,
    MEM_TYPE_GEOMETRY,
    MEM_TYPE_AUDIO,
    
    MEM_TYPE_STRING,

    MAX_MEM_TYPE
}MemType;

void memsys_init();
void memsys_shutdown();

void* memsys_alloc(u32 size, MemType type);
void memsys_free(void* mem, MemType type);

void memsys_copy(void* dst, const void* src, u64 buffer_size);
void memsys_zero(void* mem, u64 buffer_size);
void memsys_set(void* mem, i32 val, u64 buffer_size);