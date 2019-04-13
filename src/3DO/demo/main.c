#include "types.h"
#include "main.h"

#include "system_graphics.h"

#include "tools.h"
#include "ScriptMain.h"

static bool restart = false;

static bool PressedA = false;
static bool PressedB = false;
static bool PressedC = false;
static bool PressedAonce = false;
static bool PressedBonce = false;
static bool PressedConce = false;
static bool gpuOn = false;

static void quit()
{
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
    static int progress = 0;

    int x,y,yp;
    ushort *vram;

    while(!PressedAonce && !PressedBonce) {
        processJoystick();
        for(y=0; y<SCREEN_HEIGHT; ++y) {
            vram = getVideoramAddress() + (y >> 1) * (2 * SCREEN_WIDTH) + (y & 1);
            yp = ((y + 1) * progress) & 255;
            for (x=0;x<SCREEN_WIDTH; ++x) {
                *vram = x ^ yp;
                vram += 2;
            }
        }
        displayScreen();
    }
    if (PressedBonce) gpuOn = true;

    PressedAonce = false;
    PressedBonce = false;

    ++progress;
}

static void clearScreen(ushort color)
{
    // Do it twice to clear both two video pages
    clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
    displayScreen();
    clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
    displayScreen();
}

static void initSystem()
{
	OpenMathFolio();
    OpenGraphicsFolio();
    InitEventUtility(1,0,LC_Observer);
	OpenAudioFolio();
}

static void initStuff()
{
	initSystem();
	initGraphics();

	progressScreen();
	initFonts();
	progressScreen();
	initTimer();

	if (gpuOn) {
        progressScreen();
        initCCBpolys();
	}
}

static void script()
{
    if (PressedConce) vsync = !vsync;

    runAnimationScript(gpuOn);
}

static void mainLoop()
{
    progressScreen();

    clearScreen(15);

	while(!restart)
	{
	    processJoystick();

		script();

        showFPS();
		renderTextSpace();

		displayScreen();

		clearTextSpace();
	}

	quit();
}


int main()
{
	initStuff();

	mainLoop();
}
