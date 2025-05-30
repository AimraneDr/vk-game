#ifndef GAME_H
#define GAME_H

#include "engine_defines.h"
#include "data_types.h"
#include "platform/platform_types.h"
#include "renderer/render_types.h"
#include "core/input.h"
#include "core/clock.h"
#include "components/meshRenderer.h"
#include "components/camera.h"
#include "assets/asset_types.h"
#include <string/str_types.h>
#include "ecs/ecs_types.h"

typedef struct GameConfig{
    PlatformInitConfig platform;
    
    struct {
        Vec3 pos,rot,scale;
        f32 orthographicSize;
        f32 fieldOfView;
        f32 farPlane;
        f32 nearPlane;
        bool useOrthographic;
    }camera;
    
    RendererInitConfig renderer;

    bool suspendOnMinimize;

    u32 targetFPS;
}GameConfig;

typedef struct GameState_t{
    //systems
    
    PlatformState platform;
    InputManager inputer;
    Renderer renderer;
    Clock clock;
    
    //components
    
    Camera camera;
    Scene scene;
    
    //state
    
    String name;
    bool suspended;

    f64 targetFrameTime;
}GameState;

typedef struct GameInterface_t{
    /// @brief provide costume configuration for the application
    /// @param config ref to the application config struct
    void (*config)(GameConfig* config);

    /// @brief startup logic
    void (*start)(GameState* state);

    /// @brief update logic
    void (*update)(GameState* state);
    
    /// @brief cleanup logic
    void (*cleanup)(GameState* state);
}GameInterface;

API void game_run(GameInterface Interface);

#endif //GAME_H