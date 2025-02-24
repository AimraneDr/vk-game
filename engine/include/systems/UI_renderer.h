#ifndef SYSTEM_UI_RENDERER_H
#define SYSTEM_UI_RENDERER_H

#include "renderer/render_types.h"
#include "ecs/ecs_types.h"
#include "components/camera.h"

System UI_renderer_get_system_ref(Scene* scene, Renderer* r, Camera* camera);

#endif //SYSTEM_UI_RENDERER_H