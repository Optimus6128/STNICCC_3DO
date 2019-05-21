#ifndef MATHUTIL_H
#define MATHUTIL_H

#define FP_BASE 12

#define FLOAT_TO_FIXED(f,b) ((int)((f) * (1 << b)))
#define INT_TO_FIXED(i,b) ((i) * (1 << b))
#define UINT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)
#define FIXED_TO_FLOAT(x,b) ((float)(x) / (1 << b))
#define FIXED_MUL(x,y,b) (((x) * (y)) >> b)
#define FIXED_DIV(x,y,b) (((x) << b) / (y))
#define FIXED_SQRT(x,b) (sqrt((x) << b))

#define PI 3.14159265359f
#define DEG256RAD ((2 * PI) / 256.0f)

#define DIV_TAB_SIZE 4096
#define DIV_TAB_SHIFT 16

typedef struct vertex
{
    int x, y, z;
}vertex;

typedef struct lineList
{
	int *linePoints;
	int linesNum;
}lineList;

extern int divTab[DIV_TAB_SIZE];
extern int icos[256];
extern int isin[256];
extern int shr[257];

int getRand(int from, int to);
void initMathUtil(void);

#endif
