#ifndef DETAILS_H
#define DETAILS_H

#include "data_types.h"

bool isValidationLayersEnabled();
const char* const* validationLayersNames();
const u32 validationLayersCount();

const char** deviceExtensionsNames();
const u32 deviceExtensionsCount();

#endif