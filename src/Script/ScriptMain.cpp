#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>
#include <Windows.h>

#include "ScriptMain.h"
#include "..\Data\scene1.h"

#include "Modules/Drawing/DrawingPolygon.h"


static int currentFrame = -1;
static int nextFrame = 0;
static int block64index = 0;

static uchar *data = scene1_bin;

void Script::init(ScreenBuffer *screen)
{
}

static void inputScript(InputBuffer *input)
{
	input->quit = (input->keyState(SDLK_ESCAPE) == KEY_JUST_PRESSED);

	if (input->keyState(SDLK_n) == KEY_JUST_PRESSED) {
		++nextFrame;
	}
}

void interpretClearScreen()
{
	std::cout << "Must clear screen\n\n";
}

void interpretPaletteData()
{
	std::cout << "Must switch palette\n";

	ushort bitmask = *((ushort*)data);
	data += 2;

	for (int i = 0; i < 16; ++i) {
		int palNum = 15 - i;
		if (bitmask & 1) {
			ushort color = *((ushort*)data);
			data += 2;

			int r = (color >> 8) & 7;
			int g = (color >> 4) & 7;
			int b = color & 7;
			std::cout << "\tSet color " << palNum << " with values " << "(R: " << r << " , G: " << g << " , B: " << b << ")\n";
		}
		bitmask >>= 1;
	}
	std::cout << std::endl;
}

void interpretIndexedMode()
{
	static uchar vpx[256], vpy[256];

	std::cout << "Frame in indexed mode\n\n";

	int vertexNum = *data++;
	std::cout << "Number of vertices: " << vertexNum << "\n\n";

	for (int i = 0; i < vertexNum; ++i) {
		vpx[i] = *data++;
		vpy[i] = *data++;
		std::cout << "Vertex " << i << ": X=" << (int)vpx[i] << " Y=" << (int)vpy[i] << std::endl;
	}
}

void interpretNonIndexedMode()
{
	std::cout << "Frame in non-indexed mode\n\n";
}

void interpretFlag()
{
	uchar flags = *data++;

	if (flags & 1) {
		interpretClearScreen();
	}
	if (flags & 2) {
		interpretPaletteData();
	}
	if (flags & 4) {
		interpretIndexedMode();
	} else {
		interpretNonIndexedMode();
	}
}

void decodeFrame()
{
	interpretFlag();
}

static void renderScript(ScreenBuffer *screen)
{
	memset(screen->vram, 0, screen->width * screen->height * (screen->bpp >> 3));


	if (nextFrame > currentFrame) {
		currentFrame = nextFrame;
		std::cout << "\n\n\nFrame " << currentFrame << "\n==========\n\n";
		decodeFrame();
	}

	const uchar *src = &scene1_bin[0];
	uchar *dst = screen->vram;
	for (uint i = 0; i < SCENE1_BIN_SIZE; ++i) {
		*dst++ = *src++;
	}
}

void Script::run(ScreenBuffer *screen, InputBuffer *input)
{
	inputScript(input);

	renderScript(screen);
}

void Script::deinit()
{
}
