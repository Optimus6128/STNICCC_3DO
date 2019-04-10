#ifndef DRAWING_H
#define DRAWING_H

#include <math.h>
#include <iostream>

#include "Typedefs.h"
#include "ScreenBuffer.h"

#include "Modules\Misc\MathUtil.h"
#include "Modules\Image\Image.h"

#define FP_BITS 12
#define FP_AND ((1 << FP_BITS) - 1)

#define GET_A(c) (((c) >> 24) & 255)
#define GET_R(c) (((c) >> 16) & 255)
#define GET_G(c) (((c) >> 8) & 255)
#define GET_B(c) ((c) & 255)

#define GET_RGB(c,rgb) c.r = ((rgb) >> 16) & 255; c.g = ((rgb) >> 8) & 255; c.b = (rgb) & 255;
#define GET_RGBA(c,argb) c.a = ((argb) >> 24) & 255; c.r = ((argb) >> 16) & 255; c.g = ((argb) >> 8) & 255; c.b = (argb) & 255;

#define MAKE_RGBA(r,g,b,a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define MAKE_RGB(r,g,b) MAKE_RGBA((r),(g),(b),255)

#define MAKE_SHADED_RGBA(r,g,b,a,c) (((((a) * (c)) >> 8) << 24) | ((((r) * (c)) >> 8) << 16) | ((((g) * (c)) >> 8) << 8) | (((b) * (c)) >> 8))
#define MAKE_SHADED_RGB(r,g,b,c) ((255 << 24) | ((((r) * (c)) >> 8) << 16) | ((((g) * (c)) >> 8) << 8) | (((b) * (c)) >> 8))
#define SHADE_VALUE(c,a) (((c) * (a)) >> 8)

#define LUMINANCE(r,g,b) (0.3*(r) + 0.59*(g) + 0.11*(b))

struct Color
{
	int r, g, b, a;
	Color(int _r, int _g, int _b, int _a) : r(_r), g(_g), b(_b), a(_a) {}
	Color(int _r, int _g, int _b) : r(_r), g(_g), b(_b) { a = 255; }
	Color() : Color(0, 0, 0) {}

	inline Color operator+(const Color &c2) {
		Color c;
		c.r = r + c2.r; c.g = g + c2.g; c.b = b + c2.b; c.a = a + c2.a;
		return c;
	}

	inline Color operator-(const Color &c2) {
		Color c;
		c.r = r - c2.r; c.g = g - c2.g; c.b = b - c2.b; c.a = a - c2.a;
		return c;
	}
};

struct Position
{
	int x, y;

	Position(int _x, int _y) : x(_x), y(_y) {}
	Position() : Position(0, 0) {}
};

struct Point2D
{
	int x, y;

	Point2D(int _x, int _y) : x(_x), y(_y) {}
	Point2D() : Point2D(0, 0) {}

	Point2D operator+(const Point2D &p) const {
		return Point2D(x + p.x, y + p.y);
	}

	Point2D operator-(const Point2D &p) const {
		return Point2D(x - p.x, y - p.y);
	}

	Point2D operator*(float scl) const {
		return Point2D((int)(x * scl), (int)(y * scl));
	}

	Point2D rotate(float angle)
	{
		Point2D p(*this);

		int fsin = FLOAT_TO_FIXED(sin(angle), FP_BITS);
		int fcos = FLOAT_TO_FIXED(cos(angle), FP_BITS);

		p.x = FIXED_TO_INT(x * fcos - y * fsin, FP_BITS);
		p.y = FIXED_TO_INT(x * fsin + y * fcos, FP_BITS);

		return p;
	}

	void print() const
	{
		std::cout << x << ',' << y << std::endl;
	}
};

struct Box
{
	Point2D ul;
	Point2D lr;

	Box(Point2D _ul, Point2D _lr) : ul(_ul), lr(_lr) {}
	Box() : Box(Point2D(), Point2D()) {}
};

struct Column
{
	Point2D p0, p1;

	Column(Point2D _p0, Point2D _p1) : p0(_p0), p1(_p1) {}
	Column() : Column(Point2D(), Point2D()) {}
};


void initDrawingFramework(ScreenBuffer *screen);
void clearScreen(ScreenBuffer *screen, uint color = 0);

uint blendPixelColor(uint cBack, uint cFront, uint frac);

void drawZoomedImage(const Point2D &pos, float zoom, Image *img, ScreenBuffer *screen, bool fromCenter = true);
void drawZoomedImageClipped(const Point2D &pos, float zoom, Image *img, ScreenBuffer *screen, const Box &clipBox, bool fromCenter = true);
void drawRectangle(const Point2D &ul, const Point2D &lr, uint color, bool filled, ScreenBuffer *screen);

#endif
