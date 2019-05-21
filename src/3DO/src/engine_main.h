#ifndef ENGINE_MAIN_H
#define ENGINE_MAIN_H

#include "engine_mesh.h"

#define MAX_VERTICES_NUM 512

void uploadVertices(mesh *ms);
void rotateVertices(int rotX, int rotY, int rotZ);
void translateVertices(int posX, int posY, int posZ);
void projectVertices(void);
void uploadTransformAndProjectMesh(mesh *ms);
void renderTransformedGeometry(mesh *ms);
void fasterMapCel(CCB *c, Point *q);

#endif
