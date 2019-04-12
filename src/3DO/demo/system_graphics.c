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
	ioInfo.ioi_Recv.iob_Len = width*height*2;   // 2 could be because 16bit and not because number of buffers, gotta check

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

    screenPage = (screenPage+ 1) & 1;
}

void drawCels(CCB *cels)
{
    DrawCels(BitmapItems[screenPage], cels);
}
