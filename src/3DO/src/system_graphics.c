#include "types.h"
#include "main.h"

#include "system_graphics.h"

bool vsync = true;

static Item    VRAMIOReq;
static Item    vsyncItem;

static Item VideoItems[NUM_SCREEN_PAGES];
static Bitmap *VideoBuffers[NUM_SCREEN_PAGES];

static ScreenContext screenContext;

static IOInfo ioInfo;

static int screenPage = 0;


static void initSPORT()
{
	VRAMIOReq = CreateVRAMIOReq(); // Obtain an IOReq for all SPORT operations

	memset(&ioInfo,0,sizeof(ioInfo));
	ioInfo.ioi_Command = FLASHWRITE_CMD;
	ioInfo.ioi_CmdOptions = 0xffffffff;
	ioInfo.ioi_Offset = 0; // background colour
	ioInfo.ioi_Recv.iob_Buffer = VideoBuffers[screenPage]->bm_Buffer;
	ioInfo.ioi_Recv.iob_Len = SCREEN_SIZE_IN_BYTES;
}

void initGraphics()
{
	int i;

	CreateBasicDisplay(&screenContext, DI_TYPE_DEFAULT, NUM_SCREEN_PAGES);

	for(i=0;i<NUM_SCREEN_PAGES;i++)
	{
		VideoItems[i] = screenContext.sc_BitmapItems[i];
		VideoBuffers[i] = screenContext.sc_Bitmaps[i];

		SetCEControl(VideoItems[i], 0xffffffff, ASCALL);
	}

	initSPORT();

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

	ioInfo.ioi_Recv.iob_Buffer = VideoBuffers[screenPage]->bm_Buffer;
	DoIO(VRAMIOReq, &ioInfo);
}

void drawCels(CCB *cels)
{
    DrawCels(VideoItems[screenPage], cels);
}

void fadeToBlack()
{
    FadeToBlack(&screenContext, FADE_FRAMECOUNT);
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
