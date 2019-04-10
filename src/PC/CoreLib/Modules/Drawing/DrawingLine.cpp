#include "DrawingLine.h"
#include "Drawing.h"

#include "Modules\Misc\MathUtil.h"

// Line Drawing
// ============

void drawAntialiasedLine(const Point2D &p0, const Point2D &p1, uint c, ScreenBuffer *screen)
{
	int x0 = p0.x;
	int y0 = p0.y;
	int x1 = p1.x;
	int y1 = p1.y;

	if (x0 == x1 && y0 == y1) return;

	uint *vram = (uint*)screen->vram;
	const int scr_w = (int)screen->width;
	const int scr_h = (int)screen->height;
	const int scr_size = scr_w * scr_h;

	int a = (c >> 24) & 255;

	int l = 0;
	int x00, y00;
	int vramofs;

	int x, y;
	int frac;

	int dx = x1 - x0;
	int dy = y1 - y0;

	int chdx = dx, chdy = dy;
	if (dx<0) chdx = -dx;
	if (dy<0) chdy = -dy;

	if (chdy < chdx)
	{
		if (x0 > x1)
		{
			int temp_x = x0; x0 = x1; x1 = temp_x;
			int temp_y = y0; y0 = y1; y1 = temp_y;
		}

		if (dx != 0) l = (dy << FP_BITS) / dx;
		y00 = y0 << FP_BITS;
		for (x = x0; x<=x1; x++)
		{
			if (x >= 0 && x < scr_w) {
				vramofs = (y00 >> FP_BITS)*scr_w + x;
				if (vramofs >= scr_w && vramofs < scr_size - scr_w) {
					frac = y00 & FP_AND;

					*(vram + vramofs) = blendPixelColor(*(vram + vramofs), c, SHADE_VALUE(FP_AND - frac, a));
					if (y0 < scr_h - 1 && y1 < scr_h - 1) *(vram + vramofs + scr_w) = blendPixelColor(*(vram + vramofs + scr_w), c, SHADE_VALUE(frac, a));
				}
			}
			y00 += l;
		}
	}
	else
	{
		if (y0 > y1)
		{
			int temp_x = x0; x0 = x1; x1 = temp_x;
			int temp_y = y0; y0 = y1; y1 = temp_y;
		}

		if (dy != 0) l = (dx << FP_BITS) / dy;
		x00 = x0 << FP_BITS;

		for (y = y0; y<=y1; y++)
		{
			const int xp = x00 >> FP_BITS;
			if (xp >= 0 && xp < scr_w) {
				vramofs = y * scr_w + xp;
				if (vramofs >= scr_w && vramofs < scr_size - scr_w) {
					frac = x00 & FP_AND;

					*(vram + vramofs) = blendPixelColor(*(vram + vramofs), c, SHADE_VALUE(FP_AND - frac, a));
					if (x0 < scr_w - 1 && x1 < scr_w - 1) *(vram + vramofs + 1) = blendPixelColor(*(vram + vramofs + 1), c, SHADE_VALUE(frac, a));
				}
			}
			x00 += l;
		}
	}
}


void drawAntialiasedLineThick(const Point2D &p0, const Point2D &p1, uint c, ScreenBuffer *screen)
{
	int x0 = p0.x;
	int y0 = p0.y;
	int x1 = p1.x;
	int y1 = p1.y;

	if (x0 == x1 && y0 == y1) return;

	uint *vram = (uint*)screen->vram;
	const int scr_w = (int)screen->width;
	const int scr_h = (int)screen->height;
	const int scr_size = scr_w * scr_h;

	int a = (c >> 24) & 255;

	int l = 0;
	int x00, y00;
	int vramofs;

	int x, y;
	int frac;

	int dx = x1 - x0;
	int dy = y1 - y0;

	if (abs(dx) > 4096 || abs(dy) > 4096) return;	// just a precaution for some very huge values that need to be clipped but can't afford to see how to do this easilly now.

	int chdx = dx, chdy = dy;
	if (dx<0) chdx = -dx;
	if (dy<0) chdy = -dy;

	if (chdy < chdx)
	{
		if (x0 > x1)
		{
			int temp_x = x0; x0 = x1; x1 = temp_x;
			int temp_y = y0; y0 = y1; y1 = temp_y;
		}

		if (dx != 0) l = (dy << FP_BITS) / dx;
		y00 = y0 << FP_BITS;
		for (x = x0; x<x1; x++)
		{
			if (x >= 0 && x < scr_w) {
				vramofs = (y00 >> FP_BITS)*scr_w + x;
				if (vramofs >= scr_w && vramofs < scr_size - scr_w) {
					frac = y00 & FP_AND;
					*(vram + vramofs) = blendPixelColor(*(vram + vramofs), c, SHADE_VALUE(FP_AND, a));
					if (y0 > 0 && y1 > 0) *(vram + vramofs - scr_w) = blendPixelColor(*(vram + vramofs - scr_w), c, SHADE_VALUE(FP_AND - frac, a));
					if (y0 < scr_h - 1 && y1 < scr_h - 1) *(vram + vramofs + scr_w) = blendPixelColor(*(vram + vramofs + scr_w), c, SHADE_VALUE(frac, a));
				}
			}
			y00 += l;
		}
	}
	else
	{
		if (y0 > y1)
		{
			int temp_x = x0; x0 = x1; x1 = temp_x;
			int temp_y = y0; y0 = y1; y1 = temp_y;
		}

		if (dy != 0) l = (dx << FP_BITS) / dy;
		x00 = x0 << FP_BITS;
		for (y = y0; y<y1; y++)
		{
			const int xp = x00 >> FP_BITS;
			if (xp >= 0 && xp < scr_w) {
				vramofs = y * scr_w + xp;
				if (vramofs > 0 && vramofs < scr_size - 1) {
					frac = x00 & FP_AND;
					*(vram + vramofs) = blendPixelColor(*(vram + vramofs), c, SHADE_VALUE(FP_AND, a));
					if (x0 > 0 && x1 > 0) *(vram + vramofs - 1) = blendPixelColor(*(vram + vramofs - 1), c, SHADE_VALUE(FP_AND - frac, a));
					if (x0 < scr_w - 1 && x1 < scr_w - 1) *(vram + vramofs + 1) = blendPixelColor(*(vram + vramofs + 1), c, SHADE_VALUE(frac, a));
				}
			}
			x00 += l;
		}
	}
}
