#include "Fonts.h"

Fonts::Fonts()
{
	fontImgs = std::vector<Image*>(MAX_FONTS_NUM);
}

Fonts::~Fonts()
{
	for (uint i=0; i<MAX_FONTS_NUM; ++i) {
		if (fontImgs[i]!=nullptr)
			delete fontImgs[i];
	}
	fontImgs.clear();
}

bool Fonts::loadSquareBitmapFonts(const char *filename, ushort fontWidth, const std::string &charmap)
{
	Image *fontsImage = ImageLoader::load(filename);
	if (fontsImage == nullptr) return false;

	const size_t numFonts = charmap.size();
	if (numFonts == 0) return false;

	if (fontWidth == 0) return false;
	this->fontWidth = fontWidth;
	this->fontHeight = fontWidth;

	bool isFontmap1D = (fontsImage->height == fontHeight);

	// Extract the font data depending the structure inside the bitmap image (1D or 2D grid)
	if (isFontmap1D) {
		for (uint n = 0; n < numFonts; ++n) {
			Image *fontImg = new Image(fontWidth, fontHeight);
			uint fontIndex = n * fontWidth;

			for (uint y = 0; y < fontHeight; ++y) {
				for (uint x = 0; x < fontWidth; ++x) {
					fontImg->data[y * fontWidth + x] = fontsImage->data[fontIndex + y * fontsImage->width + x];
				}
			}
			fontImgs[charmap.at(n)] = fontImg;
		}
	}
	else {
		uint n = 0;
		for (uint cy = 0; cy < fontsImage->height; cy += fontHeight) {
			for (uint cx = 0; cx < fontsImage->width; cx += fontWidth) {
				Image *fontImg = new Image(fontWidth, fontHeight);
				uint fontIndex = cy * fontsImage->width + cx;
				for (uint y = 0; y < fontHeight; ++y) {
					for (uint x = 0; x < fontWidth; ++x) {
						fontImg->data[y * fontWidth + x] = fontsImage->data[fontIndex + y * fontsImage->width + x];
					}
				}
				fontImgs[charmap.at(n++)] = fontImg;
			}
		}
	}

	// Finally, calculate the mapped 1:1 base quad based on the texture dimensions
	const int halfWidth = (int)(fontWidth / 2);
	const int halfHeight = (int)(fontHeight / 2);
	const int texEdge = (TEX_COORD_RANGE - 1);

	qBase = Quad(Vertex(-halfWidth, halfHeight, 0, 255, 0, texEdge),
		Vertex(halfWidth, halfHeight, 0, 255, texEdge, texEdge),
		Vertex(halfWidth, -halfHeight, 0, 255, texEdge, 0),
		Vertex(-halfWidth, -halfHeight, 0, 255, 0, 0));

	return true;
}

bool Fonts::loadSquareBitmapFonts(const char *filename, ushort fontWidth, uint firstCharIndex, uint numChars)
{
	size_t size = (size_t)numChars;
	std::string charmap(size, '\0');

	uint currentCharIndex = firstCharIndex;
	uint count = 0;
	while (count++ != numChars)
		charmap[count] = currentCharIndex++;

	return loadSquareBitmapFonts(filename, fontWidth, charmap);
}

void Fonts::draw(char c, uint px, uint py, ScreenBuffer *screen)
{
	if (fontImgs[c] == nullptr) return;
	if ((px + fontWidth >= screen->width) || (py + fontHeight >= screen->height)) return;

	uint *vram = (uint*)screen->vram + py * screen->width + px;
	uint *src = (uint*)fontImgs[c]->data;
	for (uint y = 0; y < fontHeight; ++y) {
		for (uint x = 0; x < fontWidth; ++x) {
			uint col = *(src + y * fontWidth + x);
			if (col !=0)
				*(vram + y * screen->width + x) = col;
		}
	}
}

void Fonts::setLuminosityAsAlpha(const float opacity)
{
	for (auto fontImg : fontImgs) {
		if (fontImg != nullptr) {
			for (uint i = 0; i < fontImg->width * fontImg->height; ++i) {
				uint c = fontImg->data[i];
				uint b = c & 255;
				uint g = (c >> 8) & 255;
				uint r = (c >> 16) & 255;
				uint a = (uint)(LUMINANCE(r, g, b) * opacity);
				fontImg->data[i] = (c & 16777215) | (a << 24);
			}
		}
	}
}

void Fonts::setShade(int shade)
{
	qBase.v0.c = shade;
	qBase.v1.c = shade;
	qBase.v2.c = shade;
	qBase.v3.c = shade;
}

void Fonts::drawZR(char c, uint px, uint py, float zoom, float angle, ScreenBuffer *screen)
{
	if (fontImgs[c] == nullptr) return;

	setMainTexture((Texture*)fontImgs[c]);
	drawQuad(qBase.transform(px, py, zoom, angle), screen);
}

void Fonts::drawText(std::string text, uint px, uint py, ScreenBuffer *screen)
{
	for (auto c : text) {
		draw(c, px, py, screen);
		px += fontWidth;
	}
}
void Fonts::drawTextZ(std::string text, uint px, uint py, float zoom, ScreenBuffer *screen)
{
	for (auto c : text) {
		drawZR(c, px, py, zoom, 0.0f, screen);
		px += (uint)(zoom * fontWidth);
	}
}