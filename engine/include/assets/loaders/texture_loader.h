#pragma once


#include "assets/asset_types.h"

Asset load_texture(const char* path, u32 index);
void release_texture(Asset* asset);