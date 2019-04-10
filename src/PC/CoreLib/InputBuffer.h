#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

#include <map>

using inputMap = std::map<uint, uint>;

enum {KEY_RELEASED, KEY_JUST_PRESSED, KEY_PRESSED, KEY_JUST_RELEASED, KEY_STATES_NUM};

struct InputBuffer
{
	bool quit = false;
	bool anykey = false;

	inputMap keyboard;
	inputMap mouse;
	int mousePosX, mousePosY;
	int mouseDeltaX, mouseDeltaY;

	uint keyState(uint keyId)
	{
		uint state = KEY_RELEASED;
		if (keyboard.find(keyId) != keyboard.end()) {
			state = keyboard[keyId];
		}
		return state;
	}

	uint mbuttonState(uint mbuttonId)
	{
		uint state = KEY_RELEASED;
		if (mouse.find(mbuttonId) != mouse.end()) {
			state = mouse[mbuttonId];
		}
		return state;
	}
};

#endif
