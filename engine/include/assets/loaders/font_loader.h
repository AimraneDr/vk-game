#pragma once

#include "assets/asset_types.h"

Asset load_font_bitmap(const char* path, Vec2 atlas_size, void* atlas);
Asset load_font_ttf(const char* path, Vec2 atlas_size, void* atlas);
void release_font(Asset* font);