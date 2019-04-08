#ifndef SCRIPT_MAIN_H
#define SCRIPT_MAIN_H

#include "ScreenBuffer.h"
#include "InputBuffer.h"

class Script
{
public:
	static void init(ScreenBuffer *screen);
	static void run(ScreenBuffer *screen, InputBuffer *input);
	static void deinit();
};

#endif
