#ifndef SCRIPT_MAIN_H
#define SCRIPT_MAIN_H

#include "Types.h"

typedef struct MyPoint2D
{
    int x, y;
}MyPoint2D;

typedef struct QuadStore
{
	MyPoint2D p0, p1, p2, p3;
	int c;
}QuadStore;

void runAnimationScript(void);
void initCCBpolysFlat(void);
void initCCBPolysTexture(void);
void initBenchTextures(bool cyber);
void initCCBbuffers(void);
void initDivs(void);

#endif
