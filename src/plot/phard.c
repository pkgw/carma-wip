/*
	<phard.c>
	14may91 jm  Original code.
	02aug92 jm  Modified to return status (void -> int) rather than
		    using a passed LOGICAL pointer.
	26aug94 jm  Modified to set and reset hardcopy user variable.

Routines:
int wipphard ARGS(( Const char *device, Const char *rest ));
*/

#include "wip.h"

/* Global variables for just this file */
/* Parameter STRINGSIZE is defined in wip.h. */

/* Code */

/*
 *  Use as: PHARD PLAYBACK-DEVICE MACRO-NAME [MACRO-ARGS ...]
 *  Returns 0 on success; 1 on error.
 */
#ifdef PROTOTYPE
int wipphard(Const char *device, Const char *rest)
#else
int wipphard(device, rest)
Const char *device;
Const char *rest;
#endif /* PROTOTYPE */
{
    char *fileptr;
    char current[STRINGSIZE];
    int ishard;
    LOGICAL error;

    if ((fileptr = wipgetstring("device")) == (char *)NULL) {
      wipoutput(stderr, "Trouble retrieving the current device name.\n");
      return(1);
    }

    (void)Strcpy(current, fileptr);
    wipoutput(stdout, "Current device = [%s]\n", current);

    error = TRUE;    /* This gets reset if playback is done correctly. */
    if (wipdevice(device) == 0) {
      ishard = wipishard();   /* Returns 1 if hardcopy; 0 if terminal. */
      if (wipsetvar("hardcopy", (double)ishard))
        wipoutput(stderr, "Trouble setting hardcopy variable.\n");

      wipoutput(stdout, "Playback on device = [%s]\n", device);
      error = (wipplayback(rest) != 0) ? TRUE : FALSE;
      if ((error != TRUE) && (wipishard())) {
        if ((fileptr = wipinquire("file")) == (char *)NULL) {
          wipoutput(stderr, "Trouble getting the output file name.\n");
          error = TRUE;
        } else {
          wipoutput(stdout, "Spooling file: [%s]\n", fileptr);
          wipclose();
          error = (wipspool(fileptr) != 0) ? TRUE : FALSE;
        }
      }
    }

    if ((wipdevice(current) != 0) || (error == TRUE))
      return(1);

    ishard = wipishard();     /* Returns 1 if hardcopy; 0 if terminal. */
    if (wipsetvar("hardcopy", (double)ishard))
      wipoutput(stderr, "Trouble setting hardcopy variable.\n");

    wipoutput(stdout, "Current device = [%s]\n", current);
    return(0);
}
