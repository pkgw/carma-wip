/*
 *    "wipdefs.h"
 *
 *  History:
 *    10nov93 jm  Moved standard definitions here.
 *    22aug94 jm  Added stdarg/varargs includes.
 *    13nov96 jm  Changed debug mode to a routine rather than a global variable.
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
#define IEEEByteSwap
#endif /* linux */

#ifdef OSF
#define IEEEByteSwap
#endif /* OSF */

#ifdef WIPVMS
#define IEEEByteSwap
#endif /* WIPVMS */

/* Define a default character string size (if <stdio.h> doesn't). */
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

/* Define a Null character if it is missing. */
#ifndef Null
#define Null '\0'
#endif

#ifdef __STDC__
#if (__STDC__ == 1)
#define PROTOTYPE 1
typedef void Void;
#define Const const
#include <stdarg.h>
#else
typedef char Void;
#define Const /* NULL */
#include <varargs.h>
#endif /* (__STDC__ == 1) */
#else
typedef char Void;
#define Const /* NULL */
#include <varargs.h>
#endif /* __STDC__ */

/* Define a macro that aides in presenting prototypes. */
#ifndef ARGS
#ifdef PROTOTYPE
#define ARGS(alist) alist
#else
#define ARGS(alist) ()
#endif /* PROTOTYPE */
#endif /* ARGS */

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
