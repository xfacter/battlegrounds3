#ifndef __X_OBJ_H__
#define __X_OBJ_H__

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

typedef struct xObjLOD {
	u8 levels;
	xObj** objects;
} xObjLOD;

xObj* xObjLoad(char* filename, int optimize);

void xObjFree(xObj* object);

void xObjTranslate(xObj* object);

void xObjDraw(xObj* object, int reverse_frontface);

void xObjSetupLOD(xObjLOD* l, u8 levels);

void xObjFreeLOD(xObjLOD* l);

void xObjDrawLOD(xObjLOD* l, float start, float spacing, int reverse_frontface);

#ifdef __cplusplus
}
#endif

#endif
