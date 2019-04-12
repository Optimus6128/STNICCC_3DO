#ifndef SCRIPT_MAIN_H
#define SCRIPT_MAIN_H

typedef struct MyPoint2D
{
    int x, y;
}MyPoint2D;

typedef struct QuadStore
{
	MyPoint2D p0, p1, p2, p3;
	unsigned short c;
}QuadStore;

#define INT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)

#define FP_BITS 12

void runAnimationScript(void);
void initCCBpolys(void);

#endif
