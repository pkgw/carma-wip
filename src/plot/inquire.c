/*
	<inquire.c>
	29apr91 jm  Original code.
	03aug92 jm  Modified wipinquire() by removing LOGICAL error
		    flag.  Now, a return of NULL means an error.

Routines:
char *wipinquire ARGS(( Const char *item ));
int wipishard ARGS(( void ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/* Returns a pointer to the item inquired for if found; NULL otherwise. */
#ifdef PROTOTYPE
char *wipinquire(Const char *item)
#else
char *wipinquire(item)
Const char *item;
#endif /* PROTOTYPE */
{
      char *s;
      char string[STRINGSIZE];
      static char output[STRINGSIZE];
      int length;

      (void)Strcpy(output, item);
      if ((s = wipleading(output)) == (char *)NULL) {
        wipoutput(stderr, "Inquire: No inquire item provided.\n");
        return((char *)NULL);
      }
      wiplower(s);

      if ((Strcmp(s, "file") == 0) || (Strcmp(s, "device") == 0)) {
        (void)Strcpy(string, "FILE");
      } else if (Strcmp(s, "type") == 0) {
        (void)Strcpy(string, "TYPE");
      } else if (Strcmp(s, "dev/type") == 0) {
        (void)Strcpy(string, "DEV/TYPE");
      } else if (Strcmp(s, "version") == 0) {
        (void)Strcpy(string, "VERSION");
      } else if (Strcmp(s, "state") == 0) {
        (void)Strcpy(string, "STATE");
      } else if (Strcmp(s, "user") == 0) {
        (void)Strcpy(string, "USER");
      } else if (Strcmp(s, "now") == 0) {
        (void)Strcpy(string, "NOW");
      } else if (Strcmp(s, "hardcopy") == 0) {
        (void)Strcpy(string, "HARDCOPY");
      } else if (Strcmp(s, "terminal") == 0) {
        (void)Strcpy(string, "TERMINAL");
      } else if (Strcmp(s, "cursor") == 0) {
        (void)Strcpy(string, "CURSOR");
      } else {
        wipoutput(stderr, "Inquire: Unknown item: [%s].\n", s);
        return((char *)NULL);
      }

      length = STRINGSIZE;       /* Allow this much room for response. */
      cpgqinf(string, output, &length);
      return(output);
}

/*
 *  Returns 1 if the current device is a hardcopy unit; 0 if not or
 *  if an error occured trying to inquire.
 */
#ifdef PROTOTYPE
int wipishard(void)
#else
int wipishard()
#endif /* PROTOTYPE */
{
      char *string;

      string = wipinquire("hardcopy");
      return( (string != (char *)NULL) && (*string == 'Y') );
}
