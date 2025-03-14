#include "core/memsys.h"

#include <stdlib.h>
#include <string.h>
#include "core/debugger.h"
typedef struct MemHeader_t
{
    MemType type;
    u32 size;
} MemHeader;
typedef struct MemoryState_t
{
    u64 alloc_count;
    u64 total_alloc;
    u64 types_alloc[MAX_MEM_TYPE];
} MemoryState;

static MemoryState *_state = {0};

void memsys_init()
{
    if (_state != 0)
    {
        LOG_WARN("memory system already initialized");
        return;
    }

    _state = malloc(sizeof(MemoryState));
}
void memsys_shutdown()
{
    if (_state == 0)
    {
        LOG_WARN("memory system already shutdown");
        return;
    }
    _state = 0;
}

void *memsys_alloc(u32 size, MemType type)
{
    if (size <= 0 && type > MAX_MEM_TYPE)
        return 0;
    MemHeader *header = (MemHeader *)malloc(sizeof(MemHeader) + size);
    if (!header)
        return 0;
    header->type = type;
    header->size = size;

    _state->types_alloc[type] += size;
    _state->total_alloc += size;
    _state->alloc_count++;
    return (void *)((char*)header + 1);
}
void memsys_free(void *mem, MemType type)
{
    if (!mem)
        return;
    MemHeader *header = (MemHeader *)((char*)mem - 1);
    _state->total_alloc -= header->size;
    _state->types_alloc[header->type] -= header->size;
    _state->alloc_count--;
    free(header);
}

void memsys_copy(void *dst, const void *src, u64 buffer_size)
{
    memcpy(dst, src, buffer_size);
}
void memsys_zero(void *mem, u64 buffer_size){
    memset(mem, 0, buffer_size);
}
void memsys_set(void *mem, i32 val, u64 buffer_size){
    memset(mem, val, buffer_size);
}