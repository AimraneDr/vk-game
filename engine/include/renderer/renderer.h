#ifndef RENDERER_H
#define RENDERER_H

#include "platform/platform_types.h"
#include "render_types.h"
#include "components/camera.h"
#include "components/meshRenderer.h"
#include "components/UI/ui_types.h"
#include "assets/asset_types.h"

void renderer_init(RendererInitConfig config, Renderer* r, PlatformState* p, Scene* s);
void renderer_draw(
    Camera* camera, 
    Renderer* r, PlatformState* p, 
    Scene* scene,
    f64 deltatime,
    UI_Manager* uiManager
);
void renderer_shutdown(Renderer* r, Scene* s);


#endif //RENDERER_H