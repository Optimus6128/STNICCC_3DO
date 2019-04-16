#include "ScriptMain.h"
#include "scene1.h"
#include "main.h"
#include "system_graphics.h"
#include "tools.h"
#include "sound.h"

#define MAX_POLYS 256
#define ANIM_WIDTH 256
#define ANIM_HEIGHT 200

static unsigned int block64index = 0;

static uchar *data = &scene1_bin[0];

static int pal32[16];
static MyPoint2D pt[16];

static QuadStore quads[MAX_POLYS];
static QuadStore *quadPtr;
static int numQuads = 0;

static int leftEdgeFlat[SCREEN_HEIGHT];
static int rightEdgeFlat[SCREEN_HEIGHT];

static CCB polys[MAX_POLYS];
static MyPoint2D vi[256];

static bool mustClearScreen = false;

const int animPosX = (SCREEN_WIDTH - ANIM_WIDTH) / 2;
const int animPosY = (SCREEN_HEIGHT - ANIM_HEIGHT) / 2 - 8;

static char stbuffer[10];
static char avgfpsbuffer[16];

static bool firstTime = true;
static bool endOfBench = false;
static int startBenchTime;
static int frameNum = 0;

static void prepareEdgeListFlat(MyPoint2D *p0, MyPoint2D *p1)
{
	int *edgeListToWriteFlat;
	MyPoint2D *pTemp;

	if (p0->y == p1->y) return;

	// Assumes CCW
	if (p0->y < p1->y) {
		edgeListToWriteFlat = &leftEdgeFlat[0];
	}
	else {
		edgeListToWriteFlat = &rightEdgeFlat[0];

		pTemp = p0;
		p0 = p1;
		p1 = pTemp;
	}

    {
        const int x0 = p0->x; const int y0 = p0->y;
        const int x1 = p1->x; const int y1 = p1->y;

        const int dx = INT_TO_FIXED(x1 - x0, FP_BITS) / (y1 - y0);

        int xp = INT_TO_FIXED(x0, FP_BITS);
        int yp = y0;
        do
        {
            if (yp >= 0 && yp < SCREEN_HEIGHT)
            {
                edgeListToWriteFlat[yp] = FIXED_TO_INT(xp, FP_BITS);
            }
            xp += dx;

        } while (yp++ != y1);
    }
}

void drawFlatQuad(MyPoint2D *p, ushort color, ushort *screen)
{
	const int y0 = p[0].y;
	const int y1 = p[1].y;
	const int y2 = p[2].y;
	const int y3 = p[3].y;

	const int scrWidth = SCREEN_WIDTH;
	const int scrHeight = SCREEN_HEIGHT;

	int yMin = y0;
	int yMax = yMin;

	ushort *dst;
	int x,y;

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

        dst = screen + (y >> 1) * (scrWidth << 1) + (y & 1) + (xl << 1);
		for (x = xl; x < xr; ++x) {
			*dst = color;
			dst += 2;
		}
	}
}


void initCCBpolys()
{
	CCB *CCBPtr = &polys[0];
	int i;

	for (i=0; i<MAX_POLYS; ++i) {
		CCBPtr->ccb_NextPtr = (CCB*)(sizeof(CCB)-8);	// Create the next offset
		//CCBPtr->ccb_NextPtr = (CCB*)(sizeof(CCB));	// Create the next offset

		// Set all the defaults
        CCBPtr->ccb_Flags = CCB_LDSIZE|CCB_LDPRS|CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|CCB_ACE|CCB_BGND|CCB_NOBLK;
        CCBPtr->ccb_PIXC = 0x1F00;
        CCBPtr->ccb_PRE0 = 0x40000016;
        CCBPtr->ccb_PRE1 = 0x03FF1000;
        CCBPtr->ccb_SourcePtr = (CelData*)0;

		++CCBPtr;
	}
}

static void addPolygon(int numVertices, int paletteIndex)
{
	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

    ushort color;

    if (paletteIndex < 0) paletteIndex = 0;

    color = pal32[paletteIndex];

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

static void mapCelToFlatQuad(CCB *c, MyPoint2D *q, ushort color)
{
	const int ptX0 = q[1].x - q[0].x;
	const int ptY0 = q[1].y - q[0].y;
	const int ptX1 = q[2].x - q[3].x;
	const int ptY1 = q[2].y - q[3].y;
	const int ptX2 = q[3].x - q[0].x;
	const int ptY2 = q[3].y - q[0].y;

	const int hdx0 = ptX0 << 20;
	const int hdy0 = ptY0 << 20;
	const int hdx1 = ptX1 << 20;
	const int hdy1 = ptY1 << 20;

	c->ccb_XPos = q[0].x<<16;
	c->ccb_YPos = q[0].y<<16;

	c->ccb_HDX = hdx0;
	c->ccb_HDY = hdy0;
    c->ccb_VDX = ptX2 << 16;
	c->ccb_VDY = ptY2 << 16;

	c->ccb_HDDX = hdx1 - hdx0;
	c->ccb_HDDY = hdy1 - hdy0;

	c->ccb_PLUTPtr = (void *)((unsigned int)color<<16);
}

static void renderPolygonsSoftware()
{
    int i;
    static MyPoint2D p[4];
    ushort *screen;

    if (numQuads==0) return;

    screen = getVideoramAddress();

	for (i=0; i<numQuads; ++i) {
		p[0].x = quads[i].p0.x + animPosX; p[0].y = quads[i].p0.y + animPosY;
		p[1].x = quads[i].p1.x + animPosX; p[1].y = quads[i].p1.y + animPosY;
		p[2].x = quads[i].p2.x + animPosX; p[2].y = quads[i].p2.y + animPosY;
		p[3].x = quads[i].p3.x + animPosX; p[3].y = quads[i].p3.y + animPosY;
        drawFlatQuad(&p[0], quads[i].c, screen);
	}
}
static void renderPolygons()
{
    int i;
    static MyPoint2D p[4];

    CCB *quadCCB = &polys[0];

    if (numQuads==0) return;

	for (i=0; i<numQuads; ++i) {
		p[0].x = quads[i].p0.x + animPosX; p[0].y = quads[i].p0.y + animPosY;
		p[1].x = quads[i].p1.x + animPosX; p[1].y = quads[i].p1.y + animPosY;
		p[2].x = quads[i].p2.x + animPosX; p[2].y = quads[i].p2.y + animPosY;
		p[3].x = quads[i].p3.x + animPosX; p[3].y = quads[i].p3.y + animPosY;
		mapCelToFlatQuad(quadCCB, p, quads[i].c);
		++quadCCB;
	}
	--quadCCB;
	quadCCB->ccb_Flags |= CCB_LAST;
	drawCels(polys);
	quadCCB->ccb_Flags ^= CCB_LAST;
}

static void interpretPaletteData()
{
    int i, r, g, b;
    uchar bitmaskH = *data++;
	uchar bitmaskL = *data++;

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

			pal32[palNum] = (r << 12) | (g << 7) | (b << 2);
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
        mls = elapsed % 100;
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

void runAnimationScript(bool demo, bool gpuOn)
{
    if (firstTime) {
        startBenchTime = getTicks();
        firstTime = false;
    }

    decodeFrame();

    if (mustClearScreen) clearScreenWithRect(animPosX, animPosY, ANIM_WIDTH, ANIM_HEIGHT, 0);

    if (gpuOn) {
        renderPolygons();
    } else {
        renderPolygonsSoftware();
    }

    if (!demo) drawTimer();

    ++frameNum;
}
