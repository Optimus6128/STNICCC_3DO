#include "ScriptMain.h"
#include "scene1.h"


static int nextFrame = 0;
static uint block64index = 0;

static uchar *data = scene1_bin;

static ushort pal[16];
static MyPoint2D pt[16];

static QuadStore quads[1024];
static QuadStore *quadPtr;
static int numQuads = 0;


static int mustClearScreen = 0;

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

static void renderPolygons()
{
    int i;
	for (i=0; i<numQuads; ++i) {
		//MyPoint2D *p0 = &quads[i].p0;
		//MyPoint2D *p1 = &quads[i].p1;
		//MyPoint2D *p2 = &quads[i].p2;
		//MyPoint2D *p3 = &quads[i].p3;
		//drawFlatQuadScaled(*p0, *p1, *p2, *p3, color32from15(quads[i].c), screen);
	}
}

static ushort flipWordEndianess(ushort value)
{
	uchar vl = value >> 8;
	uchar vr = value & 255;
	return (vr << 8) | vl;
}

static void interpretPaletteData()
{
    int i, r, g, b;
	ushort bitmask = flipWordEndianess(*((ushort*)data));
	data += 2;

	for (i = 0; i < 16; ++i) {
		int palNum = 15 - i;
		if (bitmask & 1) {
			ushort color = flipWordEndianess(*((ushort*)data));
			data += 2;

			r = ((color >> 8) & 7) << 1;
			g = ((color >> 4) & 7) << 1;
			b = (color & 7) << 1;

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

	//if (mustClearScreen==1) do something
	renderPolygons();

	++nextFrame;
	drawNumber(0, 16, nextFrame);
}
