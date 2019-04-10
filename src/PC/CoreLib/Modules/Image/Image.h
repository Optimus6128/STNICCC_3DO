#ifndef IMAGE_H
#define IMAGE_H

#define DEFAULT_IMAGE_WIDTH 256
#define DEFAULT_IMAGE_HEIGHT 256

#include "Typedefs.h"
#include "ScreenBuffer.h"


struct ColorF
{
	float r, g, b, a;
	ColorF(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	ColorF(float _r, float _g, float _b) : r(_r), g(_g), b(_b) { a = 1.0f; }
	ColorF() : ColorF(0.0f, 0.0f, 0.0f) {}
};

class Image
{
public:
	uint width, height;
	uint *data;

public:
	Image(uint width = DEFAULT_IMAGE_WIDTH, uint height = DEFAULT_IMAGE_HEIGHT);
	Image(uint width, uint height, uint *data);
	~Image();

	void useLuminosityAsAlpha(const float opacity = 1.0f);

	void flipHorizontally();
	void flipVertically();

	void colorize(ColorF col);

	void draw(int px, int py, bool alpha, ScreenBuffer *screen, float alphaFade = 1.0f);

private:
	void drawSolid(int px, int py, ScreenBuffer *screen);
	void drawAlpha(int px, int py, ScreenBuffer *screen, float alphaFade = 1.0f);
};

#endif
