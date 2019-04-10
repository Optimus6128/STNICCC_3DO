#include "types.h"
#include "main.h"

#include "system_graphics.h"

#include "tools.h"


static bool PressedA = false;
static bool PressedAonce = false;


int32 getJoystickState()
{
	ControlPadEventData cpaddata;
	cpaddata.cped_ButtonBits=0;
	GetControlPad(1,0,&cpaddata);

	return ( cpaddata.cped_ButtonBits );
}

int32 processJoystick(void)
{
	int joybits;

	joybits=getJoystickState();

	if (joybits&ControlStart)
        return 1;

	if(joybits&ControlA) {
		PressedAonce = !PressedA;
		PressedA = true;
	}
	else PressedA = false;

	return(0);
}

void initSystem()
{
	OpenMathFolio();
    OpenGraphicsFolio();
    InitEventUtility(1,0,LC_Observer);
	OpenAudioFolio();
}

void initStuff()
{
	initSystem();
	initGraphics();

	initFonts();
	initTimer();
}

void script()
{
    if (PressedAonce) vsync = !vsync;
}

int32 mainLoop()
{
	for(;;)
	{
	    processJoystick();

		script();

        showFPS();
		renderTextSpace();

		displayScreen();

		++nframe;
	}
}


int main()
{
	initStuff();

	mainLoop();
}
