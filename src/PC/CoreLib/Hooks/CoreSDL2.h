#ifndef CORE_SDL2
#define CORE_SDL2

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#undef main

#include "Core.h"

class CoreSDL2 :	public Core
{
private:
	SDL_Surface *m_pSurface;
	SDL_Window *m_pWindow;

public:
	CoreSDL2();
	~CoreSDL2();

	bool init(const InitSettings& settings);
	void input();
	void render();
	int ticks();
};

#endif
