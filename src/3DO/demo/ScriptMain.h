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

void runAnimationScript(void);
void initCCBpolys(void);

#endif
