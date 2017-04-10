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
