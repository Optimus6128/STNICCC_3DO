#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <string>
#include <vector>

#include "Typedefs.h"
#include "Image.h"

enum GenImageType {BLANK, XOR, NOISE};

struct ImgGenParameters
{
	uint width, height;
	uchar r, g, b, a;


	ImgGenParameters(uint _width, uint _height, uchar _r = 255, uchar _g = 255, uchar _b = 255, uchar _a = 255) : width(_width), height(_height), r(_r), g(_g), b(_b), a(_a) {}
// have to think these, how to combine. Maybe a map of string/values or setters/getters for individual
	ImgGenParameters() : ImgGenParameters(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT, 255, 255, 255, 255) {}
};

class ImageLoader
{
public:
	static void init();
	static Image* load(const char *filename);
	static Image* generate(GenImageType type, ImgGenParameters &params = ImgGenParameters(), bool alpha = false);
};

#endif
