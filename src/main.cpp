#define WIN32_LEAN_AND_MEAN 
#define VC_EXTRALEAN 

#include "main.h"

#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <windows.h>

#include <string>
#include <vector>

#include "Typedefs.h"
#include "Core.h"
#include "Hooks\CoreSDL2.h"
#include "ScreenBuffer.h"
#include "InputBuffer.h"
#include "Script\ScriptMain.h"
#include "CoreLib/Modules/Drawing/DrawingPolygon.h"

ScreenBuffer *screen;
InputBuffer *input;

int main(int argc, char* argv[])
{
	std::vector<std::string> arguments;
	for (int i = 0; i < argc; ++i) {
		arguments.push_back(argv[i]);
	}

	Core *core = new CoreSDL2();

	core->init(InitSettings(SCREEN_WIDTH, SCREEN_HEIGHT, 32, false));

	screen = core->getScreen();
	input = core->getInput();


	initDrawingFrameworkPolygon(screen);


	Script::init(screen);
	do {
		core->input();
		Script::run(screen, input);
		core->render();
	} while (!input->quit);

	Script::deinit();

	delete(core);
    return 0;
}
