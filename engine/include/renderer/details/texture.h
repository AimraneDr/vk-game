#ifndef TEXTURE_H
#define TEXTURE_H

#include "renderer/render_types.h"

void createTexture(void* pixels, Texture* out);
void destroyTexture(Texture* tex);

void createDefaultTextures(Material* default_material);
void destroyDefaultTextures(Material* default_material);
#endif //TEXTURE_H