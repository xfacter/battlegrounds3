#ifndef __X_LOG_H__
#define __X_LOG_H__

#include "xconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

void xLogPrint(char* text);

void xLogPrintf(char* text, ... );

#ifdef __cplusplus
}
#endif

#endif
