/*
 *    <image.c>
 *
 *  History:
 *    13aug90 jm  Original code.
 *    01dec90 jm  Heavily Modified
 *    25oct91 jm  Completely overhauled.  The intent now is to take
 *                the structure that is returned from the image routines
 *                and use it as an opaque pointer to all subsequent image
 *                routines.  Keep all important parameters of the image
 *                around until a new image is called for.  At that time
 *                release the real data as well.  Currently, only one
 *                image is present at a time.
 *    26feb92 jm  Added wipimplane routine.
 *    02mar92 jm  Modified wipimage/getimage routines to permit user
 *                to specify what value bad pixels should be set to.
 *    31jul92 jm  Did some cleaning and added wipnewstring().  Also,
 *                added wipfreeimage() and made wipimagefree() static.
 *                Fixed a pointer bug in wipimsetcur() by adding last.
 *    15aug92 jm  Modified wipimsetminmax() to return status int.
 *    27may93 jm  Corrected malloc statement for pixels.  It was
 *                allocating for (float **) instead of for (FLOAT *).
 *    28jul93 jm  Added wipimlogscale() function and added force
 *                option to wipimageminmax().
 *    10nov93 jm  Modified declarations to usage of Void.
 *    17aug94 jm  Modified getimage to notify user if requested plane
 *                number is not in the range [1,NZ] and what plane will
 *                be used instead.
 *    28oct96 jm  Modified wipimage to reset subimage range and to set
 *                the local crval, etc. headers.
 *    13nov96 jm  Modified wipimage so ctype was declared as large as
 *                in the WIPIMAGE structure.  There was a problem with
 *                memory overwrite for some images.
 *    18may99 pjt simple support for cd1_1 cd2_2 instead of cdelt1/2
 *    10oct00 pjt no more PROTOTYPE
 *    
 *
 * Routines:
 * static void wipimagefree ARGS(( Void *image ));
 * static WIPIMAGE *getimage ARGS(( Const char *file, int plane, float blank ));
 * Void *wipimage ARGS(( Const char *file, int plane, float blank ));
 * void wipimagenxy ARGS(( Const Void *image, int *nx, int *ny ));
 * void wipimageminmax ARGS(( Void *image, float *min, float *max, int force ));
 * int wipimagexists ARGS(( Const Void *image ));
 * float **wipimagepic ARGS(( Const Void *image ));
 * int wipimlogscale ARGS(( Void *image, float scale ));
 * int wipimsetminmax ARGS(( Void *image, float min, float max ));
 * int wipimgethead ARGS(( Const Void *image, int axis, double *crval,
 *   double *crpix, double *cdelt, char *ctype ));
 * int wipimctype ARGS(( Const Void *image, int axis, char *ctype ));
 * char *wipimtype ARGS(( Const Void *image ));
 * int wipimplane ARGS(( Const Void *image ));
 * int wipimhdprsnt ARGS(( Const Void *image, Const char *hdname ));
 * int wipimhdval ARGS(( Const Void *image, Const char *hdname, double *retval ));
 * int wipimhdstr ARGS(( Const Void *image, Const char *hdname, char *retval, size_t maxlen ));
 * Void *wipimcur ARGS(( Const char *imagename ));
 * void wipimsetcur ARGS(( Const char *imagename, Const Void *image ));
 * void wipfreeimage ARGS(( Const char *imagename ));
 */

#define WIP_IMAGE
#define WIP_DRIVERS
#include "wip.h"

/* Global Variables needed just for this file */

/* Define a structure in which to store image data. */
/* MAXNAX and FLOAT are defined in image.h */
typedef struct {
    char   *name;                /* The input file name of this image. */
    Void   *fmt;          /* Opaque handle of different image drivers. */
    Void   *handle;    /* Opaque handle used in low level image calls. */
    char    imtype[80];       /* String describing name of image type. */
    int     plane;                /* (In range) selected plane number. */
    int     nx;                  /* Number of pixels along the X axis. */
    int     ny;                  /* Number of pixels along the Y axis. */
    int     nz;                  /* Number of pixels along the Z axis. */
    float   min;                           /* Minimum of chosen plane. */
    float   max;                           /* Maximum of chosen plane. */
    double  crval[MAXNAX];                    /* Value at CRPIX pixel. */
    double  crpix[MAXNAX];         /* Reference pixel along each axis. */
    double  cdelt[MAXNAX];   /* Step size (CRVAL units) for each axis. */
    char    ctype[MAXNAX][80];           /* String defining axis type. */
    float  *ptrows;                             /* Private work array. */
    FLOAT **pixels;                                /* Real data array. */
} WIPIMAGE;

typedef struct image_stack {
  char *name;                              /* (Lower case) stack name. */
  Void *image;                     /* (WIPIMAGE *) cast into (char *). */
  struct image_stack *next;
} IMSTACK;
static IMSTACK imstackHead = {"curimage", (Void *)NULL, (IMSTACK *)NULL};

/* Code */

static void wipimagefree(Void *image)
{
    IMFORMAT *fmt;
    WIPIMAGE *ptr;

    if (image != (Void *)NULL) {
      ptr = (WIPIMAGE *)image;
      if (ptr->pixels != (FLOAT **)NULL) Free(ptr->pixels);
      if (ptr->ptrows != (float *)NULL) freevector(ptr->ptrows);
      if (ptr->name != (char *)NULL) Free(ptr->name);
      if (ptr->fmt != (Void *)NULL) {
        fmt = (IMFORMAT *)ptr->fmt;
        fmt->imclose(ptr->handle);
      }
      Free(ptr);
    }
}

/*
 *  Fill the real array pixels[ny][nx] and the structure image.
 *  Returns a pointer to the structure if successful; NULL on error.
 */
static WIPIMAGE *getimage(Const char *filename, int plane, float blank)
{

    Void *handle;         /* Opaque handle returned from open routine. */
    char ctype1[80], ctype2[80];
    register int j, number;
    int indx;
    int nsize[MAXNAX];
    float fmin, fmax;
    float *rptr, *fptr;
    FLOAT **pixels;
    double crval1, crval2, crpix1, crpix2, cdelt1, cdelt2, tmpd;
    WIPIMAGE *image;
    IMFORMAT *fmt;

    number = sizeof(Format_Table) / sizeof(Format_Table[0]);

    for (j = 0; j < number; j++) {
      fmt = &Format_Table[j];
      if ((fmt != (IMFORMAT *)NULL) && (fmt->imopen != NULL)) {
        if ((handle = fmt->imopen(filename, MAXNAX, nsize)) != (Void *)NULL)
          break;
      }
    }

    if ((j >= number) || (fmt->imopen == NULL))
      return((WIPIMAGE *)NULL);

    if ((image = (WIPIMAGE *)Malloc(sizeof(WIPIMAGE))) == (WIPIMAGE *)NULL) {
      wipoutput(stderr, "GETIMAGE: No memory to store image information.\n");
      if (fmt->imclose != NULL) fmt->imclose(handle);
      return((WIPIMAGE *)NULL);
    }

    if ((fmt->imrdhdd == NULL) || (fmt->imrdhdr == NULL) ||
        (fmt->imrdhdi == NULL) || (fmt->imrdhda == NULL)) {
      wipoutput(stderr, "GETIMAGE: No header access functions!\n");
      wipimagefree((Void *)image);
      return((WIPIMAGE *)NULL);
    }

    fmt->imrdhdr(handle, "datamin",   &fmin, 0.0);
    fmt->imrdhdr(handle, "datamax",   &fmax, 0.0);
    fmt->imrdhdd(handle,  "crval1", &crval1, 0.0);
    fmt->imrdhdd(handle,  "crval2", &crval2, 0.0);
    fmt->imrdhdd(handle,  "crpix1", &crpix1, 0.0);
    fmt->imrdhdd(handle,  "crpix2", &crpix2, 0.0);
    fmt->imrdhdd(handle,  "cdelt1", &cdelt1, 0.0);
    fmt->imrdhdd(handle,  "cdelt2", &cdelt2, 0.0);
    if (cdelt1 == 0.0) {
      fmt->imrdhdd(handle,  "cd1_1", &cdelt1, 0.0);
      fmt->imrdhdd(handle,  "cd1_2", &tmpd, 0.0);
      if (tmpd != 0.0) 
	wipoutput(stderr, "GETIMAGE: cd1_2 = %g non-zero ...\n", tmpd);
    }
    if (cdelt2 == 0.0) {
      fmt->imrdhdd(handle,  "cd2_2", &cdelt2, 0.0);
      fmt->imrdhdd(handle,  "cd2_1", &tmpd, 0.0);
      if (tmpd != 0.0) 
	wipoutput(stderr, "GETIMAGE: cd2_1 = %g non-zero ...\n", tmpd);
    }
    fmt->imrdhda(handle,  "ctype1",  ctype1, "(none)", 80);
    fmt->imrdhda(handle,  "ctype2",  ctype2, "(none)", 80);

    image->fmt = (Void *)fmt;
    image->handle = handle;

    if ((image->name = (char *)Malloc(Strlen(filename) + 1)) != (char *)NULL)
      (void)Strcpy(image->name, filename);

    (void)Strcpy(image->imtype, fmt->imtype);
    image->nx = nsize[0];
    image->ny = nsize[1];
    image->nz = nsize[2];
    image->min = fmin;
    image->max = fmax;
    image->crval[0] = crval1;
    image->crval[1] = crval2;
    image->crpix[0] = crpix1;
    image->crpix[1] = crpix2;
    image->cdelt[0] = cdelt1;
    image->cdelt[1] = cdelt2;
    (void)Strncpy(image->ctype[0], ctype1, 80);
    image->ctype[0][79] = Null; /* Force a Null at the end of the string. */
    (void)Strncpy(image->ctype[1], ctype2, 80);
    image->ctype[1][79] = Null; /* Force a Null at the end of the string. */

    image->ptrows = fptr = vector(image->nx * image->ny);
    image->pixels = pixels = (FLOAT **)Malloc(image->ny * sizeof(FLOAT *));
    if ((fptr == (float *)NULL) || (pixels == (FLOAT **)NULL)) {
      wipoutput(stderr,
        "GETIMAGE: Not enough internal storage room for the image.\n");
      wipimagefree((Void *)image);
      return((WIPIMAGE *)NULL);
    }

    indx = MAX(MIN(plane, image->nz), 1);
    if (indx != plane) {
      wipoutput(stderr, "GETIMAGE: Plane %d is out of range...\n", plane);
      wipoutput(stderr, "GETIMAGE: Will look for plane #%d.\n", indx);
    }
    if (fmt->imsetpl(handle, 1, &indx)) {
      wipoutput(stderr, "GETIMAGE: Trouble setting plane %d...\n", indx);
      wipoutput(stderr, "GETIMAGE: Trying plane #1.\n");
      indx = 1;
      if (fmt->imsetpl(handle, 1, &indx)) {
        wipoutput(stderr, "GETIMAGE: Trouble setting plane %d...\n", indx);
        wipoutput(stderr, "GETIMAGE: Something really wrong here!\n");
        wipimagefree((Void *)image);
        return((WIPIMAGE *)NULL);
      }
    }
    image->plane = indx;

    for (indx = 0; indx < image->ny; indx++) {
      rptr = fptr + (indx * image->nx);
      if (fmt->imread(handle, indx, rptr, blank)) {
        wipoutput(stderr, "GETIMAGE: Trouble reading image row %d.n", indx+1);
        wipimagefree((Void *)image);
        return((WIPIMAGE *)NULL);
      }
      pixels[indx] = (FLOAT *)rptr;
    }

    return(image);
}

Void *wipimage(Const char *filename, int plane, float blank)
{
    char dummy[80];
    int nx, ny;
    double arg[3];
    WIPIMAGE *ptr;

    ptr = getimage(filename, plane, blank);
    if (wipimagexists((Void *)ptr) == 0) {
      wipoutput(stderr, "Image %s could not be loaded.\n", filename);
      wipimagefree((Void *)ptr);
      return((Void *)NULL);
    }

    wipimagenxy((Void *)ptr, &nx, &ny);
    wipoutput(stdout, "Image size: %d by %d.\n", nx, ny);

    wipsetsub(1, nx, 1, ny);
    if (wipimgethead((Void *)ptr, 0, &arg[0], &arg[1], &arg[2], dummy) == 0) {
      (void)wipsetvar("crvalx", arg[0]);
      (void)wipsetvar("crpixx", arg[1]);
      (void)wipsetvar("cdeltx", arg[2]);
    }
    if (wipimgethead((Void *)ptr, 1, &arg[0], &arg[1], &arg[2], dummy) == 0) {
      (void)wipsetvar("crvaly", arg[0]);
      (void)wipsetvar("crpixy", arg[1]);
      (void)wipsetvar("cdelty", arg[2]);
    }

    return((Void *)ptr);
}


void wipimagenxy(Const Void *image, int *nx, int *ny)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return;
    }

    ptr = (WIPIMAGE *)image;
    *nx = ptr->nx;
    *ny = ptr->ny;
    return;
}

void wipimageminmax(Void *image, float *min, float *max, int force)
{
    int nx, ny;
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return;
    }

    ptr = (WIPIMAGE *)image;
    if ((force > 0) || (ptr->min == ptr->max)) {
      wipoutput(stdout, "Getting image max and min...");
      Fflush(stdout);
      wipimagenxy(image, &nx, &ny);
      wipextrema(ptr->pixels, nx, ny, min, max);
      (void)wipimsetminmax(image, *min, *max);
      wipoutput(stdout, "done.\n");
    } else {
      *min = ptr->min;
      *max = ptr->max;
    }
    return;
}

/*  Returns 1 if the named image exists and has data; 0 otherwise. */
int wipimagexists(Const Void *image)
{
    int doesItExist;
    WIPIMAGE *ptr;

    ptr = (WIPIMAGE *)image;
    doesItExist = (ptr != (WIPIMAGE *)NULL) &&
                  (ptr->pixels != (FLOAT **)NULL) &&
                  (ptr->ptrows != (float *)NULL);

    return(doesItExist);
}

/*  Returns a pointer to the image data if it exists; NULL otherwise. */
float **wipimagepic(Const Void *image)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return((float **)NULL);
    }

    ptr = (WIPIMAGE *)image;
    return(ptr->pixels);
}

/*  Returns 1 on error; 0 if successful. */
int wipimlogscale(Void *image, float scale)
{
    register int x, y;
    int nx, ny;
    float **pixels;
    float val;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    wipimagenxy(image, &nx, &ny);
    pixels = wipimagepic(image);

    for (y = 0; y < ny; y++) {
      for (x = 0; x < nx; x++) {
        val = pixels[y][x];
        pixels[y][x] = (val > 0.0) ? (scale * LOG10(val)) : -50.0;
      }
    }
    return(0);
}

/*  Returns 1 on error; 0 if successful. */
int wipimsetminmax(Void *image, float min, float max)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    ptr = (WIPIMAGE *)image;
    ptr->min = min;
    ptr->max = max;
    return(0);
}

/*
 *  Returns 1 if "image" is not a defined image item or the axis input
 *  is out of range; 0 if no error (successful).
 *
 *  Note: ctype must be declared as large as the structure element!
 *  No test of this is done.
 */
int wipimgethead(Const Void *image, int axis, double *crval, double *crpix, double *cdelt, char *ctype)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    if ((axis < 0) || (axis >= MAXNAX)) {
      wipoutput(stderr, "Axis number specified is out of range!\n");
      return(1);
    }

    ptr = (WIPIMAGE *)image;
    *crval = ptr->crval[axis];
    *crpix = ptr->crpix[axis];
    *cdelt = ptr->cdelt[axis];
    (void)Strcpy(ctype, ptr->ctype[axis]);
    return(0);
}

/*
 *  If "image" is a defined image and the axis is valid, then the
 *  array pointed to by "ctype" will be filled with the name of image
 *  type.  Returns 0 if successful; 1 otherwise (if error).  This
 *  routine assumes that the size of the string pointed to by "ctype"
 *  is large enough to hold the information.
 */
int wipimctype(Const Void *image, int axis, char *ctype)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    if ((axis < 0) || (axis >= MAXNAX)) {
      wipoutput(stderr, "Axis number specified is out of range!\n");
      return(1);
    }

    ptr = (WIPIMAGE *)image;
    (void)Strcpy(ctype, ptr->ctype[axis]);
    return(0);
}

/* Returns NULL if no image; otherwise pointer to name of image type loaded. */
char *wipimtype(Const Void *image)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return((char *)NULL);
    }

    ptr = (WIPIMAGE *)image;
    return(ptr->imtype);
}

/* Returns 0 if no image is present; current plane number otherwise. */
int wipimplane(Const Void *image)
{
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(0);
    }

    ptr = (WIPIMAGE *)image;
    return(ptr->plane);
}

/* Returns 1 if header name is present; 0 otherwise. */
int wipimhdprsnt(Const Void *image, Const char *hdname)
{
    IMFORMAT *fmt;
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(0);
    }

    ptr = (WIPIMAGE *)image;
    if ((fmt = (IMFORMAT *)ptr->fmt) == (IMFORMAT *)NULL) {
      wipoutput(stderr, "No image format specified yet!\n");
      return(0);
    }

    return(fmt->imhdprsnt(ptr->handle, hdname));
}

/* Returns 0 and header value if image is present; 1 otherwise (error). */
int wipimhdval(Const Void *image, Const char *hdname, double *retval)
{
    IMFORMAT *fmt;
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    ptr = (WIPIMAGE *)image;
    if ((fmt = (IMFORMAT *)ptr->fmt) == (IMFORMAT *)NULL) {
      wipoutput(stderr, "No image format specified yet!\n");
      return(1);
    }

    fmt->imrdhdd(ptr->handle, hdname, retval, 0.0);

    return(0);
}

/*
 *  Returns the length of the requested string header if "image" is present
 *  and the header item exists; 0 otherwise (error). */
int wipimhdstr(Const Void *image, Const char *hdname, char *retval, size_t maxlen)
{
    int retlen;
    IMFORMAT *fmt;
    WIPIMAGE *ptr;

    if (wipimagexists(image) == 0) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(0);
    }

    ptr = (WIPIMAGE *)image;
    if ((fmt = (IMFORMAT *)ptr->fmt) == (IMFORMAT *)NULL) {
      wipoutput(stderr, "No image format specified yet!\n");
      return(0);
    }

    fmt->imrdhda(ptr->handle, hdname, retval, "(undefined)", maxlen);

    retlen = wiplenc(retval);           /* Remove trailing whitespace. */
    return(retlen);
}

/* Returns NULL if imagename is not present; opaque pointer otherwise. */
Void *wipimcur(Const char *imagename)
{
    char string[STRINGSIZE];
    IMSTACK *ptr;

    (void)Strncpy(string, imagename, STRINGSIZE);
    string[STRINGSIZE-1] = Null;
    wiplower(string);

    for (ptr = &imstackHead; ptr != (IMSTACK *)NULL; ptr = ptr->next)
      if (Strcmp(ptr->name, string) == 0)
        break;

    return( ((ptr == (IMSTACK *)NULL) ? (Void *)NULL : ptr->image) );
}

void wipimsetcur(Const char *imagename, Const Void *image)
{
    char string[STRINGSIZE];
    IMSTACK *ptr, *new, *last;

    (void)Strcpy(string, imagename);
    wiplower(string);

    last = (IMSTACK *)NULL;
    for (ptr = &imstackHead; ptr != (IMSTACK *)NULL; ptr = ptr->next) {
      if (Strcmp(ptr->name, string) == 0) break;
      last = ptr;
    }

    if (ptr != (IMSTACK *)NULL) {
      ptr->image = (Void *)image;
    } else {         /* Add a new entry to the end of the linked list. */
      if ((new = (IMSTACK *)Malloc(sizeof(IMSTACK))) == (IMSTACK *)NULL) {
        wipoutput(stderr, "IMSETCUR: No memory to store current image.\n");
        return;
      }
      new->name = wipnewstring(string);
      new->image = (Void *)image;
      new->next = (IMSTACK *)NULL;
      last->next = new;
    }

    return;
}

void wipfreeimage(Const char *imagename)
{
    char string[STRINGSIZE];
    IMSTACK *ptr, *last;

    (void)Strcpy(string, imagename);
    wiplower(string);

    last = (IMSTACK *)NULL;
    for (ptr = &imstackHead; ptr != (IMSTACK *)NULL; ptr = ptr->next) {
      if (Strcmp(ptr->name, string) == 0) break;
      last = ptr;
    }

    if (ptr != (IMSTACK *)NULL) {
      wipimagefree(ptr->image);
      if (ptr == &imstackHead) {  /* Never remove the "curimage" item. */
        ptr->image = (Void *)NULL;              /* Reset this pointer. */
        return;
      }
      if (ptr->name != (char *)NULL) Free(ptr->name);
      if (last != (IMSTACK *)NULL) last->next = ptr->next;
      Free(ptr);
    }
    return;
}

#ifdef TEST

/* Test the callable routines in this file. */
/* Unfortunately, this needs to be linked with the WIP library. */

#define VERSION_ID "1.0"

main(int argc, char *argv[])
{
    Void *image;
    char *infile;
    char *ptr;
    char ctype[80];
    int indx;
    int axis;
    float fmin, fmax;
    double crval, crpix, cdelt;
    float **array;

    (void)printf("%s Version %s\n\n", argv[0], VERSION_ID);
    if (argc != 2) {
      (void)printf("Usage: %s [in=]image-dataset\n", argv[0]);
      (void)printf("Test of FITS image routines.\n");
      exit(0);
    }

    infile = argv[1];
    if (Strlen(infile) > 3) {             /* see if in= was used. */
      if (Strncmp(infile, "in=", 3) == 0) /* if so, */
        infile += 3;                      /* increase pointer. */
    }

    indx = 1;
    (void)printf("TEST: Opening plane #%d of file %s.\n", indx, infile);
    image = wipimage(infile, indx, -99.0);
    if (image == (Void *)NULL) {
      (void)printf("TEST: Could not open image file %s.\n", infile);
      exit(1);
    }

    ptr = wipimtype(image);
    (void)printf("TEST: Image type is %s.\n", ptr);

    fmin = 0; fmax = 10;
    (void)printf("TEST: Setting min/max to %f/%f.\n", fmin, fmax);
    if (wipimsetminmax(image, fmin, fmax))
      (void)printf("TEST: Trouble setting image min/max.\n");

    for (axis = 0; axis < 2; axis++) {
      indx = wipimgethead(image, axis, &crval, &crpix, &cdelt, ctype);
      if (indx != 0)
        (void)printf("TEST: Trouble reading image header for axis %d.\n", axis);
      else {
        (void)printf("TEST: Image header for axis %d:\n", axis);
        (void)printf("TEST: crval=%lf; crpix=%lf; cdelt=%lf\n",
          crval, crpix, cdelt);
      }
    }

    /* Get a real header string. */
    (void)printf("TEST: Getting header ctype2.\n");
    indx = wipimhdstr(image, "ctype2", ctype, 80);
    if (indx == 0)
      (void)printf("TEST: Trouble reading image header for ctype2.\n");
    else
      (void)printf("TEST: Header ctype2 = %s.\n", ctype);

    /* Get a bogus header string. */
    (void)printf("TEST: Getting header junk.\n");
    indx = wipimhdstr(image, "junk", ctype, 80);
    if (indx == 0)
      (void)printf("TEST: Trouble reading image header for junk.\n");
    else
      (void)printf("TEST: Header junk = %s.\n", ctype);

    /* Get a real header value. */
    (void)printf("TEST: Getting header naxis2.\n");
    indx = wipimhdval(image, "naxis2", &crval);
    if (indx != 0)
      (void)printf("TEST: Trouble reading image header for naxis2.\n");
    else
      (void)printf("TEST: Header naxis2 = %f [%d].\n", crval, (int)crval);

    (void)printf("TEST: Getting image pixels.\n");
    array = wipimagepic(image);

    (void)printf("TEST: Freeing image %s.\n", infile);
    wipimagefree(image);
    (void)printf("TEST: Finished.\n", infile);
    exit(0);
}
#endif /* TEST */
