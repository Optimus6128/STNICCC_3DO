#include "DrawingCel.h"
#include "../Script/ScriptMain.h"

#include <math.h>

#include <iostream>
#include <map>
#include <functional>


#define DSHIFT 16
static int bpDistOddEven[2][MAX_BIGPOINT_DIAMETER * MAX_BIGPOINT_DIAMETER];
static int *bpDist;
static int bpSize = 5;

static int *leftEdgeFlat = nullptr;
static int *rightEdgeFlat = nullptr;

static void initBigPoints()
{
	float halfPix = 0.0f;
	for (int k=0; k<2; ++k) {
		int i = 0;
		for (int y = 0; y < MAX_BIGPOINT_DIAMETER; ++y) {
			float yc = (float)y - MAX_BIGPOINT_DIAMETER / 2.0f + halfPix;
			for (int x = 0; x < MAX_BIGPOINT_DIAMETER; ++x) {
				float xc = (float)x - MAX_BIGPOINT_DIAMETER / 2.0f + halfPix;
				float radius = sqrt(xc*xc + yc*yc);
				bpDistOddEven[k][i++] = (int)((1 << DSHIFT) * radius);
			}
		}
		halfPix = 0.5f;	// make a second set of precalcs, offseting by half pixel for the even sized points
	}
}

void setBigPointSize(int size)
{
	bpSize = size;
}

void initCelDrawing(ScreenBuffer *screen)
{
	initBigPoints();

	if (leftEdgeFlat == nullptr) leftEdgeFlat = new int[screen->height];
	if (rightEdgeFlat == nullptr) rightEdgeFlat = new int[screen->height];
}

static inline void drawBigPoint(Point2D &p, ScreenBuffer *screen, int size, uint color)
{
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;
	const int screenSize = screenWidth * screenHeight;

	const int halfRadius = (size << DSHIFT) >> 1;
	const int halfSize = halfRadius >> DSHIFT;
	const int fullPixelFrac = 1 << DSHIFT;

	uint *dst = (uint*)screen->vram;

	const int iCornerOffset = MAX_BIGPOINT_DIAMETER / 2 - halfSize;
	int i = iCornerOffset * MAX_BIGPOINT_DIAMETER + iCornerOffset;
	int py = p.y - halfSize;
	for (int y = 0; y < size; ++y) {
		int px = p.x - halfSize;
		for (int x = 0; x < size; ++x) {
			const int addr = py * screenWidth + px;
			if (addr >= 0 && addr < screenSize) {
				const int dist = bpDist[i];
				if (dist < halfRadius - fullPixelFrac) {
					*(dst + addr) = color;
				}
				else if (dist < halfRadius + fullPixelFrac) {
					const int dstOffset = py * screenWidth + px;
					const uint c0 = *(dst + dstOffset);
					const uint b0 = c0 & 255;
					const uint g0 = (c0 >> 8) & 255;
					const uint r0 = (c0 >> 16) & 255;
					const uint c1 = color;
					const uint b1 = c1 & 255;
					const uint g1 = (c1 >> 8) & 255;
					const uint r1 = (c1 >> 16) & 255;
					const int aFrac = (dist + fullPixelFrac - halfRadius) >> 1;
					const int r = (r1 * (fullPixelFrac - aFrac) + r0 * aFrac) >> DSHIFT;
					const int g = (g1 * (fullPixelFrac - aFrac) + g0 * aFrac) >> DSHIFT;
					const int b = (b1 * (fullPixelFrac - aFrac) + b0 * aFrac) >> DSHIFT;

					*(dst + addr) = (r << 16) | (g << 8) | b;
				}
			}
			++i;
			++px;
		}
		i = i - size + MAX_BIGPOINT_DIAMETER;
		++py;
	}
}

static void drawSmallCelPoints(Cel &cel, ScreenBuffer *screen)
{
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;
	const int screenSize = screenWidth * screenHeight;

	Point2D *grid = cel.gridPoints;
	uint *src = cel.ccb.source;
	uint *dst = (uint*)screen->vram;

	int i = 0;
	for (int y = 0; y < cel.ccb.height; ++y) {
		for (int x = 0; x < cel.ccb.width; ++x) {
			const int px = grid[i].x;
			const int py = grid[i].y;

			const int addr = py * screenWidth + px;
			if (addr >= 0 && addr < screenSize)
				*(dst + addr) = *src;
			++src;
			++i;
		}
		++i;
	}
}

static void drawBigCelPoints(Cel &cel, ScreenBuffer *screen, int size)
{
	Point2D *grid = cel.gridPoints;
	uint *src = cel.ccb.source;
	uint *dst = (uint*)screen->vram;

	CLAMP(size, 1, MAX_BIGPOINT_DIAMETER);

	// odd or even sized big points need kinda different precalcs to be properly centered and not cut at the border.
	bpDist = bpDistOddEven[0];
	if ((size & 1) == 0)
		bpDist = bpDistOddEven[1];

	int i = 0;
	for (int y = 0; y < cel.ccb.height; ++y) {
		for (int x = 0; x < cel.ccb.width; ++x) {
			const int px = grid[i].x;
			const int py = grid[i].y;

			uint c = *src++;
			drawBigPoint(grid[i], screen, size, c);
			++i;
		}
		++i;
	}
}

static void drawCelPoints(Cel &cel, ScreenBuffer *screen)
{
	drawSmallCelPoints(cel, screen);
}

static void drawCelPointsBig(Cel &cel, ScreenBuffer *screen)
{
	drawBigCelPoints(cel, screen, bpSize);
}

static void drawCelLines(Cel &cel, ScreenBuffer *screen)
{
	Point2D *grid = cel.gridPoints;
	uint *src = cel.ccb.source;
	uint *dst = (uint*)screen->vram;

	const int ccbWidth = cel.ccb.width;
	const int ccbHeight = cel.ccb.height;

	int i = 0;
	for (int y = 0; y < ccbHeight; ++y) {
		for (int x = 0; x < ccbWidth; ++x) {
			uint c = *src++;
			drawAntialiasedLineThick(grid[i], grid[i+1], c, screen);
			drawAntialiasedLineThick(grid[i], grid[i+ccbWidth+1], c, screen);
			if (x == ccbWidth - 1)
				drawAntialiasedLineThick(grid[i+1], grid[i+ccbWidth+2], c, screen);
			if (y == ccbHeight - 1)
				drawAntialiasedLineThick(grid[i+ccbWidth+1], grid[i+ccbWidth+2], c, screen);
			++i;
		}
		++i;
	}
}


// ======== Specialized functions for simpler flat quad rasterizer ========

static inline void prepareEdgeListFlat(Point2D *p0, Point2D *p1, ScreenBuffer *screen)
{
	if (p0->y == p1->y) return;

	// Assumes CCW
	int *edgeListToWriteFlat;
	if (p0->y < p1->y) {
		edgeListToWriteFlat = leftEdgeFlat;
	}
	else {
		edgeListToWriteFlat = rightEdgeFlat;

		Point2D *pTemp = p0;
		p0 = p1;
		p1 = pTemp;
	}

	const int x0 = p0->x; const int y0 = p0->y;
	const int x1 = p1->x; const int y1 = p1->y;

	const int screenHeight = (int)screen->height;
	const int dx = INT_TO_FIXED(x1 - x0, FP_BITS) / (y1 - y0);

	int xp = INT_TO_FIXED(x0, FP_BITS);
	int yp = y0;
	do
	{
		if (yp >= 0 && yp < screenHeight)
		{
			edgeListToWriteFlat[yp] = FIXED_TO_INT(xp, FP_BITS);
		}
		xp += dx;

	} while (yp++ != y1);
}

void drawFlatQuad(Point2D &p0, Point2D &p1, Point2D &p2, Point2D &p3, uint color, ScreenBuffer *screen)
{
	const int x0 = p0.x; const int y0 = p0.y;
	const int x1 = p1.x; const int y1 = p1.y;
	const int x2 = p2.x; const int y2 = p2.y;
	const int x3 = p3.x; const int y3 = p3.y;

	const int scrWidth = screen->width;
	const int scrHeight = screen->height;

	int yMin = y0;
	int yMax = yMin;
	if (y1 < yMin) yMin = y1;
	if (y1 > yMax) yMax = y1;
	if (y2 < yMin) yMin = y2;
	if (y2 > yMax) yMax = y2;
	if (y3 < yMin) yMin = y3;
	if (y3 > yMax) yMax = y3;

	if (yMin < 0) yMin = 0;
	if (yMax > scrHeight - 1) yMax = scrHeight - 1;

	prepareEdgeListFlat(&p0, &p1, screen);
	prepareEdgeListFlat(&p1, &p2, screen);
	prepareEdgeListFlat(&p2, &p3, screen);
	prepareEdgeListFlat(&p3, &p0, screen);

	prepareEdgeListFlat(&p1, &p0, screen);
	prepareEdgeListFlat(&p2, &p1, screen);
	prepareEdgeListFlat(&p3, &p2, screen);
	prepareEdgeListFlat(&p0, &p3, screen);

	uint *dst = (uint*)screen->vram + yMin * scrWidth;
	for (int y = yMin; y <= yMax; y++)
	{
		int xl = leftEdgeFlat[y];
		int xr = rightEdgeFlat[y];

		if (xl < 0) xl = 0;
		if (xr > scrWidth - 1) xr = scrWidth - 1;

		if (xl == xr) ++xr;

		// prepareEdgeListFlat assumes CCW
		// If quad is clockwise, following the grid texel quads in clockwise is easy. But that will change in bizarro polygono
		// I could do a check and flip x here, as it's just rendering single flat color, so one more swap would be easy

		for (int x = xl; x < xr; ++x) {
			//*(dst + x) |= color;
			*(dst + x) = color;
			++pixelsWritten;
		}
		dst += scrWidth;
	}
}

void drawFlatQuadScaled(Point2D &p0, Point2D &p1, Point2D &p2, Point2D &p3, uint color, ScreenBuffer *screen)
{
	Point2D zp0, zp1, zp2, zp3;
	float l = 0.8f;
	zp0.x = p0.x << 2; zp0.y = (int)((p0.y << 2) * l);
	zp1.x = p1.x << 2; zp1.y = (int)((p1.y << 2) * l);
	zp2.x = p2.x << 2; zp2.y = (int)((p2.y << 2) * l);
	zp3.x = p3.x << 2; zp3.y = (int)((p3.y << 2) * l);

	drawFlatQuad(zp0, zp1, zp2, zp3, color, screen);
	drawFlatQuad(zp3, zp2, zp1, zp0, color, screen);	// both clockwiseness hack
}

static void drawCelFilled(Cel &cel, ScreenBuffer *screen)
{
	Point2D *grid = cel.gridPoints;
	uint *src = cel.ccb.source;
	uint *dst = (uint*)screen->vram;

	const int ccbWidth = cel.ccb.width;
	const int ccbHeight = cel.ccb.height;

	int i = 0;
	for (int y = 0; y < ccbHeight; ++y) {
		for (int x = 0; x < ccbWidth; ++x) {
			uint c = *src++;
			if (cel.order == CelOrder::CW)
				drawFlatQuad(grid[i], grid[i + 1], grid[i + ccbWidth + 2], grid[i + ccbWidth + 1], c, screen);
			else if (cel.order == CelOrder::CCW)
				drawFlatQuad(grid[i + ccbWidth + 1], grid[i + ccbWidth + 2], grid[i + 1], grid[i], c, screen);
			else if (cel.order == CelOrder::BOTH) {
				drawFlatQuad(grid[i], grid[i + 1], grid[i + ccbWidth + 2], grid[i + ccbWidth + 1], c, screen);
				drawFlatQuad(grid[i + ccbWidth + 1], grid[i + ccbWidth + 2], grid[i + 1], grid[i], c, screen);
			}
			++i;
		}
		++i;
	}
}

static void calculateCelGridPoints(Cel &cel)
{
	CCB ccb = cel.ccb;

	int hdx = ccb.hdx;
	int hdy = ccb.hdy;
	const int vdx = ccb.vdx;
	const int vdy = ccb.vdy;
	const int hddx = ccb.hddx;
	const int hddy = ccb.hddy;

	int hpx = ccb.posX;
	int hpy = ccb.posY;

	const int width = ccb.width;
	const int height = ccb.height;

	Point2D *gp = cel.gridPoints;
	for (int y = 0; y < height+1; ++y) {
		int hpx2 = hpx << CEL_POS_TO_H_SHIFT;
		int hpy2 = hpy << CEL_POS_TO_H_SHIFT;
		for (int x = 0; x < width+1; ++x) {
			gp->x = hpx2 >> CEL_H_SHIFT;
			gp->y = hpy2 >> CEL_H_SHIFT;
			gp++;
			hpx2 += hdx;
			hpy2 += hdy;
		}
		hpx += vdx;
		hpy += vdy;
		hdx += hddx;
		hdy += hddy;
	}
}

typedef std::function<void(Cel&, ScreenBuffer*)> celDrawingFunc;
static std::map<CelDrawingMode, celDrawingFunc> celDrawingFunctionMap = {{CelDrawingMode::Points, drawCelPoints}, 
																		{CelDrawingMode::BigPoints, drawCelPointsBig },
																		{CelDrawingMode::Lines, drawCelLines},
																		{CelDrawingMode::Filled, drawCelFilled}};

void drawCel(Cel &cel, ScreenBuffer *screen)
{
	calculateCelGridPoints(cel);
	celDrawingFunctionMap[cel.drawingMode](cel, screen);
}

void mapQuadToCel(Quad &q, Cel &cel)
{
	Point2D edge[3];
	edge[0].x = q.v1.x - q.v0.x;	edge[0].y = q.v1.y - q.v0.y;
	edge[1].x = q.v2.x - q.v3.x;	edge[1].y = q.v2.y - q.v3.y;
	edge[2].x = q.v3.x - q.v0.x;	edge[2].y = q.v3.y - q.v0.y;

	cel.ccb.posX = q.v0.x << CEL_POS_SHIFT;
	cel.ccb.posY = q.v0.y << CEL_POS_SHIFT;

	const int hdx0 = (edge[0].x << CEL_H_SHIFT) / cel.ccb.width;
	const int hdy0 = (edge[0].y << CEL_H_SHIFT) / cel.ccb.width;
	const int hdx1 = (edge[1].x << CEL_H_SHIFT) / cel.ccb.width;
	const int hdy1 = (edge[1].y << CEL_H_SHIFT) / cel.ccb.width;

	cel.ccb.hdx = hdx0;
	cel.ccb.hdy = hdy0;
	cel.ccb.hddx = (hdx1 - hdx0) / cel.ccb.height;
	cel.ccb.hddy = (hdy1 - hdy0) / cel.ccb.height;

	cel.ccb.vdx = (edge[2].x << CEL_V_SHIFT) / cel.ccb.height;
	cel.ccb.vdy = (edge[2].y << CEL_V_SHIFT) / cel.ccb.height;
}