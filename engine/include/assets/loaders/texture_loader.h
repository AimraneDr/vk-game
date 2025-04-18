#pragma once


#include "assets/asset_types.h"

Asset load_texture(const char* path, void* index_ptr);
void release_texture(Asset* asset);