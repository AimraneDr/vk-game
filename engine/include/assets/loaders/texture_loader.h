#pragma once


#include "assets/asset_types.h"

Asset load_texture(const char* path);
void release_texture(Asset* asset);