/*
	<loopxecute.c>
	Original 04aug90 jm

Routines:
COMMAND *wiploopxecute ARGS(( char **line, int *ncount ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global Variables needed just for this file */

/* Code */

#ifdef PROTOTYPE
COMMAND *wiploopxecute(char **line, int *ncount)
#else
COMMAND *wiploopxecute(line, ncount)
char **line;
int *ncount;
#endif /* PROTOTYPE */
{
      char *par;
      double arg;
      COMMAND *vb;

      if ((par = wipparse(line)) == (char *)NULL) return((COMMAND *)NULL);
      if (wiparguments(&par, 1, &arg) != 1) return((COMMAND *)NULL);
      *ncount = NINT(arg);

      if ((vb = wipinterpret(line)) == (COMMAND *)NULL) return((COMMAND *)NULL);
      if (vb->macro != TRUE) {
        wipoutput(stderr, "LOOP MUST call a macro!\n");
        return((COMMAND *)NULL);
      }

      return(vb);
}
