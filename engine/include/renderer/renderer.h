#ifndef RENDERER_H
#define RENDERER_H

#include "platform/platform_types.h"
#include "render_types.h"
#include "components/camera.h"
#include "components/meshRenderer.h"
#include "components/UI/ui_types.h"
#include "assets/asset_types.h"
#include "game.h"

void renderer_init(RendererInitConfig config, GameState* gState);
void renderer_draw(GameState* gState);
void renderer_shutdown(GameState* gState);

#endif //RENDERER_H