#pragma once

#include "xconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

int xRunning();
void xExit();

/* main function, defined by user, ran through real main */
extern int xMain();

#ifdef __cplusplus
}
#endif
