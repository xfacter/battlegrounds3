/**
 * This file belongs to the 'xlib' game engine.
 * Copyright 2009 xfacter
 * Copyright 2016 wickles
 * This work is licensed under the LGPLv3
 * subject to all terms as reproduced in the included LICENSE file.
 */

#pragma once

#include "xconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

void xLogPrintf(char* text, ... );

#define X_LOG xLogPrintf

#ifdef __cplusplus
}
#endif
