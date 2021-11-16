#include "types.h"
#include "tools.h"
#include "bitfonts.h"
#include "main.h"
#include "timerutils.h"
#include "system_graphics.h"

#define MAX_STRING_LENGTH 64
#define NUM_FONTS 59


// -------------------------------------

static CCB *textCel[MAX_STRING_LENGTH];

static char sbuffer[MAX_STRING_LENGTH+1];
static uchar fontsBmp[NUM_FONTS * FONT_SIZE];
static uint16 fontsPal[32];
static uchar fontsMap[256];


// -------------------------------------

static int fps = 0, pframe = 0, nframe = 0, atime = 0;
static Item timerIOreq;

static int charPosX = 0;
static int charPosY = 0;


void initTimer()
{
    timerIOreq = GetTimerIOReq();
}

void initFonts()
{
    int i = 0;
    int n, x, y;

    for (n=0; n<32; ++n) {
        fontsPal[n] = MakeRGB15(n, n, n);
    }

	for (n=0; n<59; n++) {
		for (y=0; y<8; y++) {
			int c = bitfonts[i++];
			for (x=0; x<8; x++) {
				fontsBmp[(n << 6) + x + (y<<3)] = ((c >>  (7 - x)) & 1) * 31;
			}
		}
	}

	for (i=0; i<256; ++i) {
        uchar c = i;

        if (c>31 && c<92)
            c-=32;
        else
            if (c>96 && c<123) c-=64;
        else
            c = 255;

        fontsMap[i] = c;
	}

    for (i=0; i<MAX_STRING_LENGTH; ++i) {
        textCel[i] = CreateCel(FONT_WIDTH, FONT_HEIGHT, 8, CREATECEL_CODED, fontsBmp);
        textCel[i]->ccb_PLUTPtr = (PLUTChunk*)fontsPal;

        textCel[i]->ccb_HDX = 1 << 20;
        textCel[i]->ccb_HDY = 0 << 20;
        textCel[i]->ccb_VDX = 0 << 16;
        textCel[i]->ccb_VDY = 1 << 16;

        textCel[i]->ccb_Flags |= (CCB_ACSC | CCB_ALSC);

        if (i > 0) LinkCel(textCel[i-1], textCel[i]);
    }
}

void drawText(int xtp, int ytp, char *text)
{
    int i = 0;
    char c;

    do {
        c = fontsMap[*text++];

        textCel[i]->ccb_XPos = xtp << 16;
        textCel[i]->ccb_YPos = ytp << 16;

        textCel[i]->ccb_SourcePtr = (CelData*)&fontsBmp[c * FONT_SIZE];

        xtp+=8;
        ++charPosX;
    } while(c!=255 && ++i < MAX_STRING_LENGTH);

    --i;
	textCel[i]->ccb_Flags |= CCB_LAST;
	drawCels(textCel[0]);
	textCel[i]->ccb_Flags ^= CCB_LAST;
}

void drawZoomedText(int xtp, int ytp, char *text, int zoom)
{
    int i = 0;
    char c;

    do {
        c = fontsMap[*text++];

        textCel[i]->ccb_XPos = xtp  << 16;
        textCel[i]->ccb_YPos = ytp  << 16;

        textCel[i]->ccb_HDX = (zoom << 20) >> 8;
        textCel[i]->ccb_VDY = (zoom << 16) >> 8;

        textCel[i]->ccb_SourcePtr = (CelData*)&fontsBmp[c * FONT_SIZE];

        xtp+= ((zoom * 8) >> 8);
        ++charPosX;
    } while(c!=255 && ++i < MAX_STRING_LENGTH);

    --i;
	textCel[i]->ccb_Flags |= CCB_LAST;
	drawCels(textCel[0]);
	textCel[i]->ccb_Flags ^= CCB_LAST;
}

void resetCharPos()
{
    charPosX = 0;
    charPosY = 0;
}

void setFontColor(ushort c)
{
    fontsPal[31] = c;
}

void drawNumber(int xtp, int ytp, int num)
{
    sprintf(sbuffer, "%d", num);
    drawText(xtp, ytp, sbuffer);
}

int getTicks()
{
    //return GetTime(timerIOreq);
    return GetMSecTime(timerIOreq);
}

void showFPS()
{
    const int posX = 0;
    const int posY = 0;

    if (getTicks() - atime >= 1000)
    {
        atime = getTicks();
        fps = nframe - pframe;
        pframe = nframe;
    }
    sprintf(sbuffer, "%d", fps);
    drawText(posX, posY, sbuffer);
    ++nframe;
}

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
