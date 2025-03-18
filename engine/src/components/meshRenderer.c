#include "components/meshRenderer.h"

#include "assets/asset_manager.h"

#include <math/mat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

void createMeshRenderer(const char* asset_name, MeshRenderer* out){
    //aquire mesh data & material data
    Asset* asset = get_asset(asset_name, ASSET_TYPE_MODEL);
    asset->ref_count++;
    out->data = asset->data;
    out->material = malloc(sizeof(Material));
    out->material->albedoFactor = vec4_one();
    out->material->aoFactor = 1;
    out->material->emissiveFactor = vec4_one();
    out->material->heightScale = 1;
    out->material->metallicFactor = 1;
    out->material->roughnessFactor = 1;
    out->material->uvOffset = vec2_new(1,1);
    out->material->uvTiling = vec2_new(1,1);
}

void destroyMeshRenderer(MeshRenderer* mesh){
    //unreference mesh data & material data
    mesh->data = 0;
    free(mesh->material);
    mesh->material = 0;
}