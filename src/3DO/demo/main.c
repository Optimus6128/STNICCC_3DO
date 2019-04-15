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
static bool PressedAonce = false;
static bool PressedBonce = false;
static bool PressedConce = false;
static bool gpuOn;
static bool demo;

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
}

static void progressScreen()
{
    int x,y,yp,c;
    ushort *vram;

    while(!PressedAonce && !PressedBonce && !PressedConce) {
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
        drawText(16, 80, "Press:");
        drawText(32, 104, "A for Demo (Hardware & Sound)");
        drawText(40, 120, "B for Benchmark (Hardware)");
        drawText(48, 136, "C for Benchmark (Software)");
        displayScreen();
    }

    // A = demo, gpu
    // B = bench, gpu
    // C = bench, soft

    if (PressedAonce) { demo = true; gpuOn = true; }
    if (PressedBonce) { demo = false; gpuOn = true; }
    if (PressedConce) { demo = false; gpuOn = false; }
}

static void clearScreen(ushort color)
{
    // Do it more times to clear all video pages
    int i;
    for (i=0; i<NUM_SCREEN_PAGES; ++i) {
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

    initCCBpolys();
}

static void script()
{
    runAnimationScript(demo, gpuOn);
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
