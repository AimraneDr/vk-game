#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "engine_defines.h"
#include "asset_types.h"

void asset_manager_init();
API u32 assets_get_count(AssetType type);
API u32 assets_get_total_count();
API Asset* load_asset(const char* path, const char* name);
API Asset* get_asset(const char* name, AssetType type);
API void register_loader(AssetLoaderInterface loader, AssetType type);
API void unregister_loader(AssetType type);
API void asset_manager_load_default_loaders();
API void asset_manager_load_default_assets();
API void release_asset(const char* name, AssetType type);
void asset_manager_shutdown();

#endif // ASSET_MANAGER_H