/*
 *	Routines to access and manipulate a Miriad image.
 *
 *  History:
 *    rjs  Dark-ages Original version.
 *    rjs   6nov89   Neatly handle the case of a non-existent mask file.
 *    rjs   7feb90   Added comments, ready to be stripped out by "doc".
 *     jm  22oct91   Converted for use with WIP images.
 *     jm  14nov91   Added mask reading routine.
 *     jm  02mar92   Modified mirread to permit user to specify a
 *                   flagged pixel's value.
 *     jm  09nov93   ANSI-fied the code and changed level 2 I/O calls
 *                   to level 3 ANSI supported I/O calls.
 *     jm  12jul96   Modified to permit small masks in the header.
 *                   Also moved static Buffer into MIRIAD structure.
 *    pjt    aug99   fixed sizeing error in reading masks; (in linux, read too much)
 *    pjt  12sep99   Fixed horrid bug fclose() mask, solved linux coredumps
 *                   but still some very odd linux problems with masked files
 *    pjt   2dec00   ansi-c, tracking down that still unsolved problem
 *    pjt  21sep02   back to that unsolved problem, courtesy Tam (and some cleanup)
 *                   The problem was little endian machines with a mask file would
 *                   sometimes (depending on size) read the mask in a too small 
 *                   buffer. Solution was to teach the mask reader to use a smaller buffer
 *                   (of course miriad.c should have never been written but inherited from
 *                   the corresponding MIRIAD libraries....... i.e. xyio.c   
 *
 * Routines:
 * Void *miropen(Const char *name, int naxis, int axes[]);
 * void mirclose(Void *file);
 * int mirread(Void *file, int indx, FLOAT *array, FLOAT badpixel);
 * int mirsetpl(Void *file, int naxis, int axes[]);
 * void mirrdhdd(Void *file, Const char *keyword, double *value, double defval);
 * void mirrdhdr(Void *file, Const char *keyword, FLOAT *value, FLOAT defval);
 * void mirrdhdi(Void *file, Const char *keyword, int *value, int defval);
 * void mirrdhda(Void *file, Const char *keyword, char *value, Const char *defval, size_t maxlen);
 * int mirhdprsnt(Void *file, Const char *keyword);
 * static char *mirsrch(MIRIAD *f, Const char *keyword, long int *size, long int *offset);
 * static MIRMASK *mirfindmask(MIRIAD *f, Const char *name, int nx);
 * static int maskread(MIRIAD *f,int *flags, size_t length, long int offset);
 */

#define WIP_IMAGE
#include "image.h"

/* Define parameters needed just for this driver. */

#define CACHE_ENT      16             /* Alignment of cache items. */
#define ITEM_HDR_SIZE   4
#define H_INT_SIZE      4
#define H_REAL_SIZE     4
#define H_DBLE_SIZE     8

static char char_item[ITEM_HDR_SIZE] = {0,0,0,1};
static char  int_item[ITEM_HDR_SIZE] = {0,0,0,2};
static char real_item[ITEM_HDR_SIZE] = {0,0,0,4};
static char dble_item[ITEM_HDR_SIZE] = {0,0,0,5};

#define HEADER "/header"
#define IMAGE  "/image"
#define MASK   "/mask"

/* normally BUFFERSIZE is 128 or so, but it really can be (should be?) dynamic.
   to fix the bug with masks, the easiest is just to make BUFFERSIZE as large
   as MAXDIM.... really guys, this code should not even exist, it should be re-used
   from the xyio routines in the miriad library...  that library also supports LFS
   (files > 2GB). This routine will have too much work getting that working.
*/
#define BUFFERSIZE    10000
#define BITS_PER_INT   31
#define MASKOFFSET   (((ITEM_HDR_SIZE-1)/H_INT_SIZE + 1)*BITS_PER_INT)

/* Rounding up a number, used in debugging linux */
/* undef it if you want to revert back to the old buggy code */
/* with overwrite's  */
#define   RND(x,y)     ((y)*(((x)+(y)-1)/(y)))

static int bits[BITS_PER_INT] = {
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000};

/* Define the private structures used to handle this image. */
#define BUGSPILL 4

typedef struct {
  FILE *fdmask;          /* File descriptor of mask item. */
  int  buf[BUFFERSIZE+BUGSPILL];  /* Array used to read mask information. */
  int *flag;             /* Array used to hold masks for one row. - one int per pixel */
  long int size;         /* Size of mask item on disk (including int_item). */
  long int offset;       /* Offset into mask item stored in buf[]. */
  long int length;       /* Marker of how far into buf[] we have gone. */
  long int hdoffset;     /* Offset into header; 0 if mask not in header. */
} MIRMASK;

typedef struct {
  FILE *fdhead;      /* File descriptor of header item. */
  FILE *fdimage;     /* File descriptor of image item. */
  int naxis;         /* Number of axes in image. */
  int axes[MAXNAX];  /* Number of pixels along each axis. */
  long int offset;   /* Offset into file. */
  char *buffer;      /* Array used if data conversion needed. */
  int buflen;        /* allocated space for *buffer */
  MIRMASK *mask;     /* Pointer to mask structure. */
} MIRIAD;

/* Declare private routines. */

static char *mirsrch(MIRIAD *f, Const char *keyword, long int *size, long int *offset);
static MIRMASK *mirfindmask(MIRIAD *f, Const char *name, int nx);
static int maskread(MIRIAD *f,int *flags, size_t len, long int offset);

/* Declare static variables (if needed). */

/************************************************************************/
Void *miropen(Const char *name, int naxis, int axes[])
/* miropen -- Open an image file.

  Input:
    name	The name of the file to be opened.
    naxis	The maximum number of axes that the calling program can
		handle.  If the data file has fewer than naxis axes, the
		higher dimensions are treated as having only one element.
		If the data file has more than naxis axes, and the higher
		dimensions are more than 1 element deep, miropen bombs out.
  Output:
    axes	Array of size "naxis".  It returns the number of pixels
		along each axis.
    miropen	This is a pointer (caste to a char pointer) to a
		structure which describes the MIRIAD file.  This pointer
		is used in all subsequent calls to the mirth routines.
		miropen returns NULL if the file does not comply with
		MIRIAD file structures.
------------------------------------------------------------------------*/
{
    char naxes[16], s[ITEM_HDR_SIZE];
    char filename[BUFSIZ];
    int i, ndim, temp, buflen;
    long int size;
    size_t retval;
    MIRIAD *f;

    if (wipDebugMode() > 0)
      fprintf(stderr, "Trying MIRIAD format...\n");

    if ((f = (MIRIAD *)malloc(sizeof(MIRIAD))) == (MIRIAD *)NULL) {
      fprintf(stderr,
        "### Warning: Failed to allocate memory in miropen\n");
      return((Void *)NULL);
    }

    /* Initialize some things so mirclose() will work properly. */

    f->fdhead = (FILE *)NULL;
    f->fdimage = (FILE *)NULL;
    f->buffer = (char *)NULL;
    f->mask = (MIRMASK *)NULL;

    /* Access the image data. */

    strcpy(filename, name);
    strcat(filename, HEADER);
    if ((f->fdhead = fopen(filename, "rb")) == (FILE *)NULL) {
      if (wipDebugMode() > 0)
        fprintf(stderr, "### Warning: Failed to open [%s].\n", filename);
      mirclose((Void *)f);
      return((Void *)NULL);
    }

    strcpy(filename, name);
    strcat(filename, IMAGE);
    if ((f->fdimage = fopen(filename, "rb")) == (FILE *)NULL) {
      if (wipDebugMode() > 0)
        fprintf(stderr, "### Warning: Failed to open [%s].\n", filename);
      mirclose((Void *)f);
      return((Void *)NULL);
    }

    /* Get the number of axes and the length of each axis. */

    mirrdhdi((Void *)f, "naxis", &ndim, 0);
    if ((ndim < 1) || (ndim > MAXNAX)) {
      fprintf(stderr,
        "### Warning: Bad value of NAXIS [%d] with file [%s].\n", ndim, name);
      mirclose((Void *)f);
      return((Void *)NULL);
    }

    size = 1;
    if (wipDebugMode() > 0)
      fprintf(stderr, "MirOpen: 1 size %d\n", size);

    for (i = 0; i < ndim; i++) {
      sprintf(naxes, "naxis%d", i+1);
      mirrdhdi((Void *)f, naxes, &temp, 0);
      if ((temp < 1) || ((i >= naxis) && (temp != 1))) {
        fprintf(stderr,
          "### Warning: Cannot handle Miriad dimension %d being %d.\n",
          i+1, temp);
        mirclose((Void *)f);
        return((Void *)NULL);
      }
      if (i < naxis) axes[i] = temp;
      size *= temp;
    }
    for (i = ndim; i < naxis; i++) axes[i] = 1;

    if (wipDebugMode() > 0)
      fprintf(stderr, "MirOpen: size %d\n", size);


    /*
     *  Now compute and check that the size of the image file is OK
     *  and that it starts with the "real_item" sequence.
     */

    size = (size * H_REAL_SIZE) + ITEM_HDR_SIZE;
    if ((fseek(f->fdimage, 0L, SEEK_END) != 0) || (ftell(f->fdimage) < size)) {
      fprintf(stderr,
        "### Warning: Image file [%s] appears too small.\n", name);
      mirclose((Void *)f);
      return((Void *)NULL);
    }

    rewind(f->fdimage);                /* Rewind the image file. */
    retval = fread((Void *)s, sizeof(char), (size_t)ITEM_HDR_SIZE, f->fdimage);
    if (retval != (size_t)ITEM_HDR_SIZE) {
      fprintf(stderr,
        "### Warning: Trouble reading image file [%s].\n", name);
      mirclose((Void *)f);
      return((Void *)NULL);
    }

    if (memcmp(s, real_item, ITEM_HDR_SIZE) != 0) {
      fprintf(stderr,
        "### Warning: Bad Miriad image file magic number.\n");
      mirclose((Void *)f);
      return((Void *)NULL);
    }

    /* Copy the dimension information to the local description. */

    f->offset = 0;
    f->naxis  = naxis;
    for (i = 0; i < naxis; i++) f->axes[i] = axes[i];
    for (i = naxis; i < MAXNAX; i++) f->axes[i] = 1;

#if NO_CVT
#else
	/* there is a very strange bug here, with possible interaction
	   with PGPLOT. Allocating 'just enough' will e.g. fail on masked
  	   images with size 127 * 127: there appears to be an overwrite
	   somewhere.  I could not get it to crash on other images,
	   and crashing also depended on which pgplot device was used
	   and which pgplot version !
	   PJT  sep 1999
	   Well, buffer handles things for conversion, a mask always
	   has to be rounded up by 32.
         */
#ifdef RND
    buflen = 4*RND(axes[0],4);               /* roundup a bit */
#else
    buflen = 4*axes[0];
#endif
    if (wipDebugMode() > 0)
      fprintf(stderr, "MirOpen: buflen = %d bytes\n", buflen);
    f->buffer = (char *)malloc(buflen);
    if (f->buffer == (char *)NULL) {
      (void)fprintf(stderr,
        "### Fatal Error: Ran out of memory opening Miriad image.\n");
      mirclose((Void *)f);
      return((Void *)NULL);
    }
    f->buflen = buflen;
#endif

    /* If the file has a mask item associated with it, try to access it. */
    if (wipDebugMode() > 0)
      fprintf(stderr, "MirOpen: reading mask %s\n", filename);


    f->mask = mirfindmask(f, name, axes[0]);

    if (wipDebugMode() > 0)
      fprintf(stderr, "MirOpen: done with %s\n", filename);


    return((Void *)f);
}

/************************************************************************/
void mirclose(Void *file)
/* mirclose -- Close up an image file.

  Input:
    file	The handle of the structure returned from miropen.
------------------------------------------------------------------------*/
{
    MIRIAD *f;

    if ((f = (MIRIAD *)file) == (MIRIAD *)NULL)
      return;

    if (f->fdimage != (FILE *)NULL) {
      fclose(f->fdimage);
      f->fdimage = NULL;
    }
    if (f->fdhead != (FILE *)NULL) {
      fclose(f->fdhead);
      f->fdhead = NULL;
    }

    if (f->buffer != (char *)NULL) (void)free((Void *)f->buffer);

    if (f->mask != (MIRMASK *)NULL) {
#if 1
      if ((f->mask->hdoffset == 0) && (f->mask->fdmask != (FILE *)NULL))
#else
	/* this will cause pretty horrid things with masking present */
      if ((f->mask->hdoffset > 0) && (f->mask->fdmask != (FILE *)NULL))
#endif
	{
	  fclose(f->mask->fdmask);
	  f->mask->fdmask = NULL;
	}
      if (f->mask->flag != (int *)NULL) free((Void *)f->mask->flag);
      free((Void *)f->mask);
    }

    free((Void *)f);
    file = (Void *)NULL;

    return;
}

/************************************************************************/
int mirread(Void *file, int indx, FLOAT *array, FLOAT badpixel)
/* mirread -- Read a single row from an image.  This accesses the plane
   given by the last call to mirsetpl.

  Input:
    file	The image file handle, returned by miropen.
    indx	The row number to read. This varies from 0 to NAXIS2-1.
    badpixel	Resulting value of masked data (not used if item mask is
                not present).
  Output:
    array	A FLOAT array of NAXIS1 elements (ie. the pixel values).
    mirread	Set to 0 if read okay; != 0 otherwise.
------------------------------------------------------------------------*/
{
    int *flags;
    register int i;
    long int offset;
    size_t length, flen;
    MIRIAD *f;

    if ((f = (MIRIAD *)file) == (MIRIAD *)NULL)
      return 1;

    length = H_REAL_SIZE * f->axes[0];
    flen = length / sizeof(FLOAT);
    offset = (H_REAL_SIZE * f->offset) + (indx * length) + ITEM_HDR_SIZE;
    if (fseek(f->fdimage, offset, SEEK_SET) != 0)
      return(1);

    if (indx > f->axes[1]) {
      (void)fprintf(stderr,
        "### Error: Attempt to read beyond image boundaries in mirread.\n");
      return(1);
    }

#if NO_CVT
    if (fread((Void *)array, sizeof(FLOAT), flen, f->fdimage) != flen) {
#else
    if (fread(f->buffer, sizeof(char), length, f->fdimage) != length) {
#endif
      (void)fprintf(stderr, "### Error: I/O read error in mirread.\n");
      return(1);
    }

#if NO_CVT
#else
    unpackr_c(f->buffer, (float *)array, f->axes[0]);
#endif

    /* If mask information present, use it to set image to bogus value. */

    if (f->mask != (MIRMASK *)NULL) {
      length = f->axes[0];
      offset = f->offset + (indx * length) + MASKOFFSET;
      flags = f->mask->flag;
      if (maskread(f, flags, length, offset) < 0) {
        fprintf(stderr, "### Error: Mask I/O read error in mirread.\n");
      } else {
#if 0
	/* print out the mask as 1's and 0s' */
	for (i=0; i<f->axes[0]; i++) {
	  if (i==0) printf("MASK: ");
	  if (flags[i]) printf("1");
	  else          printf("0");
	}
	printf("\n");
#endif
        for (i = 0; i < f->axes[0]; i++) {
#ifdef TEST
          if (wipDebugMode() > 0)
            if (*flags   == 0) (void)printf("Badpixel at x=%d\n", i);
#endif /* TEST */
            if (*flags++ == 0) array[i] = badpixel;
        }
      }
    }

    return(0);
}

/************************************************************************/
int mirsetpl(Void *file, int naxis, int axes[])
/* mirsetpl -- Set which plane of a cube is to be accessed.

  Input:
    file	Handle of the image file returned by miropen.
    naxis	Size of the "axes" array.
    axes	This gives the one-relative indices, along the 3rd, 4th,
		5th, etc dimensions, of the plane that is to be accessed.
		axes[0] corresponds to the index along the 3rd dimension.
------------------------------------------------------------------------*/
{
    register int i;
    long int offset;
    MIRIAD *f;

    if ((f = (MIRIAD *)file) == (MIRIAD *)NULL)
      return 1;

    if ((naxis < 1) || ((naxis + 1) >= MAXNAX))
      return 1;

    offset = 0;
    for (i = naxis - 1; i >= 0; i--) {
      if(axes[i] < 1 || axes[i] > f->axes[i+2]) {
        fprintf(stderr,
          "### Error: Illegal coordinate index in mirsetpl.\n");
        return 1;
      }
      offset = (offset + axes[i] - 1) * f->axes[i+1];
    }
    f->offset = offset * f->axes[0];
    return 0;
}
/************************************************************************/
void mirrdhdd(Void *file, Const char *keyword, double *value, double defval)
/* rdhdd -- Read a double precision-valued header variable.

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
    char *item;
    char s[ITEM_HDR_SIZE];
    int itemp;
    long int length, offset;
    float rtemp;
    MIRIAD *f;

    /*
     *  First, assume the variable is missing.  If successful
     *  getting it, read it.
     */

    *value = defval;

    if ((f = (MIRIAD *)file) == (MIRIAD *)NULL)
      return;

    item = mirsrch(f, keyword, &length, &offset);
    if (item != (char *)NULL) {

    /* Determine the type of the value, and convert it to double precision. */

      (void)memcpy(s, item, ITEM_HDR_SIZE);
      if (memcmp(s, int_item, ITEM_HDR_SIZE) == 0) {
        offset = ROUNDUP(ITEM_HDR_SIZE, H_INT_SIZE);
        if ((offset + H_INT_SIZE) == length) {
#if NO_CVT
          (void)memcpy((char *)&itemp, item+offset, sizeof(int));
#else
          unpack32_c(item+offset, &itemp, 1);
#endif
          *value = itemp;
        }
      } else if (memcmp(s, real_item, ITEM_HDR_SIZE) == 0) {
        offset = ROUNDUP(ITEM_HDR_SIZE, H_REAL_SIZE);
        if ((offset + H_REAL_SIZE) == length){
#if NO_CVT
          (void)memcpy((char *)&rtemp, item+offset, sizeof(float));
#else
          unpackr_c(item+offset, &rtemp, 1);
#endif
          *value = rtemp;
        }
      } else if (memcmp(s, dble_item, ITEM_HDR_SIZE) == 0) {
        offset = ROUNDUP(ITEM_HDR_SIZE, H_DBLE_SIZE);
        if ((offset + H_DBLE_SIZE) == length) {
#if NO_CVT
          (void)memcpy((char *)value, item+offset, sizeof(double));
#else
          unpackd_c(item+offset, value, 1);
#endif
        }
      }
      (void)free((Void *)item);
    }
    return;
}

/************************************************************************/
void mirrdhdr(Void *file, Const char *keyword, FLOAT *value, FLOAT defval)
/* mirrdhdr -- Read a real-valued header variable.

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
    mirrdhdd(file, keyword, &dvalue, ddefval);
    *value = dvalue;
    return;
}

/************************************************************************/
void mirrdhdi(Void *file, Const char *keyword, int *value, int defval)
/* mirrdhdi -- Read an integer-valued header variable.

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
    mirrdhdd(file, keyword, &dvalue, ddefval);
    *value = dvalue;
    return;
}

/************************************************************************/
void mirrdhda(Void *file, Const char *keyword, char *value, Const char *defval, size_t maxlen)
/* mirrdhda -- Read a string-valued header variable.

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
    char *item;
    char s[ITEM_HDR_SIZE];
    int varlen;
    long int length, offset;
    MIRIAD *f;

    /*
     *  First, assume the variable is missing.  If successful finding
     *  it, then read it.
     */
    *value = Null;
    if (maxlen <= 1)
      return;

    length = MIN(strlen(defval), maxlen - 1);
    (void)memcpy(value, defval, length);
    *(value+length) = Null;

    if ((f = (MIRIAD *)file) == (MIRIAD *)NULL)
      return;

    item = mirsrch(f, keyword, &length, &offset);
    if (item != (char *)NULL) {
      varlen = MIN((length - ITEM_HDR_SIZE), maxlen - 1);
      if (varlen > 0) {
        (void)memcpy(s, item, ITEM_HDR_SIZE);
        if (memcmp(s, char_item, ITEM_HDR_SIZE) == 0) {
          (void)memcpy(value, item+ITEM_HDR_SIZE, (size_t)varlen);
          *(value+varlen) = Null;
        }
      }
      (void)free((Void *)item);
    }

    return;
}

/**********************************************************************/
int mirhdprsnt(Void *file, Const char *keyword)
/*
  Returns 1 if keyword is present in header; 0 otherwise.
------------------------------------------------------------------------*/
{
    char *item;
    long int length, offset;
    MIRIAD *f;

    if ((f = (MIRIAD *)file) == (MIRIAD *)NULL)
      return(1);

    item = mirsrch(f, keyword, &length, &offset);
    if (item != (char *)NULL) {
      (void)free((Void *)item);
      return(1); /* Header found. */
    }
    return(0); /* Header not found. */
}

/**********************************************************************/
static char *mirsrch(MIRIAD *f, Const char *keyword, long int *size, long int *seekloc)
/*
  This searches for a MIRIAD header item.
  Returns a char string of the header item found; NULL otherwise.
------------------------------------------------------------------------*/
{
    char *item;
    char s[CACHE_ENT];
    long int offset;
    long int itemsize;
    FILE *fd;

    *size = 0;
    *seekloc = 0;
    offset = 0;
    fd = f->fdhead;
    if (fseek(fd, offset, SEEK_SET) != 0)
      return((char *)NULL);

    while (fread((Void *)s, sizeof(char), (size_t)CACHE_ENT, fd) == (size_t)CACHE_ENT) {
      offset += CACHE_ENT;
      itemsize = (long int)(*(s + CACHE_ENT - 1));

      if (strcmp(s, keyword) == 0) {
        if (fseek(fd, offset, SEEK_SET) != 0)
          return((char *)NULL);

        if ((item = (char *)malloc((size_t)itemsize)) == (char *)NULL)
          return((char *)NULL);

        if (fread((Void *)item, sizeof(char), (size_t)itemsize, fd) != (size_t)itemsize) {
          (void)free((Void *)item);
          return((char *)NULL);
        }
        *size = itemsize;
        *seekloc = offset;
        return(item);
      }

      offset += ROUNDUP(itemsize, CACHE_ENT);
      if (fseek(fd, offset, SEEK_SET) != 0)
        return((char *)NULL);
    }

    return((char *)NULL);
}

/************************************************************************/
static MIRMASK *mirfindmask(MIRIAD *f, Const char *name, int nx)
{
    char *item;
    char s[ITEM_HDR_SIZE];
    char filename[BUFSIZ];
    int inheader;
    long int size, offset;
    size_t retval;
    FILE *fd;
    MIRMASK *mask;    /* will be allocated and returned */

    inheader = 0;

    if (wipDebugMode() > 0)
      fprintf(stderr,"MirFindMask: %s\n",name);

    strcpy(filename, name);
    strcat(filename, MASK);
    if ((fd = fopen(filename, "rb")) == (FILE *)NULL) {        /* if mask file not found */
      if (wipDebugMode() > 0)
        fprintf(stderr,
		"### Warning: Failed to open mask file [%s]; will try header.\n",
		filename);

      item = mirsrch(f, "mask", &size, &offset);               /* find it in 'header' */
      if (item == (char *)NULL) {
        if (wipDebugMode() > 0)
          fprintf(stderr,
		  "### Warning: Failed to find mask entry in the header either.\n");
        return (MIRMASK *)NULL;
      } else {
        memcpy(s, item, ITEM_HDR_SIZE);     /* Save the data type. */
        free((Void *)item);      /* Free this allocated memory up. */
      }

      /* Mask is in the header (the image must be small). */

      inheader = 1;
      fd = f->fdhead;
    }

    /* A mask item is associated with this image. */

    if ((mask = (MIRMASK *)malloc(sizeof(MIRMASK))) == (MIRMASK *)NULL) {
      fprintf(stderr,
        "### Warning: Failed to allocate mask memory in miropen.\n");
      if (inheader == 0) fclose(fd);
      return (MIRMASK *)NULL;
    }

    mask->fdmask = fd;

    if (inheader) {
      mask->hdoffset = offset;
      mask->size = size;
    } else {
      mask->hdoffset = 0;

      size = H_INT_SIZE * (MASKOFFSET/BITS_PER_INT);
      if (wipDebugMode() > 0)
	fprintf(stderr,"MirFindMask: MASKOFFSET=%d size=%d\n",
		MASKOFFSET,size);

      if ((fseek(mask->fdmask, 0L, SEEK_END) != 0) ||
          ((mask->size = ftell(mask->fdmask)) <= size)) {
        fprintf(stderr,
          "### Warning: Miriad mask item appears bad in miropen.\n");
        fclose(mask->fdmask);
        free((Void *)mask);
        return (MIRMASK *)NULL;
      }

      rewind(mask->fdmask);             /* Rewind the mask item. */
      retval = fread((Void *)s,sizeof(char),(size_t)ITEM_HDR_SIZE,mask->fdmask);
      if (retval != (size_t)ITEM_HDR_SIZE) {
        fprintf(stderr,
          "### Warning: Trouble reading mask file of [%s].\n", filename);
        fclose(mask->fdmask);
        free((Void *)mask);
        return (MIRMASK *)NULL;
      }
    }

    if (memcmp(s, int_item, ITEM_HDR_SIZE) != 0) {
      fprintf(stderr,
	      "### Warning: Miriad mask item not integer valued.\n");
      if (inheader == 0) fclose(mask->fdmask);
      free((Void *)mask);
      return (MIRMASK *)NULL;
    }

    mask->size = (mask->size / H_INT_SIZE) * BITS_PER_INT;
    mask->offset = -BUFFERSIZE * BITS_PER_INT;
    mask->length = 0;

    /* RND3 or RND4 ??? -- but should it really matter ?? */
#ifdef RND
    nx = RND(nx,4);
#endif
    /* printf("DEBUG: malloc %d int's (possibly RND) for the mask\n",nx); */
    mask->flag = (int *)malloc(sizeof(int)*nx);

    if (mask->flag == (int *)NULL) {
      fprintf(stderr,
        "### Warning: Failed to allocate Miriad mask array memory.\n");
      if (inheader == 0) (void)fclose(mask->fdmask);
      free((Void *)mask);
      return (MIRMASK *)NULL;
    }

    return mask;
}

/**********************************************************************/
static int maskread(MIRIAD *f, int *flags, size_t length, long int offset)
/*
  This reads the mask information for a MIRIAD image item.
  Returns number of mask entries found; -1 otherwise.
------------------------------------------------------------------------*/
{
    int i, boff, blen, bitmask, len, buflen;
    int *idat, *flag0;
    long int itemsize;
    long int n;
    size_t isize;
    FILE *fd;
    MIRMASK *mask;

    /* printf("DEBUG: maskread(%x,%d,%d)\n",flags,length,offset); */
    mask = f->mask;
    n = length;
    flag0 = flags;
    fd = mask->fdmask;

#if NO_CVT
    buflen = BUFFERSIZE;
#else
    buflen = f->buflen/BITS_PER_INT;
#endif

    while (n > 0) {
      /* Get a buffer full of information, if it is needed. */
      boff = offset - mask->offset;
      if ((boff < 0) || (boff >= mask->length)) {
        mask->offset = (offset / BITS_PER_INT) * BITS_PER_INT;
        mask->length = mask->size - mask->offset;
        mask->length = MIN(mask->length, (buflen * BITS_PER_INT));
	if (mask->length <= 0) {
          fprintf(stderr, "Read past end of mask file.\n");
          return -1;
        }

        itemsize = (mask->offset / BITS_PER_INT) * H_INT_SIZE;
        itemsize += mask->hdoffset;
        if (fseek(fd, itemsize, SEEK_SET) != 0)
          return -1;

        itemsize = (mask->length / BITS_PER_INT) * H_INT_SIZE;

#if NO_CVT
        isize = (size_t)(itemsize / sizeof(int));
	/* printf("DEBUG: NO_CVT fread(int;isize=%d)\n",isize); */
	if (isize*sizeof(int) > BUFFERSIZE) {
	  fprintf(stderr,"### Fatal error: BUFFERSIZE overrun fread\n");
	  return -1;
	}

        if (fread((Void *)mask->buf, sizeof(int), isize, fd) != isize)
#else
	isize = (size_t)itemsize;
	/* next line has read_overflow in not RND */
	/* printf("DEBUG: no NO_CVT fread(char,isize=%d)\n",isize); */
        if (fread(f->buffer, sizeof(char), isize, fd) != isize)
#endif
	  {
	    fprintf(stderr,"### Fatal Error: I/O read error in maskread.\n");
	    return -1;
	  }

#if NO_CVT
#else
	isize /= sizeof(int);		/* PJT added 29-aug-99 */
#if 1
	if (isize > BUFFERSIZE+BUGSPILL) {
	  fprintf(stderr,"### Fatal error: %d BUFFERSIZE overrun unpack32_c\n",isize*sizeof(int));
	  return -1;
	}
#endif
        unpack32_c(f->buffer, mask->buf, isize);
#endif
      }

      boff = offset - mask->offset;
      i = boff / BITS_PER_INT;
      idat = mask->buf + i;
      len = MIN((mask->length - boff), n);
      boff -= (i * BITS_PER_INT);
      n -= len;
      offset += len;

      /* Copy the flags to the output array "flags". */

      while (len > 0) {
        blen = MIN((BITS_PER_INT - boff), len);
        bitmask = *idat++;
        if (bitmask == 0x7FFFFFFF) for (i = 0; i < blen; i++) *flags++ = 1;
        else if (bitmask == 0)     for (i = 0; i < blen; i++) *flags++ = 0;
        else                       for (i = boff; i < (boff + blen); i++)
                                  *flags++ = (bits[i] & bitmask) ? 1 : 0;
        len -= blen;
        boff = 0;
      }
    }

    n = (long int)length - (flags - flag0);
    if (n < 0)
      fprintf(stderr, "### Error: Buffer overflow in maskread!\n");

    return(flags - flag0);
}

#ifdef TEST

/*
 * The remainder of the file provides code to test the callable
 * routines in this file.  Compile the code with -DTEST and run
 * it with one argument: the name of a miriad data set.
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
      (void)printf("Test of Miriad image routines.\n");
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
    file = miropen(infile, naxis, axes);
    if (file == (Void *)NULL) {
      (void)printf("TEST: ###### Failed to open [%s].\n", infile);
      exit(0);
    }

    mirrdhdi(file, "naxis1", &indx, -1);
    (void)printf("TEST: naxis1 is %d.\n", indx);
    mirrdhdr(file, "datamax", &fmax, -1.0);
    (void)printf("TEST: datamax is %f.\n", fmax);
    mirrdhdd(file, "crval1", &crval1, -1.0);
    (void)printf("TEST: crval1 is %f.\n", crval1);
    mirrdhda(file, "ctype1", ctype1, "(none)", 80);
    (void)printf("TEST: ctype1 is %s. ", ctype1);
    (void)printf("TEST: strlen(ctype1) is %d.\n", strlen(ctype1));

    array = (FLOAT *)malloc(indx * sizeof(FLOAT));
    if (array == (FLOAT *)NULL) {
      (void)printf("TEST: ##### Failed to allocate %d FLOAT elements.\n", indx);
    } else {
      mirrdhdi(file, "naxis3", &indx, 1);
      if (indx > 2) indx = 2;
      (void)printf("TEST: Setting plane %d.\n", indx);
      if (mirsetpl(file, 1, &indx))
        (void)printf("TEST: ##### Failed to set plane %d.\n", indx);
      mirrdhdi(file, "naxis2", &indx, 2);
      indx /= 2;
      (void)printf("TEST: Reading row %d.\n", indx);
      if (mirread(file, indx, array, BADPIXEL))
        (void)printf("TEST: ##### Failed to read row %d.\n", indx);
      (void)free((Void *)array);
    }

    (void)printf("TEST: Closing file [%s].\n", infile);
    mirclose(file);
    (void)printf("TEST: Finished.\n");
}
#endif /* TEST */

#if 0
/* faking this will make the mask bug go away */
void free(void *ptr)
{
  printf("DEBUG: free=0x%x\n",ptr);
  return;
}
#endif
