#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "engine_defines.h"
#include "asset_types.h"

void asset_manager_init();
API Asset* load_asset(const char* path, const char* name);
API Asset* get_asset(const char* name, AssetType type);
API void release_asset(const char* name, AssetType type);
void asset_manager_shutdown();

#endif // ASSET_MANAGER_H