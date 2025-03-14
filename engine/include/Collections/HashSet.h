#pragma once

#include "data_types.h"
#include "engine_defines.h"

/**
 * @brief Represents a simple hashset. Members of this structure
 * should not be modified outside the functions associated with it.
 * 
 * For non-pointer types, set retains a copy of the value.For 
 * pointer types, make sure to use the _ptr setter and getter. Set
 * does not take ownership of pointers or associated memory allocations,
 * and should be managed externally.
 */
typedef struct hashset {
    u64 element_size;
    u32 element_count;
    bool is_pointer_type;
    void* memory;
} hashset;

/**
 * @brief Creates a hashset and stores it in out_hashset.
 * 
 * @param element_size The size of each element in bytes.
 * @param element_count The maximum number of elements. Cannot be resized.
 * @param is_pointer_type Indicates if this hashset will hold pointer types.
 * @param out_hashset A pointer to a hashset in which to hold relevant data.
 */
API void hashset_create(u64 element_size, u32 element_count, bool is_pointer_type, hashset* out_hashset);

/**
 * @brief Destroys the provided hashset. Does not release memory for pointer types.
 * 
 * @param set A pointer to the set to be destroyed.
 */
API void hashset_destroy(hashset* set);

/**
 * @brief Stores a copy of the data in value in the provided hashset. 
 * Only use for sets which were *NOT* created with is_pointer_type = true.
 * 
 * @param set A pointer to the set to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value The value to be set. Required.
 * @return True, or false if a null pointer is passed.
 */
API bool hashset_set(hashset* set, const char* name, void* value);

/**
 * @brief Stores a pointer as provided in value in the hashset.
 * Only use for sets which were created with is_pointer_type = true.
 * 
 * @param set A pointer to the set to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value A pointer value to be set. Can pass 0 to 'unset' an entry.
 * @return True; or false if a null pointer is passed or if the entry is 0.
 */
API bool hashset_set_ptr(hashset* set, const char* name, void** value);

/**
 * @brief Obtains a copy of data present in the hashset.
 * Only use for sets which were *NOT* created with is_pointer_type = true.
 * 
 * @param set A pointer to the set to retrieved from. Required.
 * @param name The name of the entry to retrieved. Required.
 * @param value A pointer to store the retrieved value. Required.
 * @return True; or false if a null pointer is passed.
 */
API bool hashset_get(hashset* set, const char* name, void* out_value);

/**
 * @brief Obtains a pointer to data present in the hashset.
 * Only use for sets which were created with is_pointer_type = true.
 * 
 * @param set A pointer to the set to retrieved from. Required.
 * @param name The name of the entry to retrieved. Required.
 * @param value A pointer to store the retrieved value. Required.
 * @return True if retrieved successfully; false if a null pointer is passed or is the retrieved value is 0.
 */
API bool hashset_get_ptr(hashset* set, const char* name, void** out_value);

/**
 * @brief Fills all entries in the hashset with the given value.
 * Useful when non-existent names should return some default value.
 * Should not be used with pointer set types.
 * 
 * @param set A pointer to the set filled. Required.
 * @param value The value to be filled with. Required.
 * @return True if successful; otherwise false.
 */
API bool hashset_fill(hashset* set, void* value);