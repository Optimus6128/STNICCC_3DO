#ifndef TOOLS_H
#define TOOLS_H

#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define FONT_SIZE (FONT_WIDTH * FONT_HEIGHT)

void initFonts(void);
void initTimer(void);
void drawText(int xtp, int ytp, char *text);
void drawZoomedText(int xtp, int ytp, char *text, int zoom);
void drawNumber(int xtp, int ytp, int num);

void resetCharPos(void);
void printWait(char *text, int ms);
void printWaitValue(char *text, int value, int ms);

void showFPS(void);
int getTicks(void);

void setPal(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16 *pal);
void setPalWithFades(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16* pal, int numFades, int r2, int g2, int b2);
void setFontColor(ushort c);

#define READ_FROM_SHORT_POINTER(ptr) (ushort)(((int)*((uchar*)ptr) << 8) | ((int)*((uchar*)ptr + 1)))

#endif
