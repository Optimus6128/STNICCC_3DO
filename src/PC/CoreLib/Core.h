#ifndef CORE_H
#define CORE_H

#include "ScreenBuffer.h"
#include "InputBuffer.h"

struct InitSettings
{
	InitSettings(int scrW = DEFAULT_SCREEN_WIDTH, int scrH = DEFAULT_SCREEN_HEIGHT, int bpp = DEFAULT_SCREEN_BPP, bool fs = false, bool usecores = false)
	{
		screenWidth = scrW;
		screenHeight = scrH;
		screenBpp = bpp;
		fullscreen = fs;
		multithreading = usecores;
	}

	int screenWidth, screenHeight;
	int screenBpp;
	bool fullscreen;
	bool multithreading;
};

class Core
{
protected:
	ScreenBuffer m_screen;
	InputBuffer m_input;
	int m_numCores;

public:
	virtual ~Core() {};

	virtual bool init(const InitSettings& settings) = 0;
	virtual void input() = 0;
	virtual void render() = 0;
	virtual int ticks() = 0;

	ScreenBuffer* getScreen() { return &m_screen; }
	InputBuffer* getInput() { return &m_input; }
	void setNumCores(int nCores) { m_numCores = nCores; }
	int getNumCores() { return m_numCores; }
};

#endif
