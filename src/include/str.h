/*
	<str.h>
	04oct91 jm  Original code.
	01nov91 jm  Added FILES section.
        01aug92 jm  Merged multiple include files into one.
        09aug93 jm  Made initial array static.
*/

#ifdef WIP_STRING
#undef WIP_STRING

typedef struct wip_string {
  char *name;                             /* Name of string variable. */
  char value[STRINGSIZE];                  /* Translation for "name". */
  LOGICAL delete;		 /* TRUE means string can be deleted. */
  struct wip_string *next;
} WSTRINGS;

/* Initialize the WSTRINGS structure. */
/* HELPFILE is Defined externally (at compile time). */

static WSTRINGS initialStrArray[] = {
/*      "name",       value,  delete?,  next */
 {    "boxdef",    "BCNTSV",    FALSE,  NULL},
 {  "helpfile",    HELPFILE,    FALSE,  NULL},
 {   "levtype",         "a",    FALSE,  NULL},
 {   "yheader",        "rd",    FALSE,  NULL},
 {   "xheader",        "rd",    FALSE,  NULL},
 { "imagefile",          "",    FALSE,  NULL},
 {  "datafile",          "",    FALSE,  NULL},
 {    "device",  "/XWINDOW",    FALSE,  NULL},
 {     "print",  "lpr %s &",    FALSE,  NULL},
 {      "ybox",    "BCNTSV",    FALSE,  NULL},
 {      "xbox",     "BCNTS",    FALSE,  NULL},
};

#endif /* WIP_STRING */
