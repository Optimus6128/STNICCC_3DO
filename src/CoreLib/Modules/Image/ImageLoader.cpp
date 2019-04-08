#include <SDL2/SDL_image.h>

#include <string>

#include "ImageLoader.h"

#include "Modules\Drawing\Drawing.h"

void ImageLoader::init()
{
}

Image* getImageFromSurface(SDL_Surface *surf)
{
	Image *img = new Image(surf->w, surf->h);
	int bits = surf->format->BitsPerPixel;

	unsigned char *src8;
	unsigned int *dest = img->data;

	int a, r, g, b, c;

	for (uint y = 0; y<img->height; y++)
	{
		src8 = (unsigned char*)surf->pixels + y * surf->pitch;
		for (uint x = 0; x<img->width; x++)
		{
			if (bits >= 24)
			{
				b = *src8++;
				g = *src8++;
				r = *src8++;

				if (bits == 24)
					a = 255;
				else
					a = *src8++;
			}
			else if (bits == 8)
			{
				c = *src8++;
				r = g = b = a = c;
			}

			*dest++ = (a << 24) | (b << 16) | (g << 8) | r;
		}
	}

	return img;
}

Image* ImageLoader::load(const char *filename)
{
 	SDL_Surface *imgSurface = IMG_Load(filename);
	if (imgSurface) {
		Image *img = getImageFromSurface(imgSurface);
		SDL_FreeSurface(imgSurface);
		return img;
	}
	else {
		std::cout << "Failed to load image: " << std::string(filename) << std::endl;
		return nullptr;
	}
}

Image* ImageLoader::generate(GenImageType type, ImgGenParameters &params, bool alpha)
{
	Image *img = new Image(params.width, params.height);
	uint *data = img->data;

	switch (type)
	{
	case GenImageType::BLANK:
		memset(img->data, MAKE_RGBA(params.r, params.g, params.b, params.a), img->width * img->height * sizeof(*img->data));
		break;

	case GenImageType::XOR:
		for (uint y = 0; y < img->height; ++y)
		{
			for (uint x = 0; x < img->width; ++x)
			{
				uint c = (x ^ y) & 255;
				if (alpha)
					*data++ = MAKE_SHADED_RGBA(params.r, params.g, params.b, params.a, c);
				else
					*data++ = MAKE_SHADED_RGB(params.r, params.g, params.b, c);
			}
		}
		break;

	case GenImageType::NOISE:
		for (uint i = 0; i < img->width * img->height; ++i) {
			uint c = rand() & 255;
			if (alpha)
				*data++ = MAKE_SHADED_RGBA(params.r, params.g, params.b, params.a, c);
			else
				*data++ = MAKE_SHADED_RGB(params.r, params.g, params.b, c);
		}
		break;

	default:
		return generate(GenImageType::BLANK, params);
		break;
	}

	return img;
}
