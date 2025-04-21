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

typedef struct Asset_loader_t
{
    /// @brief type of asset this loader can load
    /// @note if type is ASSET_TYPE_NONE, the loader is null and will not be used
    AssetType type;
    bool initialized;

    /// @brief function pointers to the loader functions

    void (*init)();
    Asset (*load)(const char *path, void *data0);
    void (*unload)(Asset *asset);
    void (*shutdown)();
} AssetLoader;

typedef struct AssetManager_internal_state_t
{
    AssetLoader loaders[ASSET_MAX_TYPE];

    hashset *assets[ASSET_MAX_TYPE];
    String *loaded_names[ASSET_MAX_TYPE];
    bool asset_type_is_dirty[ASSET_MAX_TYPE];
    u32 asset_counts[ASSET_MAX_TYPE];
    u32 asset_count;
} AssetManagerInternalState;

static AssetManagerInternalState _state = {0};

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
        _state.loaders[i] = (AssetLoader){.type = ASSET_TYPE_NONE};
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

void register_loader(AssetLoaderInterface loader, AssetType type)
{
    if (type > ASSET_MAX_TYPE)
    {
        LOG_ERROR("register_loader() : trying to register an unknown asset type.");
        return;
    }
    if (_state.loaders[type].type != ASSET_TYPE_NONE)
    {
        LOG_ERROR("register_loader() : loader already registered for this type, unregister it first.00");
        return;
    }
    _state.loaders[type].init = loader.init;
    _state.loaders[type].load = loader.load;
    _state.loaders[type].unload = loader.unload;
    _state.loaders[type].shutdown = loader.shutdown;
    _state.loaders[type].initialized = false;
    _state.loaders[type].type = type;
}
void unregister_loader(AssetType type)
{
    if (type > ASSET_MAX_TYPE)
    {
        LOG_ERROR("unregister_loader() : trying to unregister an unknown asset type");
        return;
    }
    if (_state.loaders[type].type == ASSET_TYPE_NONE)
    {
        LOG_WARN("unregister_loader() : loader not registered for this type");
        return;
    }
    _state.loaders[type].init = 0;
    _state.loaders[type].load = 0;
    _state.loaders[type].unload = 0;
    _state.loaders[type].shutdown = 0;
    _state.loaders[type].initialized = false;
    _state.loaders[type].type = ASSET_TYPE_NONE;
}

void asset_manager_load_default_loaders()
{
    /*      Models      */
    AssetLoaderInterface model_loader = {
        .init = 0,
        .load = load_obj,
        .unload = release_obj,
        .shutdown = 0,
    };
    register_loader(model_loader, ASSET_TYPE_MODEL);

    /*      Textures      */
    AssetLoaderInterface texture_loader = {
        .init = 0,
        .load = load_texture,
        .unload = release_texture,
        .shutdown = 0,
    };
    register_loader(texture_loader, ASSET_TYPE_TEXTURE);

    /*      Fonts      */
    AssetLoaderInterface font_loader = {
        .init = init_font_loader,
        .load = load_font,
        .unload = release_font,
        .shutdown = shutdown_font_loader,
    };
    register_loader(font_loader, ASSET_TYPE_FONT);
}

Asset *load_asset(const char *path, const char *name)
{
    String extension = path_get_file_extension(path);
    Asset *new = malloc(sizeof(Asset));
    AssetType type = ASSET_TYPE_NONE;
    if (str_equals_val(extension, "obj"))
    {
        type = ASSET_TYPE_MODEL;

        void *temp = 0;
        if (_state.assets[type])
            hashset_get_ptr(_state.assets[type], name, &temp);
        if (temp)
        {
            release_asset(name, type);
        }
        if (!_state.loaders[type].initialized && _state.loaders[type].init)
        {
            _state.loaders[type].init();
            _state.loaders[type].initialized = true;
        }

        if (_state.loaders[type].load)
            *new = _state.loaders[type].load(path, 0);
        else
        {
            LOG_ERROR("load_asset() : failed to load asset, loader not registered");
            return 0;
        }

        if (new->type == type)
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
        type = ASSET_TYPE_TEXTURE;

        if (!_state.loaders[type].initialized && _state.loaders[type].init)
        {
            _state.loaders[type].init();
            _state.loaders[type].initialized = true;
        }

        if (_state.loaders[type].load)
            *new = _state.loaders[type].load(path, &_state.asset_counts[type]);
        else
        {
            LOG_ERROR("load_asset() : failed to load asset, loader not registered");
            return 0;
        }

        if (new->type == type)
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
    else if (str_equals_val(extension, "fnt") ||
             str_equals_val(extension, "ttf"))
    {
        type = ASSET_TYPE_FONT;

        if (!_state.loaders[type].initialized && _state.loaders[type].init)
        {
            _state.loaders[type].init();
            _state.loaders[type].initialized = true;
        }

        if (_state.loaders[type].load)
            *new = _state.loaders[type].load(path, name);
        else
        {
            LOG_ERROR("load_asset() : failed to load asset, loader not registered");
            return 0;
        }

        if (new->type == ASSET_TYPE_FONT)
        {
            if (!storeAsset(new, name))
            {
                LOG_ERROR("load_asset() : failed to store font %s", name);
                return 0;
            }
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
    if (_state.assets[type] == 0)
    {
        LOG_TRACE("get_asset() : asset not found, type is empty");
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

    if (asset->type != ASSET_TYPE_NONE || asset->type != ASSET_MAX_TYPE)
    {
        if (_state.loaders[asset->type].unload)
            _state.loaders[asset->type].unload(asset);
        else
        {
            LOG_ERROR("release_asset() : failed to unload asset, loader not registered");
        }
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

        if (_state.loaders[i].shutdown)
        {
            _state.loaders[i].shutdown();
        }
        _state.loaders[i] = (AssetLoader){.type = ASSET_TYPE_NONE};
    }
}