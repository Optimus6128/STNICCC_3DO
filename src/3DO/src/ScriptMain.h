#ifndef SCRIPT_MAIN_H
#define SCRIPT_MAIN_H

#include "Types.h"

#define ANIM_WIDTH 256
#define ANIM_HEIGHT 200
#define ANIM_SIZE (ANIM_WIDTH * ANIM_HEIGHT)


typedef struct MyPoint2D
{
    int x, y;
}MyPoint2D;

typedef struct QuadStore
{
	MyPoint2D p0, p1, p2, p3;
	int c;
}QuadStore;

void runAnimationScript(int ticks);
void initCCBpolysFlat(void);
void initCCBPolysTexture(void);
void initBenchTextures(bool cyber);
void initCCBbuffers(void);
void initDivs(void);
void initTest3D(void);

#endif
