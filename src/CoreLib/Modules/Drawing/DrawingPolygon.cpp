#include "DrawingPolygon.h"
#include "DrawingCel.h"

static Edge *leftEdge = nullptr;
static Edge *rightEdge = nullptr;

static int edgeYmin = 0;
static int edgeYmax = 0;

static Texture *mainTexture;

static void(*drawEdge)(int, int, ScreenBuffer*);

static TextureWrappingType textureWrapping = TextureWrappingType::Clamp;

void initDrawingFrameworkPolygon(ScreenBuffer *screen)
{
	if (leftEdge == nullptr) leftEdge = new Edge[screen->height];
	if (rightEdge == nullptr) rightEdge = new Edge[screen->height];

	setMainTexture(new Texture(ImageLoader::generate(GenImageType::NOISE, ImgGenParameters(64,64, 128,192,255,0))));
	//setMainTexture(new Texture(ImageLoader::generateText("Malaka")));
	setTextureFilter(TextureFilterType::Nearest);

	initCelDrawing(screen);
}

// Triangle Drawing
// ================



static inline void drawEdgeNearest(int y, int dx, ScreenBuffer *screen)
{
	const Edge *l = &leftEdge[y];
	const Edge *r = &rightEdge[y];

	const int x0 = l->x; const int u0 = l->u; const int v0 = l->v; const int c0 = l->c;
	const int x1 = r->x; const int u1 = r->u; const int v1 = r->v; const int c1 = r->c;


	uint *vram = (uint*)screen->vram + y * screen->width + x0;
	const int texwidth = mainTexture->width;
	const int texheight = mainTexture->height;

	int c = INT_TO_FIXED(c0, FP_BITS);
	int u = INT_TO_FIXED(u0, FP_BITS);
	int v = INT_TO_FIXED(v0, FP_BITS);
	const int dc = (INT_TO_FIXED(c1 - c0, FP_BITS) / dx);
	const int du = (INT_TO_FIXED(u1 - u0, FP_BITS) / dx);
	const int dv = (INT_TO_FIXED(v1 - v0, FP_BITS) / dx);
	for (int x = x0; x < x1; x++)
	{
		int tex_u = (FIXED_TO_INT(((u * texwidth) >> TEX_COORD_RANGE_BITS), FP_BITS));
		int tex_v = (FIXED_TO_INT(((v * texheight) >> TEX_COORD_RANGE_BITS), FP_BITS));

		tex_u -= (tex_v / texwidth);

		if (textureWrapping == TextureWrappingType::Clamp) {
			CLAMP_POSITIVE_RIGHT(tex_u, texwidth - 1);
			CLAMP_POSITIVE_RIGHT(tex_v, texheight - 1);
		} else {
			tex_u = tex_u & (texwidth - 1);
			tex_v = tex_v & (texheight - 1);
		}

		uint col = mainTexture->data[tex_v * texwidth + tex_u];
		const uchar b = (col & 255);
		const uchar g = ((col >> 8) & 255);
		const uchar r = ((col >> 16) & 255);
		const uchar a = col >> 24;

		if (a!=0)
			*vram = MAKE_SHADED_RGB(r, g, b, FIXED_TO_INT(c, FP_BITS));

		++vram;

		c += dc;
		u += du;
		v += dv;
	}
}

static inline void drawEdgeBilinear(int y, int dx, ScreenBuffer *screen)
{
	const Edge *l = &leftEdge[y];
	const Edge *r = &rightEdge[y];

	const int x0 = l->x; const int u0 = l->u; const int v0 = l->v; const int c0 = l->c;
	const int x1 = r->x; const int u1 = r->u; const int v1 = r->v; const int c1 = r->c;


	uint *vram = (uint*)screen->vram + y * screen->width + x0;
	const int texwidth = mainTexture->width;
	const int texheight = mainTexture->height;

	int c = INT_TO_FIXED(c0, FP_BITS);
	int u = INT_TO_FIXED(u0, FP_BITS);
	int v = INT_TO_FIXED(v0, FP_BITS);
	const int dc = (INT_TO_FIXED(c1 - c0, FP_BITS) / dx);
	const int du = (INT_TO_FIXED(u1 - u0, FP_BITS) / dx);
	const int dv = (INT_TO_FIXED(v1 - v0, FP_BITS) / dx);

	for (int x = x0; x < x1; x++)
	{
		const int realU = (u * texwidth) >> TEX_COORD_RANGE_BITS;
		const int realV = (v * texheight) >> TEX_COORD_RANGE_BITS;
		const int tex_u_frac = realU & FP_AND;
		const int tex_v_frac = realV & FP_AND;

		int tex_u0 = FIXED_TO_INT(realU , FP_BITS);
		int tex_v0 = FIXED_TO_INT(realV , FP_BITS);
		CLAMP_POSITIVE_RIGHT(tex_u0, texwidth - 1);
		CLAMP_POSITIVE_RIGHT(tex_v0, texheight - 1);
		int tex_u1 = tex_u0 + 1;
		int tex_v1 = tex_v0 + 1;
		CLAMP_POSITIVE_RIGHT(tex_u1, texwidth - 1);
		CLAMP_POSITIVE_RIGHT(tex_v1, texheight - 1);

		const uint c00 = mainTexture->data[tex_v0 * texwidth + tex_u0];
		const uint b00 = c00 & 255;
		const uint g00 = (c00 >> 8) & 255;
		const uint r00 = (c00 >> 16) & 255;
		const uint a00 = c00 >> 24;

		const uint c10 = mainTexture->data[tex_v0 * texwidth + tex_u1];
		const uint b10 = c10 & 255;;
		const uint g10 = (c10 >> 8) & 255;
		const uint r10 = (c10 >> 16) & 255;
		const uint a10 = c10 >> 24;

		const uint c01 = mainTexture->data[tex_v1 * texwidth + tex_u0];
		const uint b01 = c01 & 255;;
		const uint g01 = (c01 >> 8) & 255;
		const uint r01 = (c01 >> 16) & 255;
		const uint a01 = c01 >> 24;

		const uint c11 = mainTexture->data[tex_v1 * texwidth + tex_u1];
		const uint b11 = c11 & 255;;
		const uint g11 = (c11 >> 8) & 255;
		const uint r11 = (c11 >> 16) & 255;
		const uint a11 = c11 >> 24;

		const uint ru = (tex_u_frac * r10 + (FP_AND - tex_u_frac) * r00) >> FP_BITS;
		const uint rl = (tex_u_frac * r11 + (FP_AND - tex_u_frac) * r01) >> FP_BITS;
		const uint gu = (tex_u_frac * g10 + (FP_AND - tex_u_frac) * g00) >> FP_BITS;
		const uint gl = (tex_u_frac * g11 + (FP_AND - tex_u_frac) * g01) >> FP_BITS;
		const uint bu = (tex_u_frac * b10 + (FP_AND - tex_u_frac) * b00) >> FP_BITS;
		const uint bl = (tex_u_frac * b11 + (FP_AND - tex_u_frac) * b01) >> FP_BITS;
		const uint au = (tex_u_frac * a10 + (FP_AND - tex_u_frac) * a00) >> FP_BITS;
		const uint al = (tex_u_frac * a11 + (FP_AND - tex_u_frac) * a01) >> FP_BITS;

		const uint c8 = FIXED_TO_INT(c, FP_BITS);
		const uint r = (((tex_v_frac * rl + (FP_AND - tex_v_frac) * ru) >> FP_BITS) * c8) >> 8;
		const uint g = (((tex_v_frac * gl + (FP_AND - tex_v_frac) * gu) >> FP_BITS) * c8) >> 8;
		const uint b = (((tex_v_frac * bl + (FP_AND - tex_v_frac) * bu) >> FP_BITS) * c8) >> 8;
		
		// If shade alpha. Need to think how to handle this in the future, because we don't normally want to shade alpha along gouraud shading everything else.
		// Normally we should have rShade, gShade, bShade, aShade, or multiple copies of this function, with or without alpha. Or something else.
		// Will need to add these comments because I got blocked again by such.
		const uint a = (((tex_v_frac * al + (FP_AND - tex_v_frac) * au) >> FP_BITS) * c8) >> 8;
		// You would need to bring the below line back in future stuff or solve the above, make a switcher, maybe some flags or something else.
		//const uint a = (tex_v_frac * al + (FP_AND - tex_v_frac) * au) >> FP_BITS;

		const uint cBgrnd = *vram;
		const uint bBgrnd = cBgrnd & 255;
		const uint gBgrnd = (cBgrnd >> 8) & 255;
		const uint rBgrnd = (cBgrnd >> 16) & 255;

		const uint rFinal = (a * r + (255 - a) * rBgrnd) >> 8;
		const uint gFinal = (a * g + (255 - a) * gBgrnd) >> 8;
		const uint bFinal = (a * b + (255 - a) * bBgrnd) >> 8;

		*vram++ = (rFinal << 16) | (gFinal << 8) | bFinal;

		c += dc;
		u += du;
		v += dv;
	}
}

static inline void drawEdges(ScreenBuffer *screen)
{
	const int scr_w = (int)screen->width;

	for (int y = edgeYmin; y <= edgeYmax; y++)
	{
		Edge *l = &leftEdge[y];
		Edge *r = &rightEdge[y];

		const int x0 = l->x;
		const int x1 = r->x;

		int dx = x1 - x0;
		if (x0 < 0) {
			if (x0 != x1) {
				l->u += (((r->u - l->u) * -x0) / dx);
				l->v += (((r->v - l->v) * -x0) / dx);
				l->c += (((r->c - l->c) * -x0) / dx);
			}
			l->x = 0;
			dx = x1;
		}
		if (x1 > scr_w - 1) {
			r->x = scr_w - 1;
		}

		if (l->x < r->x) {
			drawEdge(y, dx, screen);
		}
	}
}

static inline void prepareEdgeList(Vertex *v0, Vertex *v1, ScreenBuffer *screen)
{
	if (v0->y == v1->y) return;

	Edge *edgeListToWrite;
	if (v0->y < v1->y) {
		edgeListToWrite = leftEdge;
	}
	else {
		edgeListToWrite = rightEdge;

		Vertex *vTemp = v0;
		v0 = v1;
		v1 = vTemp;
	}

	const int x0 = v0->x; const int y0 = v0->y;
	const int x1 = v1->x; const int y1 = v1->y;
	const int c0 = v0->c; const int c1 = v1->c;
	const int tc_u0 = v0->u; const int tc_u1 = v1->u;
	const int tc_v0 = v0->v; const int tc_v1 = v1->v;

	const int scr_h = (int)screen->height;

	const int dy = y1 - y0;
	const int dx = INT_TO_FIXED(x1 - x0, FP_BITS) / dy;
	const int dc = INT_TO_FIXED(c1 - c0, FP_BITS) / dy;
	const int du = INT_TO_FIXED(tc_u1 - tc_u0, FP_BITS) / dy;
	const int dv = INT_TO_FIXED(tc_v1 - tc_v0, FP_BITS) / dy;

	int xp = INT_TO_FIXED(x0, FP_BITS);
	int c = INT_TO_FIXED(c0, FP_BITS);
	int u = INT_TO_FIXED(tc_u0, FP_BITS);
	int v = INT_TO_FIXED(tc_v0, FP_BITS);
	int yp = y0;
	do
	{
		if (yp >= 0 && yp < scr_h)
		{
			edgeListToWrite[yp].x = FIXED_TO_INT(xp, FP_BITS);
			edgeListToWrite[yp].c = FIXED_TO_INT(c, FP_BITS);
			edgeListToWrite[yp].u = FIXED_TO_INT(u, FP_BITS);
			edgeListToWrite[yp].v = FIXED_TO_INT(v, FP_BITS);
		}
		xp += dx;
		c += dc;
		u += du;
		v += dv;
	} while (yp++ != y1);
}

void drawTriangle(Vertex &v0, Vertex &v1, Vertex &v2, ScreenBuffer *screen)
{
	const int x0 = v0.x; const int y0 = v0.y;
	const int x1 = v1.x; const int y1 = v1.y;
	const int x2 = v2.x; const int y2 = v2.y;

	const int scrWidth = screen->width;
	const int scrHeight = screen->height;

	/*if (((x0 < 0 || x0 > scrWidth - 1) || (y0 < 0 || y0 > scrHeight - 1)) &&
		((x1 < 0 || x1 > scrWidth - 1) || (y1 < 0 || y1 > scrHeight - 1)) &&
		((x2 < 0 || x2 > scrWidth - 1) || (y2 < 0 || y2 > scrHeight - 1))) {
		if (x0 > 0  || x1 > 0 || x2 >> 0)
			printf("%d\n", x2);
		return;
		I should do proper clipping
	}*/

	const int n = (x0 - x1) * (y2 - y1) - (x2 - x1) * (y0 - y1);
	if (n < 0) return;
	// CW and CCW in the future?

	int yMin = y0;
	int yMax = yMin;
	if (y1 < yMin) yMin = y1;
	if (y1 > yMax) yMax = y1;
	if (y2 < yMin) yMin = y2;
	if (y2 > yMax) yMax = y2;
	edgeYmin = yMin;
	edgeYmax = yMax;

	if (edgeYmin < 0) edgeYmin = 0;
	if (edgeYmax > scrHeight - 1) edgeYmax = scrHeight - 1;

	if (edgeYmax > edgeYmin) {
		prepareEdgeList(&v0, &v1, screen);
		prepareEdgeList(&v1, &v2, screen);
		prepareEdgeList(&v2, &v0, screen);

		drawEdges(screen);
	}
}

void drawQuad(Quad &q, ScreenBuffer *screen)
{
	drawTriangle(q.v0, q.v1, q.v2, screen);
	drawTriangle(q.v0, q.v2, q.v3, screen);
}





// Rendering state changing
// ========================

void setMainTexture(Texture *tex)
{
	mainTexture = tex;
}

void setTextureFilter(TextureFilterType filterType)
{
	switch (filterType) {
	case TextureFilterType::Nearest:
		drawEdge = drawEdgeNearest;
		break;

	case TextureFilterType::Bilinear:
		drawEdge = drawEdgeBilinear;
		break;
	}
}

void setTextureWrapping(TextureWrappingType wrappingType)
{
	textureWrapping = wrappingType;
}
