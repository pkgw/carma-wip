/*
 *	<image.h> --- Image include file.
 *	26oct91 jm  Original code.
 *      10nov93 jm  Completely rewritten.
 *
 */

#ifdef WIP_IMAGE
#undef WIP_IMAGE

#include "wipdefs.h"

/*  Define the maximum number of allowed axes. */

#define MAXNAX 7

/*  Typedefs and defines dealing with formats and conversions. */

typedef float FLOAT;

#if defined(IEEEByteSwap) || defined(unicos) || defined(vaxc)
#define NO_CVT 0
typedef int INT16;
extern void unpack16_c ARGS(( char *in,    int *out, int n ));
extern void unpack32_c ARGS(( char *in,    int *out, int n ));
extern void unpackr_c  ARGS(( char *in,  float *out, int n ));
extern void unpackd_c  ARGS(( char *in, double *out, int n ));
#else
#define NO_CVT 1
typedef short int INT16;
#define unpack16_c(in,out,n)  /* Empty */
#define unpack32_c(in,out,n)  /* Empty */
#define unpackr_c(in,out,n)   /* Empty */
#define unpackd_c(in,out,n)   /* Empty */
#endif

/* Some useful utilities follow. */

#ifndef MAX
#define MAX(a,b)     ((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)     ((a) <= (b) ? (a) : (b))
#endif

/* old #define ROUNDUP(a,b) ((b)*(((a)-1)/(b)+1)) */
#define ROUNDUP(a,b) ((b)*(((a)+(b)-1)/(b)))

#include "drivers.h"

#endif /* WIP_IMAGE */
