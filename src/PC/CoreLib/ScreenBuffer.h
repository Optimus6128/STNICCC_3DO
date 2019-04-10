#ifndef SCREEN_BUFFER_H
#define SCREEN_BUFFER_H

#include "Typedefs.h"

#define DEFAULT_SCREEN_WIDTH 640
#define DEFAULT_SCREEN_HEIGHT 480
#define DEFAULT_SCREEN_BPP 32

struct ScreenBuffer
{
	uint width, height;
	uint bpp;
	unsigned char *vram;
};

#endif
