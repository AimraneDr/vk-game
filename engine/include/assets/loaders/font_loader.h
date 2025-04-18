#pragma once

#include "assets/asset_types.h"

void init_font_loader();
void shutdown_font_loader();

Asset load_font(const char* path, void* name);
void release_font(Asset* font);

GlyphSet* get_glyphset(Font* font, u32 size);

void load_char(Font* tf, char c, u32 size);