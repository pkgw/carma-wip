/*
	<spool.c>
	29apr91 jm  Original code.
	26jul91 jm  Added wipcommand code.
        02aug92 jm  Modified both routines to return status (void -> int)
		    rather than using a passed LOGICAL pointer.
        21jan98 jm  Modified handling of spooling for VMS machines.

Routines:
int wipspool ARGS(( Const char *file ));
int wipcommand ARGS(( Const char *command ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*  Returns 0 on success; 1 on error. */
#ifdef PROTOTYPE
int wipspool(Const char *file)
#else
int wipspool(file)
Const char *file;
#endif /* PROTOTYPE */
{
    char *lpr, *ptr;
    char fmt[BUFSIZ];
    char outbuf[BUFSIZ];
    int okayState;

#ifdef WIPVMS
    okayState = 1;
#else
    okayState = 0;
#endif /* WIPVMS */

    if ((ptr = wipgetstring("print")) == (char *)NULL) {
      wipoutput(stderr, "HARDCOPY: Error finding printing command.\n");
      return(1);
    }

    if (wiplenc(ptr) < 1)  /* If string is empty, no printing desired. */
      return(0);

    if (Strncmp(ptr, "ignore", 6) == 0) /* Still, no printing desired. */
      return(0);

    lpr = Strcpy(outbuf, ptr);

    if ((ptr = Strchr(lpr, '&')) != (char *)NULL) {
      while(*ptr) {                      /* Remove all '&' characters. */
        if (*ptr == '&') *ptr = ' ';
        ptr++;
      }
    }

#ifdef WIPVMS
    if (Strstr(lpr, "%s") == (char *)NULL) {
      SPrintf(fmt, "%s %%s", lpr);
    } else {
      SPrintf(fmt, "%s", lpr);
    }
#else
    if (Strstr(lpr, "%s") == (char *)NULL) {
      SPrintf(fmt, "%s %%s &\n", lpr);
    } else {
      SPrintf(fmt, "%s &\n", lpr);
    }
#endif /* WIPVMS */

   /*
    *  "fmt" now contains the printer command along with a "%s" for
    *  the file name.
    */

    if ((ptr = wipleading(file)) == (char *)NULL) {
      wipoutput(stderr, "HARDCOPY: Can't get the name of the file to spool.\n");
      return(1);
    }

    SPrintf(outbuf, fmt, ptr);
    wipoutput(stdout, "HARDCOPY: Spooling command:\n %s", outbuf);
    if (System(outbuf) != okayState) {
      wipoutput(stderr, "HARDCOPY: Error spooling WIP printer file.\n");
      return(1);
    }

    return(0);
}

/*  Returns 0 on success; 1 on error. */
#ifdef PROTOTYPE
int wipcommand(Const char *command)
#else
int wipcommand(command)
Const char *command;
#endif /* PROTOTYPE */
{
      char outbuf[BUFSIZ];

      (void)Strcpy(outbuf, command);
      if (System(outbuf)) {
        wipoutput(stderr, "Error sending command:\n%s\n", command);
        return(1);
      }
      return(0);
}
