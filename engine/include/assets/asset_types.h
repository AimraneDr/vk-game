#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include "data_types.h"
#include <string/str_types.h>

#include <math/mathTypes.h>


typedef enum AssetType_e{
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MODEL,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_SOUND,
    ASSET_TYPE_FONT,
    ASSET_TYPE_UNKNOWN,
    
    
    ASSET_MAX_TYPE
}AssetType;

typedef struct Asset_t{
    AssetType type;
    void* data;
    String name;
    u32 ref_count;
}Asset;

typedef struct Glyph_t{
    u16 id;
    Vec2 uv0,uv1;
    u32 width, height;
    Vec2i offset;
    u32 advance;
}Glyph;

typedef struct Font_t{
    u32 glyph_count;
    f32 size;
    Glyph* glyphs;
    void* atlas;
}Font;

#endif //ASSET_TYPES_H