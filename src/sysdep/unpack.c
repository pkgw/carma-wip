/*
	<pack.c>
	??????? rjs Original version.
	?????89 bs  Improved efficiency using "register" declarations.
	01nov89 rjs Incoporated Brian's changes.
	03mar92 jm  Modified code for WIP usage (from Miriad).
	19mar95 jm  Added code for IEEE style machines.
	21jan98 jm  Added missing semicolons in unpackr_c().
	13sep06 pjt Cleanup while ensure 64bit compliance

Routines:
void unpack16_c ARGS(( char *in,    int *out, int n ));
void unpack32_c ARGS(( char *in,    int *out, int n ));
void  unpackr_c ARGS(( char *in,  float *out, int n ));
void  unpackd_c ARGS(( char *in, double *out, int n ));
*/

/* Global variables for just this file */

#include "wip.h"

#ifdef IEEEByteSwap
/***********************************************************************/
/*
 *  The unpack routines -- these convert between the host format and
 *  the disk format.  Disk format is IEEE 32 and 64 bit reals, and 2's
 *  complement integers.  Byte order is the FITS byte order (most
 *  significant bytes first).
 *
 *  This version is for a machine which uses IEEE internally, but which
 *  uses least significant bytes first (little endian), e.g. PCs and
 *  Alphas.
 *
 *  IEEE History:
 *    rjs  21nov94 Original version.
 *     jm  19mar95 Modified for use in WIP.
 */
/***********************************************************************/

/*  Unpack an array of 16 bit integers into 32 bit integers. */
/***********************************************************************/
void unpack16_c(char *in, int *out, int n)
{
    char *s;
    register int i;

    s = (char *)out;
    for (i = 0; i < n; i++) {
      *s++ = *(in+1);
      *s++ = *in;
      if (0x80 & *in) {
        *s++ = 0xFF;
        *s++ = 0xFF;
      } else {
        *s++ = 0;
        *s++ = 0;
      }
      in += 2;
    }
    return;
}

/*  Unpack an array of 32 bit integers into integers. */
/***********************************************************************/
void unpack32_c(char *in, int *out, int n)
{
    register int i;
    char *s;

    s = (char *)out;
    for (i = 0; i < n; i++) {
      *s++ = *(in+3);
      *s++ = *(in+2);
      *s++ = *(in+1);
      *s++ = *in;
      in += 4;
    }
    return;
}

/*  Unpack an array of IEEE reals into reals -- just do byte reversal. */
/***********************************************************************/
void unpackr_c(char *in, float *out, int n)
{
    char *s;
    register int i;

    s = (char *)out;
    for (i = 0; i < n; i++) {
      *s++ = *(in+3);
      *s++ = *(in+2);
      *s++ = *(in+1);
      *s++ = *in;
      in += 4;
    }
    return;
}

/*
 *  Unpack an array of doubles.  This involves simply performing
 *  byte reversal.
 */
/***********************************************************************/
void unpackd_c (char *in, double *out, int n)
{
    char *s;
    register int i;

    s = (char *)out;
    for (i = 0; i < n; i++) {
      *s++ = *(in+7);
      *s++ = *(in+6);
      *s++ = *(in+5);
      *s++ = *(in+4);
      *s++ = *(in+3);
      *s++ = *(in+2);
      *s++ = *(in+1);
      *s++ = *in;
      in += 8;
    }
}

#endif /* IEEEByteSwap */
/***********************************************************************/
/***********************************************************************/
#ifdef unicos

#define TWO15  0x8000
#define TWO16  0x10000
#define TWO31  0x80000000
#define TWO32  0x100000000
#define HILONG 0xFFFFFFFF00000000
#define LOLONG 0x00000000FFFFFFFF
#define WORD0  0x000000000000FFFF
#define WORD1  0x00000000FFFF0000
#define WORD2  0x0000FFFF00000000
#define WORD3  0xFFFF000000000000

/* Masks for IEEE floating format (both hi and lo types). */

#define IEEE_HISIGN     0x8000000000000000
#define IEEE_HIEXPO     0x7F80000000000000
#define IEEE_HIMANT     0x007FFFFF00000000
#define IEEE_LOSIGN     0x0000000080000000
#define IEEE_LOEXPO     0x000000007F800000
#define IEEE_LOMANT     0x00000000007FFFFF
#define IEEE_DMANT      0x000FFFFFFFFFFFF0
#define IEEE_DEXPO      0x7FF0000000000000

/* Masks for Cray floating format. */

#define CRAY_MANT       0x0000FFFFFF000000    /* Including unhidden bit. */
#define CRAY_MANT1      0x00007FFFFF000000           /* No unhidden bit. */
#define CRAY_DMANT      0x0000FFFFFFFFFFFF
#define CRAY_DMANT1     0x00007FFFFFFFFFFF
#define CRAY_EXPO       0x7FFF000000000000
#define SIGN            0x8000000000000000

/* Mask of a pointer to char giving the character offset in a Cray word. */

#define CHAR_OFFSET 0xE000000000000000

/* Code */

/*  Unpack an array of 16 bit integers into integers. */
/***********************************************************************/
void unpack16_c(char *in, int *out, int n)
{
  int temp,offset,i;
  int *ind;

  if (n <= 0) return;                         /* Return if nothing to do. */
  temp = (int)in;
  offset = ( temp & CHAR_OFFSET ) >> 62;     /* Get offset of first word. */
  ind = (int *)(temp & ~CHAR_OFFSET);            /* Get address of words. */

/* Handle the first few which are not word aligned. */

  switch (offset){
    case 1:  temp = (*ind >> 32) & WORD0;
             *out++ = (temp < TWO15 ? temp : temp - TWO16);
             if(--n == 0) break;
    case 2:  temp = (*ind >> 16) & WORD0;
             *out++ = (temp < TWO15 ? temp : temp - TWO16);
             if(--n == 0) break;
    case 3:  temp = (*ind++    ) & WORD0;
             *out++ = (temp < TWO15 ? temp : temp - TWO16);
             if(--n == 0) break;
  }

/* Handle those that are Cray-word-aligned. */

  for(i=0; i < n-3; i=i+4){
    temp = (*ind >> 48) & WORD0;
    *out++ = (temp < TWO15 ? temp : temp - TWO16);
    temp = (*ind >> 32) & WORD0;
    *out++ = (temp < TWO15 ? temp : temp - TWO16);
    temp = (*ind >> 16) & WORD0;
    *out++ = (temp < TWO15 ? temp : temp - TWO16);
    temp = (*ind++    ) & WORD0;
    *out++ = (temp < TWO15 ? temp : temp - TWO16);
  }
  n -= i;

/* Handle the last few which are not Cray-word-aligned. */

  if(n-- > 0){
    temp = (*ind >> 48) & WORD0;
    *out++ = (temp < TWO15 ? temp : temp - TWO16);
    if(n-- > 0){
      temp = (*ind >> 32) & WORD0;
      *out++ = (temp < TWO15 ? temp : temp - TWO16);
      if(n-- > 0){
        temp = (*ind >> 16) & WORD0;
        *out++ = (temp < TWO15 ? temp : temp - TWO16);
      }
    }
  }
}

/*  Unpack an array of 32 bit integers into integers. */
/***********************************************************************/
void unpack32_c(char *in, int *out, int n)
{
  int temp,offset,i;
  int *ind;

  if (n <= 0) return;                         /* Return if nothing to do. */
  temp = (int)in;
  offset = ( temp & CHAR_OFFSET ) >> 63;     /* Get offset of first word. */
  ind = (int *)(temp & ~CHAR_OFFSET);            /* Get address of words. */

/* Handle one which is not Cray word aligned. */

  if (offset == 1) {
    temp = (*ind++ & LOLONG);
    *out++ = (temp < TWO31 ? temp : temp - TWO32);
  }
  n -= offset;

/* Handle those which are Cray word aligned. */

  for(i=0; i < n-1; i=i+2){
    temp = (*ind >> 32) & LOLONG;
    *out++ = (temp < TWO31 ? temp : temp - TWO32);
    temp = (*ind++    ) & LOLONG;
    *out++ = (temp < TWO31 ? temp : temp - TWO32);
  }
  n -= i;

/* Possibly handle a last one which is not Cray word aligned. */

  if(n==1){
    temp = (*ind >> 32) & LOLONG;
    *out++ = (temp < TWO31 ? temp : temp - TWO32);
  }
}

/*  Unpack an array of IEEE reals into Cray reals. */
/***********************************************************************/
void unpackr_c(char *in, float *out, int n)
{
  int temp,tin,offset,i,bias;
  int *ind,*outd;

  if (n <= 0) return;                         /* Return if nothing to do. */
  temp = (int)in;
  offset = ( temp & CHAR_OFFSET ) >> 63;     /* Get offset of first word. */
  ind = (int *)(temp & ~CHAR_OFFSET);            /* Get address of words. */
  outd = (int *)out;
  bias = ((16384-126) <<48) + (1 << 47);

/* Handle the first one if it is not aligned on a Cray word. */

  if (offset == 1) {
    tin = *ind++;
    *outd++ = (tin & IEEE_LOEXPO ? (((tin & IEEE_LOEXPO) << 25)+bias) |
        ((tin & IEEE_LOMANT) << 24) | ((tin & IEEE_LOSIGN) << 32) : 0);
  }
  n -= offset;

/* Handle the bulk of them that are aligned on Cray words. */

  for (i = 0; i < n - 1; i = i + 2) {
    tin = *ind++;
    *outd++ = (tin & IEEE_HIEXPO ? (((tin & IEEE_HIEXPO) >> 7)+bias) |
        ((tin & IEEE_HIMANT) >> 8 ) |  (tin & IEEE_HISIGN)        : 0);
    *outd++ = (tin & IEEE_LOEXPO ? (((tin & IEEE_LOEXPO) << 25)+bias) |
        ((tin & IEEE_LOMANT) << 24) | ((tin & IEEE_LOSIGN) << 32) : 0);
  }
  n -= i;

/* Handle the last one, if needed, which is not aligned on a Cray word. */

  if (n == 1) {
    tin = *ind;
    *outd++ = (tin & IEEE_HIEXPO ? (((tin & IEEE_HIEXPO) >> 7)+bias) |
        ((tin & IEEE_HIMANT) >> 8 ) |  (tin & IEEE_HISIGN)        : 0);
  }
}

/*
 *  Unpack an array of IEEE double precision numbers into Cray reals.
 *  This assumes that a "double" and a "float" are identical.
 */
/***********************************************************************/
void unpackd_c(char *in, double *out, int n)
{
  int bias,i,tin;
  int *ind,*outd;

  ind = (int *)in;
  outd = (int *)out;
  bias = ((16384 - 1022) << 48) | (1 << 47);

  for (i = 0; i < n; i++){
    tin = *ind++;
    *outd++ = (tin & IEEE_DEXPO ? (tin & SIGN) |
      (((tin & IEEE_DEXPO) >> 4) + bias) | ((tin & IEEE_DMANT) >> 5) : 0 );
  }
}

#endif /* unicos */
/***********************************************************************/
/***********************************************************************/
#ifdef vaxc

/***********************************************************************/
void unpack16_c(char *in, int *out, int n)
{
    register short int *from;
    register int i;

    from = (short int *)in;
    for (i = 0; i < n; i++)
      *out++ = ntohs(*from++);

    return;
}

/***********************************************************************/
void unpack32_c(char *in, int *out, int n)
{
    register int *from;
    register int i;

    from = (int *)in;
    for (i = 0; i < n; i++)
      *out++ = ntohl(*from++);

    return;
}

/***********************************************************************/
void unpackr_c(char *in, float *out, int n)
{
    register char *from;
    register int i;
    union {char c[4]; float f;} x;

    from = (char *)in;
    for (i = 0; i < n; i++) {
      x.f = 0;
      x.c[1] = *from++;
      x.c[0] = *from++;
      x.c[3] = *from++;
      x.c[2] = *from++;
      *out++ = 4.0 * x.f;
    }

    return;
}

/***********************************************************************/
void unpackd_c(char *in, double *out, int n)
{
    register int *from;
    register int i;

    from = (int *)in;
    for (i = 0; i < n; i++)
      *out++ = ntohl(*from++);

    return;
}

#endif /* vaxc */
