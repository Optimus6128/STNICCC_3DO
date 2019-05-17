#ifndef SYSTEM_GRAPHICS_H
#define SYSTEM_GRAPHICS_H

#include "displayutils.h"
#include "debug.h"
#include "nodes.h"
#include "kernelnodes.h"
#include "list.h"
#include "folio.h"
#include "task.h"
#include "kernel.h"
#include "mem.h"
#include "operamath.h"
#include "math.h"
#include "semaphore.h"
#include "io.h"
#include "strings.h"
#include "stdlib.h"
#include "event.h"
#include "controlpad.h"

#include "stdio.h"
#include "graphics.h"
#include "3dlib.h"
#include "Init3DO.h"
#include "Form3DO.h"
#include "Parse3DO.h"
#include "Utils3DO.h"
#include "3d_examples.h"
#include "getvideoinfo.h"


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define NUM_SCREEN_PAGES 4

void initGraphics(void);
void displayScreen(void);
void drawCels(CCB *cels);

void fadeToBlack(void);
void clearScreenWithRect(int posX, int posY, int width, int height, unsigned int color);
void clearAllScreens(ushort color);

ushort *getVideoramAddress(void);
void setScreenClipping(int posX, int posY, int width, int height);
void resetScreenClipping(void);

extern bool vsync;

#endif