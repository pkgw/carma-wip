/*
	<vectors.h>
	04oct91 jm  Original code.
	01aug92 jm  Merged multiple include files into one.
	09aug93 jm  Made initial array static.
	29mar95 jm  Maxsize=0 now means set to user variable maxarray.
*/

#ifdef WIP_VECTORS
#undef WIP_VECTORS

typedef struct wip_vector {
    char *name;			/* Name of command (case dependent!!). */
    int size;			/* Declared maximum size of this array. */
    int npts;			/* Number of elements in this array. */
    float *value;		/* Array of values. */
    LOGICAL resize;		/* TRUE means array size can be adjusted. */
    LOGICAL delete;		/* TRUE means array can be deleted. */
    struct wip_vector *next;	/* pointer to next variable in list. */
} VECTOR;

/* Initialize the VECTOR structure. */

/*
 *  Items set to maxsize=0 will be set to the value of the user variable
 *  "maxarray" when the array is initialized.
 */
static VECTOR initialVecArray[] = {
/*     "name",      maxsize,  cursize value, resize?, delete?,   next */
 {   "levels", MAXCONTLEVEL,     0,    NULL,   FALSE,   FALSE,   NULL},
 { "transfer",            6,     0,    NULL,   FALSE,   FALSE,   NULL},
 {   "pstyle",            0,     0,    NULL,   FALSE,   FALSE,   NULL},
 {      "err",            0,     0,    NULL,   FALSE,   FALSE,   NULL},
 {        "y",            0,     0,    NULL,   FALSE,   FALSE,   NULL},
 {        "x",            0,     0,    NULL,   FALSE,   FALSE,   NULL},
};

#endif /* WIP_VECTORS */
