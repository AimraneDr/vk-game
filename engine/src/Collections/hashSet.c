#include "Collections/HashSet.h"

#include "core/memsys.h"
#include "core/debugger.h"

u64 hash_name(const char* name, u32 element_count) {
    // A multipler to use when generating a hash. Prime to hopefully avoid collisions.
    static const u64 multiplier = 97;

    unsigned const char* us;
    u64 hash = 0;

    for (us = (unsigned const char*)name; *us; us++) {
        hash = hash * multiplier + *us;
    }

    // Mod it against the size of the set.
    hash %= element_count;

    return hash;
}

void hashset_create(u64 element_size, u32 element_count, bool is_pointer_type, hashset* out_hashset) {
    if (!out_hashset) {
        LOG_ERROR("hashset_create failed! Pointer to out_hashset are required.");
        return;
    }
    if (!element_count || !element_size) {
        LOG_ERROR("element_size and element_count must be a positive non-zero value.");
        return;
    }

    // TODO: Might want to require an allocator and allocate this memory instead.
    out_hashset->memory = memsys_alloc(element_count * element_count, MEM_TYPE_HASHSET);
    out_hashset->element_count = element_count;
    out_hashset->element_size = element_size;
    out_hashset->is_pointer_type = is_pointer_type;
    memsys_zero(out_hashset->memory, element_size * element_count);
}

void hashset_destroy(hashset* set) {
    if (set) {
        // TODO: If using allocator above, free memory here.
        memsys_free(set->memory, MEM_TYPE_HASHSET);
        memsys_zero(set, sizeof(hashset));
    }
}

bool hashset_set(hashset* set, const char* name, void* value) {
    if (!set || !name || !value) {
        LOG_ERROR("hashset_set requires set, name and value to exist.");
        return false;
    }
    if (set->is_pointer_type) {
        LOG_ERROR("hashset_set should not be used with sets that have pointer types. Use hashset_set_ptr instead.");
        return false;
    }

    u64 hash = hash_name(name, set->element_count);
    memsys_copy(set->memory + (set->element_size * hash), value, set->element_size);
    return true;
}

bool hashset_set_ptr(hashset* set, const char* name, void** value) {
    if (!set || !name) {
        LOG_WARN("hashset_set_ptr requires set and name  to exist.");
        return false;
    }
    if (!set->is_pointer_type) {
        LOG_ERROR("hashset_set_ptr should not be used with sets that do not have pointer types. Use hashset_set instead.");
        return false;
    }

    u64 hash = hash_name(name, set->element_count);
    ((void**)set->memory)[hash] = value ? *value : 0;
    return true;
}

bool hashset_get(hashset* set, const char* name, void* out_value) {
    if (!set || !name || !out_value) {
        LOG_WARN("hashset_get requires set, name and out_value to exist.");
        return false;
    }
    if (set->is_pointer_type) {
        LOG_ERROR("hashset_get should not be used with sets that have pointer types. Use hashset_set_ptr instead.");
        return false;
    }
    u64 hash = hash_name(name, set->element_count);
    memsys_copy(out_value, set->memory + (set->element_size * hash), set->element_size);
    return true;
}

bool hashset_get_ptr(hashset* set, const char* name, void** out_value) {
    if (!set || !name || !out_value) {
        LOG_WARN("hashset_get_ptr requires set, name and out_value to exist.");
        return false;
    }
    if (!set->is_pointer_type) {
        LOG_ERROR("hashset_get_ptr should not be used with sets that do not have pointer types. Use hashset_get instead.");
        return false;
    }

    u64 hash = hash_name(name, set->element_count);
    *out_value = ((void**)set->memory)[hash];
    return *out_value != 0;
}

bool hashset_fill(hashset* set, void* value) {
    if (!set || !value) {
        LOG_WARN("hashset_fill requires set and value to exist.");
        return false;
    }
    if (set->is_pointer_type) {
        LOG_ERROR("hashset_fill should not be used with sets that have pointer types.");
        return false;
    }

    for (u32 i = 0; i < set->element_count; ++i) {
        memsys_copy(set->memory + (set->element_size * i), value, set->element_size);
    }

    return true;
}