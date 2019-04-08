#ifndef DRAWING_CEL_H
#define DRAWING_CEL_H

#include <math.h>
#include <stdio.h>

#include "Typedefs.h"
#include "ScreenBuffer.h"

#include "Drawing.h"
#include "DrawingLine.h"
#include "DrawingPolygon.h"

#include "Modules\Misc\MathUtil.h"
#include "Modules\Image\Image.h"

#define CEL_POS_SHIFT 16
#define CEL_H_SHIFT 20
#define CEL_V_SHIFT 16
#define CEL_POS_TO_H_SHIFT (CEL_H_SHIFT - CEL_POS_SHIFT)

enum class CelOrder
{
	CW,
	CCW,
	BOTH
};

#define MAX_BIGPOINT_DIAMETER 32

enum class CelDrawingMode
{
	Points,
	BigPoints,
	Lines,
	Filled
};

struct CCB
{
	int width, height;
	uint *source;

	int posX, posY;
	int hdx, hdy;
	int vdx, vdy;
	int hddx, hddy;

	CCB(int _width = 0, int _height = 0, uint* _source = nullptr) : width(_width), height(_height), source(_source), posX(0), posY(0), hdx(1 << CEL_H_SHIFT), hdy(0), vdx(0), vdy(1 << CEL_V_SHIFT), hddx(0), hddy(0) {};
};

struct Cel
{
	CCB ccb;
	Point2D *gridPoints;

	CelDrawingMode drawingMode;
	CelOrder order;

	Cel(CCB _ccb) : ccb(_ccb) {
		gridPoints = new Point2D[(ccb.width + 1) * (ccb.height + 1)];
		drawingMode = CelDrawingMode::Filled;
		order = CelOrder::CCW;
	};
};



void initCelDrawing(ScreenBuffer *screen);

void drawCel(Cel &cel, ScreenBuffer *screen);
void setBigPointSize(int size);
void mapQuadToCel(Quad &q, Cel &cel);

#endif
