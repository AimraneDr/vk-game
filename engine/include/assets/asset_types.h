#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include "data_types.h"
#include <string/str_types.h>

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
}Asset;

typedef struct AssetManager_t {
    Asset* assets[ASSET_MAX_TYPE];
    u32 asset_count;
} AssetManager;

#endif //ASSET_TYPES_H