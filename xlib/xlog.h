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
