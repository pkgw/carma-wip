/*
    <process.c>
    21jul90 jm  Original C code.
    04may91 jm  Added return value for ENDIF.
    19nov91 jm  Modified for maxecute call change.
    16aug92 jm  Cleaned up code a bit.
    16sep92 jm  Modified to allow "if" to call "loop" command.
    21jul94 jm  Changed first argument name from a reserved name
                (thanks to Gary Fuller for pointing this out).

Routines:
int wipprocess ARGS(( char *cmdline, int *mode, LOGICAL keep ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */
/* Parameter STRINGSIZE defined in wip.h. */

/* Code */

/* WIPPROCESS returns 0 if no error; -1 if ENDIF found; 1 otherwise. */
#ifdef PROTOTYPE
int wipprocess(char *cmdline, int *mode, LOGICAL keep)
#else
int wipprocess(cmdline, mode, keep)
char *cmdline;
int *mode;
LOGICAL keep;
#endif /* PROTOTYPE */
{
    char *line;
    char full[STRINGSIZE];
    int loopcount;
    COMMAND *comm;
    static COMMAND *insmac;

    line = wipleading(cmdline);
    if ((comm = wipinterpret(&line)) == (COMMAND *)NULL) goto INPERR;
    (void)Strcpy(full, comm->name);
    if ((line != (char *)NULL) && (*line != Null)) {
      Strcat(full, " ");
      Strcat(full, line);
    }

    switch (*mode) {
      case 1:                                     /* Interactive mode. */
        loopcount = 1;
        if (Strcmp(comm->name, "loop") == 0) {
          if ((comm->insert == TRUE) && (keep == TRUE)) {
            wipsaveline(BUFFER, full);
            keep = FALSE;
          }
          comm = wiploopxecute(&line, &loopcount);
          if (comm == (COMMAND *)NULL) goto INPERR;
        } else if (Strcmp(comm->name, "if") == 0) {
          if ((comm->insert == TRUE) && (keep == TRUE)) {
            wipsaveline(BUFFER, full);
            keep = FALSE;
          }
          comm = wipifxecute(&line);
          if (comm == (COMMAND *)NULL) goto INPERR;
          if (comm == (COMMAND *)ENDIF)   /* Special return for ENDIF. */
            return(-1);
          if (Strcmp(comm->name, "loop") == 0) {
            if ((comm = wiploopxecute(&line, &loopcount)) == (COMMAND *)NULL)
              goto INPERR;
          }
        }

        if (Strcmp(comm->name, "end") == 0) {
          *mode = 0;
          break;
        }

        if (Strcmp(comm->name, "define") == 0) {
          if ((insmac = wipstartmac(line)) == (COMMAND *)NULL) goto INPERR;
          *mode = 2;
          break;
        }

        if (Strcmp(comm->name, "insert") == 0) {
          if ((insmac = wipstartins(line)) == (COMMAND *)NULL) goto INPERR;
          *mode = 3;
          break;
        }

        if (Strcmp(comm->name, "putlabel") == 0) {
          line = fixputlabel(comm->name, line, keep);
          (void)Strcpy(full, comm->name);
          if ((line != (char *)NULL) && (*line != Null)) {
            Strcat(full, " ");
            Strcat(full, line);
          }
        }

        if (Strcmp(comm->name, "cursor") == 0) {
          wipfixcurs(comm->name, line, keep);
        }

        if (comm->macro == TRUE) {
          if (wipmaxecute(comm, loopcount, line)) goto INPERR;
        } else {
          if (wipexecute(comm->name, line)) goto INPERR;
        }

        if ((comm->insert == TRUE) && (keep == TRUE)) {
          wipsaveline(BUFFER, full);
        }
        break;
      case 2:                                    /* Macro define mode. */
        if (Strcmp(comm->name, "end") == 0) {
          *mode = 1;
          break;
        }

        if (Strcmp(comm->name, "define") == 0) {
          wipoutput(stderr, "Macros cannot be nested.\n");
          goto INPERR;
        }

        if (Strcmp(comm->name, "insert") == 0) {
          wipoutput(stderr, "Macros cannot insert.\n");
          goto INPERR;
        }

        wipsaveline(insmac, full);
        break;
      case 3:                                          /* Insert mode. */
        if (Strcmp(comm->name, "end") == 0) {
          *mode = 1;
          break;
        }

        if (Strcmp(comm->name, "define") == 0) {
          wipoutput(stderr, "Insert cannot define.\n");
          goto INPERR;
        }

        if (Strcmp(comm->name, "insert") == 0) {
          wipoutput(stderr, "Insert cannot be nested.\n");
          goto INPERR;
        }

        wipinsert(insmac, full);
        break;
      default:                                        /* Unknown mode. */
        wipoutput(stderr, "Unknown process mode [%d].\n", *mode);
        goto INPERR;
    }
                                    /* Return for no error conditions. */
    return(0);

INPERR:                                /* Return for error conditions. */
    return(1);
}
