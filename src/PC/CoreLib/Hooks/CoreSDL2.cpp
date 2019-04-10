#include <thread>
#include <string>
#include <iostream>

#include "CoreSDL2.h"
#include "InputBuffer.h"
#include "../main.h"

CoreSDL2::CoreSDL2()
{
	SDL_Init(SDL_INIT_EVERYTHING);
}

CoreSDL2::~CoreSDL2()
{
	SDL_Quit();
}

bool CoreSDL2::init(const InitSettings& settings)
{
	int windowMode = 0;
	if (settings.fullscreen) {
		windowMode = SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED;
		//windowMode = SDL_WINDOW_FULLSCREEN;
	}

	m_pWindow = SDL_CreateWindow("STNICCC test", settings.screenWidth / 4, settings.screenHeight / 4, settings.screenWidth, settings.screenHeight, windowMode);
	if (!m_pWindow)
		return false;

	m_pSurface = SDL_GetWindowSurface(m_pWindow);
	if (!m_pSurface)
		return false;

	m_screen.width = m_pSurface->w;
	m_screen.height = m_pSurface->h;
	m_screen.bpp = settings.screenBpp;	// should get SDL format? In case it returns something different than what given in settings
	m_screen.vram = (unsigned char*)m_pSurface->pixels;

	return true;
}

void groundInput(inputMap &iMap)
{
	for (auto &key : iMap) {
		if (key.second == KEY_JUST_PRESSED)
			key.second = KEY_PRESSED;

		if (key.second == KEY_JUST_RELEASED)
			key.second = KEY_RELEASED;
	}
}

void CoreSDL2::input()
{
	SDL_Event event;

	groundInput(m_input.keyboard);
	groundInput(m_input.mouse);

	m_input.anykey = false;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			m_input.quit = true;
			break;

		case SDL_KEYDOWN:
		{
			m_input.keyboard[event.key.keysym.sym] = KEY_JUST_PRESSED;
			m_input.anykey = true;
		}
		break;

		case SDL_KEYUP:
		{
			m_input.keyboard[event.key.keysym.sym] = KEY_JUST_RELEASED;
			//m_input.anykey = true;
		}
		break;

		case SDL_MOUSEMOTION:
			SDL_GetMouseState(&m_input.mousePosX, &m_input.mousePosY);
		break;

		case SDL_MOUSEBUTTONDOWN:
			m_input.mouse[event.button.button] = KEY_JUST_PRESSED;
		break;

		case SDL_MOUSEBUTTONUP:
			m_input.mouse[event.button.button] = KEY_JUST_RELEASED;
		break;
		}
	}
}

void CoreSDL2::render()
{
	SDL_UpdateWindowSurface(m_pWindow);
}

int CoreSDL2::ticks()
{
	return SDL_GetTicks();
}
