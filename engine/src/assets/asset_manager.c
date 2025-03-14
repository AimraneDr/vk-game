#include "assets/asset_manager.h"

#include "assets/loaders/model_loader.h"
#include "assets/loaders/texture_loader.h"
#include "core/files.h"
#include "core/debugger.h"
#include "meshTypes.h"

#include <stdlib.h>
#include <collections/DynamicArray.h>
#include <Collections/HashSet.h>
#include <string/str.h>

#define DEFAULT_ASSETS_PATH "./resources/"

typedef struct Assets_internal_state_t
{
    hashset *assets[ASSET_MAX_TYPE];
    String *loaded_names[ASSET_MAX_TYPE];
    u32 asset_count;
} AssetsInternalState;

static AssetsInternalState _state = {0};

void asset_manager_init()
{
    _state.asset_count = 0;
    for (u8 i = 0; i < ASSET_MAX_TYPE; i++)
    {
        _state.assets[i] = 0;
        _state.loaded_names[i] = 0;
    }
}

bool storeAsset(Asset *asset, const char *name)
{
    if (_state.assets[asset->type] == 0)
    {
        _state.assets[asset->type] = malloc(sizeof(hashset));
        hashset_create(sizeof(Asset), 255, true, _state.assets[asset->type]);
    }
    if (!hashset_set_ptr(_state.assets[asset->type], name, (void**)&asset))
    {
        LOG_ERROR("Failed to store asset");
        return false;
    }
    if (_state.loaded_names[asset->type] == 0)
    {
        _state.loaded_names[asset->type] = DynamicArray_Create(String);
    }
    DynamicArray_Push(_state.loaded_names[asset->type], str_new(name));
    return true;
}

Asset *load_asset(const char *path, const char *name)
{
    String extension = file_extension(path);
    Asset *new = malloc(sizeof(Asset));
    if (str_compare_val(extension, "obj") == 0)
    {
        *new = load_obj(path);
        if (new->type == ASSET_TYPE_MODEL)
        {
            if (!storeAsset(new, name))
            {
                return 0;
            }
        }
        else
        {
            LOG_ERROR("load_asset() : failed to load asset");
            return 0;
        }
    }
    else if (
        str_compare_val(extension, "jpeg") == 0 ||
        str_compare_val(extension, "jpg") == 0 ||
        str_compare_val(extension, "png") == 0)
    {
        *new = load_texture(path);
        if (new->type == ASSET_TYPE_TEXTURE)
        {
            if (!storeAsset(new, name))
            {
                return 0;
            }
        }
        else
        {
            LOG_ERROR("load_asset() : failed to load asset");
            return 0;
        }
    }
    Asset *out;
    hashset_get_ptr(_state.assets[new->type], name, (void**)&out);
    return out;
}

Asset *get_asset(const char *name, AssetType type)
{
    if (type > ASSET_MAX_TYPE)
    {
        LOG_ERROR("get_asset() : trying to get an unknown asset type");
        return 0;
    }
    if (!name || string_len(name) == 0)
    {
        LOG_ERROR("get_asset() : name is required to be non null and has a length greater than 0");
        return 0;
    }
    Asset *asset;
    hashset_get_ptr(_state.assets[type], name, (void**)&asset);
    return asset;
}

void release_asset(const char *name, AssetType type)
{
    if (type > ASSET_MAX_TYPE)
    {
        LOG_ERROR("release_asset() : trying to release an unknown asset type");
        return;
    }
    if (!name || string_len(name) == 0)
    {
        LOG_ERROR("release_asset() : name is required to be non null and has a length greater than 0");
        return;
    }
    Asset *asset;
    hashset_get_ptr(_state.assets[type], name, (void**)&asset);

    switch (asset->type)
    {
    case ASSET_TYPE_MODEL:
        release_obj(asset);
        break;
    case ASSET_TYPE_TEXTURE:
        release_texture(asset);
        break;
    default:
        break;
    }
    hashset_set_ptr(_state.assets[type], name, 0);
    free(asset);
}

void asset_manager_shutdown()
{
    for (u8 i = 0; i < ASSET_MAX_TYPE; i++)
    {
        if (_state.loaded_names[i] != 0)
        {
            while (DynamicArray_Length(_state.loaded_names[i]) > 0)
            {
                String name;
                DynamicArray_Pop(_state.loaded_names[i], &name);
                Asset *temp = 0;
                if (hashset_get_ptr(_state.assets[i], name.val, (void**)&temp))
                {
                    release_asset(name.val, i);
                }
                str_free(&name);
            }
            DynamicArray_Destroy(_state.loaded_names[i]);
        }
        if (_state.assets[i] != 0)
        {
            hashset *set = _state.assets[i];
            hashset_destroy(set);
        }
    }
}