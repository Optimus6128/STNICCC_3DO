#ifndef TOOLS_H
#define TOOLS_H

#define TEXT_SPACE_WIDTH 32
#define TEXT_SPACE_HEIGHT 16
#define TEXT_SPACE_SIZE (TEXT_SPACE_WIDTH * TEXT_SPACE_HEIGHT)

void initFonts(void);
void initTimer(void);
void drawText(int xtp, int ytp, int cn, char *text);
void drawNumber(int xtp, int ytp, int num);
void showFPS(void);
void renderTextSpace(void);
void clearTextSpace(void);
int getTicks(void);
void setPal(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16 *pal);
void setPalWithFades(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16* pal, int numFades, int r2, int g2, int b2);

#endif
