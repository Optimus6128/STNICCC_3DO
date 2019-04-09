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
static uint block64index = 0;

static uchar *data = scene1_bin;

static ushort pal[16];
static Point2D pt[16];

bool endOfAllFrames = false;


void Script::init(ScreenBuffer *screen)
{
}

static void inputScript(InputBuffer *input)
{
	input->quit = (input->keyState(SDLK_ESCAPE) == KEY_JUST_PRESSED);

	/*if (input->keyState(SDLK_n) == KEY_JUST_PRESSED) {
		++nextFrame;
	}*/
}

static void interpretClearScreen()
{
	//std::cout << "Must clear screen\n\n";
}

static inline ushort flipWordEndianess(ushort value)
{
	uchar vl = value >> 8;
	uchar vr = value & 255;
	return (vr << 8) | vl;
}

static void interpretPaletteData()
{
	//std::cout << "Must switch palette\n";

	ushort bitmask = flipWordEndianess(*((ushort*)data));
	data += 2;

	for (int i = 0; i < 16; ++i) {
		int palNum = 15 - i;
		if (bitmask & 1) {
			ushort color = flipWordEndianess(*((ushort*)data));
			data += 2;

			int r = ((color >> 8) & 7) << 1;
			int g = ((color >> 4) & 7) << 1;
			int b = (color & 7) << 1;
			//std::cout << "\tSet color " << palNum << " with values " << "(R: " << r << " , G: " << g << " , B: " << b << ")\n";
			pal[palNum] = (r << 10) | (g << 5) | b;
		}
		bitmask >>= 1;
	}
	//std::cout << std::endl;
}

static void interpretDescriptorSpecial(uchar descriptor)
{
	switch (descriptor)
	{
	case 0xff:
	{
		//std::cout << "End of frame\n\n";
	}
	break;

	case 0xfe:
	{
		//std::cout << "End of frame and skip at next 64k block\n\n";
		++block64index;
		data = &scene1_bin[block64index << 16];
	}
	break;

	case 0xfd:
	{
		//std::cout << "That's all folks!\n\n\n";

		// Option 1, restart
		data = &scene1_bin[0];
		block64index = 0;

		// Option 2, quit?
		//endOfAllFrames = true;
	}
	break;
	}
}

static void interpretDescriptorNormal(uchar descriptor, int &polyNumVertices, int &colorIndex)
{
	colorIndex = (int)(descriptor >> 4);
	polyNumVertices = (int)(descriptor & 15);

	//std::cout << "Poly N=" << polyNumVertices << " C=" << colorIndex;
}

static void renderPolygon(Point2D *pt, int numVertices, int paletteIndex)
{
	ushort color = pal[paletteIndex];


}

static void interpretIndexedMode()
{
	static Point2D vi[256];

	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	//std::cout << "Frame in indexed mode\n\n";

	int vertexNum = *data++;
	//std::cout << "Number of vertices: " << vertexNum << "\n\n";

	for (int i = 0; i < vertexNum; ++i) {
		vi[i].x = (int)*data++;
		vi[i].y = (int)*data++;
		//std::cout << "Vertex " << i << ": X=" << vi[i].x << " Y=" << vi[i].y << std::endl;
	}

	while(true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, polyNumVertices, polyPaletteIndex);

		for (int n = 0; n < polyNumVertices; ++n) {
			int vertexId = *data++;
			//std::cout << " " << vertexId;
			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}
		//std::cout << std::endl;
		renderPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void interpretNonIndexedMode()
{
	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	//std::cout << "Frame in non-indexed mode\n\n";

	while (true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, polyNumVertices, polyPaletteIndex);

		for (int n = 0; n < polyNumVertices; ++n) {
			pt[n].x = *data++;
			pt[n].y = *data++;
			//std::cout << "(" << pt[n].x << "," << pt[n].y << ") ";
		}
		//std::cout << std::endl;
		renderPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void interpretFlag()
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

static void decodeFrame()
{
	interpretFlag();
}

static void renderScript(ScreenBuffer *screen)
{
	memset(screen->vram, 0, screen->width * screen->height * (screen->bpp >> 3));


	if (nextFrame > currentFrame) {
		currentFrame = nextFrame;
		//std::cout << "\n\n\nFrame " << currentFrame << "\n==========\n\n";
		decodeFrame();
	}

	++nextFrame;
}

void Script::run(ScreenBuffer *screen, InputBuffer *input)
{
	inputScript(input);
	renderScript(screen);

	if (endOfAllFrames) input->quit = true;
}

void Script::deinit()
{
}
