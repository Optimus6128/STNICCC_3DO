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

#include "Modules/Drawing/DrawingCel.h"


static int currentFrame = -1;
static int nextFrame = 0;
static uint block64index = 0;

static uchar *data = scene1_bin;

static ushort pal[16];
static Point2D pt[16];

static QuadStore quads[1024];
static QuadStore *quadPtr;
static int numQuads = 0;


static bool mustClearScreen = false;
static bool endOfAllFrames = false;


void Script::init(ScreenBuffer *screen)
{
}

static uint color32from15(ushort color)
{
	int r = ((color >> 10) & 31) << 3;
	int g = ((color >> 5) & 31) << 3;
	int b = (color & 31) << 3;
	return (r << 16) | (g << 8) | b;
}

static bool isPolygonConvex(Point2D *pt, int numVertices)
{
	int zcross0;

	for (int i = 0; i < numVertices; ++i) {
		const int i0 = i;
		const int i1 = (i + 1) % numVertices;
		const int i2 = (i + 2) % numVertices;

		vec2i v0;	v0.x = pt[i1].x - pt[i0].x;  v0.y = pt[i1].y - pt[i0].y;
		vec2i v1;	v1.x = pt[i2].x - pt[i1].x;  v1.y = pt[i2].y - pt[i1].y;

		int zcross = v0.x * v1.y - v0.y * v1.x;
		if (i == 0) {
			zcross0 = zcross;
		} else {
			zcross *= zcross0;
			if (zcross < 0) {
				std::cout << "GOTCHA!\n";
				return false;
			}
		}
	}
	return true;
}

static void addPolygon(Point2D *pt, int numVertices, int paletteIndex)
{
	ushort color = pal[paletteIndex];

	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

	//if (!isPolygonConvex(pt, numVertices)) color = 31 << 10;

	while(pStartIndex < maxIndex)
	{
		quadPtr->p0.x = pt[pBaseIndex].x;		quadPtr->p0.y = pt[pBaseIndex].y;
		quadPtr->p1.x = pt[pStartIndex].x;		quadPtr->p1.y = pt[pStartIndex].y;
		quadPtr->p2.x = pt[pStartIndex+1].x;	quadPtr->p2.y = pt[pStartIndex + 1].y;

		pStartIndex += 2;
		if (pStartIndex > maxIndex) pStartIndex = maxIndex;
		quadPtr->p3.x = pt[pStartIndex].x;		quadPtr->p3.y = pt[pStartIndex].y;

		quadPtr->c = color;

		++quadPtr;
		++numQuads;
	}
}

static void renderPolygons(ScreenBuffer *screen)
{
	for (int i=0; i<numQuads; ++i) {
		Point2D *p0 = &quads[i].p0;
		Point2D *p1 = &quads[i].p1;
		Point2D *p2 = &quads[i].p2;
		Point2D *p3 = &quads[i].p3;
		drawFlatQuadScaled(*p0, *p1, *p2, *p3, color32from15(quads[i].c), screen);
		//printf("Quad: %d     P0: %d,%d   P1: %d,%d   P2: %d,%d   P3:%d,%d\n", i, p0->x, p0->y, p1->x, p1->y, p2->x, p2->y, p3->x, p3->y);
	}
}

static void inputScript(InputBuffer *input)
{
	input->quit = (input->keyState(SDLK_ESCAPE) == KEY_JUST_PRESSED);

	if (input->keyState(SDLK_n) == KEY_JUST_PRESSED) {
		++nextFrame;
	}
}

static ushort flipWordEndianess(ushort value)
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
		currentFrame = -1;
		nextFrame = 0;

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
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
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
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void decodeFrame()
{
	uchar flags = *data++;

	mustClearScreen = false;
	numQuads = 0;
	quadPtr = &quads[0];

	if (flags & 1) {
		mustClearScreen = true;
	}
	if (flags & 2) {
		interpretPaletteData();
	}
	if (flags & 4) {
		interpretIndexedMode();
	}
	else {
		interpretNonIndexedMode();
	}
}

static void renderScript(ScreenBuffer *screen)
{
	if (nextFrame > currentFrame) {
		currentFrame = nextFrame;
		//std::cout << "\n\n\nFrame " << currentFrame << "\n==========\n\n";
		std::cout << currentFrame << std::endl;
		decodeFrame();
		++nextFrame;

		if (mustClearScreen) memset(screen->vram, 0, screen->width * screen->height * (screen->bpp >> 3));

		renderPolygons(screen);
	}
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
