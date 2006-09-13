/*
 *      This module consists of a number of routines to perform
 *      I/O on a FITS image file.
 *
 *  This should be reasonably easy adapted to any system which supports
 *  a standard C compiler and standard I/O subroutines.
 *  system subroutines. However, note the following:
 *  * Currently only machines which support FITS integers and IEEE
 *    floating point data, as their host formats are supported.
 *  * A #define of BSD should be inserted on Berkeley UNIX machines.
 *  * A typdef FLOAT is used for the standard floating point data type.
 *    This can be either "float" or "double".
 *
 *  History:
 *    15jul90 rjs  Original version.
 *    28jul90 pjt  hurah - replace wrong in NEMO.
 *    30jul90 rjs  Fixed bug in fitsetpl. Added comments. Improved
 *                 some error messages.
 *     8aug90 rjs  Fixed bug in fitread, when reading 4-byte integers.
 *    17apr91 rjs  Adapted for AIPS2 test.
 *    22oct91  jm  Adapted for WIP images.
 *    02mar92  jm  Added BLANK keyword usage.  I am worried about how
 *                 this was implemented: should BLANK value testing
 *                 take place "before" unpacking?
 *    02mar92  jm  Modified fitread to allow users to set BLANK value.
 *    14mar92  jm  Fixed a syntax bug in fitread.
 *    05jul92  jm  Increased f->ncards by one after reading the END
 *                 keyword.  This is because fitsrch() is zero based
 *                 and f->ncards should be one based.
 *    10nov93  jm  ANSI-fied the code and changed level 2 I/O calls to
 *                 level 3 ANSI supported I/O calls.
 *    01aug95  jm  Added byte size fits files.
 *    13sep06 pjt  add TYPE_DOUBLE for 64bit support (BITPIX=-64)
 *
 * Routines:
 * Void *fitopen(Const char *name, int naxis, int axes[]);
 * void fitclose(Void *file);
 * int fitread(Void *file, int indx, FLOAT *array, FLOAT badpixel);
 * int fitsetpl(Void *file, int naxis, int axes[]);
 * void fitrdhdd(Void *file, Const char *keyword, double *value, double defval);
 * void fitrdhdr(Void *file, Const char *keyword, FLOAT *value, FLOAT defval);
 * void fitrdhdi(Void *file, Const char *keyword, int *value, int defval);
 * void fitrdhda(Void *file, Const char *keyword, char *value, Const char *defval, size_t maxlen);
 * int fithdprsnt(Void *file, Const char *keyword);
 * static int fitsrch(FITS *f, Const char *keyword, char card[80]);
 */

#define WIP_IMAGE
#include "image.h"

/* Define parameters needed just for this driver. */

#define TYPE_BYTES  1
#define TYPE_FLOAT  2
#define TYPE_16INT  3
#define TYPE_32INT  4
#define TYPE_DOUBLE 5

/* Define private structure used to handle image. */

typedef struct {
    FILE *fd;             /* Handle into file. */
    int type;             /* Image type. */
    int ncards;           /* Number of header items. */
    int naxis;            /* Number of axes. */
    int axes[MAXNAX];     /* Number of pixels along each axes. */
    long offset;          /* Offset (in pixels) to line in a plane. */
    int isblanked;        /* 1 if keyword BLANK is present; 0 otherwise. */
    int blank;            /* Pix-value assigned to undefined-value pixels. */
    FLOAT bscale, bzero;  /* Scale factors. */
} FITS;

/* Declare private routine. */

static int fitsrch ARGS(( FITS *f, Const char *keyword, char card[80] ));

/* Declare static variables. */

static char *Buf1 = (char *)NULL;
static char *Buf2 = (char *)NULL;
static size_t Maxdim = 0;

/***********************************************************************/
Void *fitopen(Const char *name, int naxis, int nsize[])
/*
  This opens a FITS file and readies it for i/o.

  Inputs:
    name	A string giving the name of the file to be opened.
    naxis	The maximum number of dimensions of the image. When
		opening a file, fitopen makes sure that the number of
		dimensions does not exceed this.
  Output:
    nsize	It is of size "naxis".  It returns the number of
		pixels along each axis.
    fitopen	This is a pointer (caste to a char pointer) to a structure
		which describes the file. This pointer is used in all
		subsequent calls to the fitsio routines.  This is
		returned as NULL if this file does not agree with the
		FITS format.
-----------------------------------------------------------------------*/
{
    char *ptr;
    char keyword[9], line[81];
    int n, t, i, bitpix;
    long int size, offset;
    FITS *f;

    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "Trying FITS format...\n");

    if ((f = (FITS *)malloc(sizeof(FITS))) == (FITS *)NULL) {
      (void)fprintf(stderr,
        "### Warning: Failed to allocate memory in fitopen.\n");
      return((Void *)NULL);
    }

    ptr = (char *)name;
    if ((f->fd = fopen(ptr, "rb")) == (FILE *)NULL) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr, "### Warning: Failed to open [%s].\n", name);
      (void)free((Void *)f);
      return((Void *)NULL);
    }

    /*
     *  Check that the file has the SIMPLE and END keywords.
     *  Then calculate the byte offset to the start of the image data.
     */

    if ((fitsrch(f, "SIMPLE  ", line) != 0) ||
        ((f->ncards = fitsrch(f, "END     ", line)) < 0)) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr,
          "### Warning: File [%s] does not seem to be FITS.\n", name);
      fitclose((Void *)f);
      return((Void *)NULL);
    }
    f->ncards++;       /* Increase f->ncards by 1 for the END keyword. */
    offset = 2880 * (((80 * f->ncards) + 2879) / 2880);

    /* Determine things about the file. */

    fitrdhdi((Void *)f, "BITPIX", &bitpix, 0);
    switch (bitpix) {
      case   8: f->type = TYPE_BYTES;  break;
      case  16: f->type = TYPE_16INT;  break;
      case  32: f->type = TYPE_32INT;  break;
      case -32: f->type = TYPE_FLOAT;  break;
      case -64: f->type = TYPE_DOUBLE; break;
      default:
        (void)fprintf(stderr,
          "### Warning: Unsupported FITS BITPIX value [%d] in file [%s].\n",
          bitpix, name);
        fitclose((Void *)f);
        return((Void *)NULL);
    }

    if (f->type == TYPE_FLOAT && sizeof(FLOAT) != 4) {
      (void)fprintf(stderr,
        "### Warning: fitopen compiled with FLOAT not 4 bytes. Cannot continue\n");
      return((Void *)NULL);
    } else {
      (void)fprintf(stderr,"### pjt debug\n");
    }



    fitrdhdi((Void *)f, "NAXIS", &n, 0);
    if ((n < 1) || (n > MAXNAX)) {
      (void)fprintf(stderr,
        "### Warning: Bad FITS NAXIS value [%d] in file [%s].\n", n, name);
      fitclose((Void *)f);
      return((Void *)NULL);
    }

    size = 1;
    for (i = 0; i < n; i++) {
      (void)sprintf(keyword, "NAXIS%d", i+1);
      fitrdhdi((Void *)f, keyword, &t, 0);
      if ((t < 1) || ((i >= naxis) && (t != 1))) {
        (void)fprintf(stderr,
          "### Warning: Cannot handle FITS dimension %d of %s being %d.\n",
          i+1, name, t);
        fitclose((Void *)f);
        return((Void *)NULL);
      }
      if (i < naxis) nsize[i] = t;
      size *= t;
    }
    for (i = n; i < naxis; i++) nsize[i] = 1;
    fitrdhdr((Void *)f, "BSCALE", &(f->bscale), 1.0);
    fitrdhdr((Void *)f, "BZERO", &(f->bzero), 0.0);

    /* Check that the file is of the right size. */

    switch (f->type) {
      case TYPE_BYTES:  break;
      case TYPE_16INT:  size *= 2; break;
      case TYPE_DOUBLE: size *= 8; break;
      default:          size *= 4; break;
    }
    size += offset;
    if ((fseek(f->fd, 0L, SEEK_END) != 0) || (ftell(f->fd) < size)) {
      (void)fprintf(stderr, "### Warning: FITS File [%s] appears too small.\n",
        name);
      fitclose((Void *)f);
      return((Void *)NULL);
    }

    /* Save dimension info. */

    f->offset = 0;
    f->naxis = naxis;
    for (i = 0; i < naxis; i++) f->axes[i] = nsize[i];
    for (i = naxis; i < MAXNAX; i++) f->axes[i] = 1;

    /* Find out if there is blanking going on in the image. */

    f->isblanked = fithdprsnt((Void *)f, "BLANK");
    if (f->isblanked) {
      fitrdhdi((Void *)f, "BLANK", &(f->blank), 0);
      if (f->type == TYPE_FLOAT || f->type == TYPE_DOUBLE) {
        (void)fprintf(stderr,
          "### This FITS file has an unconventional style of blanking.\n");
        (void)fprintf(stderr, "### I am not sure this will work properly!\n");
      } else {
        (void)fprintf(stderr, "### This file has blanking!\n");
      }
    }

    /* Make sure we have enough memory to deal with this file. */

    if (Maxdim < nsize[0]) {
      Maxdim = nsize[0];
      Buf1 = (Buf1 == (char *)NULL) ? (char *)malloc(2*sizeof(int) * nsize[0]) :
                              (char *)realloc(Buf1, (2*sizeof(int) * nsize[0]));
      Buf2 = (Buf2 == (char *)NULL) ? (char *)malloc(8 * nsize[0]) :
                              (char *)realloc(Buf2, (8 * nsize[0]));
      if ((Buf1 == (char *)NULL) || (Buf2 == (char *)NULL)) {
        (void)fprintf(stderr,
          "### Fatal Error: Ran out of memory opening FITS file.\n");
        fitclose((Void *)f);
        return((Void *)NULL);
      }
    }

    printf("PJT Size: %d : %d x %d\n",f->naxis,f->axes[0],f->axes[1]);

    /* Return with the opaque pointer. */

    return((Void *)f);
}

/***********************************************************************/
void fitclose(Void *file)
/*
  This closes a FITS file, and deletes any memory associated with it.

  Input:
    file	This is the pointer returned by fitopen.
-----------------------------------------------------------------------*/
{
    FITS *f;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return;

    if (f->fd != (FILE *)NULL) (void)fclose(f->fd);
    (void)free((Void *)f);
    file = (Void *)NULL;
}

/***********************************************************************/
int fitread(Void *file, int indx, FLOAT *data, FLOAT badpixel)
/*
  This reads a row of a FITS image.

  Input:
    file	The pointer to the data structure returned by the fitopen
		routine.
    indx	The row number to be read. This varies from 0 to naxis2-1.
    badpixel	All bad pixels are set to this value.
  Output:
    data	A FLOAT array of naxis1 elements, being the pixel values
		read.
    fitread	Set to 0 if read okay; !=0 otherwise.
-----------------------------------------------------------------------*/
{
    unsigned char *bdat;
    int *idat;
    int i, bytes, iblank;
    long int offset;
    size_t flen, length;
    FITS *f;
    FLOAT bscale,bzero;
    INT16 *jdat;
    double *ddat;
    union {int i; float f;} xunion;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return(1);

    switch (f->type) {
      case TYPE_BYTES:  bytes = 1; break;
      case TYPE_16INT:  bytes = 2; break;
      case TYPE_DOUBLE: bytes = 8; break;
      default:          bytes = 4; break;
    }
    length = bytes * f->axes[0];
    flen = length / sizeof(FLOAT);
    offset = (indx * length) + (bytes * f->offset);
    offset += (2880 * (((80 * f->ncards) + 2879) / 2880));
    if (fseek(f->fd, offset, SEEK_SET) != 0)
      return(1);

    if (indx > f->axes[1]) {
      (void)fprintf(stderr,
        "### Error: Attempt to read beyond image boundaries in fitread.\n");
      return(1);
    }

#if NO_CVT
    if (f->type == TYPE_FLOAT) {
      if (fread((Void *)data, sizeof(FLOAT), flen, f->fd) != flen) {
        (void)fprintf(stderr, "### Error: I/O read error FLOAT in fitread.\n");
        return(1);
      }
    } else if (f->type == TYPE_DOUBLE) {
      if (fread((Void *)data, sizeof(double), flen, f->fd) != flen) {
        (void)fprintf(stderr, "### Error: I/O read error double in fitread.\n");
        return(1);
      }

    } else {
      if (fread((Void *)Buf2, sizeof(char), length, f->fd) != length) {
        (void)fprintf(stderr, "### Error: I/O read error in fitread.\n");
        return(1);
      }
    }
#else
    if (fread((Void *)Buf2, sizeof(char), length, f->fd) != length) {
      (void)fprintf(stderr, "### Error: I/O read error in fitread.\n");
      return(1);
    }
#endif

    /* We have the data now. Convert and scale it. */

    bscale = f->bscale;
    bzero = f->bzero;

    if (f->type == TYPE_BYTES) {
      bdat = (unsigned char *)Buf2;
      iblank = f->blank;
      if (f->isblanked) {
        for (i = 0; i < f->axes[0]; i++) {
          if ((int)bdat[i] != iblank) {
            data[i] = (bscale * (FLOAT)bdat[i]) + bzero;
          } else {
            if (wipDebugMode() > 0)
              (void)fprintf(stderr,
                "### Bad FITS BYTE pixel at (%d, %d).\n", i, indx);
            data[i] = badpixel;
          }
        }
      } else {
        for (i = 0; i < f->axes[0]; i++)
          data[i] = (bscale * (FLOAT)bdat[i]) + bzero;
      }
    } else if (f->type == TYPE_16INT) {

#if NO_CVT
      jdat = (INT16 *)Buf2;
      iblank = f->blank;
#else
      jdat = (INT16 *)Buf1;
      unpack16_c(Buf2, jdat, f->axes[0]);
      unpack16_c((char *)(&(f->blank)), &iblank, 1);
#endif

      if (f->isblanked) {
        for (i = 0; i < f->axes[0]; i++) {
          if (jdat[i] != iblank) {
            data[i] = (bscale * jdat[i]) + bzero;
          } else {
            if (wipDebugMode() > 0)
              (void)fprintf(stderr,
                "### Bad FITS INT16 pixel at (%d, %d).\n", i, indx);
            data[i] = badpixel;
          }
        }
      } else {
        for (i = 0; i < f->axes[0]; i++)
          data[i] = (bscale * jdat[i]) + bzero;
      }
    } else if (f->type == TYPE_32INT) {

#if NO_CVT
      idat = (int *)Buf2;
      iblank = f->blank;
#else
      idat = (int *)Buf1;
      unpack32_c(Buf2, idat, f->axes[0]);
      unpack32_c((char *)(&(f->blank)), &iblank, 1);
#endif

      if (f->isblanked) {
        for (i = 0; i < f->axes[0]; i++) {
          if (idat[i] != iblank) {
            data[i] = (bscale * idat[i]) + bzero;
          } else {
            if (wipDebugMode() > 0)
              (void)fprintf(stderr,
                "### Bad FITS INT32 pixel at (%d, %d)\n", i, indx);
            data[i] = badpixel;
          }
        }
      } else {
        for (i = 0; i < f->axes[0]; i++)
          data[i] = (bscale * idat[i]) + bzero;
      }
    } else if (f->type == TYPE_DOUBLE) {
#if NO_CVT
      ddat = (double *)Buf2;
      iblank = f->blank;
#else
      ddat = (double *)Buf1;
      unpackd_c(Buf2, ddat, f->axes[0]);
      unpack32_c((char *)(&(f->blank)), &iblank, 1);
#endif
      if (f->isblanked) {
        for (i = 0; i < f->axes[0]; i++) {
          xunion.f = data[i];
          if (xunion.i != iblank) {
            data[i] = (bscale * data[i]) + bzero;
          } else {
            if (wipDebugMode() > 0)
              (void)fprintf(stderr,
                "### Bad FITS double pixel at (%d, %d)\n", i, indx);
            data[i] = badpixel;
          }
        }
      } else if ((bscale != 1) || (bzero != 0)) {
        for (i = 0; i < f->axes[0]; i++)
          data[i] = (bscale * data[i]) + bzero;
      }

    } else {

#if NO_CVT
      iblank = f->blank;
#else
      unpackr_c(Buf2, data, f->axes[0]);
      unpack32_c((char *)(&(f->blank)), &iblank, 1);
#endif

      if (f->isblanked) {
        for (i = 0; i < f->axes[0]; i++) {
          xunion.f = data[i];
          if (xunion.i != iblank) {
            data[i] = (bscale * data[i]) + bzero;
          } else {
            if (wipDebugMode() > 0)
              (void)fprintf(stderr,
                "### Bad FITS FLOAT pixel at (%d, %d)\n", i, indx);
            data[i] = badpixel;
          }
        }
      } else if ((bscale != 1) || (bzero != 0)) {
        for (i = 0; i < f->axes[0]; i++)
          data[i] = (bscale * data[i]) + bzero;
      }
    }

    return 0;
}

/***********************************************************************/
int fitsetpl(Void *file, int naxis, int axes[])
/*
  This sets the plane to be accessed in a FITS file which has more than
  two dimensions.

  Input:
    file	The pointer returned by fitopen.
    naxis	This gives the size of the axes[] array.
    axes	This gives the indices of the higher dimensions of the
		FITS image. They are one-relative. axes[0] gives the
		index along the 3rd dimension, axes[1] is the indice
		along the 4th dimension, etc.
  Output:
    fitsetpl	Set to 0 if no error; 1 otherwise.
-----------------------------------------------------------------------*/
{
    register int i;
    long int offset;
    FITS *f;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return(1);

    if ((naxis < 1) || ((naxis + 1) >= MAXNAX))
      return(1);

    offset = 0;
    for (i = naxis - 1; i >= 0; i--) {
      if ((axes[i] < 1) || (axes[i] > f->axes[i+2])) {
        (void)fprintf(stderr,
          "### Fatal Error: Illegal coordinate index in fitsetpl.\n");
        return(1);
      }
      offset = (offset + axes[i] - 1) * f->axes[i+1];
    }
    f->offset = offset * f->axes[0];
    return(0);
}

/***********************************************************************/
void fitrdhdd(Void *file, Const char *keyword, double *value, double def)
/*
  This reads the value of a double-valued FITS keyword from the file header.

  Input:
    file	The pointer returned by fitopen.
    keyword	The keyword to search for.
    def		If the keyword is not found, this "default value" is
		returned.
  Output:
    value	The value read from the FITS header.
-----------------------------------------------------------------------*/
{
    char *s, *s1;
    char card[81];
    FITS *f;

    *value = def;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return;

    if (fitsrch(f, keyword, card) < 0)
      return;

    card[80] = Null;
    s = card + strlen(keyword);
    while ((*s != Null) && isspace(*s)) s++;
    while ((*s != Null) && (*s == '=')) s++;
    while ((*s != Null) && isspace(*s)) s++;
    s1 = s;

    while ((*s != Null) &&
           (isdigit(*s) || (*s == '+') || (*s == '-') ||
            (*s == '.') || (*s == 'E') || (*s == 'D') ||
            (*s == 'e') || (*s == 'd'))) {
      if (*s == 'd') *s = 'e';
      if (*s == 'D') *s = 'E';
      s++;
    }
    *s = Null;

    if (sscanf(s1, "%lg", value) != 1) *value = def;

    return;
}

/***********************************************************************/
void fitrdhdr(Void *file, Const char *keyword, FLOAT *value, FLOAT def)
/*
  This reads the value of a real-valued FITS keyword from the file header.

  Input:
    file	The pointer returned by fitopen.
    keyword	The keyword to search for.
    def		If the keyword is not found, this "default value" is
		returned.
  Output:
    value	The value read from the FITS header.
-----------------------------------------------------------------------*/
{
    double temp, dtemp;

    dtemp = def;
    fitrdhdd(file, keyword, &temp, dtemp);
    *value = temp;
    return;
}

/***********************************************************************/
void fitrdhdi(Void *file, Const char *keyword, int *value, int def)
/*
  This reads the value of a integer-valued FITS keyword from the file header.

  Input:
    file	The pointer returned by fitopen.
    keyword	The keyword to search for.
    def		If the keyword is not found, this "default value" is
		returned.
  Output:
    value	The value read from the FITS header.
-----------------------------------------------------------------------*/
{
    double temp, dtemp;

    dtemp = def;
    fitrdhdd(file, keyword, &temp, dtemp);
    *value = temp;
    return;
}

/***********************************************************************/
void fitrdhda(Void *file, Const char *keyword, char *value, Const char *defval, size_t maxlen)
/*
  This reads the value of a character FITS keyword from the file header.

  Input:
    file	The pointer returned by fitopen.
    keyword	The keyword to search for.
    defval	If keyword is not found, this default value is returned.
    maxlen	The maximum length of the output string.
  Output:
    value	The value read from the FITS header.
-----------------------------------------------------------------------*/
{
    char *s, *s1;
    char quotechar;
    char card[81];
    int quote;
    size_t length;
    FITS *f;

    *value = Null;
    if (maxlen <= 1)
      return;

    length = MIN(strlen(defval), maxlen - 1);
    (void)memcpy(value, defval, length);
    *(value+length) = Null;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return;

    s1 = (char *)defval;
    if (fitsrch(f, keyword, card) >= 0) {
      card[80] = Null;
      s = card + strlen(keyword);
      while ((*s != Null) && isspace(*s)) s++;
      while ((*s != Null) && (*s == '=')) s++;
      while ((*s != Null) && isspace(*s)) s++;
      quotechar = *s;                     /* Save the quote character. */
      quote = ((*s == '\'') || (*s == '"'));
      if (quote)  s++;                    /* Skip the quote character. */
      s1 = s;
      while (*s != Null) {
        if (quote && (*s == quotechar)) /* Stop when quote char found. */
          break;
        s++;
      }
      *s = Null;
      if (s == s1)                  /* True only if no string present. */
        s1 = (char *)defval;
    }

    length = MIN(strlen(s1), maxlen - 1);
    (void)memcpy(value, s1, length);
    *(value+length) = Null;

    return;
}

/***********************************************************************/
void fitrdhd(Void *file, int n, char card[81])
/*
  This reads a FITS card. No check is made that the request is valid,
  and it sets the string to NULL if an i/o error is detected.
-----------------------------------------------------------------------*/
{
    long int offset;
    FITS *f;

    card[0] = Null;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return;

    offset = 80 * n;
    if (fseek(f->fd, offset, SEEK_SET) != 0)
      return;

    if (fread((Void *)card, sizeof(char), 80, f->fd) != 80) {
      (void)fprintf(stderr,
        "### Fatal Error: Error reading a FITS header card.\n");
      card[0] = Null;
    }

    card[80] = Null;
    return;
}

/***********************************************************************/
int fithdprsnt(Void *file, Const char *keyword)
/*
  Returns 1 if keyword is present in header; 0 otherwise.
-----------------------------------------------------------------------*/
{
    char card[81];
    FITS *f;

    if ((f = (FITS *)file) == (FITS *)NULL)
      return(0);

    if (fitsrch(f, keyword, card) < 0)
      return(0);                                  /* Header not found. */

    return(1);                                        /* Header found. */
}

/***********************************************************************/
static int fitsrch(FITS *f, Const char *keyword, char card[80])
/*
  This searches for a FITS keyword in a file.
  Returns -1 if header not present; 0 indexed card number otherwise.
-----------------------------------------------------------------------*/
{
    char key[80];
    int ncard;
    size_t length;

    length = strlen(keyword);
    for (ncard = 0; ncard < length; ncard++) {
      if (islower((int)keyword[ncard]))
        key[ncard] = toupper((int)keyword[ncard]);
      else
        key[ncard] = keyword[ncard];
    }
    key[length] = Null;

    ncard = 0;
    (void)rewind(f->fd);
    while (fread((Void *)card, sizeof(char), 80, f->fd) == 80) {
      if (((card[length] == ' ') || (card[length] == '=')) &&
           (strncmp(card, key, length) == 0)) return(ncard);
      else if (strncmp(card, "END     ", 8) == 0) return(-1);
      ncard++;
    }

    return(-1);
}

#ifdef TEST
/*
 * The remainder of the file provides code to test the callable
 * routines in this file.  Compile the code with -DTEST and run
 * it with one argument: the name of a fits file.
 */

#define VERSION_ID "1.0"
#define BADPIXEL  -99.0
int debugMode = 0;

int wipDebugMode(void)
{
    return(debugMode);
}

main(int argc, char *argv[])
{
    Void *file;
    char *infile;
    char ctype1[80];
    int indx;
    int naxis = MAXNAX;
    int axes[MAXNAX];
    float fmax;
    double crval1;
    FLOAT *array;

    (void)printf("%s Version %s\n\n", argv[0], VERSION_ID);
    if (argc < 2) {
      (void)printf("Usage: %s [in=]image-dataset\n", argv[0]);
      (void)printf("Test of FITS image routines.\n");
      exit(0);
    }

    infile = argv[1];
    if ((argc > 2) && (infile[0] == '-') && (infile[1] == 'g')) {
      debugMode = 1;
      infile = argv[2];
    }
    if ((int)strlen(infile) > 3) {        /* see if in= was used. */
      if (strncmp(infile, "in=", 3) == 0) /* if so, */
        infile += 3;                      /* increase pointer. */
    }

    (void)printf("TEST: Opening file %s.\n", infile);
    file = fitopen(infile, naxis, axes);
    if (file == (Void *)NULL) {
      (void)printf("TEST: ###### Failed to open %s.\n", infile);
      exit(0);
    }

    fitrdhdi(file, "naxis1", &indx, -1);
    (void)printf("TEST: naxis1 is %d.\n", indx);
    fitrdhdr(file, "datamax", &fmax, -1.0);
    (void)printf("TEST: datamax is %f.\n", fmax);
    fitrdhdd(file, "crval1", &crval1, -1.0);
    (void)printf("TEST: crval1 is %lf.\n", crval1);
    fitrdhda(file, "ctype1", ctype1, "(none)", 80);
    (void)printf("TEST: ctype1 is %s. ", ctype1);
    (void)printf("TEST: strlen(ctype1) is %d.\n", strlen(ctype1));

    array = (FLOAT *)malloc(indx * sizeof(FLOAT));
    if (array == (FLOAT *)NULL) {
      (void)printf("TEST: ##### Failed to allocate %d FLOAT elements.\n", indx);
    } else {
      fitrdhdi(file, "naxis3", &indx, 1);
      if (indx > 2) indx = 2;
      (void)printf("TEST: Setting plane %d.\n", indx);
      if (fitsetpl(file, 1, &indx))
        (void)printf("TEST: ##### Failed to set plane %d.\n", indx);
      indx = 0;
      (void)printf("TEST: Reading row %d.\n", indx);
      if (fitread(file, indx, array, BADPIXEL))
        (void)printf("TEST: ##### Failed to read row %d.\n", indx);
      fitrdhdi(file, "naxis2", &indx, 2);
      indx /= 2;
      (void)printf("TEST: Reading row %d.\n", indx);
      if (fitread(file, indx, array, BADPIXEL))
        (void)printf("TEST: ##### Failed to read row %d.\n", indx);
      (void)free((Void *)array);
    }

    (void)printf("TEST: Closing file %s.\n", infile);
    fitclose(file);
    (void)printf("TEST: Finished.\n", infile);
}
#endif /* TEST */
