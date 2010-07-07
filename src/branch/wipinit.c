/*
	<wipinit.c>
	19jul90 jm  Original code.
	07jun91 jm  Added macro command input.
	02nov91 jm  Modified to initialize global variables.
	01aug92 jm  Moved Firstime into wipinit().  Also added notice
		    to user warning of "string" usage modification.
	01oct92 jm  Removed wipvarinit call... it no longer exists.
	21jul93 jm  Modified for syntax change of wipinput().
	06aug93 jm  Added new and free to list of items in the init-file.
	21jan98 jm  Modified definition of WIPINIT for VMS machines.

Routines:
int wipinit ARGS(( void ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

COMMAND *HEAD   = (COMMAND *)NULL;
COMMAND *BUFFER = (COMMAND *)NULL;
COMMAND *ENDIF  = (COMMAND *)NULL;

#define WIP_COMMANDS
#include "commands.h"   /* COMMAND definitions */

#ifdef WIPVMS
#define WIPINIT "%swipinit.com"    /* Name of the initialization file. */
#else
#define WIPINIT "%s/.wipinit"      /* Name of the initialization file. */
#endif /* WIPVMS */

/* Code */

/* Returns 0 if this has already been done or okay; 1 if error. */
#ifdef PROTOTYPE
int wipinit(void)
#else
int wipinit()
#endif /* PROTOTYPE */
{
    char *ep, *ptr;
    char buf[BUFSIZ];
    register int j;
    static short int Firstime = 1;
    FILE *fp;

    if (!Firstime) return(0);              /*  Only do this file once! */
    Firstime = 0;

/*  Initialize the COMMAND structure with contents of command.h. */

    HEAD = (COMMAND *)NULL;   /* HEAD is a pointer to the 1st COMMAND. */
    for (j = 0; Strcmp(vocab[j].name, "buffer") != 0; j++)
      add_command(&vocab[j]);

/*  Handle PCMACRO = "buffer". */

    if ((BUFFER = wipstartmac("buffer")) == (COMMAND *)NULL) {
      wipoutput(stderr, "EXITING because of memory allocation failure!\n");
      wipoutput(stderr, "I cannot continue without the command BUFFER!\n");
      return(1);
    }
    BUFFER->insert = FALSE;
    BUFFER->delete = FALSE;

/*  Handle special COMMAND = "ENDIF". */

    if ((ENDIF = create_command()) == (COMMAND *)NULL) {
      wipoutput(stderr, "EXITING because of memory allocation failure!\n");
      wipoutput(stderr, "I cannot continue without the command ENDIF!\n");
      return(1);
    }
    ENDIF->name = "ENDIF";
    ENDIF->ncom = 0;
    ENDIF->insert = FALSE;
    ENDIF->macro = FALSE;
    ENDIF->delete = FALSE;
    ENDIF->pcmac = (PCMACRO *)NULL;
    ENDIF->help = (PCMACRO *)NULL;

/*  Check for presence of the help file. */

    if ((ep = GetEnv("WIPHELP")) != (char *)NULL)
      (void)wipsetstring("helpfile", ep);     /* Ignore status return. */

/*  Check for presence of the user's initialization file. */

    if ((ep = GetEnv("HOME")) != (char *)NULL) {
      SPrintf(buf, WIPINIT, ep);
      if ((fp = Fopen(buf, "r")) == (FILE *)NULL) {
        wipoutput(stderr, "Could not open user's WIP initialization file %s\n",buf);
      } else {
        while ((j = wipinput(fp, (char *)NULL, buf, BUFSIZ)) != EOF) {
          if (j == Null) continue;                /* Skip blank lines. */
          if ((ptr = wipleading(buf)) == (char *)NULL)/* and comments. */
            continue;
          if ((ep = wipparse(&ptr)) == (char *)NULL)/* Skip empty lines. */
            continue;
          wiplower(ep);           /* Make the command name lower case. */
          if (Strcmp(ep, "set") == 0) {
            (void)wipsetuser(ptr);             /* Ignore error return. */
          } else if (Strcmp(ep, "macro") == 0) {
            if ((ep = wipparse(&ptr)) != (char *)NULL)
              (void)wipmacroinput(ep);         /* Ignore error return. */
          } else if (Strcmp(ep, "new") == 0) {
            (void)wipnewitem(ptr);             /* Ignore error return. */
          } else if (Strcmp(ep, "free") == 0) {
            (void)wipfreeitem(ptr);            /* Ignore error return. */
          } else if (Strcmp(ep, "string") == 0) { /* FIX FOR OLDER VERSIONS. */
            /***********************************************************/
            wipoutput(stderr,
              "NOTICE: The 'string' command has changed syntax and usage.\n");
            wipoutput(stderr,
              "NOTICE: Change all references of 'string x y' in your WIP\n");
            wipoutput(stderr,
              "\tinitialization file to 'set \"x\" y' commands.\n");
            wipoutput(stderr,
              "NOTICE: Also refer to 'help string' for further information.\n");
            /***********************************************************/
            (void)wipsetuser(ptr);             /* Ignore error return. */
          }
        }
        Fclose(fp);
      }
    }

    return(0);
}
