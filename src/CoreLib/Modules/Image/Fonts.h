#ifndef FONTS_H
#define FONTS_H

#include "Typedefs.h"
#include "Image.h"
#include "ImageLoader.h"
#include "ScreenBuffer.h"

#include "Modules\Drawing\DrawingPolygon.h"
#include "Modules\Drawing\Drawing.h"

#include <string>
#include <vector>

#define MAX_FONTS_NUM 256

class Fonts
{
private:
	std::vector<Image*> fontImgs;
	ushort fontWidth, fontHeight;
	Quad qBase;

	void draw(char c, uint px, uint py, ScreenBuffer *screen);
	void drawZR(char c, uint px, uint py, float zoom, float angle, ScreenBuffer *screen);

public:
	Fonts();
	~Fonts();

	ushort getWidth() { return fontWidth; };
	ushort getHeight() { return fontHeight; };

	bool loadSquareBitmapFonts(const char *filename, ushort fontWidth, const std::string &charmap);
	bool loadSquareBitmapFonts(const char *filename, ushort fontWidth, uint firstCharIndex, uint numChars);

	void setLuminosityAsAlpha(const float opacity = 1.0f);
	void setShade(int shade = 255);

	void drawText(std::string text, uint px, uint py, ScreenBuffer *screen);
	void drawTextZ(std::string text, uint px, uint py, float zoom, ScreenBuffer *screen);
};

#endif
