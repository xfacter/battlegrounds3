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

typedef struct xObj {
	int type;
	int vtype;
	ScePspFVector3 bbox[8];
	ScePspFVector3 scale;
	ScePspFVector3 pos;
    int num_verts;
	void* vertices;
} xObj;

xObj* xObjLoad(char* filename, int optimize);

void xObjFree(xObj* object);

void xObjTranslate(xObj* object);

void xObjDraw(xObj* object, int reverse_frontface);

#ifdef __cplusplus
}
#endif
