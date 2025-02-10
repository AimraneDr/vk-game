#include "assets/asset_manager.h"

#include "assets/details/model_loader.h"
#include <collections/DynamicArray.h>
#include <string/str.h>
#include "core/files.h"

void* createAssetTypeArray(void){
    Asset* out = DynamicArray_Create(Asset);
    return out;
}

void destroyAssetTypeArray(Asset* arr){
    DynamicArray_Destroy(arr);
}


void asset_manager_init(AssetManager* manager){
    manager->asset_count = 0;
    for(u8 i=0; i< ASSET_MAX_TYPE; i++){
        manager->assets[i] = 0;
    }
}

Asset load_asset(AssetManager* manager, const char* name){
    String extension = file_extension(name);
    Asset new;
    if(str_compare_val(extension, "obj") == 0){
        new = load_obj(name);
        if(new.type == ASSET_TYPE_MODEL){
            if(manager->assets[ASSET_TYPE_MODEL] == 0)
                manager->assets[ASSET_TYPE_MODEL] = createAssetTypeArray();
            DynamicArray_Push(manager->assets[ASSET_TYPE_MODEL], new);
        }
    }
    return new;
}

void release_asset(AssetManager* manager, Asset* asset){
    switch (asset->type)
    {
    case ASSET_TYPE_MODEL:
    // remove from darray
        release_obj(asset);
        break;
    default:
        break;
    }
    release_obj(asset);
}

void asset_manager_shutdown(AssetManager* manager){
    for(u8 i=0; i< ASSET_MAX_TYPE; i++){
        if(manager->assets[i] != 0){
            u32 len = DynamicArray_Length(manager->assets[i]);
            if(len > 0){
                for(u32 j=0; j< len; j++){
                    release_asset(manager, &manager->assets[i][j]);
                }
            }
            destroyAssetTypeArray(manager->assets[i]);
        }
    }
}