#ifndef RENDERER_H
#define RENDERER_H

#include "platform/platform_types.h"
#include "render_types.h"
#include "components/cameraComponent.h"
#include "assets/asset_types.h"

void renderer_init(Renderer* r, PlatformState* p, AssetManager* assetsM);
void renderer_draw(Camera_Component* camera, Renderer* r, PlatformState* p, AssetManager* assetsM, f64 deltatime);
void renderer_shutdown(Renderer* r);


#endif //RENDERER_H