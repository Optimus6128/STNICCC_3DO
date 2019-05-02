#ifndef MAIN_H
#define MAIN_H

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

#define BG_COLOR 15

#define DEBUG_ON false


extern bool PressedAonce;
extern bool PressedBonce;
extern bool PressedConce;
extern bool PressedLonce;
extern bool PressedRonce;

extern bool gpuOn;
extern bool demo;
extern bool benchTexture;
extern bool benchScreens;

void clearScreen(ushort color);
void clearScreenWait(ushort color, int ticks);

#endif
