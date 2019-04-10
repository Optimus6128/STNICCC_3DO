#ifndef DRAWING_LINE_H
#define DRAWING_LINE_H

#include "Typedefs.h"
#include "ScreenBuffer.h"
#include "Drawing.h"

void drawAntialiasedLine(const Point2D &p0, const Point2D &p1, uint c, ScreenBuffer *screen);
void drawAntialiasedLineThick(const Point2D &p0, const Point2D &p1, uint c, ScreenBuffer *screen);

#endif
