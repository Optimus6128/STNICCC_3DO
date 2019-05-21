#include "types.h"
#include "tools.h"
#include "mathutil.h"
#include "main.h"

int divTab[DIV_TAB_SIZE];
int icos[256];
int isin[256];
int shr[257]; // ugly way to get precalced fast right shift for division with power of two numbers

int getRand(int from, int to)
{
    int rnd;
    if (from > to) return 0;
    if (from==to) return to;
    rnd = from + (rand() % (to - from));
    return rnd;
}

int getShr(int n)
{
    int b = -1;
    do{
        b++;
    }while((n>>=1)!=0);
    return b;
}

static void initDivs()
{
    int i, ii;
    for (i=0; i<DIV_TAB_SIZE; ++i) {
        ii = i - DIV_TAB_SIZE / 2;
        if (i==0) ++ii;

        divTab[i] = (1 << DIV_TAB_SHIFT) / ii;
    }
}

static void initShr()
{
    int i;
    for (i=1; i<=256; i++)
    {
        shr[i] = getShr(i);
    }
}

static void initSines()
{
    int i;
    for(i=0; i<256; i++)
    {
        isin[i] = SinF16(i << 16) >> 4;
        icos[i] = CosF16(i << 16) >> 4;
    }
}

void initMathUtil()
{
    initShr();
    initDivs();
    initSines();
}
