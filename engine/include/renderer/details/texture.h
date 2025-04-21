#ifndef TEXTURE_H
#define TEXTURE_H

#include "renderer/render_types.h"

void createTexture(void* pixels, Texture* out);
void destroyTexture(Texture* tex);

void createDefaultTextures(Material* default_material);
void destroyDefaultTextures(Material* default_material);

void fillTextureRegion(Texture* texture, void* pixels, u32 x, u32 y, u32 width, u32 height);
#endif //TEXTURE_H