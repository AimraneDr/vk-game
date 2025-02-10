#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "engine_defines.h"
#include "asset_types.h"

void asset_manager_init(AssetManager* manager);
API Asset load_asset(AssetManager* manager, const char* name);
API void release_asset(AssetManager* manager, Asset* asset);
void asset_manager_shutdown(AssetManager* manager);

#endif // ASSET_MANAGER_H