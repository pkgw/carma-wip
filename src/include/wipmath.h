/*
	"wipmath.h" -- Math function declarations.
	includes <math.h> and requires the -lm flag on compilation.

	11apr90 jm  Original code.
	15aug92 jm  Modified NINT to work correctly for negative values.
*/

#ifndef WIP_MATH
#define WIP_MATH

#include <math.h>

#ifndef PI
#define PI (3.1415926535897932384626434)
#endif
#define RPDEG (PI / 180.0)

#ifndef MIN
#define MIN(a,b)   ((a) <= (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)   ((a) >= (b) ? (a) : (b))
#endif
#define IN_RANGE(x,L,U) ((U) > (L) ? (x) >= (L) && (x) <= (U)\
                                   : (x) >= (U) && (x) <= (L))

#define ABS(a)     ((a) >= 0 ? (a) : -(a))
#define INT(x)     ((int)floor((double)(x)))
#define EXP(x)     (exp((double)(x)))
#define LOG(x)     (log((double)(x)))
#define LOG10(x)   (log((double)(x)) / log(10.0))
#define SIN(x)     (sin((double)(x)))
#define COS(x)     (cos((double)(x)))
#define TAN(x)     (tan((double)(x)))
#define ASIN(x)    (asin((double)(x)))
#define ACOS(x)    (acos((double)(x)))
#define ATAN(x)    (atan((double)(x)))
#define SQRT(x)    (sqrt((double)(x)))
#define ATAN2(y,x) (atan2((double)(y), (double)(x)))
#define POW(x,y)   (pow((double)(x), (double)(y)))

/*  #define NINT(x)    ((int)floor((double)((x) + 0.5)))  */
#define NINT(x) ((x) >= 0 ? (int)floor((double)((x) + 0.5)) : \
                            (int)ceil((double)((x) - 0.5)))

#endif /* WIP_MATH */
