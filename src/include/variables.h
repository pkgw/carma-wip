/*
	<variable.h>
	03oct91 jm  Original code.
	22feb92 jm  Added hardcopy VARIABLE.
	01aug92 jm  Merged multiple include files into one.
	16sep92 jm  (Re)Moved "npts" item to a vector function.
	09aug93 jm  Made initial array static.
	12dec94 jm  Added cursmode, bgci, and itf.
	29mar95 jm  Added maxarray.
	13nov96 jm  Added debugmode.
*/

#ifdef WIP_VARIABLES
#undef WIP_VARIABLES

typedef struct wip_variable {
    char *name;			/* Name of command (case dependent!!). */
    double value;		/* Value of this variable. */
    LOGICAL delete;		/* TRUE means variable can be deleted. */
    struct wip_variable *next;	/* pointer to next variable in list. */
} VARIABLE;

/* Initialize the VARIABLE structure. */

static VARIABLE initialVarArray[] = {
/*     "name",   value, delete?,   next */
 {       "pi",      PI,   FALSE,   NULL},
 { "maxarray", 20000.0,   FALSE,   NULL},
 {"debugmode",     0.0,   FALSE,   NULL},
 { "hardcopy",     1.0,   FALSE,   NULL},
 { "cursmode",     0.0,   FALSE,   NULL},
 {   "slevel",     1.0,   FALSE,   NULL},
 {   "cdelty",     0.0,   FALSE,   NULL},
 {   "crpixy",     0.0,   FALSE,   NULL},
 {   "crvaly",     0.0,   FALSE,   NULL},
 {   "cdeltx",     0.0,   FALSE,   NULL},
 {   "crpixx",     0.0,   FALSE,   NULL},
 {   "crvalx",     0.0,   FALSE,   NULL},
 {  "ysubmar",     2.0,   FALSE,   NULL},
 {  "xsubmar",     2.0,   FALSE,   NULL},
 {    "nysub",     0.0,   FALSE,   NULL},
 {    "nxsub",     0.0,   FALSE,   NULL},
 {    "ytick",     0.0,   FALSE,   NULL},
 {    "xtick",     0.0,   FALSE,   NULL},
 {    "suby2",     0.0,   FALSE,   NULL},
 {    "suby1",     0.0,   FALSE,   NULL},
 {    "subx2",     0.0,   FALSE,   NULL},
 {    "subx1",     0.0,   FALSE,   NULL},
 {     "cmax",     1.0,   FALSE,   NULL},
 {     "cmin",     0.0,   FALSE,   NULL},
 {      "itf",     0.0,   FALSE,   NULL},
 {     "bgci",    -1.0,   FALSE,   NULL},
 {   "lwidth",     1.0,   FALSE,   NULL},
 {   "lstyle",     1.0,   FALSE,   NULL},
 {     "font",     1.0,   FALSE,   NULL},
 {     "fill",     1.0,   FALSE,   NULL},
 {    "color",     1.0,   FALSE,   NULL},
 {    "angle",     0.0,   FALSE,   NULL},
 {   "expand",     1.0,   FALSE,   NULL},
 {     "nsig",     3.0,   FALSE,   NULL},
 {      "vy2",     1.0,   FALSE,   NULL},
 {      "vy1",     0.0,   FALSE,   NULL},
 {      "vx2",     1.0,   FALSE,   NULL},
 {      "vx1",     0.0,   FALSE,   NULL},
 {       "y2",     1.0,   FALSE,   NULL},
 {       "y1",     0.0,   FALSE,   NULL},
 {       "x2",     1.0,   FALSE,   NULL},
 {       "x1",     0.0,   FALSE,   NULL},
 {       "cy",     0.0,   FALSE,   NULL},
 {       "cx",     0.0,   FALSE,   NULL},
};

#endif /* WIP_VARIABLES */
