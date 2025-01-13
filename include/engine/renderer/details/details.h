#ifndef DETAILS_H
#define DETAILS_H

#include "engine/data_types.h"

bool isValidationLayersEnabled();
char* const* validationLayersNames();
const u32 validationLayersCount();

char* const* deviceExtensionsNames();
const u32 deviceExtensionsCount();

#endif