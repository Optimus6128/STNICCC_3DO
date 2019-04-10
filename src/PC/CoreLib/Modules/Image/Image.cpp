#include <string.h>
#include "Image.h"

#include "Modules\Drawing\Drawing.h"
#include "Modules\Misc\MathUtil.h"

Image::Image(uint width, uint height)
{
	this->width = width;
	this->height = height;
	this->data = new uint[width * height];
}

Image::Image(uint width, uint height, uint *data)
{
	this->width = width;
	this->height = height;
	this->data = data;
}

Image::~Image()
{
	delete data;
}

void Image::useLuminosityAsAlpha(const float opacity)
{
	for (uint i=0; i<width * height; ++i)
	{
		uint c = data[i];
		uint b = c & 255;
		uint g = (c >> 8) & 255;
		uint r = (c >> 16) & 255;
		uint a = (uint)(LUMINANCE(r, g, b) * opacity);
		data[i] = (c & 16777215) | (a << 24);
	}
}

void Image::colorize(ColorF col)
{
	uint size = width * height;

	for (uint i = 0; i < size; ++i) {
		uint c = data[i];
		int b = (int)((c & 255) * col.b);
		int g = (int)(((c >> 8) & 255) * col.g);
		int r = (int)(((c >> 16) & 255) * col.r);
		CLAMP(r, 0, 255);
		CLAMP(g, 0, 255);
		CLAMP(b, 0, 255);
		data[i] = (c & 0xFF000000) | (r << 16) | (g << 8) | b;
	}
}

void Image::flipHorizontally()
{
	for (uint x = 0; x < width / 2; ++x) {
		uint *srcLeft = &data[x];
		uint *srcRight = &data[width - 1 - x];
		for (uint y = 0; y < height; ++y) {
			uint cTemp = *srcLeft;
			*srcLeft = *srcRight;
			*srcRight = cTemp;
			srcLeft += width;
			srcRight += width;
		}
	}
}

void Image::flipVertically()
{
	for (uint y = 0; y < height / 2; ++y) {
		uint *srcUp = &data[y * width];
		uint *srcDown = &data[(height - 1 - y) * width];
		for (uint x = 0; x < width; ++x) {
			uint cTemp = *srcUp;
			*srcUp++ = *srcDown;
			*srcDown++ = cTemp;
		}
	}
}

void Image::draw(int px, int py, bool alpha, ScreenBuffer *screen, float alphaFade)
{
	if (alpha)
		drawAlpha(px, py, screen, alphaFade);
	else
		drawSolid(px, py, screen);
}

void Image::drawSolid(int px, int py, ScreenBuffer *screen)
{
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;

	for (uint y = 0; y < height; ++y)
	{
		int yi = py + y;
		if (yi >= 0 && yi < screenHeight)
		{
			uint *src = this->data + y * width;
			uint *dst = (uint*)screen->vram + yi * screenWidth;
			for (uint x = 0; x < width; ++x)
			{
				int xi = px + x;
				uint c = *src++;
				if (xi >=0 && xi < screenWidth)
					*(dst + px + x) = c;
			}
		}
	}
}

// A bit copy-pasta here for now
void Image::drawAlpha(int px, int py, ScreenBuffer *screen, float alphaFade)
{
	const int screenWidth = (int)screen->width;
	const int screenHeight = (int)screen->height;

	CLAMP01(alphaFade);
	if (alphaFade == 0.0f) return;

	uint aFade = (uint)(alphaFade * 256);
	for (uint y = 0; y < height; ++y)
	{
		int yi = py + y;
		if (yi >= 0 && yi < screenHeight)
		{
			uint *src = this->data + y * width;
			uint *dst = (uint*)screen->vram + yi * screenWidth;
			for (uint x = 0; x < width; ++x)
			{
				int xi = px + x;
				uint c = *src++;
				int a = ((c >> 24) * aFade) >> 8;
				if (xi >= 0 && xi < screenWidth) {
					int r1 = GET_R(c);
					int g1 = GET_G(c);
					int b1 = GET_B(c);

					uint cBack = *(dst + px + x);
					int r0 = GET_R(cBack);
					int g0 = GET_G(cBack);
					int b0 = GET_B(cBack);

					int r = ((r1 * a) >> 8) + ((r0 * (256 - a)) >> 8);
					int g = ((g1 * a) >> 8) + ((g0 * (256 - a)) >> 8);
					int b = ((b1 * a) >> 8) + ((b0 * (256 - a)) >> 8);

					*(dst + px + x) = MAKE_RGB(r, g, b);
				}
			}
		}
	}
}
