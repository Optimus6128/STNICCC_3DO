#include "types.h"
#include "main.h"
#include "tools.h"
#include "engine_texture.h"
#include "mathutil.h"
#include "ScriptMain.h"

texture *textures[TEXTURE_NUM];

uint16 defaultPal[32];

texture* initTexture(int width, int height, uint16 *pal, int type)
{
    int i, x, y, xc, yc, c;
	int size = width * height;

    texture *tex = (texture*)malloc(sizeof(texture));

    tex->width = width;
    tex->height = height;
    tex->bitmap = (ubyte*)malloc(size * sizeof(ubyte));
    tex->palette = pal;

    switch(type)
    {
        case TEXTURE_FLAT:
            for (i=0; i<size; i++)
                tex->bitmap[i] = 31;
        break;

		case TEXTURE_NOISE:
            for (i=0; i<size; i++)
                tex->bitmap[i] = rand();
        break;

        case TEXTURE_XOR:
            i = 0;
            for (y=0; y<height; y++)
            {
                for (x=0; x<width; x++)
                {
                    tex->bitmap[i++] = (x ^ y) & 31;
                }
            }
        break;

        case TEXTURE_GRID:
            i = 0;
            for (y=0; y<height; y++)
            {
                yc = y - (height >> 1);
                for (x=0; x<width; x++)
                {
                    xc = x - (width >> 1);
                    c = (xc * xc * xc * xc + yc * yc * yc * yc) >> 21;
                    if (c > 31) c = 31;
                    tex->bitmap[i++] = c;
                }
            }
        break;
    }

    return tex;
}

texture *getTexture(int textureNum)
{
    return textures[textureNum];
}

texture *createTexture(int width, int height, uint8 *bitmap, uint16 *pal)
{
    texture *tex = (texture*)malloc(sizeof(texture));

    tex->width = width;
    tex->height = height;
    tex->bitmap = bitmap;
    tex->palette = pal;

    return tex;
}

void initTextures()
{
    setPal(0,31, 0,0,0, 255,255,255, defaultPal);

    textures[TEXTURE_FLAT] = initTexture(8, 8, defaultPal, TEXTURE_XOR);
	textures[TEXTURE_NOISE] = initTexture(16, 16, defaultPal, TEXTURE_NOISE);
    textures[TEXTURE_XOR] = initTexture(128, 128, defaultPal, TEXTURE_XOR);
    textures[TEXTURE_GRID] = initTexture(128, 128, defaultPal, TEXTURE_GRID);
}
