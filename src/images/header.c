/*
 *    <header.c>
 *
 *  History:
 *    22jul91 jm  Original code.
 *    26jul91 jm  Corrected offset values and added others in wipheader().
 *		  This meant removing image dependent code to the /images
 *		  directory and added call to wipheadlim.
 *    28oct91 jm  Modified for new image variable and also to set
 *                default scale/off to 1/0 if no header info available.
 *    17feb92 jm  Modified limits in wipheader() to account for pixel size.
 *		  I have expanded the input pixel range by 0.5 at each edge.
 *    15aug92 jm  Modified to return status int, increased error checking,
 *		  and cleaned up code a bit.
 *    10nov93 jm  Modified declarations to usage of Void.
 *    12dec94 jm  Merged wipheadlim() and wipheader() into one file
 *		  making the former static since only the later calls it.
 *
 * Routines:
 * static int wipheadlim ARGS(( Const Void *image, Const char *xtype,
 *  Const char *ytype, float *xscale, float *xoff, float *yscale, float *yoff));
 * int wipheader ARGS(( int blcx, int blcy, int trcx, int trcy,
 *  Const char *xtype, Const char *ytype ));
 */

#include "wip.h"

/* Global Variables needed just for this file */

static struct {
  char *name;
  short int okay;
} projections[] = {
  {"-tan", 1},
  {"-sin", 1},
  {"-arc", 0},
  {"-ncp", 0},
  {"-stg", 0},
  {"-ait", 0},
  {"-gls", 0},
  {"-mer", 0},
};

#define NumberOfProjections (sizeof(projections) / sizeof(projections[0]))

/* Code */

/*
 *  Returns scale and offset values for the currently defined "image".
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
static int wipheadlim(Const Void *image, Const char *xtype, Const char *ytype,
  float *xscale, float *xoff, float *yscale, float *yoff)
#else
static int wipheadlim(image, xtype, ytype, xscale, xoff, yscale, yoff)
Const Void *image;
Const char *xtype;
Const char *ytype;
float *xscale, *xoff, *yscale, *yoff;
#endif /* PROTOTYPE */
{
    char *ptr;
    char style[30];
    char ctypex[80], ctypey[80];
    register int j;
    float rafacx, rafacy;
    float off, scale;
    float xconvert, yconvert;
    double crvalx, crpixx, cdeltx;
    double crvaly, crpixy, cdelty;
    double cosdec;
    LOGICAL error;

    *xoff = *yoff = 0.0;
    *xscale = *yscale = 1.0;
    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    crvalx = wipgetvar("crvalx", &error);  if (error == TRUE) goto MISTAKE;
    crpixx = wipgetvar("crpixx", &error);  if (error == TRUE) goto MISTAKE;
    cdeltx = wipgetvar("cdeltx", &error);  if (error == TRUE) goto MISTAKE;
    crvaly = wipgetvar("crvaly", &error);  if (error == TRUE) goto MISTAKE;
    crpixy = wipgetvar("crpixy", &error);  if (error == TRUE) goto MISTAKE;
    cdelty = wipgetvar("cdelty", &error);  if (error == TRUE) goto MISTAKE;

    if (wipimctype(image, 0, ctypex) || wipimctype(image, 1, ctypey)) {
      wipoutput(stderr, "Something wrong with this image header.\n");
      return(1);
    }
    wiplower(ctypex);
    wiplower(ctypey);

    if (Strncmp(ctypey, "dec", 3) == 0) {
      cosdec = crvaly;
    } else if (Strncmp(ctypex, "dec", 3) == 0) {
      cosdec = crvalx;
    } else {
      cosdec = 0.0;
    }
    if (Strncmp(ctypex, "ra", 2) == 0) {
      rafacx = 15.0;
      rafacy = 1.0;
    } else if (Strncmp(ctypey, "ra", 2) == 0) {
      rafacx = 1.0;
      rafacy = 15.0;
    } else {
      rafacx = 1.0;
      rafacy = 1.0;
    }

    /*  Test for an unsupported projection type. */
    if (Strchr(ctypex, '-') != (char *)NULL) {
      for (j = 0; j < NumberOfProjections; j++) {
        if (Strstr(ctypex, projections[j].name) != (char *)NULL) {
          if (projections[j].okay) {
            break;
          } else {
            wipoutput(stderr, "Unsupported projection: [%s].\n", ctypex);
            return(1);
          }
        }
      }
    }
    if (Strchr(ctypey, '-') != (char *)NULL) {
      for (j = 0; j < NumberOfProjections; j++) {
        if (Strstr(ctypey, projections[j].name) != (char *)NULL) {
          if (projections[j].okay) {
            break;
          } else {
            wipoutput(stderr, "Unsupported projection: [%s].\n", ctypey);
            return(1);
          }
        }
      }
    }

    ptr = wipimtype(image);
    if (Strcmp(ptr, "miriad") == 0) {
      xconvert = (3600.0 / rafacx) / RPDEG;
      yconvert = (3600.0 / rafacy) / RPDEG;
      cosdec = COS(cosdec);
    } else if (Strcmp(ptr, "fits") == 0) {
      xconvert = 3600.0 / rafacx;
      yconvert = 3600.0 / rafacy;
      cosdec = COS(cosdec * RPDEG);
    } else {
      xconvert = 1.0;
      yconvert = 1.0;
      cosdec = COS(cosdec);
    }

    (void)Strcpy(style, xtype);
    if ((ptr = wipleading(style)) == (char *)NULL) ptr = "";
    wiplower(ptr);

    if ((crvalx == 0.0) && (crpixx == 0.0) && (cdeltx == 0.0)) {
      scale = 1.0;
      off = 0.0;
    } else if (Strncmp(ptr, "rd", 2) == 0) {
      scale = xconvert * cdeltx / cosdec;
      off = (xconvert * crvalx) - (crpixx * scale);
    } else if (Strncmp(ptr, "so", 2) == 0) {
      scale = xconvert * rafacx * cdeltx;
      off = -crpixx * scale;
    } else if (Strncmp(ptr, "mo", 2) == 0) {
      scale = xconvert * rafacx * cdeltx / 60.0;
      off = -crpixx * scale;
    } else if (Strncmp(ptr, "po", 2) == 0) {
      scale = 1.0;
      off = -crpixx;
    } else if (Strncmp(ptr, "go", 2) == 0) {
      scale = cdeltx;
      off = -crpixx * scale;
    } else if (Strncmp(ptr, "gl", 2) == 0) {
      scale = cdeltx;
      off = crvalx - (crpixx * scale);
    } else {
      scale = 1.0;
      off = 0.0;
    }

    *xoff = off;
    *xscale = scale;

    (void)Strcpy(style, ytype);
    if ((ptr = wipleading(style)) == (char *)NULL) ptr = "";
    wiplower(ptr);

    if ((crvaly == 0.0) && (crpixy == 0.0) && (cdelty == 0.0)) {
      scale = 1.0;
      off = 0.0;
    } else if (Strncmp(ptr, "rd", 2) == 0) {
      scale = yconvert * cdelty;
      off = (yconvert * crvaly) - (crpixy * scale);
    } else if (Strncmp(ptr, "so", 2) == 0) {
      scale = yconvert * rafacy * cdelty;
      off = -crpixy * scale;
    } else if (Strncmp(ptr, "mo", 2) == 0) {
      scale = yconvert * rafacy * cdelty / 60.0;
      off = -crpixy * scale;
    } else if (Strncmp(ptr, "po", 2) == 0) {
      scale = 1.0;
      off = -crpixy;
    } else if (Strncmp(ptr, "go", 2) == 0) {
      scale = cdelty;
      off = -crpixy * scale;
    } else if (Strncmp(ptr, "gl", 2) == 0) {
      scale = cdelty;
      off = crvaly - (crpixy * scale);
    } else {
      scale = 1.0;
      off = 0.0;
    }

    *yoff = off;
    *yscale = scale;

    return(0);

MISTAKE:
    wipoutput(stderr, "Trouble getting image header items.\n");
    return(1);
}

/* Returns 1 if error occured; 0 if no error. */
#ifdef PROTOTYPE
int wipheader(int blcx, int blcy, int trcx, int trcy, Const char *xtype, Const char *ytype)
#else
int wipheader(blcx, blcy, trcx, trcy, xtype, ytype)
int blcx, blcy, trcx, trcy;
Const char *xtype;
Const char *ytype;
#endif /* PROTOTYPE */
{
    Void *curimage;
    float xscale, xoff, yscale, yoff;
    float xmin, xmax, ymin, ymax;
    float tr[6];

    curimage = wipimcur("curimage");
    if (wipimagexists(curimage) == 0) {
      wipoutput(stderr, "An image needs to be defined first!\n");
      return(1);
    }

    /*
     *  Because pixels have size, we must add an offset of one-half
     *  a pixel in each direction (expanding the scale in both
     *  directions).  Contour plots should still be okay(?).
     */

    xmin = blcx - 0.5;
    xmax = trcx + 0.5;
    ymin = blcy - 0.5;
    ymax = trcy + 0.5;

    if (wipheadlim(curimage, xtype, ytype, &xscale, &xoff, &yscale, &yoff)) {
      wipoutput(stderr, "Trouble getting image header scale factors.\n");
      return(1);
    }

    xmin = (xscale * xmin) + xoff;
    xmax = (xscale * xmax) + xoff;
    ymin = (yscale * ymin) + yoff;
    ymax = (yscale * ymax) + yoff;
    cpgswin(xmin, xmax, ymin, ymax);
    wiplimits();

    tr[0] = xoff;
    tr[1] = xscale;
    tr[2] = 0.0;
    tr[3] = yoff;
    tr[4] = 0.0;
    tr[5] = yscale;
    wipsetr(tr);

    return(0);
}
