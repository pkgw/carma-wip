/*
	<inoutput.c>
	18jul90 jm  Original code.
	16aug92 jm  Added wipexit() routine.
	21jul93 jm  Modified wipinput to use fgets rather than getc.
		    Also added #ifdef for READLINE use.
	25jul94 jm  Modified wipoutput* to flush if last character is a \n.
	22aug94 jm  Combined all individual wipoutput* calls into one
		    varargs form.  Once that method is tested, I will remove
		    the individual calls....  Also added ability to turn
		    off READLINE history archiving with a command line option.
	12dec94 jm  Removed individual flavors of wipoutout*.  Also
		    added another option so the user can disable READLINE
		    via a command line option.
	21jan98 jm  VMS reserves the readonly keyword, so it was changed
		    to justread in wipbegin().  Also modified definition
		    of historyFile variable for VMS machines (is this
		    even available for VMS machines?).

Routines:
void wipsetQuiet ARGS(( int quiet ));
void wipbegin ARGS(( int disable, int justread ));
void wipexit ARGS(( int status ));
void wipoutput ARGS(( FILE *fp, Const char *fmt, ... ));
int  wipinput ARGS((FILE *file, Const char *prompt, char *line, size_t maxlen));
*/

#include "wip.h"

/* Global variables for just this file */

static int quietMode = 0;

#ifdef READLINE

static LOGICAL WriteHistory;
static LOGICAL UseReadLine;
#ifdef WIPVMS
static char *historyFile = "wiphistory.log";
#else
static char *historyFile = "./.wiphistory";
#endif /* WIPVMS */

extern int read_history ARGS(( char *filename ));
extern int write_history ARGS(( char *filename ));
extern char *readline ARGS(( char *prompt ));
extern void add_history ARGS(( char *string ));

#endif /* READLINE */

/* Code */

#ifdef PROTOTYPE
void wipsetQuiet(int quiet)
#else
void wipsetQuiet(quiet)
int quiet;
#endif /* PROTOTYPE */
{
    quietMode = quiet;
    return;
}

#ifdef PROTOTYPE
void wipbegin(int disable, int justread)
#else
void wipbegin(disable, justread)
int disable;
int justread;
#endif /* PROTOTYPE */
{
#ifdef READLINE
    int err;

    UseReadLine = (disable == 0) ? TRUE : FALSE;
    WriteHistory = (justread == 0) ? TRUE : FALSE;

    if (UseReadLine == TRUE) {
      wipoutput(stdout, "GNU READLINE installed.\n");
      if (WriteHistory == TRUE)
        wipoutput(stdout, "The command history file is maintained in [%s].\n",
          historyFile);

      if (((err = read_history(historyFile)) != 0) && (err != 2)) {
        wipoutput(stderr, "Error [%d] reading READLINE history file.\n", err);
      }
    }

#endif /* READLINE */

    return;
}

#ifdef PROTOTYPE
void wipexit(int status)
#else
void wipexit(status)
int status;
#endif /* PROTOTYPE */
{
#ifdef READLINE
    int err;

    if ((UseReadLine == TRUE) && (WriteHistory == TRUE)) {
      if ((err = write_history(historyFile)) != 0) {
        wipoutput(stderr, "Error [%d] writing READLINE history file.\n", err);
      }
    }

#endif /* READLINE */

    exit(status);
}

#ifndef NOVARARGS
/*VARARGS2*/
#ifdef PROTOTYPE
void wipoutput(FILE *fp, Const char *fmt, ...)
#else
void wipoutput(fp, fmt, va_alist)
FILE *fp;
Const char *fmt;
va_dcl /* Put no semicolon here! */
#endif /* PROTOTYPE */
{
    va_list ap;

    if ((fp == stdout) && (quietMode > 0)) {    /* No messages wanted. */
      return;
    }

#ifdef PROTOTYPE
    va_start(ap, fmt);
#else
    va_start(ap);
#endif /* PROTOTYPE */

    VFPrintf(fp, fmt, ap);
    if (strchr(fmt, '\n') != (char *)NULL) {
#ifdef WIPVMS
      /*
       * This next statement is added to keep error messages
       * from being overwritten by the next prompt.
       */
      if (fp == stderr) {
        FPrintf(fp, "\n");
      }
#endif /* WIPVMS */
      Fflush(fp);
    }


    va_end(ap);

    return;
}
#endif /* !NOVARARGS */

/*
 *  If "prompt" is not an empty string, it is output prior to reading.
 *  The string "line" is filled with the input line (NULL terminated).
 *  Returns EOF (see <stdio.h>) at the end of file, NULL for an empty
 *  input line, or the number of characters read.
 */
#ifdef PROTOTYPE
int wipinput(FILE *file, Const char *prompt, char *line, size_t maxchar)
#else
int wipinput(file, prompt, line, maxchar)
FILE *file;
Const char *prompt;
char *line;
size_t maxchar;
#endif /* PROTOTYPE */
{
    size_t nch;
    LOGICAL promptExists;

    line[0] = Null;           /* Initialize output to an empty string. */

    if (maxchar <= 1) {               /* This will always be an error! */
      wipoutput(stderr, "There is an internal error in the wipinput() call.\n");
      return((int)Null);
    }

    promptExists = ((char *)prompt != (char *)NULL) ? TRUE : FALSE ;

#ifdef READLINE

                            /* Prompts should only exist for stdin...? */
    if ((UseReadLine == TRUE) && (promptExists == TRUE)) {
      char *ptr;

      if ((ptr = readline((char *)prompt)) == (char *)NULL) {
        return((int)EOF);      /* readline() returns NULL only on EOF. */
      }
      (void)Strncpy(line, ptr, maxchar-1);
      line[maxchar-1] = Null;
      Free(ptr);
    } else

#endif /* READLINE */

    {       /* Braces needed for use with READLINE; no harm otherwise. */
      if (promptExists == TRUE)
        wipoutput(stdout, prompt);

      if (Fgets(line, (int)maxchar, file) == (char *)NULL) {
        if (Feof(file) != 0) return((int)EOF);
        if (Ferror(file) != 0) return((int)Null);
      }
    }

    /* If there is a '\n' at the end of the string, then overwrite it. */
    if (((nch = Strlen(line)) > 0) && (line[nch - 1] == '\n')) {
      line[--nch] = Null;
    }

#ifdef READLINE

    if ((UseReadLine == TRUE) && (promptExists == TRUE)) {
      if (nch > 0)   /* Only add to the history if something is there. */
        add_history(line);
    }

#endif /* READLINE */

    return((int)nch);
}
