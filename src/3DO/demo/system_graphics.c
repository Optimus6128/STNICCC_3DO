#include "types.h"
#include "main.h"

#include "system_graphics.h"

bool vsync = true;

static Item    gVRAMIOReq;
static Item    vsyncItem;

static Item BitmapItems[NUM_SCREEN_PAGES];
static Bitmap *Bitmaps[NUM_SCREEN_PAGES];

static ScreenContext Screencontext;

static IOInfo ioInfo;

static int screenPage = 0;

void initGraphics()
{
	int i;
	int width,height;

	OpenGraphics(&Screencontext,NUM_SCREEN_PAGES);

	for(i=0;i<NUM_SCREEN_PAGES;i++)
	{
		BitmapItems[i] = Screencontext.sc_BitmapItems[i];
		Bitmaps[i] = Screencontext.sc_Bitmaps[i];

		SetCEControl(BitmapItems[i], 0xffffffff, ASCALL);

		EnableHAVG( BitmapItems[i] );
		EnableVAVG( BitmapItems[i] );
	}

	width = Bitmaps[0]->bm_Width;
	height = Bitmaps[0]->bm_Height;

	gVRAMIOReq = CreateVRAMIOReq();		// Obtain an IOReq for all SPORT operations

	memset(&ioInfo,0,sizeof(ioInfo));
	ioInfo.ioi_Command = FLASHWRITE_CMD;
	ioInfo.ioi_CmdOptions = 0xffffffff;
	ioInfo.ioi_Offset = 0x00000000; // background colour
	ioInfo.ioi_Recv.iob_Buffer = Bitmaps[0]->bm_Buffer;
	ioInfo.ioi_Recv.iob_Len = width*height*NUM_SCREEN_PAGES;   // 2 could be because 16bit and not because number of buffers, gotta check

	vsyncItem = GetVBLIOReq();
}

ushort *getVideoramAddress()
{
    return (ushort*)Bitmaps[screenPage]->bm_Buffer;
}

void displayScreen()
{
    DisplayScreen(Screencontext.sc_Screens[screenPage], 0 );
    if (vsync) WaitVBL(vsyncItem, 1);

    screenPage = (screenPage+ 1) % NUM_SCREEN_PAGES;
}

void drawCels(CCB *cels)
{
    DrawCels(BitmapItems[screenPage], cels);
}

void fadeToBlack()
{
    FadeToBlack(&Screencontext, FADE_FRAMECOUNT);
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
