/*
	"sysdep.h"
	Original 6feb91 jm

	System dependent code declarations.
	This include file contains just about every compiler dependent
	call possible.  It is assumed that the number of parameters
	will always be the same (hopefully true!).  Most are defined
	rather than extern'd so that system implimentations as macros
	will still hold rather than force a routine call each time.
*/

#ifndef WIP_SYSDEP
#define WIP_SYSDEP

#define Strlen       strlen
#define Strcpy       strcpy
#define Strncpy      strncpy
#define Strcat       (void)strcat
#define Strcmp       strcmp
#define Strncmp      strncmp
#define Strchr       strchr
#define Strstr       strstr

#define Free(x)      (void)free((Void *)(x))
#define Malloc       malloc

#define Fopen        fopen
#define Fclose       (void)fclose
#define Rewind       (void)rewind
#define Fseek        fseek
#define Ftell        ftell
#define Fflush       (void)fflush
#define Tmpnam       tmpnam
#define Remove       remove

#define Clearerr     clearerr
#define Feof         feof
#define Ferror       ferror

#define SScanf       (int)sscanf
#define SPrintf      (void)sprintf
#define FPrintf      (void)fprintf
#define VFPrintf     (void)vfprintf
#define Fgets        fgets

#ifdef convex
extern char *getenv ARGS(( Const char *name ));
#endif /* convex */

#define GetEnv       getenv

#ifdef NO_SYSTEM
#define System(comm) ((int)fprintf(stderr, \
   "I cannot pass the following string to the O/S.\nString = %s\n", comm))
#else
#define System(comm) ((int)(system(comm)))
#endif /* NO_SYSTEM */

#endif /* WIP_SYSDEP */
