/*
	"wip.h"
	11apr90 jm  Original code.
	21jul91 jm  Made STRINGSIZE parameter global.
	02nov91 jm  Removed arrays, plot, and files for stack includes.
	01aug92 jm  Split up stack include into variables, vectors, and str
		    include files.  These files are only included by the
		    particular file that needs them (see ../variables).
        10nov93 jm  Added new opaque pointer type Void.  Also added a
		    definition for a Null character.  Moved most system
		    includes and standard definitions into wipdefs.h.
		    Moved some character macros into this file.
        09jan98 jm  Removed any previous definition of TRUE/FALSE.
*/

#ifndef WIP_H
#define WIP_H

#include "wipdefs.h"

/* Define a default string size and contour level array size. */
#define STRINGSIZE  256
#define MAXCONTLEVEL 40

/* Prevent a compiler from predefining these values. */
#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

/* Define what a LOGICAL is and what values it can take on. */
typedef enum {FALSE, TRUE} LOGICAL;

/* Define a few macros used by several routines. */
#define ESC         (92)    /* char(92) = "\" */
#define WHITE(x)    (isspace((x)) || ((x) == ','))
#define BRACE(x)    (((x) == '(') || ((x) == '[') || ((x) == '{'))

#include "vocab.h"
#include "wipmath.h"
#include "image.h"
#include "sysdep.h"
#include "declare.h"

#include "cpgplot.h"

#endif /* WIP_H */
