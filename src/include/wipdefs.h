/*
 *    "wipdefs.h"
 *
 *  History:
 *    10nov93 jm  Moved standard definitions here.
 *    22aug94 jm  Added stdarg/varargs includes.
 *    13nov96 jm  Changed debug mode to a routine rather than a global variable.
 *    30nov00 pjt Only ansi-c now
 *    25feb07 pjt fix for intel mac
 *
 */

#ifndef WIP_DEFS
#define WIP_DEFS

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef convex
#include <stdlib.h>
#endif /* !convex */

#ifdef linux
/*			this is bad, would fail on power-pc etc. need different check */
#define IEEEByteSwap
#endif /* linux */

#ifdef darwin
/* on Intel, define IEEEByteSwap, on PowerPC leave alone */
#ifdef i386
#define IEEEByteSwap
#endif
#endif

#ifdef OSF
/*			probably bad idea too */
#define IEEEByteSwap
#endif /* OSF */

#ifdef WIPVMS
#define IEEEByteSwap
#endif /* WIPVMS */

/* Define a default character string size (if <stdio.h> doesn't). */
#ifndef BUFSIZ
#define BUFSIZ 2048
#endif

/* Define a Null character if it is missing. */
#ifndef Null
#define Null '\0'
#endif

/* only Ansi C is now supported */

#define PROTOTYPE 1
typedef void Void;
#define Const const
#include <stdarg.h>
#define ARGS(alist) alist


/*
 *  The next two definitions are used by the fseek() and ftell() routines;
 *  these definitions should be set by <stdio.h>.
 */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#endif /* WIP_DEFS */
