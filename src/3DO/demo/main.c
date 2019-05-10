#include "types.h"
#include "main.h"

#include "system_graphics.h"

#include "tools.h"
#include "ScriptMain.h"
#include "sound.h"

#include "fx_stars0.h"

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
bool fpsOn = true;

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

static void menuScreen()
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

    // clean up some keys before start
    PressedLonce = false;
    PressedRonce = false;
    restart = false;
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

    initCCBbuffers();
}

static int startT;

static void demoScript()
{
    int demoT = getTicks() - startT;
    static int tz1 = 256;
    static int tz2 = 256;
    static int tz3 = 256;
    static int tz4 = 256;
    const int spz = 16;

    const int animTstart = 30000;
    const int starsTend = animTstart - 128;

    int fpals = demoT >> 4;
    if (demoT > 255 && demoT < starsTend - 256) fpals = 256;
    if (demoT >= starsTend - 256) fpals = starsTend - demoT;
    if (demoT < 0) demoT = 0;


    if (demoT < starsTend + 256) {
        clearScreenWithRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    }

    if (demoT >= animTstart)
        runAnimationScript();

    if (demoT < starsTend) {
        stars0Run(getTicks(), fpals);
    }

    //18
    //23-24
    //29-30
    //35-36
    //drawNumber(0, 0, musicStatus);

    if (musicStatus >= 18 && musicStatus <=20) {drawZoomedText(24,64, "Optimus", tz1); if (tz1 <= 512) tz1 += spz; };
    if (musicStatus >= 23 && musicStatus <=26) {drawZoomedText(160,96, "Presents", tz2); if (tz2 <= 512) tz2 += spz; };
    if (musicStatus >= 29 && musicStatus <=32) {drawZoomedText(32,128, "STNICCC Demo", tz3); if (tz3 <= 512) tz3 += spz; };
    if (musicStatus >= 35 && musicStatus <=38) {drawZoomedText(192,160, "For 3DO", tz4); if (tz4 <= 512) tz4 += spz; };
}


static void mainLoop()
{
    menuScreen();

    if (demo)
        clearAllScreens(0);
    else
        clearAllScreens(BG_COLOR);

    if (benchTexture) {
        initBenchTextures(true);
        initCCBPolysTexture();
    }
    else
        initCCBpolysFlat();

    if (benchScreens)
        initBenchTextures(false);

    vsync = false;
    if (demo) {
        vsync = true;
        stars0Init();
        startMusic();
    }
    if (DEBUG_ON) vsync = true;


    startT = getTicks();
	while(!restart)
	{
	    processJoystick();

		if (demo) {
            demoScript();
		} else {
            runAnimationScript();
		}

        if (!demo && fpsOn) showFPS();
		displayScreen();
        if (musicStatus==0) { startT = getTicks(); };
	}

	quit();
}


int main()
{
	initStuff();

	mainLoop();
}
