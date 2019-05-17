#include "ScriptMain.h"
#include "scene1.h"
#include "main.h"
#include "system_graphics.h"
#include "tools.h"
#include "sound.h"

#define MAX_POLYS 256
#define ANIM_WIDTH 256
#define ANIM_HEIGHT 200
#define ANIM_SIZE (ANIM_WIDTH * ANIM_HEIGHT)

#define DIV_TAB_SIZE 4096
#define DIV_TAB_SHIFT 16

#define ATARI_PAL_NUM 16
#define MAX_POLYGON_PTS 16

static int32 divTab[DIV_TAB_SIZE];

static uint32 block64index = 0;

static uchar *data = &scene1_bin[0];

static ushort pal16[ATARI_PAL_NUM];
static MyPoint2D pt[MAX_POLYGON_PTS];

static ushort texPals[ATARI_PAL_NUM][32];

static QuadStore quads[MAX_POLYS];
static QuadStore *quadPtr;
static int numQuads = 0;

static int leftEdgeFlat[SCREEN_HEIGHT];
static int rightEdgeFlat[SCREEN_HEIGHT];

static CCB *polys[MAX_POLYS];
static CCB polysFlat[MAX_POLYS];
static CCB polysTexture[MAX_POLYS];
static MyPoint2D vi[256];

static uchar *texBufferFlat[ATARI_PAL_NUM];
static const int flatTexWidth = 1;  static const int flatTexWidthShr = 0;
static const int flatTexHeight = 8;  static const int flatTexHeightShr = 3;
static const int flatTexStride = 8;

static bool mustClearScreen = false;

static const int animPosX = (SCREEN_WIDTH - ANIM_WIDTH) / 2;
static const int animPosY = (SCREEN_HEIGHT - ANIM_HEIGHT) / 2 - 8;

static char stbuffer[10];
static char avgfpsbuffer[16];
static char texnumbuffer[8];

static bool firstTime = true;
static bool endOfBench = false;
static int startBenchTime;
static int frameNum = 0;

static CCB *bufferCel8;
static uchar buffer8[ANIM_SIZE];

#define NUM_BENCH_TEXTURES 4
#define NUM_BENCH_KINDS (1 + NUM_BENCH_TEXTURES)
static int texNum = 2;
static int texSize[NUM_BENCH_TEXTURES] =    { 16, 32, 64, 128 };
static int texShr[NUM_BENCH_TEXTURES] =     { 4,  5,  6,  7 };
static uchar *texBuffer[NUM_BENCH_TEXTURES];
static char* benchKindText[NUM_BENCH_KINDS] = { "flat", "16x16", "32x32", "64x64", "128x128" };

static int benchKind = 0;   // 0 = FLAT, 1-4 = TEXTURED (texNum = benchKind-1)

#define NUM_BENCH_FRAMES 6
static int benchFrame[NUM_BENCH_FRAMES] = { 0, 110, 525, 750, 1500, 1661 };
static int benchFrameStatQuads[NUM_BENCH_FRAMES] = { 164, 40, 63, 60, 89, 56 };
static int benchFrameStatCoverage[NUM_BENCH_FRAMES] = { 17, 96, 111, 130, 103, 168 };

static int benchFrameFps[NUM_BENCH_FRAMES][NUM_BENCH_KINDS];
static int maxFrameFps[NUM_BENCH_FRAMES];
static int benchFrameIndex = 0;
static CCB *lastQuadCCB;

static bool timeForResults = false;


void initDivs()
{
    int i, ii;
    for (i=0; i<DIV_TAB_SIZE; ++i) {
        ii = i - DIV_TAB_SIZE / 2;
        if (i==0) ++ii;

        divTab[i] = (1 << DIV_TAB_SHIFT) / ii;
    }
}

static void prepareEdgeListFlat(MyPoint2D *p0, MyPoint2D *p1)
{
	int *edgeListToWriteFlat;
	MyPoint2D *pTemp;

	if (p0->y == p1->y) return;

	// Assumes CCW
	if (p0->y < p1->y) {
		edgeListToWriteFlat = leftEdgeFlat;
	}
	else {
		edgeListToWriteFlat = rightEdgeFlat;

		pTemp = p0;
		p0 = p1;
		p1 = pTemp;
	}

    {
        const int x0 = p0->x; const int y0 = p0->y;
        const int x1 = p1->x; const int y1 = p1->y;

        const int dx = ((x1 - x0) * divTab[y1 - y0 + DIV_TAB_SIZE / 2]) >>  (DIV_TAB_SHIFT - FP_BITS);

        int xp = INT_TO_FIXED(x0, FP_BITS);
        int count = y1 - y0;

        edgeListToWriteFlat = &edgeListToWriteFlat[y0];

        do
        {
            *edgeListToWriteFlat++ = FIXED_TO_INT(xp, FP_BITS);
            xp += dx;
        } while(count-- != 0);
    }
}

void drawFlatQuad8(MyPoint2D *p, uchar color, uchar *screen)
{
	const int y0 = p[0].y;
	const int y1 = p[1].y;
	const int y2 = p[2].y;
	const int y3 = p[3].y;

	const int scrWidth = SCREEN_WIDTH;
	const int scrHeight = SCREEN_HEIGHT;

	int yMin = y0;
	int yMax = yMin;

	uchar *dst;
	int y;


	if (y1 < yMin) yMin = y1;
	if (y1 > yMax) yMax = y1;
	if (y2 < yMin) yMin = y2;
	if (y2 > yMax) yMax = y2;
	if (y3 < yMin) yMin = y3;
	if (y3 > yMax) yMax = y3;

	if (yMin < 0) yMin = 0;
	if (yMax > scrHeight - 1) yMax = scrHeight - 1;


	prepareEdgeListFlat(&p[0], &p[1]);
	prepareEdgeListFlat(&p[1], &p[2]);
	prepareEdgeListFlat(&p[2], &p[3]);
	prepareEdgeListFlat(&p[3], &p[0]);


	for (y = yMin; y <= yMax; y++)
	{
		int xl = leftEdgeFlat[y];
		int xr = rightEdgeFlat[y];

		if (xl < 0) xl = 0;
		if (xr > scrWidth - 1) xr = scrWidth - 1;

		if (xl == xr) ++xr;
        dst = screen + (y << 8) + xl;
        memset(dst, color, xr-xl);
	}
}

void initBenchTextures(bool cyber)
{
    int x,y,i,n,c,xc,yc;

    for (n=0; n<NUM_BENCH_TEXTURES; ++n) {
        const int width = texSize[n];
        const int height = texSize[n];
        texBuffer[n] = (uchar*)malloc(width * height * sizeof(uchar));

        i = 0;
        if (cyber) {
            for (y=0; y<height; y++)
            {
                yc = y - (height >> 1);
                for (x=0; x<width; x++)
                {
                    xc = x - (width >> 1);
                    c = (xc * xc * xc * xc + yc * yc * yc * yc) >> (7 + n * 4);
                    if (c > 31) c = 31;
                    texBuffer[n][i++] = c;
                }
            }
        } else {
            for (y=0; y<height; ++y) {
                for (x=0; x<width; ++x) {
                    texBuffer[n][i++] = (x^y) & 31;
                }
            }
        }
    }
}

void initCCBpolysFlat()
{
    static bool mustBuildFlatTextures = true;

	CCB *CCBPtr;
    int i;

    if (mustBuildFlatTextures) {
        const int flatTextSize = flatTexStride * flatTexHeight;
        for (i=0; i<ATARI_PAL_NUM; ++i) {
            texBufferFlat[i] = (uchar*)malloc(flatTextSize * sizeof(uchar));  // creating 1x2 texture but with 4 bytes stride
            memset(texBufferFlat[i], i, flatTextSize);
        }
        mustBuildFlatTextures = false;
    }


    CCBPtr = &polysFlat[0];
    for (i=0; i<MAX_POLYS; ++i) {
		CCBPtr->ccb_NextPtr = (CCB*)(sizeof(CCB)-8);	// Create the next offset

        CCBPtr->ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_PPABS|CCB_LDPLUT|CCB_USEAV;
        CCBPtr->ccb_PIXC = 0x1F00;
        CCBPtr->ccb_PRE0 = 0x00000005 | ((flatTexHeight - 1) << 6);
        CCBPtr->ccb_PRE1 = (((flatTexStride >> 2) - 2) << 16) | (flatTexWidth - 1);

        CCBPtr->ccb_PLUTPtr = (PLUTChunk*)pal16;

        polys[i] = CCBPtr++;
    }
}

void initCCBPolysTexture()
{
	CCB *CCBPtr;
    int i;
    const int texWidth = texSize[texNum];
    const int texHeight = texWidth;

    CCBPtr = &polysTexture[0];
    for (i=0; i<MAX_POLYS; ++i) {
		CCBPtr->ccb_NextPtr = (CCB*)(sizeof(CCB)-8);	// Create the next offset

        CCBPtr->ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_PPABS|CCB_LDPLUT|CCB_USEAV;
        CCBPtr->ccb_PIXC = 0x1F00;
        CCBPtr->ccb_PRE0 = 0x00000005 | ((texHeight - 1) << 6);
        CCBPtr->ccb_PRE1 = (((texWidth >> 2) - 2) << 16) | (texWidth - 1);

        CCBPtr->ccb_SourcePtr = (CelData*)texBuffer[texNum];

        polys[i] = CCBPtr++;
    }
}

void initCCBbuffers()
{
    bufferCel8 = CreateCel(ANIM_WIDTH, ANIM_HEIGHT, 8, CREATECEL_CODED, buffer8);
    bufferCel8->ccb_PLUTPtr = (PLUTChunk*)pal16;

    bufferCel8->ccb_XPos = animPosX << 16;
    bufferCel8->ccb_YPos = animPosY << 16;

    bufferCel8->ccb_Flags |= CCB_NOBLK;
}

static void addPolygon(int numVertices, int paletteIndex)
{
	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

    ushort color;

    if (paletteIndex < 0) paletteIndex = 0;

    color = paletteIndex;

	if (numVertices < 3 || numVertices > 16) return;

	while(pStartIndex < maxIndex)
	{
		quadPtr->p0.x = pt[pBaseIndex].x;		quadPtr->p0.y = pt[pBaseIndex].y;
		quadPtr->p1.x = pt[pStartIndex].x;		quadPtr->p1.y = pt[pStartIndex].y;
		quadPtr->p2.x = pt[pStartIndex+1].x;	quadPtr->p2.y = pt[pStartIndex + 1].y;

		pStartIndex += 2;
		if (pStartIndex > maxIndex) pStartIndex = maxIndex;
		quadPtr->p3.x = pt[pStartIndex].x;		quadPtr->p3.y = pt[pStartIndex].y;

		quadPtr->c = color;

		++quadPtr;
		++numQuads;
	}
}

static void renderPolygonsFlat(QuadStore *q, CCB *cel, int num)
{
    int i;
    CCB *startingCel = cel;

    if (num==0) return;

	for (i=0; i<num; ++i) {
        const MyPoint2D *p0 = &q->p0;
        const MyPoint2D *p1 = &q->p1;
        const MyPoint2D *p2 = &q->p2;
        const MyPoint2D *p3 = &q->p3;

        const int ptX0 = p1->x - p0->x;
        const int ptY0 = p1->y - p0->y;
        const int ptX1 = p2->x - p3->x;
        const int ptY1 = p2->y - p3->y;
        const int ptX2 = p3->x - p0->x;
        const int ptY2 = p3->y - p0->y;

        const int hdx0 = (ptX0 << 20) >> flatTexWidthShr;
        const int hdy0 = (ptY0 << 20) >> flatTexWidthShr;
        const int hdx1 = (ptX1 << 20) >> flatTexWidthShr;
        const int hdy1 = (ptY1 << 20) >> flatTexWidthShr;

        cel->ccb_XPos = p0->x<<16;
        cel->ccb_YPos = p0->y<<16;

        cel->ccb_HDX = hdx0;
        cel->ccb_HDY = hdy0;
        cel->ccb_VDX = (ptX2 << 16) >> flatTexHeightShr;
        cel->ccb_VDY = (ptY2 << 16) >> flatTexHeightShr;

        cel->ccb_HDDX = (hdx1 - hdx0) >> flatTexHeightShr;
        cel->ccb_HDDY = (hdy1 - hdy0) >> flatTexHeightShr;

        cel->ccb_SourcePtr = (CelData*)texBufferFlat[q->c];

        ++q;
        ++cel;
	}

	lastQuadCCB = --cel;
	lastQuadCCB->ccb_Flags |= CCB_LAST;
	drawCels(startingCel);
	lastQuadCCB->ccb_Flags ^= CCB_LAST;
}

static void renderPolygonsTextured(QuadStore *q, CCB *cel, int num)
{
    int i;
    CCB *startingCel = cel;

    const int shrWidth = texShr[texNum];
    const int shrHeight = texShr[texNum];

    if (num==0) return;

	for (i=0; i<num; ++i) {
        const MyPoint2D *p0 = &q->p0;
        const MyPoint2D *p1 = &q->p1;
        const MyPoint2D *p2 = &q->p2;
        const MyPoint2D *p3 = &q->p3;

        const int ptX0 = p1->x - p0->x;
        const int ptY0 = p1->y - p0->y;
        const int ptX1 = p2->x - p3->x;
        const int ptY1 = p2->y - p3->y;
        const int ptX2 = p3->x - p0->x;
        const int ptY2 = p3->y - p0->y;

        const int hdx0 = (ptX0 << 20) >> shrWidth;
        const int hdy0 = (ptY0 << 20) >> shrWidth;
        const int hdx1 = (ptX1 << 20) >> shrWidth;
        const int hdy1 = (ptY1 << 20) >> shrWidth;

        cel->ccb_XPos = p0->x<<16;
        cel->ccb_YPos = p0->y<<16;

        cel->ccb_HDX = hdx0;
        cel->ccb_HDY = hdy0;
        cel->ccb_VDX = (ptX2 << 16) >> shrHeight;
        cel->ccb_VDY = (ptY2 << 16) >> shrHeight;

        cel->ccb_HDDX = (hdx1 - hdx0) >> shrHeight;
        cel->ccb_HDDY = (hdy1 - hdy0) >> shrHeight;

        cel->ccb_PLUTPtr = (PLUTChunk*)texPals[q->c];

        ++q;
        ++cel;
	}

	lastQuadCCB = --cel;
	lastQuadCCB->ccb_Flags |= CCB_LAST;
	drawCels(startingCel);
	lastQuadCCB->ccb_Flags ^= CCB_LAST;
}

static void renderPolygonsSoftware8()
{
    int i;
    static MyPoint2D p[4];
    uchar *screen = (uchar*)bufferCel8->ccb_SourcePtr;

	for (i=0; i<numQuads; ++i) {
		p[0].x = quads[i].p0.x; p[0].y = quads[i].p0.y;
		p[1].x = quads[i].p1.x; p[1].y = quads[i].p1.y;
		p[2].x = quads[i].p2.x; p[2].y = quads[i].p2.y;
		p[3].x = quads[i].p3.x; p[3].y = quads[i].p3.y;
        drawFlatQuad8(&p[0], (uchar)quads[i].c, screen);
	}

	drawCels(bufferCel8);
}

static void renderPolygons()
{
    int i;

    // Because hardware screen offset doesn't work yet here, I temporary fix it with this code
    // I know it works on Doom 3DO, but maybe some more things have to be enabled on the VideoItems
    for (i=0; i<numQuads; ++i) {
        quads[i].p0.x += animPosX; quads[i].p0.y += animPosY;
        quads[i].p1.x += animPosX; quads[i].p1.y += animPosY;
        quads[i].p2.x += animPosX; quads[i].p2.y += animPosY;
        quads[i].p3.x += animPosX; quads[i].p3.y += animPosY;
    }

    //setScreenClipping(animPosX, animPosY, ANIM_WIDTH, ANIM_HEIGHT);

    if (benchKind == 0)
        renderPolygonsFlat(quads, polys[0], numQuads);
    else
        renderPolygonsTextured(quads, polys[0], numQuads);

    //resetScreenClipping();
}

static void interpretPaletteData()
{
    int i, r, g, b;
    uchar bitmaskH = *data++;
	uchar bitmaskL = *data++;
	ushort c;

	int bitmask = (bitmaskH << 8) | bitmaskL;

	for (i = 0; i < 16; ++i) {
		int palNum = i;
		if (bitmask & 0x8000) {
			uchar colorH = *data++;
			uchar colorL = *data++;
			int color = (colorH << 8) | colorL;

			r = (color >> 8) & 7;
			g = (color >> 4) & 7;
			b = color & 7;

			c = (r << 12) | (g << 7) | (b << 2);

			pal16[palNum] = c;
			if (benchTexture || benchScreens) {
                setPal(0,31, 0,0,0, r<<5, g<<5, b<<5, texPals[palNum]);
			}
		}
		bitmask <<= 1;
	}
}

static void interpretDescriptorSpecial(uchar descriptor)
{
	switch (descriptor)
	{
        case 0xff:
        {
            // End of Frame
        }
        break;

        case 0xfe:
        {
            // End of frame and skip at next 64k block

            ++block64index;
            data = &scene1_bin[block64index << 16];
        }
        break;

        case 0xfd:
        {
            // That's all folks!

            // restart
            data = &scene1_bin[0];
            block64index = 0;
            endOfBench = true;
        }
        break;
	}
}

static void interpretIndexedMode()
{
    int i, n;

	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	int vertexNum = *data++;

	for (i = 0; i < vertexNum; ++i) {
		vi[i].x = (int)*data++;
		vi[i].y = (int)*data++;
	}

	while(true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

        polyPaletteIndex = (int)(descriptor >> 4) & 15;
        polyNumVertices = (int)(descriptor & 15);

		for (n = 0; n < polyNumVertices; ++n) {
			int vertexId = *data++;
			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}

		addPolygon(polyNumVertices, polyPaletteIndex);
	}

	interpretDescriptorSpecial(descriptor);
}

static void interpretNonIndexedMode()
{
    int n;
	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	while (1) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

        polyPaletteIndex = (int)(descriptor >> 4);
        polyNumVertices = (int)(descriptor & 15);

		for (n = 0; n < polyNumVertices; ++n) {
			pt[n].x = *data++;
			pt[n].y = *data++;
		}
		addPolygon(polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void decodeFrame()
{
	uchar flags = *data++;

	mustClearScreen = false;
	numQuads = 0;
	quadPtr = &quads[0];

	if (flags & 1) {
		mustClearScreen = true;
	}
	if (flags & 2) {
		interpretPaletteData();
	}
	if (flags & 4) {
		interpretIndexedMode();
	}
	else {
		interpretNonIndexedMode();
	}
}

void hackNumToTwoDigitChars(char *buff, int num)    // no time to think of a better way
{
    if (num < 10) {
        buff[0] = '0';
        buff[1] = 48 + num;
    } else {
        sprintf(buff, "%d", num);
    }
}

void drawTimer()
{
    static int min, sec, mls, avgfps;
    int elapsed = getTicks() - startBenchTime;

    if (endOfBench) {
        int c = elapsed >> 5;
        setFontColor(MakeRGB15(c, c, c));
    } else {
        min = elapsed / 60000;
        sec = (elapsed / 1000) % 60;
        mls = (elapsed % 1000) / 10;
        avgfps = (frameNum * 1000) / elapsed;
    }

    clearScreenWithRect(128, 224, 64, 8, BG_COLOR);

    hackNumToTwoDigitChars(&stbuffer[0], min);
    stbuffer[2] = ':';
    hackNumToTwoDigitChars(&stbuffer[3], sec);
    stbuffer[5] = ':';
    hackNumToTwoDigitChars(&stbuffer[6], mls);

    drawText(128, 224, stbuffer);

    if (endOfBench) {
        setFontColor(MakeRGB15(31, 24, 16));
        sprintf(avgfpsbuffer, "Avg FPS: %d", avgfps);
        drawText(216, 224, avgfpsbuffer);
        setFontColor(MakeRGB15(31, 31, 31));
    }
}

static void benchTextureControls()
{
    benchKind = 1;  // to force texture pal in this mode

    if (PressedLonce) {
        --texNum;
        if (texNum < 0) texNum = NUM_BENCH_TEXTURES - 1;
        initCCBPolysTexture();
        PressedLonce = false;
    }
    if (PressedRonce) {
        ++texNum;
        if (texNum==NUM_BENCH_TEXTURES) texNum = 0;
        initCCBPolysTexture();
        PressedRonce = false;
    }
    sprintf(texnumbuffer, "%dX%d", texSize[texNum], texSize[texNum]);
    clearScreenWithRect(0, SCREEN_HEIGHT - 8, 64, 8, BG_COLOR);
    drawText(0, SCREEN_HEIGHT - 8, texnumbuffer);
}

static void benchFrameEndScreen()
{
    static int timara = 0;
    int i, j, cy, perOff = 2;
    const int lineCol = (15 << 5) | 31;
    const int barCol = (24 << 10) | (20 << 5);

    clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 7 << 5);

    cy = 0;
    clearScreenWithRect(100, 0, 2, SCREEN_HEIGHT, lineCol);

    for (j=0; j<NUM_BENCH_FRAMES; ++j) {
        if (cy!=0) clearScreenWithRect(0, cy-1, SCREEN_WIDTH, 1, lineCol);
        drawText(0, cy+4, "Frame"); drawNumber(48, cy+4, benchFrame[j]);
        drawText(0, cy+4+FONT_HEIGHT, "Polys"); drawNumber(48, cy+4+FONT_HEIGHT, benchFrameStatQuads[j]);
        drawText(0, cy+4+2*FONT_HEIGHT, "Cover"); drawNumber(48, cy+4+2*FONT_HEIGHT, benchFrameStatCoverage[j]);
        if (benchFrameStatCoverage[j] >= 100) perOff = 3;
        drawText(48+perOff*FONT_WIDTH, cy+4+2*FONT_HEIGHT, "%");

        for (i=0; i<NUM_BENCH_KINDS; ++i) {
            int length = (((benchFrameFps[j][i] * (252 - 162)) / maxFrameFps[j]) * timara) >> 6;
            clearScreenWithRect(162, cy+1, length, 6, barCol);
            drawText(102, cy, benchKindText[i]);
            drawNumber(256, cy, benchFrameFps[j][i]);
            cy+=FONT_HEIGHT;
        }
    }
    ++timara;
    if (timara  > 64) timara = 64;
    vsync = true;
}

static void benchFrameLoop()
{
    int i;
    const int benchFrameTicks = 4000;

    timeForResults = endOfBench;
    fpsOn = !timeForResults;

    if (benchFrameIndex==NUM_BENCH_FRAMES) {
            timeForResults = true;
            fpsOn = false;
        return;
    }

    if (benchFrame[benchFrameIndex] == frameNum) {
        for (i=0; i<NUM_BENCH_KINDS; ++i) {
            int startTicks = getTicks();
            int benchRepeatsNum = 0;

            benchKind = i;
            if (benchKind == 0) {
                initCCBpolysFlat();
                renderPolygons();
            } else {
                texNum = benchKind - 1;
                initCCBPolysTexture();
                renderPolygons();
            }

            clearAllScreens(0);
            do {
                if (benchRepeatsNum < NUM_SCREEN_PAGES) clearScreenWithRect(animPosX, animPosY, ANIM_WIDTH, ANIM_HEIGHT, 0);

                lastQuadCCB->ccb_Flags |= CCB_LAST;
                drawCels(polys[0]);
                lastQuadCCB->ccb_Flags ^= CCB_LAST;

                showFPS();
                displayScreen();

                ++benchRepeatsNum;
            } while(getTicks() - startTicks <= benchFrameTicks);

            benchFrameFps[benchFrameIndex][i] = (benchRepeatsNum * 1000) / benchFrameTicks;
            if (i==0) {
                maxFrameFps[benchFrameIndex] = benchFrameFps[benchFrameIndex][i];
            } else {
                if (maxFrameFps[benchFrameIndex] < benchFrameFps[benchFrameIndex][i]) {
                    maxFrameFps[benchFrameIndex] = benchFrameFps[benchFrameIndex][i];
                }
            }
        }
        benchKind = 0;
        initCCBpolysFlat();
        renderPolygons();
        ++benchFrameIndex;
    }
}

void runAnimationScript()
{
    if (timeForResults) {
        benchFrameEndScreen();
    } else {

        if (firstTime) {
            startBenchTime = getTicks();
            firstTime = false;
        }

        if (benchTexture) benchTextureControls();

        if (demo & (frameNum & 1)) {

        } else {
            decodeFrame();
        }

        if (benchScreens) benchFrameLoop();

        if (mustClearScreen) clearScreenWithRect(animPosX, animPosY, ANIM_WIDTH, ANIM_HEIGHT, 0);

        if (gpuOn) {
            renderPolygons();
        } else {
            if (mustClearScreen) memset(buffer8, 0, ANIM_SIZE);
            renderPolygonsSoftware8();
        }

        if (!demo && !benchScreens) drawTimer();

        ++frameNum;
    }
}
