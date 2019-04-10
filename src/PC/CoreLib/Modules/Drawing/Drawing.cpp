#include "DrawingLine.h"
#include "DrawingPolygon.h"
#include "Drawing.h"

#include "Modules\Misc\MathUtil.h"

void initDrawingFramework(ScreenBuffer *screen)
{
	initDrawingFrameworkPolygon(screen);
}

uint blendPixelColor(uint cBack, uint cFront, uint frac)
{
	uint r0 = GET_R(cBack);
	uint g0 = GET_G(cBack);
	uint b0 = GET_B(cBack);

	uint r1 = GET_R(cFront);
	uint g1 = GET_G(cFront);
	uint b1 = GET_B(cFront);

	r0 = ((FP_AND - frac) * r0 + frac * r1) >> FP_BITS;
	g0 = ((FP_AND - frac) * g0 + frac * g1) >> FP_BITS;
	b0 = ((FP_AND - frac) * b0 + frac * b1) >> FP_BITS;

	return (r0 << 16) | (g0 << 8) | b0;
}

void clearScreen(ScreenBuffer *screen, uint color)
{
	const int size = (int)(screen->width * screen->height);
	uint *dst = (uint*)screen->vram;
	for (int i = 0; i < size; ++i)
		*dst++ = color;
}

void drawZoomedImage(const Point2D &pos, float zoom, Image *img, ScreenBuffer *screen, bool fromCenter)
{
	const int imgWidth = (int)img->width;
	const int imgHeight = (int)img->height;
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;

	int zoomWidth = (int)(imgWidth * zoom);
	int zoomHeight = (int)(imgHeight * zoom);

	Point2D posUL = pos;
	if (fromCenter) {
		posUL.x -= (int)(zoom * (imgWidth / 2));
		posUL.y -= (int)(zoom * (imgHeight / 2));
	}

	int fuUL = 0;
	if (posUL.x < 0) {
		zoomWidth += posUL.x;
		fuUL = INT_TO_FIXED((int)(-posUL.x / zoom), FP_BITS);
		posUL.x = 0;
	}

	int fvUL = 0;
	if (posUL.y < 0) {
		zoomHeight += posUL.y;
		fvUL = INT_TO_FIXED((int)(-posUL.y / zoom), FP_BITS);
		posUL.y = 0;
	}

	if (posUL.x + zoomWidth >= screenWidth)
		zoomWidth = screenWidth - posUL.x;
	if (posUL.y + zoomHeight >= screenHeight)
		zoomHeight = screenHeight - posUL.y;

	uint *dst = (uint*)screen->vram + posUL.y * screenWidth + posUL.x;

	int du = FLOAT_TO_FIXED(1.0f / zoom, FP_BITS);
	int dv = du;

	int fv = fvUL;
	for (int y = 0; y < zoomHeight; ++y) {
		int fu = fuUL;
		int v = FIXED_TO_INT(fv, FP_BITS);
		uint *src = img->data + v * imgWidth;
		for (int x = 0; x < zoomWidth; ++x) {
			int u = FIXED_TO_INT(fu, FP_BITS);
			*(dst + x) = *(src + u);
			fu += du;
		}
		dst += screenWidth;
		fv += dv;
	}
}

void drawZoomedImageClipped(const Point2D &pos, float zoom, Image *img, ScreenBuffer *screen, const Box &clipBox, bool fromCenter)
{
	const int imgWidth = (int)img->width;
	const int imgHeight = (int)img->height;
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;

	int zoomWidth = (int)(imgWidth * zoom);
	int zoomHeight = (int)(imgHeight * zoom);

	Point2D posUL = pos;
	if (fromCenter) {
		posUL.x -= (int)(zoom * (imgWidth / 2));
		posUL.y -= (int)(zoom * (imgHeight / 2));
	}

	int fuUL = 0;
	if (posUL.x < 0) {
		zoomWidth += posUL.x;
		fuUL = INT_TO_FIXED((int)(-posUL.x / zoom), FP_BITS);
		posUL.x = 0;
	}

	int fvUL = 0;
	if (posUL.y < 0) {
		zoomHeight += posUL.y;
		fvUL = INT_TO_FIXED((int)(-posUL.y / zoom), FP_BITS);
		posUL.y = 0;
	}

	if (posUL.x + zoomWidth >= screenWidth)
		zoomWidth = screenWidth - posUL.x;
	if (posUL.y + zoomHeight >= screenHeight)
		zoomHeight = screenHeight - posUL.y;

	uint *dst = (uint*)screen->vram + posUL.y * screenWidth + posUL.x;

	int du = FLOAT_TO_FIXED(1.0f / zoom, FP_BITS);
	int dv = du;

	int fv = fvUL;
	for (int y = 0; y < zoomHeight; ++y) {
		const int pixY = posUL.y + y;
		int fu = fuUL;
		int v = FIXED_TO_INT(fv, FP_BITS);
		uint *src = img->data + v * imgWidth;
		for (int x = 0; x < zoomWidth; ++x) {
			const int pixX = posUL.x + x;
			if (!(pixX < clipBox.ul.x || pixX > clipBox.lr.x || pixY < clipBox.ul.y || pixY > clipBox.lr.y)) {
				int u = FIXED_TO_INT(fu, FP_BITS);
				*(dst + x) = *(src + u);
			}
			fu += du;
		}
		dst += screenWidth;
		fv += dv;
	}
}

void drawRectangle(const Point2D &ul, const Point2D &lr, uint color, bool filled, ScreenBuffer *screen)
{
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;

	if (ul.x >= screenWidth || ul.y >= screenHeight || lr.x < 0 || lr.y < 0 || ul.x > lr.x || ul.y > lr.y) return;

	Point2D posUL = ul;
	CLAMP(posUL.x, 0, screenWidth-1);
	CLAMP(posUL.y, 0, screenHeight - 1);
	const int lengthX = lr.x - posUL.x + 1;
	const int lengthY = lr.y - posUL.y + 1;

	uint *dst = (uint*)screen->vram + posUL.y * screenWidth + posUL.x;

	if (filled) {
		for (int y = 0; y < lengthY; ++y) {
			for (int x = 0; x < lengthX; ++x) {
				*(dst + x) = color;
			}
			dst += screenWidth;
		}
	}
	else {
		for (int x = 0; x < lengthX; ++x) {
			*(dst + x) = color;
			*(dst + (lengthY - 1) * screenWidth + x) = color;
		}
		for (int y = 0; y < lengthY; ++y) {
			*dst = color;
			*(dst + lengthX - 1) = color;
			dst += screenWidth;
		}
	}
}
