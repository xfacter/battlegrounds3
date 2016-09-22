#ifndef __X_LOG_H__
#define __X_LOG_H__

#include "xconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

void xLogPrintf(char* text, ... );

#define X_LOG xLogPrintf

#ifdef __cplusplus
}
#endif

#endif
