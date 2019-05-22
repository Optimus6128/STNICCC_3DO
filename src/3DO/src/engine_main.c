#include "types.h"
#include "main.h"
#include "tools.h"

#include "engine_mesh.h"
#include "engine_texture.h"
#include "engine_main.h"

#include "system_graphics.h"

#include "mathutil.h"

vertex vertices[MAX_VERTICES_NUM];
int lastVertexNum = 0;

const int projFactor = 256;
const int projFactorShr = 8;


void fasterMapCel(CCB *c, Point *q)
{
	Point edge[3];
	frac16 hdx0, hdy0, hdx1, hdy1, hdd;
    int shrWidth = shr[c->ccb_Width];
    int shrHeight = shr[c->ccb_Height];

	/*
	 * Compute the edge differences shifted off by 1 bit to
	 * prevent overflow  we lose the LSB but if the edge lengths
	 * are small the CCB will not be modified.
	 */
	edge[0].pt_X = q[1].pt_X - q[0].pt_X;
	edge[0].pt_Y = q[1].pt_Y - q[0].pt_Y;
	edge[1].pt_X = q[2].pt_X - q[3].pt_X;
	edge[1].pt_Y = q[2].pt_Y - q[3].pt_Y;
	edge[2].pt_X = q[3].pt_X - q[0].pt_X;
	edge[2].pt_Y = q[3].pt_Y - q[0].pt_Y;

	c->ccb_XPos = (q[0].pt_X<<16) + (1 << 15);
	c->ccb_YPos = (q[0].pt_Y<<16) + (1 << 15);

	hdx0 = (edge[0].pt_X << 16) >> shrWidth;
	hdy0 = (edge[0].pt_Y << 16) >> shrWidth;
	hdx1 = (edge[1].pt_X << 16) >> shrWidth;
	hdy1 = (edge[1].pt_Y << 16) >> shrWidth;

	hdd = (hdx1 - hdx0) >> shrHeight;

	c->ccb_HDX = hdx0 << 4;
	c->ccb_HDDX = hdd << 4;

	hdd = (hdy1 - hdy0) >> shrHeight;

	c->ccb_HDY = hdy0 << 4;
	c->ccb_HDDY = hdd << 4;

	c->ccb_VDX = (edge[2].pt_X << 16) >> shrHeight;
	c->ccb_VDY = (edge[2].pt_Y << 16) >> shrHeight;
}

void rotateVertices(int rotX, int rotY, int rotZ)
{
    int i, x, y, z;
    int cosxr, cosyr, coszr;
    int sinxr, sinyr, sinzr;
    int xvx, xvy, xvz;
    int yvx, yvy, yvz;
    int zvx, zvy, zvz;

    if (rotX==0 && rotY==0 && rotZ==0) return;

    cosxr = icos[rotX & 255]; cosyr = icos[rotY & 255]; coszr = icos[rotZ & 255];
    sinxr = isin[rotX & 255]; sinyr = isin[rotY & 255]; sinzr = isin[rotZ & 255];

    xvx = FIXED_MUL(cosyr, coszr, FP_BASE);
    xvy = FIXED_MUL(FIXED_MUL(sinxr, sinyr, FP_BASE), coszr, FP_BASE) - FIXED_MUL(cosxr, sinzr, FP_BASE);
    xvz = FIXED_MUL(FIXED_MUL(cosxr, sinyr, FP_BASE), coszr, FP_BASE) + FIXED_MUL(sinxr, sinzr, FP_BASE);
    yvx = FIXED_MUL(cosyr, sinzr, FP_BASE);
    yvy = FIXED_MUL(cosxr, coszr, FP_BASE) + FIXED_MUL(FIXED_MUL(sinxr, sinyr, FP_BASE), sinzr, FP_BASE);
    yvz = FIXED_MUL(-sinxr, coszr, FP_BASE) + FIXED_MUL(FIXED_MUL(cosxr, sinyr, FP_BASE), sinzr, FP_BASE);
    zvx = -sinyr;
    zvy = FIXED_MUL(sinxr, cosyr, FP_BASE);
    zvz = FIXED_MUL(cosxr, cosyr, FP_BASE);

    for (i=0; i<lastVertexNum; i++)
    {
        x = vertices[i].x;
        y = vertices[i].y;
        z = vertices[i].z;
        vertices[i].x = FIXED_TO_INT(x * xvx + y * xvy + z * xvz, FP_BASE);
        vertices[i].y = FIXED_TO_INT(x * yvx + y * yvy + z * yvz, FP_BASE);
        vertices[i].z = FIXED_TO_INT(x * zvx + y * zvy + z * zvz, FP_BASE);
    }
}

void translateVertices(int posX, int posY, int posZ)
{
    int i;

    if (posX==0 && posY==0 && posZ==0) return;

    for (i=0; i<lastVertexNum; i++)
    {
        vertices[i].x += posX;
        vertices[i].y += posY;
        vertices[i].z += posZ;
    }
}

void projectVertices()
{
    int i;
    for (i=0; i<lastVertexNum; i++)
    {
        if (vertices[i].z>0)
        {
            vertices[i].x = SCREEN_WIDTH / 2 + (vertices[i].x << projFactorShr) / vertices[i].z;
            vertices[i].y = SCREEN_HEIGHT / 2 - (vertices[i].y << projFactorShr) / vertices[i].z;
        }
    }
}

void uploadVertices(mesh *ms)
{
    memcpy(vertices, ms->vrtx, ms->vrtxNum * sizeof(vertex));
    lastVertexNum = ms->vrtxNum;
}

void uploadTransformAndProjectMesh(mesh *ms)
{
    uploadVertices(ms);
	rotateVertices(ms->rotX, ms->rotY, ms->rotZ);
    translateVertices(ms->posX, ms->posY, ms->posZ);
    projectVertices();
}

void renderTransformedGeometry(mesh *ms)
{
    int i, j;
    int *indices = ms->index;
	Point quad[4];

    j = 0;
    for (i=0; i<ms->indexNum; i+=4)
    {
		quad[0].pt_X = vertices[indices[i]].x; quad[0].pt_Y = vertices[indices[i]].y;
		quad[1].pt_X = vertices[indices[i+1]].x; quad[1].pt_Y = vertices[indices[i+1]].y;
		quad[2].pt_X = vertices[indices[i+2]].x; quad[2].pt_Y = vertices[indices[i+2]].y;
		quad[3].pt_X = vertices[indices[i+3]].x; quad[3].pt_Y = vertices[indices[i+3]].y;

        //fasterMapCel(ms->quad[j++].cel, quad);
        MapCel(ms->quad[j++].cel, quad);
    }

    drawCels(ms->quad[0].cel);
}
