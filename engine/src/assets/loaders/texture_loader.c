#include "assets/loaders/texture_loader.h"

#include "renderer/details/texture.h"
#include <math/mathUtils.h>
#include "core/memsys.h"
#include "core/debugger.h"

#include <string/str.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Asset load_texture(const char* path, void* index_ptr){
    u32 index = *(u32*)index_ptr;
    i32 texWidth, texHeight, texChannels;
    // name = "./../resources/textures/viking_room.png";
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        LOG_ERROR("failed to load texture image!");
        return (Asset){0};
    }
        
    Texture* tex = memsys_alloc(sizeof(Texture), MEM_TYPE_TEXTURE);
    tex->Idx = index;
    tex->width = texWidth;
    tex->height = texHeight;
    tex->mipLevels = (u16)FLOOR(log2(MAX(texWidth, texHeight))) + 1;
    
    createTexture(pixels, tex);
    stbi_image_free(pixels);
    
    Asset new = {0};
    new.data = tex;
    new.ref_count = 0;
    new.type = ASSET_TYPE_TEXTURE;
    new.name = str_new(path);
    return new;   
}

void release_texture(Asset* asset){
    if (!asset || asset->type != ASSET_TYPE_TEXTURE || !asset->data) {
        LOG_ERROR("release_texture() : trying to release a non valid asset or non texture asset");
        return;
    }

    destroyTexture((Texture*)asset->data);

    memsys_free(asset->data, MEM_TYPE_TEXTURE);
    str_free(&asset->name);
}