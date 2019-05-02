#include "types.h"
#include "main.h"

#include "system_graphics.h"

#include "tools.h"
#include "ScriptMain.h"
#include "sound.h"

static bool restart = false;

static bool PressedA = false;
static bool PressedB = false;
static bool PressedC = false;
static bool PressedL = false;
static bool PressedR = false;

bool PressedAonce = false;
bool PressedBonce = false;
bool PressedConce = false;
bool PressedLonce = false;
bool PressedRonce = false;

bool gpuOn;
bool demo;
bool benchTexture;
bool benchScreens;

static void quit()
{
    endMusic();
    KillEventUtility();
	fadeToBlack();
}

static unsigned int getJoystickState()
{
	ControlPadEventData cpaddata;
	cpaddata.cped_ButtonBits=0;
	GetControlPad(1,0,&cpaddata);

	return ( cpaddata.cped_ButtonBits );
}

static void processJoystick()
{
	int joybits;

	joybits=getJoystickState();

	if (joybits&ControlStart) {
        restart = true;
	}

	if(joybits&ControlA) {
		PressedAonce = !PressedA;
		PressedA = true;
	}
	else PressedA = false;

	if(joybits&ControlB) {
		PressedBonce = !PressedB;
		PressedB = true;
	}
	else PressedB = false;

	if(joybits&ControlC) {
		PressedConce = !PressedC;
		PressedC = true;
	}
	else PressedC = false;

	if(joybits&ControlLeftShift) {
		PressedLonce = !PressedL;
		PressedL = true;
	}
	else PressedL = false;

	if(joybits&ControlRightShift) {
		PressedRonce = !PressedR;
		PressedR = true;
	}
	else PressedR = false;
}

static void progressScreen()
{
    int x,y,yp,c;
    ushort *vram;

    while(!PressedAonce && !PressedBonce && !PressedConce && !PressedLonce && !PressedRonce) {
        processJoystick();
        for(y=0; y<SCREEN_HEIGHT; ++y) {
            vram = getVideoramAddress() + (y >> 1) * (2 * SCREEN_WIDTH) + (y & 1);
            yp = y - SCREEN_HEIGHT / 2;
            if (yp==0) yp = 1;
            yp = (262144/2) / (yp * yp);
            if (yp > 31) yp = 31;
            if (yp < 0) yp = 0;
            yp = 31 - yp;
            c = ((yp >> 1) << 10) | ((yp >> 2) << 5) | (yp >> 1);
            for (x=0;x<SCREEN_WIDTH; ++x) {
                *vram = c;
                vram += 2;
            }
        }
        drawText(16, 72, "Press:");
        drawText(32, 96, "A for Demo (Hardware & Sound)");
        drawText(40, 112, "B for Benchmark (Hardware)");
        drawText(48, 128, "C for Benchmark (Software)");
        drawText(56, 144, "L for Benchmark (Texture)");
        drawText(64, 160, "R for Benchmark (Frames)");
        displayScreen();
    }

    // A = demo, gpu
    // B = bench, gpu
    // C = bench, soft
    // L = bench, gpu, texture
    // R = bench, gpu, screens

    demo = PressedAonce;
    gpuOn = !PressedConce;
    benchTexture = PressedLonce;
    benchScreens = PressedRonce;
    restart = false;    // to avoid fading out before the demo starts
}

void clearScreen(ushort color)
{
    // Do it more times to clear all video pages
    int i;
    for (i=0; i<NUM_SCREEN_PAGES; ++i) {
        clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
        displayScreen();
    }
}

void clearScreenWait(ushort color, int ticks)
{
    // Do it more times to clear all video pages
    int i;
    for (i=0; i<ticks; ++i) {
        clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
        displayScreen();
    }
}

static void initSystem()
{
	OpenMathFolio();
    OpenGraphicsFolio();
    InitEventUtility(1,0,LC_Observer);
}

static void initStuff()
{
	initSystem();
	initGraphics();

	initFonts();
	initTimer();

    initDivs();

    initCCBpolys();
    initCCBbuffers();
}

static void script()
{
    runAnimationScript();
}

static void mainLoop()
{
    progressScreen();

    clearScreen(BG_COLOR);

    vsync = false;
    if (demo) {
        vsync = true;
        startMusic();
    }
    if (DEBUG_ON) vsync = true;

	while(!restart)
	{
	    processJoystick();

		script();

        showFPS();

		displayScreen();
	}

	quit();
}


int main()
{
	initStuff();

	mainLoop();
}
