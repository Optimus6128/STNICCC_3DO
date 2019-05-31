#include "types.h"
#include "main.h"

#include "system_graphics.h"

bool vsync = true;

static Item    gVRAMIOReq;
static Item    vsyncItem;

static Item VideoItems[NUM_SCREEN_PAGES];
static Bitmap *VideoBuffers[NUM_SCREEN_PAGES];

static ScreenContext screenContext;

static IOInfo ioInfo;

static int screenPage = 0;

void initGraphics()
{
	int i;
	int width,height;

	OpenGraphics(&screenContext,NUM_SCREEN_PAGES);

	for(i=0;i<NUM_SCREEN_PAGES;i++)
	{
		VideoItems[i] = screenContext.sc_BitmapItems[i];
		VideoBuffers[i] = screenContext.sc_Bitmaps[i];

		SetCEControl(VideoItems[i], 0xffffffff, ASCALL);

		EnableHAVG( VideoItems[i] );
		EnableVAVG( VideoItems[i] );
	}

	width = VideoBuffers[0]->bm_Width;
	height = VideoBuffers[0]->bm_Height;

	gVRAMIOReq = CreateVRAMIOReq();		// Obtain an IOReq for all SPORT operations

	memset(&ioInfo,0,sizeof(ioInfo));
	ioInfo.ioi_Command = FLASHWRITE_CMD;
	ioInfo.ioi_CmdOptions = 0xffffffff;
	ioInfo.ioi_Offset = 0x00000000; // background colour
	ioInfo.ioi_Recv.iob_Buffer = VideoBuffers[0]->bm_Buffer;
	ioInfo.ioi_Recv.iob_Len = width*height*NUM_SCREEN_PAGES;   // 2 could be because 16bit and not because number of buffers, gotta check

	vsyncItem = GetVBLIOReq();
}

ushort *getVideoramAddress()
{
    return (ushort*)VideoBuffers[screenPage]->bm_Buffer;
}

void displayScreen()
{
    DisplayScreen(screenContext.sc_Screens[screenPage], 0 );
    if (vsync) WaitVBL(vsyncItem, 1);

    screenPage = (screenPage+ 1) % NUM_SCREEN_PAGES;
}

void drawCels(CCB *cels)
{
    DrawCels(VideoItems[screenPage], cels);
}

void drawCel(CCB *cel)
{
    cel->ccb_Flags |= CCB_LAST;
    DrawCels(VideoItems[screenPage], cel);
    cel->ccb_Flags &= ~CCB_LAST;
}

void fadeToBlack()
{
    FadeToBlack(&screenContext, FADE_FRAMECOUNT);
}

void clearScreenWithRect(int posX, int posY, int width, int height, unsigned int color)
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

void clearAllScreens(ushort color)
{
    // Do it more times to clear all video pages
    int i;
    for (i=0; i<NUM_SCREEN_PAGES; ++i) {
        clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
        displayScreen();
    }
}

void setScreenClipping(int posX, int posY, int width, int height)
{
    // Doesn't work yet
    int i;
    for (i=0; i<NUM_SCREEN_PAGES; ++i) {
        SetClipOrigin(VideoItems[i], posX, posY);
        SetClipWidth(VideoItems[i], width);
        SetClipHeight(VideoItems[i], height);
    }
}

void resetScreenClipping()
{
    // Doesn't work yet
    setScreenClipping(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}
