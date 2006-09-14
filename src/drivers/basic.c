/*
 *      Routines to access and manipulate an image.
 *
 *  History:
 *    01sep89 wls  Original version.
 *    06nov91  jm  Converted for use with WIP images.
 *    02mar92  jm  Modified basread for badpixels.
 *    28sep92  jm  Set axes[2] to imagesize/planesize.  This
 *                 permits multiple planes in a basic image.
 *    10nov93  jm  ANSI-fied the code and changed level 2 I/O calls
 *                 to level 3 ANSI supported I/O calls.
 *    30sep96  jm  Modified so byte swapped machines do NOT swap by
 *                 default (since basic images are usually in local
 *                 format anyway).  However, I have modified the format
 *                 so that one can swap (Upper case datatype) if needed.
 *    14sep06 pjt  minor cleanup
 *
 *----------------------------------------------------------------------
 *
 *  This loader assumes that the file pointed to contains a simple
 *  densely-packed 2-dimensional array of pixels.  Most other file
 *  formats will contain something like this somewhere, so this
 *  should enable loading most anything if no other driver exists
 *  for that format.
 *
 *  The necessary information to load the file is parsed from the
 *  file name:
 * 	filename`columnsXrows[`offset]
 *
 *  where:
 *   filename 		is the name of file on disk;
 *   columns and rows	are in units of pixels;
 *   X (datatype):	the type of pixels.  A single character
 *			that separates rows from columns:
 *			b  	unsigned bytes
 * 			s	signed 2 byte integers
 *			l	signed 4 byte integers
 *			r	4 byte floating point
 *			d	8 byte double precision floating point
 *			Use UPPER CASE to force a byte swap.
 *   offset		The offset from beginning of the file in bytes.
 *			Offset is set, by default to 0.
 *
 *----------------------------------------------------------------------
 * Routines:
 *  Void *basopen(Const char *name, int naxis, int axes[]);
 *  void basclose(Void *file);
 *  int basread(Void *file, int indx, FLOAT *array, FLOAT badpixel);
 *  int bassetpl(Void *file, int naxis, int axes[]);
 *  void basrdhdd(Void *file, Const char *keyword, double *value,double defval);
 *  void basrdhdr(Void *file, Const char *keyword, FLOAT *value, FLOAT defval);
 *  void basrdhdi(Void *file, Const char *keyword, int *value, int defval);
 *  void basrdhda(Void *file, Const char *keyword, char *value, Const char *defval, size_t maxlen);
 *  int bashdprsnt(Void *file, Const char *keyword);
 */

#define WIP_IMAGE
#include "image.h"

/* Define parameters needed just for this driver. */

#define TYPE_BYTE   1
#define TYPE_16INT  2
#define TYPE_32INT  3
#define TYPE_FLOAT  4
#define TYPE_DOUBLE 5

/* Define private structure used to handle image. */

typedef struct {
    FILE *fd;             /* handle into file. */
    int imtype;           /* description of byte size. */
    int bytesize;         /* size of one item in bytes. */
    int naxis;            /* number of axes. */
    int axes[MAXNAX];     /* array of number of pixels in each direction. */
    long int start;       /* initial offset of data area into file. */
    long int offset;      /* offset from "start" to desired plane. */
    int doswapping;       /* Non-zero if byte-swapping is requested. */
} BASIC;

/* Declare static variables. */

static char *Buf1 = (char *)NULL;
static char *Buf2 = (char *)NULL;
static size_t Maxdim = 0;

/************************************************************************/
Void *basopen(Const char *name, int naxis, int axes[])
/* basopen -- Open an image file.

  Input:
    name        The name of the file to be opened.
    naxis       The maximum number of axes that the calling program can
                handle.  If the data file has fewer than naxis axes, the
                higher dimensions are treated as having only one element.
                If the data file has more than naxis axes, and the higher
                dimensions are more than 1 element deep, basopen bombs out.
  Output:
    axes        Array of size "naxis".  It returns the number of pixels
                along each axis.
    basopen     This is a pointer (caste to a char pointer) to a
                structure which describes the BASIC file.  This pointer
                is used in all subsequent calls to the basic routines.
                basopen returns NULL if the file does not comply with
                BASIC file structures.
------------------------------------------------------------------------*/
{
    register char *p;
    char filename[BUFSIZ];
    int nx, ny, imtype;
    int doswapping;
    long int imsize, imoffset;
    long int planesize, size;
    long int imagesize;
    size_t i;
    FILE *fd;
    BASIC *f;

    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "Trying BASIC format...\n");

    if (naxis < 2) {
      (void)fprintf(stderr,
        "### Warning: There must be more than %d dimensions.\n", naxis);
      return((Void *)NULL);
    }

    (void)strcpy(filename, name);

    /*  Check that the header is in the right form for BASIC format? */
    /*  This means it must contain at least one backquote. */
    if ((p = strchr(filename, '`')) == (char *)NULL) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr,
          "### Warning: Required backquote not found in [%s].\n", name);
      return((Void *)NULL);
    }
    *p++ = Null; 

    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "BASIC: filename = %s.\n", filename);

    /*  First, there must be a non-zero length span of numbers. */
    if ((i = strspn(p, "0123456789")) == (size_t)0) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr,
          "### Warning: Required X dimension not found in [%s`%s].\n",
          filename, p);
      return((Void *)NULL);
    }

    /* Column number must be positive definite. */
    if ((nx = atoi(p)) <= 0) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr, "### Warning: Trouble decoding X dimension.\n");
      return((Void *)NULL);
    }

    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "BASIC: ncols = %d.\n", nx);

    p += i;

    /*  The next character should determine the image type. */
    /*  It must be one of the listed types. */
    /*  imsize is the size of one data element in bytes. */
    doswapping = isupper(*p);
    switch (*p++) {
      case 'b': 
      case 'B': 
        imsize = sizeof(unsigned char);
        imtype = TYPE_BYTE;
        break;
      case 's':
      case 'S':
        imsize = sizeof(short int);
        imtype = TYPE_16INT;
        break;
      case 'l':
      case 'L':
        imsize = sizeof(long int);
        imtype = TYPE_32INT;
        break;
      case 'r':
      case 'R':
        imsize = sizeof(float);
        imtype = TYPE_FLOAT;
        break;
      case 'd':
      case 'D':
        imsize = sizeof(double);
        imtype = TYPE_DOUBLE;
        break;
      default:
        if (wipDebugMode() > 0)
          (void)fprintf(stderr,
            "### Warning: Incorrect image type format: [%c].\n", *(p-1));
        return((Void *)NULL);
    }
		
    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "BASIC: image pixel size = %ld.\n", imsize);

    /* Third, there must be another non-zero length span of numbers. */
    if ((i = strspn(p, "0123456789")) == (size_t)0) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr, "### Warning: Required Y dimension not found.\n");
      return((Void *)NULL);
    }

    /* Row number must be positive definite. */
    if ((ny = atoi(p)) <= 0) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr, "### Warning: Trouble decoding Y dimension.\n");
      return((Void *)NULL);
    }

    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "BASIC: nrows = %d.\n", ny);

    p += i;

    /* File name must end here or continue with an offset specifier. */
    if ((*p != Null) && (*p != '`')) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr,
          "### Warning: Incorrect file format ending: [%s].\n", p);
      return((Void *)NULL);
    }

    imoffset = 0;  /* old ip->physstart */

    /* Finally, check for an offset. */
    if (*p++ == '`') {
      if ((i = strspn(p, "0123456789")) == (size_t)0) {
        if (wipDebugMode() > 0)
          (void)fprintf(stderr,
            "### Warning: Incorrect file format offset: %s.\n", p);
        return((Void *)NULL);
      }

      imoffset = atoi(p);
      p += i;

      /*  The name must end here */
      if ((*p != Null) && (*p != '`')) {
        if (wipDebugMode() > 0)
          (void)fprintf(stderr,
            "### Warning: Incorrect file format ending: %s.\n", p);
        return((Void *)NULL);
      }
    }

    if (wipDebugMode() > 0)
      (void)fprintf(stdout, "BASIC: offset = %ld.\n", imoffset);

    /* Try to open the file. */
    if ((fd = fopen(filename, "rb")) == (FILE *)NULL) {
      if (wipDebugMode() > 0)
        (void)fprintf(stderr, "### Warning: Failed to open [%s].\n", filename);
      return((Void *)NULL);
    }

    /* Total size of an image plane in bytes. */
    planesize = imsize * nx * ny;
    size = planesize + imoffset;

    if ((fseek(fd, 0L, SEEK_END) != 0) || ((imagesize = ftell(fd)) < size)) {
      (void)fprintf(stderr,
        "### Warning: Image part of [%s] appears too small.\n", name);
      (void)fclose(fd);
      return((Void *)NULL);
    }

    if ((f = (BASIC *)malloc(sizeof(BASIC))) == (BASIC *)NULL) {
      (void)fprintf(stderr,
        "### Warning: Failed to allocate memory in basopen.\n");
      (void)fclose(fd);
      return((Void *)NULL);
    }

    axes[0] = nx;
    axes[1] = ny;
    axes[2] = (imagesize - imoffset + planesize - 1) / planesize;
    for (i = 3; i < naxis; i++) axes[i] = 1;

    f->fd = fd;
    f->naxis = naxis;
    for (i = 0; i < naxis; i++) f->axes[i] = axes[i];
    for (i = naxis; i < MAXNAX; i++) f->axes[i] = 1;
    f->imtype = imtype;
    f->bytesize = imsize;
    f->start = imoffset;
    f->offset = 0;
    f->doswapping = doswapping;
#if NO_CVT
    f->doswapping = 0;    /* This is forced to zero here. */
#endif

    /* Make sure we have enough memory to deal with this file. */

    if (Maxdim < (sizeof(imtype) * axes[0])) {
      Maxdim = sizeof(imtype) * axes[0];
      if (f->imtype == TYPE_DOUBLE) {
        Buf1 = (Buf1 == (char *)NULL) ? (char *)malloc(sizeof(double)*axes[0]) :
                                (char *)realloc(Buf1,sizeof(double)*axes[0]);
        Buf2 = (Buf2 == (char *)NULL) ? (char *)malloc(8*axes[0]) :
                                (char *)realloc(Buf2,8*axes[0]);
      } else {
        Buf1 = (Buf1 == (char *)NULL) ? (char *)malloc(sizeof(int)*axes[0]) :
                                (char *)realloc(Buf1,sizeof(int)*axes[0]);
        Buf2 = (Buf2 == (char *)NULL) ? (char *)malloc(4*axes[0]) :
                                (char *)realloc(Buf2,4*axes[0]);
      }
      if ((Buf1 == (char *)NULL) || (Buf2 == (char *)NULL)) {
        (void)fprintf(stderr,
          "### Fatal Error: Ran out of memory opening basic image.\n");
        basclose((Void *)f);
        return((Void *)NULL);
      }
    }

  return((Void *)f);
}

/************************************************************************/
void basclose(Void *file)
/* basclose -- Close up an image file.

  Input:
    file        The handle of the structure returned from basopen.
------------------------------------------------------------------------*/
{
    BASIC *f;

    if ((f = (BASIC *)file) == (BASIC *)NULL)
      return;

    if (f->fd != (FILE *)NULL) (void)fclose(f->fd);
    (void)free((Void *)f);
    file = (Void *)NULL;

    return;
}

/* ARGSUSED */
/************************************************************************/
int basread(Void *file, int indx, FLOAT *array, FLOAT badpixel)
/* basread -- Read a single row from an image.  This accesses the plane
   given by the last call to bassetpl.

  Input:
    file        The image file handle, returned by basopen.
    indx        The row number to read. This varies from 0 to NAXIS2-1.
    badpixel    Value to set a bad input pixel on return. (Unused here.)
  Output:
    array       A FLOAT array of NAXIS1 elements (ie. the pixel values).
    basread     Set to 0 if read okay; != 0 otherwise.
------------------------------------------------------------------------*/
{
    unsigned char *bdat;
    int *idat;
    register int i;
    long int offset;
    size_t length;
    INT16 *jdat;
    FLOAT *fdat;
    double *ddat;
    BASIC *f;

    if ((f = (BASIC *)file) == (BASIC *)NULL)
      return(1);

    length = f->bytesize * f->axes[0];
    offset = (f->bytesize * f->offset) + (indx * length) + f->start;
    if (fseek(f->fd, offset, SEEK_SET) != 0)
      return(1);

    if (indx > f->axes[1]) {
      (void)fprintf(stderr,
        "### Error: Attempt to read beyond image boundaries in basread.\n");
      return(1);
    }

    if (fread((Void *)Buf2, sizeof(char), length, f->fd) != length) {
      (void)fprintf(stderr, "### Error: I/O read error in basread.\n");
      return(1);
    }

    /* We have the data now. Convert it. */

    if (f->imtype == TYPE_BYTE) {
      bdat = (unsigned char *)Buf2;
      for (i = 0; i < f->axes[0]; i++) array[i] = (FLOAT)bdat[i];
    } else if (f->imtype == TYPE_16INT) {
      if (f->doswapping > 0) {
        jdat = (INT16 *)Buf1;
        unpack16_c(Buf2, jdat, f->axes[0]);
      } else {
        jdat = (INT16 *)Buf2;
      }
      for (i = 0; i < f->axes[0]; i++) array[i] = (FLOAT)jdat[i];
    } else if (f->imtype == TYPE_32INT) {
      if (f->doswapping > 0) {
        idat = (int *)Buf1;
        unpack32_c(Buf2, idat, f->axes[0]);
      } else {
        idat = (int *)Buf2;
      }
      for (i = 0; i < f->axes[0]; i++) array[i] = (FLOAT)idat[i];
    } else if (f->imtype == TYPE_FLOAT) {
      if (f->doswapping > 0) {
        unpackr_c(Buf2, array, f->axes[0]);
      } else {
        fdat = (FLOAT *)Buf2;
        for (i = 0; i < f->axes[0]; i++) array[i] = (FLOAT)fdat[i];
      }
    } else { /* DOUBLE */
      if (f->doswapping > 0) {
        ddat = (double *)Buf1;
        unpackd_c(Buf2, ddat, f->axes[0]);
      } else {
        ddat = (double *)Buf2;
      }
      for (i = 0; i < f->axes[0]; i++) array[i] = (FLOAT)ddat[i];
    }

    return(0);
}

/************************************************************************/
int bassetpl(Void *file, int naxis, int axes[])
/* bassetpl -- Set which plane of a cube is to be accessed.

  Input:
    file	Handle of the image file returned by basopen.
    naxis	Size of the "axes" array.
    axes	This gives the one-relative indices, along the 3rd, 4th,
		5th, etc dimensions, of the plane that is to be accessed.
		axes[0] corresponds to the index along the 3rd dimension.
------------------------------------------------------------------------*/
{
    register int i;
    long int offset;
    BASIC *f;

    if ((f = (BASIC *)file) == (BASIC *)NULL)
      return(1);

    if ((naxis < 1) || ((naxis + 1) >= MAXNAX))
      return(1);

    offset = 0;
    for (i = naxis - 1; i >= 0; i--) {
      if ((axes[i] < 1) || (axes[i] > f->axes[i+2])) {
        (void)fprintf(stderr,
          "### Error: Illegal coordinate index in bassetpl.\n");
        return(1);
      }
      offset = (offset + axes[i] - 1) * f->axes[i+1];
    }
    f->offset = offset * f->axes[0];
    return(0);
}

/* ARGSUSED */
/************************************************************************/
void basrdhdd(Void *file, Const char *keyword, double *value, double defval)
/* basrdhdd -- Read a double precision-valued header variable.

  Input:
    file	The file handle of the data set.
    keyword	The name of the header variable.
    defval	The default value to return, if the header variable
		is not found.
  Output:
    value	The value of the header variable. This will be the default
		value, if the variable is missing from the header.
------------------------------------------------------------------------*/
{
    /* Basic images have no headers, so just return default value. */
    *value = defval;
    return;
}

/* ARGSUSED */
/************************************************************************/
void basrdhdr(Void *file, Const char *keyword, FLOAT *value, FLOAT defval)
/* basrdhdr -- Read a real-valued header variable.

  Input:
    file	The file handle of the data set.
    keyword	The name of the header variable.
    defval	The default value to return, if the header variable
		is not found.
  Output:
    value	The value of the header variable. This will be the default
		value, if the variable is missing from the header.
------------------------------------------------------------------------*/
{
    double dvalue, ddefval;

    ddefval = defval;
    basrdhdd(file, keyword, &dvalue, ddefval);
    *value = dvalue;
    return;
}

/* ARGSUSED */
/************************************************************************/
void basrdhdi(Void *file, Const char *keyword, int *value, int defval)
/* basrdhdi -- Read an integer-valued header variable.

  Input:
    file	The file handle of the data set.
    keyword	The name of the header variable.
    defval	The default value to return, if the header variable
		is not found.
  Output:
    value	The value of the header variable. This will be the default
		value, if the variable is missing from the header.
------------------------------------------------------------------------*/
{
    double dvalue, ddefval;

    ddefval = defval;
    basrdhdd(file, keyword, &dvalue, ddefval);
    *value = dvalue;
    return;
}

/* ARGSUSED */
/************************************************************************/
void basrdhda(Void *file, Const char *keyword, char *value, Const char *defval, size_t maxlen)
/* basrdhda -- Read a string-valued header variable.

  Input:
    file	The file handle of the data set.
    keyword	The name of the header variable.
    defval	The default value to return, if the header variable
		is not found.
    maxlen	The maximum length of the output string.
  Output:
    value	The value of the header variable. This will be the
		default value if the variable is missing from the header.
------------------------------------------------------------------------*/
{
    /* Basic images have no headers, so just return default value. */
    size_t length;

    *value = Null;
    if (maxlen <= 1)
      return;

    length = MIN(strlen(defval), maxlen - 1);
    (void)memcpy(value, defval, length);
    *(value+length) = Null;
    return;
}

/* ARGSUSED */
/**********************************************************************/
int bashdprsnt(Void *file, Const char *keyword)
/*
  Returns 1 if keyword is present in header; 0 otherwise.
------------------------------------------------------------------------*/
{
    /* Basic images have no headers, so keyword is NEVER preset. */
    return(0); /* Not found. */
}

#ifdef TEST
/*
 * The remainder of the file provides code to test the callable
 * routines in this file.  Compile the code with -DTEST and run
 * it with one argument: the name of a BASIC image.
 */
#define VERSION_ID "1.0"
#define BADPIXEL -99.0
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
      (void)printf("Test of Basic image routines.\n");
      exit(0);
    }

    infile = argv[1];
    if ((argc > 2) && (infile[0] == '-') && (infile[1] == 'g')) {
      debugMode = 1;
      infile = argv[2];
    }
    if (strlen(infile) > 3) {             /* see if in= was used. */
      if (strncmp(infile, "in=", 3) == 0) /* if so, */
        infile += 3;                      /* increase pointer. */
    }
 
    (void)printf("TEST: Opening file %s.\n", infile);
    file = basopen(infile, naxis, axes);
    if (file == (Void *)NULL) {
      (void)printf("TEST: ###### Failed to open %s.\n", infile);
      exit(0);
    }

    basrdhdi(file, "naxis1", &indx, -1);
    (void)printf("TEST: naxis1 is %d.\n", indx);
    basrdhdr(file, "datamax", &fmax, -1.0);
    (void)printf("TEST: datamax is %f.\n", fmax);
    basrdhdd(file, "crval1", &crval1, -1.0);
    (void)printf("TEST: crval1 is %lf.\n", crval1);
    basrdhda(file, "ctype1", ctype1, "(none)", 80);
    (void)printf("TEST: ctype1 is %s. ", ctype1);
    (void)printf("TEST: strlen(ctype1) is %d.\n", strlen(ctype1));
  
    if (indx < 0) indx = axes[0];
    array = (FLOAT *)malloc(indx * sizeof(FLOAT));
    if (array == (FLOAT *)NULL) {
      (void)printf("TEST: ##### Failed to allocate %d FLOAT elements.\n", indx);
    } else {
      basrdhdi(file, "naxis3", &indx, 1);
      if (indx > 2) indx = 2;
      (void)printf("TEST: Setting plane %d.\n", indx);
      if (bassetpl(file, 1, &indx))
        (void)printf("TEST: ##### Failed to set plane %d.\n", indx);
      indx = 0;
      (void)printf("TEST: Reading row %d.\n", indx);
      if (basread(file, indx, array, BADPIXEL))
        (void)printf("TEST: ##### Failed to read row %d.\n", indx);
      basrdhdi(file, "naxis2", &indx, 2);
      indx /= 2;
      (void)printf("TEST: Reading row %d.\n", indx);
      if (basread(file, indx, array, BADPIXEL))
        (void)printf("TEST: ##### Failed to read row %d.\n", indx);
      (void)free((Void *)array);
    }

    (void)printf("TEST: Closing file %s.\n", infile);
    basclose(file);
    (void)printf("TEST: Finished.\n", infile);
}
#endif /* TEST */
