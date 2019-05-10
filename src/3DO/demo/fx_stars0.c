#include "types.h"
#include "main.h"
#include "tools.h"
#include "fx_stars0.h"
#include "sprite_engine.h"
#include "system_graphics.h"

#define NUM_STARS0 96
#define NUM_FADE_PALS0 32

static sprite *stars[NUM_STARS0];

static ubyte starbmp[STAR0_SIZE];
static uint16 starpal[32 * NUM_FADE_PALS0 * 2]; // there is a memory leak I can't find now

static ubyte rotblobbmp[ROTBLOB_SIZE];
static uint16 rotblobpal[32 * NUM_FADE_PALS0 * 2];  // that's why *2, to protect it with a hack
static uint16 rotblobhitpal[32];

static starTrans star[NUM_STARS0];

static int starZoom[2048];

static int fadePals = 0;

static void randomizeStar(int starNum)
{
    int r = rand() & 255;
    starTrans *starPtr = &star[starNum];

    starPtr->z = (rand() & 1023) + 1024;
    starPtr->v = -7 + (rand() & 3);

    if ((starPtr->t == STAR0_TYPE_ROTBLOB) || (r > 240))
    {
        starPtr->x = (rand() & 127) - 64;
        starPtr->y = (rand() & 127) - 64;
        stars[starNum]->cel->ccb_SourcePtr = (CelData*)rotblobbmp;
        starPtr->t = STAR0_TYPE_ROTBLOB;
    }
    else
    {
        starPtr->x = (rand() & 255) - 128;
        starPtr->y = (rand() & 255) - 128;
    }
}

static void rotBlobBitmapInit()
{
    int x,y,i;
    int xc, yc;
    int c, r;

    i = 0;
    for (y=0; y<ROTBLOB_HEIGHT; y++)
    {
        yc = y - ROTBLOB_HEIGHT / 2;
        for (x=0; x<ROTBLOB_WIDTH; x++)
        {
            xc = x - ROTBLOB_WIDTH / 2;
            r = xc*xc + yc*yc + ((SinF16(3.0 * Atan2F16(xc, yc)) + 65536) >> 12);
            if (r==0) r = 1;
            c = 1024 / r;
            if (c > 31) c = 31;
            if (c < 8) c = 0;
            rotblobbmp[i++] = c;
        }
    }
}

void stars0Init()
{
    int x,y,i;
    int xc, yc;
    int c, r;

    i = 0;
    for (i=1; i<2048; i++)
        starZoom[i] = 32768 / i;

    i = 0;
    for (y=0; y<STAR0_HEIGHT; y++)
    {
        yc = y - STAR0_HEIGHT / 2;
        for (x=0; x<STAR0_WIDTH; x++)
        {
            xc = x - STAR0_WIDTH / 2;
            r = xc*xc + yc*yc;
            if (r==0) r = 1;
            c = 1024 / r;
            if (c > 31) c = 31;
            if (c < 8) c = 0;
            starbmp[i++] = c;
        }
    }

    rotBlobBitmapInit();

    for (i=0; i<NUM_FADE_PALS0; i++)
    {
        setPal(0,31, 0,0,0, (255 * i) / NUM_FADE_PALS0, (223 * i) / NUM_FADE_PALS0, (160 * i) / NUM_FADE_PALS0, &rotblobpal[i << 5]);
        setPal(0,31, 0,0,0, (127 * i) / NUM_FADE_PALS0, (191 * i) / NUM_FADE_PALS0, (255 * i) / NUM_FADE_PALS0, &starpal[i << 5]);
    }
    setPal(0,31, 0,0,0, 255,0,0, rotblobhitpal);

    for (i=0; i<NUM_STARS0; i++)
    {
        stars[i] = newSprite(STAR0_WIDTH, STAR0_HEIGHT, starbmp);
        //setPalette(stars[i], starpal);
        setSpriteAlpha(stars[i], true);
        if (i > 0) LinkCel(stars[i-1]->cel, stars[i]->cel);

        star[i].t = STAR0_TYPE_BLOB;
        randomizeStar(i);
    }
}

static int alternate = 0;

static void updateStars(uint32 time)
{
    int i, px, py, zpal, angle;

    starTrans *starPtr = star;

    angle = time << 14;

    //isHit = false;
    for (i=0; i<NUM_STARS0; i++)
    {
        int sz = starPtr->z;
        px = (starPtr->x << 8) / sz;
        py = (starPtr->y << 8) / sz; // with 512 and zoom 32768 and 256 stars crashes.

        stars[i]->posX = SCREEN_WIDTH / 2 + px; stars[i]->posY = SCREEN_HEIGHT / 2 + py;

        zpal = (((1024 - sz) * NUM_FADE_PALS0) >> 10) + (NUM_FADE_PALS0 >> 3);


        zpal = (zpal * fadePals) >> 8;

        if (zpal < 0) zpal = 0;
        if (zpal > NUM_FADE_PALS0 - 1) zpal = NUM_FADE_PALS0 - 1;

        if (starPtr->t == STAR0_TYPE_BLOB)
        {
            stars[i]->zoom = starZoom[sz] >> 1; //32768 / starZ[i];
            setPalette(stars[i], &starpal[zpal << 5]);
            mapZoomSprite(stars[i]);
        }
        else
        {
            stars[i]->zoom = starZoom[sz];// << 1;

            stars[i]->angle = angle;
            setPalette(stars[i], &rotblobpal[zpal << 5]);

            if (sz > (1023 / 2))
            {
                if ((i & 3) == alternate)
                    mapZoomRotateSprite(stars[i]);
            }
            else if (sz > (511 / 2))
            {
                if ((i & 1) == (alternate & 1))
                    mapZoomRotateSprite(stars[i]);
            }
            else
            {
                mapZoomRotateSprite(stars[i]);
            }
        }

        starPtr->z += starPtr->v;
        if (starPtr->z <=0) randomizeStar(i);
        ++starPtr;
    }

    alternate = (alternate + 1) & 3;
}

void stars0Run(uint32 time, int fpals)
{
    fadePals = fpals;

    updateStars(time);
    drawSprite(stars[0]);
}
