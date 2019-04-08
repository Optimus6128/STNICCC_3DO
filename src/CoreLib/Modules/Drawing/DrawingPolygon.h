#ifndef DRAWING_POLYGON_H
#define DRAWING_POLYGON_H

#include <math.h>
#include <stdio.h>

#include "Typedefs.h"
#include "ScreenBuffer.h"
#include "Drawing.h"

#include "Modules\Misc\MathUtil.h"
#include "Modules\Image\Image.h"
#include "Modules\Image\ImageLoader.h"


#define TEX_COORD_RANGE 256
#define TEX_COORD_RANGE_BITS 8

enum TextureFilterType {Nearest, Bilinear};
enum TextureWrappingType {Clamp, Repeat};

struct Vertex
{
	int x, y, z;
	int c, u, v;

	Vertex(int _x, int _y, int _z, int _c, int _u, int _v) : x(_x), y(_y), z(_z), c(_c), u(_u), v(_v) {}
	Vertex(int _x, int _y) : x(_x), y(_y) {}
	Vertex(const Point2D &p) : x(p.x), y(p.y) {}
	Vertex() : Vertex(0, 0, 0, 0, 0, 0) {}

	void scale(float scl) {
		x = (int)(x * scl);
		y = (int)(y * scl);
		z = (int)(z * scl);
	}

	void interpolate(const Vertex &v0, const Vertex &v1, float t) {
		x = (int)(v0.x * (1.0f - t) + v1.x * t);
		y = (int)(v0.y * (1.0f - t) + v1.y * t);
		z = (int)(v0.z * (1.0f - t) + v1.z * t);
	}
};

struct Edge
{
	int x;
	int c, u, v;

	Edge(int _x, int _c, int _u, int _v) : x(_x), c(_c), u(_u), v(_v) {}
	Edge() : Edge(0, 0, 0, 0) {}
};

struct Texture : public Image
{
	Texture(Image *img)
	{
		width = img->width;
		height = img->height;
		data = img->data;
	}
};

struct QuadTransform
{
	Point2D pos;
	float zoom;
	float angle;

	QuadTransform(Point2D &_pos, float _zoom, float _angle) : pos(_pos), zoom(_zoom), angle(_angle) {}
	QuadTransform() : QuadTransform(Point2D(), 1.0f, 0.0f) {}
};

struct Quad
{
	Vertex v0, v1, v2, v3;

	Quad(Vertex &_v0, Vertex &_v1, Vertex &_v2, Vertex &_v3) : v0(_v0), v1(_v1), v2(_v2), v3(_v3) {}
	Quad() : Quad(Vertex(), Vertex(), Vertex(), Vertex()) {}

	Quad transform(int px, int py, float zoom = 1.0f, float angle = 0.0f)
	{
		Quad q(*this);

		int fsin = FLOAT_TO_FIXED(sin(angle) * zoom, FP_BITS);
		int fcos = FLOAT_TO_FIXED(cos(angle) * zoom, FP_BITS);

		auto transformVertex = [&](Vertex &vTrans, Vertex &vSrc)
		{
			vTrans.x = FIXED_TO_INT(vSrc.x * fcos - vSrc.y * fsin, FP_BITS) + px;
			vTrans.y = FIXED_TO_INT(vSrc.x * fsin + vSrc.y * fcos, FP_BITS) + py;
		};

		transformVertex(q.v0, v0);
		transformVertex(q.v1, v1);
		transformVertex(q.v2, v2);
		transformVertex(q.v3, v3);
		return q;
	}

	Quad transform(QuadTransform &qt)
	{
		return transform(qt.pos.x, qt.pos.y, qt.zoom, qt.angle);
	}

	Quad transform3D(int px, int py, float zoom = 1.0f, float angleX = 0.0f, float angleY = 0.0f, float angleZ = 0.0f)
	{
		Quad q(*this);

		int fsinX = FLOAT_TO_FIXED(sin(angleX), FP_BITS);
		int fcosX = FLOAT_TO_FIXED(cos(angleX), FP_BITS);
		int fsinY = FLOAT_TO_FIXED(sin(angleY), FP_BITS);
		int fcosY = FLOAT_TO_FIXED(cos(angleY), FP_BITS);
		int fsinZ = FLOAT_TO_FIXED(sin(angleZ), FP_BITS);
		int fcosZ = FLOAT_TO_FIXED(cos(angleZ), FP_BITS);

		auto transformVertex = [&](Vertex &vTrans, Vertex &vSrc)
		{
			Vertex vTempX, vTempY, vTempZ;

			vTempZ.x = FIXED_TO_INT(vSrc.x * fcosZ - vSrc.y * fsinZ, FP_BITS);
			vTempZ.y = FIXED_TO_INT(vSrc.x * fsinZ + vSrc.y * fcosZ, FP_BITS);
			vTempZ.z = vSrc.z;

			vTempX.x = FIXED_TO_INT(vTempZ.x * fcosY - vTempZ.z * fsinY, FP_BITS);
			vTempX.y = vTempZ.y;
			vTempX.z = FIXED_TO_INT(vTempZ.x * fsinY + vTempZ.z * fcosY, FP_BITS);

			vTempY.x = vTempX.x;
			vTempY.y = FIXED_TO_INT(vTempX.y * fcosX - vTempX.z * fsinX, FP_BITS);
			vTempY.z = FIXED_TO_INT(vTempX.y * fsinX + vTempX.z * fcosX, FP_BITS);

			vTrans.x = (int)(vTempY.x * zoom + px);
			vTrans.y = (int)(vTempY.y * zoom + py);
			vTrans.z = vTempY.z;
		};

		transformVertex(q.v0, v0);
		transformVertex(q.v1, v1);
		transformVertex(q.v2, v2);
		transformVertex(q.v3, v3);

		return q;
	}
};

void initDrawingFrameworkPolygon(ScreenBuffer *screen);

void drawTriangle(Vertex &v0, Vertex &v1, Vertex &v2, ScreenBuffer *screen);
void drawQuad(Quad &q, ScreenBuffer *screen);

void setMainTexture(Texture *tex);
void setTextureFilter(TextureFilterType filterType);
void setTextureWrapping(TextureWrappingType wrappingType);

#endif
