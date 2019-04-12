#include "ScriptMain.h"
#include "scene1.h"
#include "main.h"
#include "system_graphics.h"
#include "tools.h"

#define MAX_POLYS 256
#define ANIM_WIDTH 256
#define ANIM_HEIGHT 200

static int nextFrame = 0;
static unsigned int block64index = 0;

static uchar *data = &scene1_bin[0];

static ushort pal[16];
static MyPoint2D pt[16];

static QuadStore quads[MAX_POLYS];
static QuadStore *quadPtr;
static int numQuads = 0;

static int leftEdgeFlat[SCREEN_HEIGHT];
static int rightEdgeFlat[SCREEN_HEIGHT];

static CCB polys[MAX_POLYS];

static int mustClearScreen = 0;



const int animPosX = (SCREEN_WIDTH - ANIM_WIDTH) / 2;
const int animPosY = (SCREEN_HEIGHT - ANIM_HEIGHT) / 2;

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
	//const int x0 = p[0].x;
	const int y0 = p[0].y;
	//const int x1 = p[1].x;
	const int y1 = p[1].y;
	//const int x2 = p[2].x;
	const int y2 = p[2].y;
	//const int x3 = p[3].x;
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

/*static int isPolygonConvex(MyPoint2D *pt, int numVertices)
{
    int i;
	int zcross, zcross0;

	for (i = 0; i < numVertices; ++i) {
		const int i0 = i;
		const int i1 = (i + 1) % numVertices;
		const int i2 = (i + 2) % numVertices;

		MyPoint2D v0, v1;

		v0.x = pt[i1].x - pt[i0].x;  v0.y = pt[i1].y - pt[i0].y;
		v1.x = pt[i2].x - pt[i1].x;  v1.y = pt[i2].y - pt[i1].y;

		zcross = v0.x * v1.y - v0.y * v1.x;
		if (i == 0) {
			zcross0 = zcross;
		} else {
			zcross *= zcross0;
			if (zcross < 0) {
				return 0;
			}
		}
	}
	return 1;
}*/

static void addPolygon(MyPoint2D *pt, int numVertices, int paletteIndex)
{
	ushort color = pal[paletteIndex];

	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

	if (numVertices < 3) return;

	//if (0==isPolygonConvex(pt, numVertices)) color = 31 << 10;

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

static void clearScreenWithRect(int posX, int posY, int width, int height, unsigned int color)
{
	static CCB clearCCB;

	clearCCB.ccb_Flags = CCB_LDSIZE|CCB_LDPRS|CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_LAST;

	clearCCB.ccb_PIXC = 0x1F00;
	clearCCB.ccb_PRE0 = 0x40000016;
	clearCCB.ccb_PRE1 = 0x03FF1000;
	clearCCB.ccb_SourcePtr = (CelData*)0;
	clearCCB.ccb_PLUTPtr = (void*)(color<<16);
	clearCCB.ccb_XPos = posX<<16;
	clearCCB.ccb_YPos = posY<<16;
	clearCCB.ccb_HDX = width<<20;
	clearCCB.ccb_HDY = 0<<20;
	clearCCB.ccb_VDX = 0<<16;
	clearCCB.ccb_VDY = height<<16;

	drawCels(&clearCCB);
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

static void renderPolygons()
{
    int i;
    static MyPoint2D p[4];
    ushort *screen;

    CCB *quadCCB = &polys[0];

    if (numQuads==0) return;

    screen = getVideoramAddress();

	for (i=0; i<numQuads; ++i) {
		p[0].x = quads[i].p0.x + animPosX; p[0].y = quads[i].p0.y + animPosY;
		p[1].x = quads[i].p1.x + animPosX; p[1].y = quads[i].p1.y + animPosY;
		p[2].x = quads[i].p2.x + animPosX; p[2].y = quads[i].p2.y + animPosY;
		p[3].x = quads[i].p3.x + animPosX; p[3].y = quads[i].p3.y + animPosY;
		//mapCelToFlatQuad(quadCCB, p, quads[i].c);
        drawFlatQuad(&p[0], quads[i].c, screen);
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
		int palNum = 15 - i;
		if (bitmask & 1) {
			uchar colorH = *data++;
			uchar colorL = *data++;
			int color = (colorH << 8) | colorL;

			r = ((color >> 8) & 7) << 2;
			g = ((color >> 4) & 7) << 2;
			b = (color & 7) << 2;

			pal[palNum] = (r << 10) | (g << 5) | b;
		}
		bitmask >>= 1;
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
            nextFrame = 0;
        }
        break;
	}
}

static void interpretIndexedMode()
{
    int i, n;
	static MyPoint2D vi[256];

	uchar descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	int vertexNum = *data++;

	for (i = 0; i < vertexNum; ++i) {
		vi[i].x = (int)*data++;
		vi[i].y = (int)*data++;
	}

	while(1) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

        polyPaletteIndex = (int)(descriptor >> 4);
        polyNumVertices = (int)(descriptor & 15);

		for (n = 0; n < polyNumVertices; ++n) {
			int vertexId = *data++;

			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
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
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void decodeFrame()
{
	uchar flags = *data++;

	mustClearScreen = 0;
	numQuads = 0;
	quadPtr = &quads[0];

	if (flags & 1) {
		mustClearScreen = 1;
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

void runAnimationScript()
{
    decodeFrame();

    if (nextFrame < 2) clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 31);

    if (mustClearScreen==1) clearScreenWithRect(animPosX, animPosY, ANIM_WIDTH, ANIM_HEIGHT, 0);
    renderPolygons();

    ++nextFrame;

	drawNumber(0, 8, nextFrame);
}
