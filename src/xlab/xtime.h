/**
 * This file belongs to the 'xlab' game engine.
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

void xTimeInit();
void xTimeUpdate();
float xTimeGetDeltaTime();
int xTimeFpsApprox();
float xTimeSecPassed();

#ifdef __cplusplus
}
#endif
