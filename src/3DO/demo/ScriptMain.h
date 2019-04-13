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
	unsigned short c;
}QuadStore;


#define FP_BITS 12

#define INT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)


void runAnimationScript(bool gpuOn);
void initCCBpolys(void);

#endif
