/*
	<palette.c>
	12dec94 jm  Original C code (based on PGPLOT example).
	13feb95 jm  Added levels argument to adjust color index range.
	27sep96 jm  Added cyclic table.

Routines:
void wippalette ARGS(( int which, int levels ));
*/

#include "wip.h"

/* Global Variables needed just for this file */

static float grayL[] = {0.0, 1.0};
static float grayR[] = {0.0, 1.0};
static float grayG[] = {0.0, 1.0};
static float grayB[] = {0.0, 1.0};

static float rainbowL[] = {-0.5, 0.0, 0.17, 0.33, 0.50, 0.67, 0.83, 1.0, 1.7};
static float rainbowR[] = { 0.0, 0.0,  0.0,  0.0,  0.6,  1.0,  1.0, 1.0, 1.0};
static float rainbowG[] = { 0.0, 0.0,  0.0,  1.0,  1.0,  1.0,  0.6, 0.0, 1.0};
static float rainbowB[] = { 0.0, 0.3,  0.8,  1.0,  0.3,  0.0,  0.0, 0.0, 1.0};

static float heatL[] = {0.0, 0.2, 0.4, 0.6, 1.0};
static float heatR[] = {0.0, 0.5, 1.0, 1.0, 1.0};
static float heatG[] = {0.0, 0.0, 0.5, 1.0, 1.0};
static float heatB[] = {0.0, 0.0, 0.0, 0.3, 1.0};

static float irafL[] = {0.0, 0.5, 0.5, 0.7, 0.7, 0.85, 0.85, 0.95, 0.95, 1.0};
static float irafR[] = {0.0, 1.0, 0.0, 0.0, 0.3,  0.8,  0.3,  1.0,  1.0, 1.0};
static float irafG[] = {0.0, 0.5, 0.4, 1.0, 0.0,  0.0,  0.2,  0.7,  1.0, 1.0};
static float irafB[] = {0.0, 0.0, 0.0, 0.0, 0.4,  1.0,  0.0,  0.0, 0.95, 1.0};

static float aipsL[] = {0.0, 0.1, 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4, 0.5,
                        0.5, 0.6, 0.6, 0.7, 0.7, 0.8, 0.8, 0.9, 0.9, 1.0};
static float aipsR[] = {0.0, 0.0, 0.3, 0.3, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
static float aipsG[] = {0.0, 0.0, 0.3, 0.3, 0.0, 0.0, 0.0, 0.0, 0.8, 0.8,
                        0.6, 0.6, 1.0, 1.0, 1.0, 1.0, 0.8, 0.8, 0.0, 0.0};
static float aipsB[] = {0.0, 0.0, 0.3, 0.3, 0.7, 0.7, 0.7, 0.7, 0.9, 0.9,
                        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

static float tjpL[] = {0.0, 0.5, 0.5, 1.0};
static float tjpR[] = {0.2, 0.6, 0.6, 1.0};
static float tjpG[] = {0.0, 0.0, 0.5, 1.0};
static float tjpB[] = {1.0, 0.0, 0.0, 0.0};

static float saoAL[] = {0.0, 0.125, 0.25, 0.5, 0.64, 0.77, 1.0};
static float saoAR[] = {0.0, 0.0,   0.0,  1.0, 1.0,  1.0,  1.0};
static float saoAG[] = {0.0, 0.0,   1.0,  0.0, 0.0,  0.0,  1.0};
static float saoAB[] = {0.0, 0.0,   0.0,  1.0, 0.5,  0.0,  0.0};

static float saoBBL[] = {0.0, 0.25, 0.5, 0.75, 1.0};
static float saoBBR[] = {0.0, 0.0,  1.0, 1.0,  1.0};
static float saoBBG[] = {0.0, 0.0,  0.0, 1.0,  1.0};
static float saoBBB[] = {0.0, 0.0,  0.0, 0.0,  1.0};

static float saoHEL[] = {0.0, 0.015, 0.03,  0.065, 0.125, 0.25, 0.5,  1.0};
static float saoHER[] = {0.0, 0.5,   0.5,   0.5,   0.5,   0.5,  0.75, 1.0};
static float saoHEG[] = {0.0, 0.0,   0.0,   0.0,   0.5,   0.75, 0.81, 1.0};
static float saoHEB[] = {0.0, 0.125, 0.375, 0.625, 0.625, 0.25, 0.25, 1.0};

static float saoI8L[] = {0.0,   0.125, 0.25, 0.5, 0.5, 0.75, 0.75, 1.0, 1.0};
static float saoI8R[] = {0.087, 0.087, 0.0,  0.0, 0.0, 1.0,  1.0,  1.0, 1.0};
static float saoI8G[] = {0.087, 0.087, 1.0,  0.0, 1.0, 0.0,  1.0,  0.0, 1.0};
static float saoI8B[] = {0.087, 0.087, 0.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0};

static float dsL[] = {0.0,   0.008, 0.055, 0.11,  0.173, 0.323, 0.331,
                      0.449, 0.457, 0.496, 0.567, 0.701, 0.843, 0.850,
                      0.858, 0.913, 1.0};
static float dsR[] = {0.0,   0.282, 0.651, 0.0,   0.0,   0.0,   0.0,
                      0.0,   0.0,   0.639, 0.973, 0.98,  0.996, 1.0,
                      1.0,   0.98,  1.0};
static float dsG[] = {0.0,   0.278, 0.0,   0.0,   0.486, 0.961, 0.973,
                      1.0,   1.0,   0.953, 0.973, 0.533, 0.267, 0.0,
                      0.294, 0.584,  1.0};
static float dsB[] = {0.0,   0.239, 0.647, 1.0,   1.0,   0.961, 0.898,
                      0.486, 0.0,   0.0,   0.0,   0.0,   0.0,   0.0,
                      0.294, 0.588, 1.0};

/* This cycles from Red to Green to Blue to Red.... */
static float cyclicL[] = {0.0, 0.166, 0.333, 0.5,   0.666, 0.833, 1.0};
static float cyclicR[] = {1.0, 1.0,   0.0,   0.0,   0.0,   1.0,   1.0};
static float cyclicG[] = {0.0, 1.0,   1.0,   1.0,   0.0,   0.0,   0.0};
static float cyclicB[] = {0.0, 0.0,   0.0,   1.0,   1.0,   1.0,   0.0};

/* Code */

/*
 *  Set a "palette" of colors in the range of color indices used by
 *  the device.  Incorrect palette items are quietly ignored(?).
 */
#ifdef PROTOTYPE
void wippalette(int which, int levels)
#else
void wippalette(which, levels)
int which;
int levels;
#endif /* PROTOTYPE */
{
    int n, pal;
    int cmin, cmax;
    float *l, *r, *g, *b;
    float rd[2], gd[2], bd[2];
    float contrast;

    if (which < 0) {
      pal = -which;
      contrast = -1.0;
    } else {
      pal = which;
      contrast = 1.0;
    }

    n = 0;
    switch (pal) {
      case 0: /* device default */
        cpgqcr(0, &rd[0], &gd[0], &bd[0]);
        cpgqcr(1, &rd[1], &gd[1], &bd[1]);
        l = grayL; r = rd; g = gd; b = bd;
        n = sizeof(grayL) / sizeof(grayL[0]);
        break;
      case 1: /* gray scale */
        l = grayL; r = grayR; g = grayG; b = grayB;
        n = sizeof(grayL) / sizeof(grayL[0]);
        break;
      case 2: /* rainbow */
        l = rainbowL; r = rainbowR; g = rainbowG; b = rainbowB;
        n = sizeof(rainbowL) / sizeof(rainbowL[0]);
        break;
      case 3: /* heat */
        l = heatL; r = heatR; g = heatG; b = heatB;
        n = sizeof(heatL) / sizeof(heatL[0]);
        break;
      case 4: /* weird IRAF */
        l = irafL; r = irafR; g = irafG; b = irafB;
        n = sizeof(irafL) / sizeof(irafL[0]);
        break;
      case 5: /* AIPS */
        l = aipsL; r = aipsR; g = aipsG; b = aipsB;
        n = sizeof(aipsL) / sizeof(aipsL[0]);
        break;
      case 6: /* TJP */
        l = tjpL; r = tjpR; g = tjpG; b = tjpB;
        n = sizeof(tjpL) / sizeof(tjpL[0]);
        break;
      case 7: /* saoA */
        l = saoAL; r = saoAR; g = saoAG; b = saoAB;
        n = sizeof(saoAL) / sizeof(saoAL[0]);
        break;
      case 8: /* saoBB */
        l = saoBBL; r = saoBBR; g = saoBBG; b = saoBBB;
        n = sizeof(saoBBL) / sizeof(saoBBL[0]);
        break;
      case 9: /* saoHE */
        l = saoHEL; r = saoHER; g = saoHEG; b = saoHEB;
        n = sizeof(saoHEL) / sizeof(saoHEL[0]);
        break;
      case 10: /* saoI8 */
        l = saoI8L; r = saoI8R; g = saoI8G; b = saoI8B;
        n = sizeof(saoI8L) / sizeof(saoI8L[0]);
        break;
      case 11: /* WLS' ds */
        l = dsL; r = dsR; g = dsG; b = dsB;
        n = sizeof(dsL) / sizeof(dsL[0]);
        break;
      case 12: /* cyclic */
        l = cyclicL; r = cyclicR; g = cyclicG; b = cyclicB;
        n = sizeof(cyclicL) / sizeof(cyclicL[0]);
        break;
      default: /* unknown type */
        wipoutput(stderr, "Palette type [%d] unknown.\n", which);
        break;
    }

    if (n > 0) {
      if (levels == 0) {  /* If levels is 0, reset to device standard. */
        cpgqcol(&cmin, &cmax);
        cmin = 16;
        if (cmax < cmin) cmax = 0;
        wipsetcir(cmin, cmax);
      } else if (levels > 0) {     /* Request for new range of levels. */
        cpgqcir(&cmin, &cmax);
        cmin = 16;
        cmax = cmin + levels - 1;
        wipsetcir(cmin, cmax);
      }    /* Otherwise, don't change the range of color index levels. */
      cpgctab(l, r, g, b, n, contrast, 0.5);
    }

    return;
}
