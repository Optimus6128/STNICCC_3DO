#include "system_graphics.h"
#include "sprite_engine.h"
#include "main.h"

sprite *newSprite(int width, int height, ubyte *bmp)
{
    sprite *spr = (sprite*)malloc(sizeof(sprite));

    spr->width = width;
    spr->height = height;

    // Memory leaks. CreateCel with null will make space for new bitmap, while not needed. So 100 sprites with same bitmap = 99 extra needlesly bitmap data malloced.
    spr->cel = CreateCel(width, height, 8, CREATECEL_CODED, bmp);
    // Also, trying free after null creation will not entirely clean sprite. Making abc[1] and giving it even if size 1 worked better.
    //free(spr->cel->ccb_SourcePtr);
    // At the end I ask to the coder to provide source data bmp always in the creation

    spr->cel->ccb_Flags |= CCB_ACSC;
    spr->cel->ccb_Flags |= CCB_ALSC;

    spr->posX = spr->posY = 0;
    spr->angle = 0;
    spr->zoom = 256;

    return spr;
}

void setPalette(sprite *spr, uint16* pal)
{
    spr->cel->ccb_PLUTPtr = (PLUTChunk*)pal;
}

void setSpriteAlpha(sprite *spr, bool enabled)
{
    if (enabled)
        spr->cel->ccb_PIXC = TRANSLUCENT_CEL;
    else
        spr->cel->ccb_PIXC = SOLID_CEL;
}

void makeThemAll(sprite *spr)
{
    int i, px, py;
    int isin, icos;

    int posX = -(((spr->width >> 1) * spr->zoom) >> 8);
    int posY = -(((spr->height >> 1) * spr->zoom) >> 8);
    int sizeX = (spr->width * spr->zoom) >> 8;
    int sizeY = (spr->height * spr->zoom) >> 8;

    //SetQuadFromPosAndSize(spr->quad, posX, posY, sizeX, sizeY);
	spr->quad[kQuadBL].pt_X = spr->quad[kQuadTL].pt_X = posX;
	spr->quad[kQuadTR].pt_Y = spr->quad[kQuadTL].pt_Y = posY;
	spr->quad[kQuadBR].pt_X = spr->quad[kQuadTR].pt_X = posX + sizeX;
	spr->quad[kQuadBL].pt_Y = spr->quad[kQuadBR].pt_Y = posY + sizeY;

    isin = SinF16(spr->angle);
    icos = CosF16(spr->angle);

    for (i=0; i<4; i++)
    {
        px = spr->quad[i].pt_X;
        py = spr->quad[i].pt_Y;

        spr->quad[i].pt_X = ((px * isin + py * icos) >> 16) + spr->posX;
        spr->quad[i].pt_Y = ((px * icos - py * isin) >> 16) + spr->posY;
    }
}

void mapSprite(sprite *spr)
{
    spr->cel->ccb_XPos = spr->posX << 16;
    spr->cel->ccb_YPos = spr->posY << 16;
}

void mapStretchSpriteX(sprite *spr)
{
    spr->cel->ccb_XPos = (spr->posX - (((spr->width >> 1) * spr->zoom) >> 8)) << 16;
    spr->cel->ccb_YPos = (spr->posY - (spr->height >> 1)) << 16;

	spr->cel->ccb_HDX = spr->zoom<<12;
	//spr->cel->ccb_VDY = spr->zoom<<8;
}

void mapStretchSpriteY(sprite *spr)
{
    spr->cel->ccb_XPos = (spr->posX - (spr->width >> 1)) << 16;
    spr->cel->ccb_YPos = (spr->posY - (((spr->height >> 1) * spr->zoom) >> 8)) << 16;

	//spr->cel->ccb_HDX = spr->zoom<<12;
	spr->cel->ccb_VDY = spr->zoom<<8;
}

void mapZoomSprite(sprite *spr)
{
    spr->cel->ccb_XPos = (spr->posX - (((spr->width >> 1) * spr->zoom) >> 8)) << 16;
    spr->cel->ccb_YPos = (spr->posY - (((spr->height >> 1) * spr->zoom) >> 8)) << 16;

	spr->cel->ccb_HDX = spr->zoom<<12;
	spr->cel->ccb_VDY = spr->zoom<<8;
}

void mapZoomRotateSprite(sprite *spr)
{
    makeThemAll(spr);

    spr->cel->ccb_XPos = spr->quad[kQuadTL].pt_X << 16;
    spr->cel->ccb_YPos = spr->quad[kQuadTL].pt_Y << 16;

	spr->cel->ccb_HDX = ((spr->quad[kQuadTR].pt_X - spr->quad[kQuadTL].pt_X) << 20) >> 5; // / spr->width;
    spr->cel->ccb_HDY = ((spr->quad[kQuadTR].pt_Y - spr->quad[kQuadTL].pt_Y) << 20) >> 5; // / spr->width;
    spr->cel->ccb_VDX = ((spr->quad[kQuadBL].pt_X - spr->quad[kQuadTL].pt_X) << 16) >> 5; // / spr->height;
	spr->cel->ccb_VDY = ((spr->quad[kQuadBL].pt_Y - spr->quad[kQuadTL].pt_Y) << 16) >> 5; // / spr->height;
}

void drawSprite(sprite *spr)
{
    drawCels(spr->cel);
}
