#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include "assets/asset_types.h"

Asset load_obj(const char* path);
void release_obj(Asset* asset);

#endif //MODEL_LOADER_H