#include "components/meshRenderer.h"

#include "assets/asset_manager.h"

#include <math/mat.h>

void createMeshRenderer(const char* asset_name, MeshRenderer* out){
    //aquire mesh data & material data
    Asset* asset = get_asset(asset_name, ASSET_TYPE_MODEL);
    asset->ref_count++;
    out->data = asset->data;
}

void destroyMeshRenderer(MeshRenderer* mesh){
    //unreference mesh data & material data
    mesh->data = 0;
    mesh->material = (Material){0};
}