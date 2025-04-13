#include "assets/asset_manager.h"

#include "assets/loaders/model_loader.h"
#include "assets/loaders/texture_loader.h"
#include "assets/loaders/font_loader.h"
#include "core/files.h"
#include "core/debugger.h"
#include "meshTypes.h"

#include <stdlib.h>
#include <collections/DynamicArray.h>
#include <Collections/HashSet.h>
#include <string/str.h>

// temp
#include "renderer/render_types.h"
#include <math/vec2.h>

#define DEFAULT_ASSETS_PATH "./resources/"

typedef struct Assets_internal_state_t
{
    hashset *assets[ASSET_MAX_TYPE];
    String *loaded_names[ASSET_MAX_TYPE];
    bool asset_type_is_dirty[ASSET_MAX_TYPE];
    u32 asset_counts[ASSET_MAX_TYPE];
    u32 asset_count;
} AssetsInternalState;

static AssetsInternalState _state = {0};

u32 assets_get_count(AssetType type)
{
    return _state.asset_counts[type];
}
u32 assets_get_total_count()
{
    return _state.asset_count;
}
bool assets_is_type_dirty(AssetType type)
{
    return _state.asset_type_is_dirty[type];
}

void asset_manager_init()
{
    _state.asset_count = 0;
    for (u8 i = 0; i < ASSET_MAX_TYPE; i++)
    {
        _state.assets[i] = 0;
        _state.loaded_names[i] = 0;
        _state.asset_type_is_dirty[i] = false;
        _state.asset_counts[i] = 0;
    }
}

bool storeAsset(Asset *asset, const char *name)
{
    if (_state.assets[asset->type] == 0)
    {
        _state.assets[asset->type] = malloc(sizeof(hashset));
        hashset_create(sizeof(Asset), 255, true, _state.assets[asset->type]);
    }
    if (!hashset_set_ptr(_state.assets[asset->type], name, (void **)&asset))
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
    String extension = path_get_file_extension(path);
    Asset *new = malloc(sizeof(Asset));
    if (str_equals_val(extension, "obj"))
    {
        void *temp = 0;
        if (_state.assets[ASSET_TYPE_MODEL])
            hashset_get_ptr(_state.assets[ASSET_TYPE_MODEL], name, &temp);
        if (temp)
        {
            release_asset(name, ASSET_TYPE_MODEL);
        }
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
        str_equals_val(extension, "jpeg") ||
        str_equals_val(extension, "jpg") ||
        str_equals_val(extension, "png"))
    {
        *new = load_texture(path, _state.asset_counts[ASSET_TYPE_TEXTURE]);
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
    else if (str_equals_val(extension, "fnt"))
    {
        String pngFile = str_join(str_join(path_get_file_dir(path), path_get_file_name(path)), str_new(".png"));
        Asset *atlas = load_asset((const char *)pngFile.val, str_join(str_new(name), str_new("_atlas")).val);
        *new = load_font_bitmap(path, vec2_new(((Texture *)atlas->data)->width, ((Texture *)atlas->data)->height), atlas->data);
        if (new->type == ASSET_TYPE_FONT)
        {
            // load the font atlas
            if (!atlas)
            {
                LOG_ERROR("load_asset() : failed to load font atlas %s", pngFile.val);
                return 0;
            }
            if (!storeAsset(new, name))
            {
                LOG_ERROR("load_asset() : failed to store font %s", name);
                return 0;
            }
        }
    }
    else if (str_equals_val(extension, "ttf"))
    {
        *new = load_font_ttf(path, vec2_zero(), 0);
        if (new->type != ASSET_TYPE_FONT)
        {
            LOG_ERROR("load_asset() : failed to load font (%s).", path);
            return 0;
        }
        if (!storeAsset(new, name))
        {
            LOG_ERROR("load_asset() : failed to store font %s", name);
            return 0;
        }
    }
    else
    {
        LOG_ERROR("load_asset() : Unknown/unsupported asset type (%s)", extension.val);
        str_free(&extension);
        return 0;
    }
    str_free(&extension);
    Asset *out = 0;
    hashset_get_ptr(_state.assets[new->type], name, (void **)&out);
    if (out)
    {
        _state.asset_type_is_dirty[new->type] = true;
        _state.asset_counts[new->type]++;
        _state.asset_count++;
    }
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
    hashset_get_ptr(_state.assets[type], name, (void **)&asset);
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
    Asset *asset = 0;
    hashset_get_ptr(_state.assets[type], name, (void **)&asset);

    if (!asset)
    {
        LOG_ERROR("release_asset() : asset not found");
        return;
    }

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
    _state.asset_type_is_dirty[type] = true;
    _state.asset_counts[type]--;
    _state.asset_count--;
    for (u32 i = 0; i < DynamicArray_Length(_state.loaded_names[type]); i++)
    {
        String *namesList = _state.loaded_names[type];
        if (str_equals_val(namesList[i], name))
        {
            DynamicArray_PopAt(_state.loaded_names[type], i, 0);
            break;
        }
    }
    free(asset);
}

void asset_manager_shutdown()
{
    for (u8 i = ASSET_MAX_TYPE - 1; i >= 0 && i < ASSET_MAX_TYPE; i--)
    {
        if (_state.loaded_names[i] != 0)
        {
            while (DynamicArray_Length(_state.loaded_names[i]) > 0)
            {
                String name;
                DynamicArray_Pop(_state.loaded_names[i], &name);
                Asset *temp = 0;
                if (hashset_get_ptr(_state.assets[i], name.val, (void **)&temp))
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