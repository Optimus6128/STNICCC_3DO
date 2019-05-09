#ifndef FX_STARS0_H
#define FX_STARS0_H

#define STAR0_WIDTH 32
#define STAR0_HEIGHT 32
#define STAR0_SIZE (STAR0_WIDTH * STAR0_HEIGHT)

#define ROTBLOB_WIDTH 32
#define ROTBLOB_HEIGHT 32
#define ROTBLOB_SIZE (ROTBLOB_WIDTH * ROTBLOB_HEIGHT)

enum {STAR0_TYPE_BLOB, STAR0_TYPE_ROTBLOB};

typedef struct starTrans
{
    int x,y,z,v,t;
}starTrans;

void stars0Init(void);
void stars0Run(uint32 time);

#endif
