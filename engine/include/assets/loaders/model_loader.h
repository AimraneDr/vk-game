#pragma once

#include "assets/asset_types.h"

Asset load_obj(const char* path);
void release_obj(Asset* asset);