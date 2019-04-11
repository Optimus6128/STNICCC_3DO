#include "types.h"
#include "tools.h"
#include "bitfonts.h"
#include "main.h"
#include "timerutils.h"

ubyte textSpaceBuffer[TEXT_SPACE_SIZE];
uint16 textSpacePal[32];
CCB *textSpaceCel;
Point textSpaceQuad[4];

int fps = 0, pframe = 0, nframe = 0, atime = 0;

Item timerIOreq;

// -------------------------------------

char sbuffer[256];
unsigned char fonts[59*64];

void initTimer()
{
    timerIOreq = GetTimerIOReq();
}

void initFonts()
{
    int n, x, y;
    int i = 0;

    textSpaceCel = CreateCel(TEXT_SPACE_WIDTH, TEXT_SPACE_HEIGHT, 8, CREATECEL_CODED, textSpaceBuffer);

    for (y = 0; y < 32; y++)
    {
        ubyte c = y;
        if (c==0) c = 1;
        textSpacePal[y] = MakeRGB15(c, c, c);
    }

    textSpaceCel->ccb_SourcePtr = (CelData*)textSpaceBuffer;
    textSpaceCel->ccb_PLUTPtr = (PLUTChunk*)textSpacePal;

	for (n=0; n<59; n++)
	{
		for (y=0; y<8; y++)
		{
			int c = bitfonts[i++];
			for (x=0; x<8; x++)
			{
				fonts[(n << 6) + x + (y<<3)] = ((c >>  (7 - x)) & 1) * 31;
			}
		}
	}
}

void drawFont(int xp, int yp, int ch)
{
    int x,y;
    uint8 *vram;
    if (xp <0 || xp > TEXT_SPACE_WIDTH - 8) return;

    vram = (uint8*)(textSpaceBuffer + xp + yp * TEXT_SPACE_WIDTH);
    for (y=0; y<8; y++)
    {
        int yc = yp + y;
        if ((yc>-1) && (yc<TEXT_SPACE_HEIGHT))
        {
            int yi = y << 3;
            for (x=0; x<8; x++)
            {
                *vram++ = fonts[(ch << 6) + yi + x];
            }
            vram-=8;
        }
        vram+=TEXT_SPACE_WIDTH;
    }
}

void drawText(int xtp, int ytp, int cn, char *text)
{
    int n;
	for (n = 0; n<cn; n++)
	{
		char c = *text++;
        if (c>96 && c<123) c-=32;

   		if (c>31 && c<92) drawFont(xtp, ytp, c - 32);
   			else if (c==0) n = cn;
   		xtp+=8; if (xtp>TEXT_SPACE_WIDTH -8 -1) n = cn; // memory leak needs -1? Still memory leak dot
	}
}

void drawNumber(int xtp, int ytp, int num)
{
    sprintf(sbuffer, "%d", num);
    drawText(xtp, ytp, 10, sbuffer);
}

int getTicks()
{
    //return GetTime(timerIOreq);
    return GetMSecTime(timerIOreq);
}

void SetQuadFromPosAndSize(Point *aQuad, int32 xPos, int32 yPos, int32 width, int32 height)
{
	aQuad[0].pt_X = aQuad[3].pt_X = xPos;
	aQuad[0].pt_Y = aQuad[1].pt_Y = yPos;
	aQuad[1].pt_X = aQuad[2].pt_X = xPos + width;
	aQuad[2].pt_Y = aQuad[3].pt_Y = yPos + height;
}

void clearTextSpace()
{
    memset(textSpaceBuffer, 0, TEXT_SPACE_SIZE);
}

void renderTextSpace()
{
    SetQuadFromPosAndSize(textSpaceQuad, 0, 0, TEXT_SPACE_WIDTH, TEXT_SPACE_HEIGHT);
    MapCel(textSpaceCel, textSpaceQuad);
    drawCels(textSpaceCel);
}

void showFPS()
{
    int i;
    if (getTicks() - atime >= 1000)
    {
        atime = getTicks();
        fps = nframe - pframe;
        pframe = nframe;
    }
    sprintf(sbuffer, "%d", fps);
    drawText(0, 0, 8, sbuffer);
}

/*void showFPS()
{
    int i;
    if (getTicks() - atime >= 2)
    {
        atime = getTicks();
        fps = (nframe - pframe) >> 1; // every 2 seconds
        pframe = nframe;
    }
    sprintf(sbuffer, "%d", fps);
    drawText(0, 0, 8, sbuffer);
}*/

void setPalWithFades(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16* pal, int numFades, int r2, int g2, int b2)
{
    int i, j, rr, gg, bb;
	float rr2, gg2, bb2;
	float ddr, ddg, ddb;
    float dc = (float)(c1 - c0);
    float dr = (float)(r1 - r0) / dc;
    float dg = (float)(g1 - g0) / dc;
    float db = (float)(b1 - b0) / dc;
    float r = (float)r0;
    float g = (float)g0;
    float b = (float)b0;

    pal+=c0;
    for (i=c0; i<=c1; i++)
    {
        rr = (int)r >> 3;
        gg = (int)g >> 3;
        bb = (int)b >> 3;
        *pal = (rr << 10) | (gg << 5) | bb;
		rr2 = r; gg2 = g; bb2 = b;
		ddr = ((float)r2 - r) / (float)numFades;
		ddg = ((float)g2 - g) / (float)numFades;
		ddb = ((float)b2 - b) / (float)numFades;
		for (j=1; j<numFades; j++)
		{
			rr = (int)rr2 >> 3;
			gg = (int)gg2 >> 3;
			bb = (int)bb2 >> 3;
			*(pal + (j << 5)) = (rr << 10) | (gg << 5) | bb;
			rr2 += ddr;
			gg2 += ddg;
			bb2 += ddb;
		}
        r += dr;
        g += dg;
        b += db;
		pal++;
    }
}

void setPal(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16* pal)
{
    int i, rr, gg, bb;
    float dc = (float)(c1 - c0);
    float dr = (float)(r1 - r0) / dc;
    float dg = (float)(g1 - g0) / dc;
    float db = (float)(b1 - b0) / dc;
    float r = (float)r0;
    float g = (float)g0;
    float b = (float)b0;

    pal+=c0;
    for (i=c0; i<=c1; i++)
    {
        rr = (int)r >> 3;
        gg = (int)g >> 3;
        bb = (int)b >> 3;
        *pal++ = (rr << 10) | (gg << 5) | bb;
        r += dr;
        g += dg;
        b += db;
    }
}
