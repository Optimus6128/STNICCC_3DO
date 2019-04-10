#ifndef SCRIPT_MAIN_H
#define SCRIPT_MAIN_H

#include "ScreenBuffer.h"
#include "InputBuffer.h"
#include "Modules/Drawing/Drawing.h"

struct QuadStore
{
	Point2D p0, p1, p2, p3;
	ushort c;
};

struct vec2i
{
	int x, y;
};

class Script
{
public:
	static void init(ScreenBuffer *screen);
	static void run(ScreenBuffer *screen, InputBuffer *input);
	static void deinit();
};

#endif
