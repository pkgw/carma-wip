/*
	<insert.c>
	24jul90 jm  Original code.
	02aug92 jm  Modified wipdelete() to return status (void -> int)
		    rather than using a passed LOGICAL pointer.  Also
		    fixed a few bugs and cleaned up a bit.
        09oct00 pjt no more PROTOTYPE #ifdef


Routines:
void wipsaveline ARGS(( COMMAND *macro, Const char *line ));
COMMAND *wipstartins ARGS(( char *string ));
void wipinsert ARGS(( COMMAND *command, Const char *string ));
int wipdelete ARGS(( char *string ));
COMMAND *wipstartmac ARGS(( Const char *string ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

static int lineno = 0;

/* Code */

void wipsaveline(COMMAND *macro, Const char *line)
{
    if (macro == (COMMAND *)NULL) {
      wipoutput(stderr, "wipsaveline: Macro does not exist.\n");
      return;
    }

    add_to_macro(macro, line);
}
/*
 *  Returns pointer to NULL on error or pointer to command structure
 *  if found and command is a macro.
 */
COMMAND *wipstartins(char *string)
{
    char *par;
    double arg = -1;
    COMMAND *vb;

    vb = BUFFER;
    if ((par = wipparse(&string)) != (char *)NULL) {
      if (wiparguments(&par, 1, &arg) == 1) {
        if ((par = wipparse(&string)) != (char *)NULL) {
          if ((vb = find_command(BUFFER, par, 0)) == (COMMAND *)EOF) {
            wipoutput(stderr, "Macro %s not found in the command list.\n", par);
            return((COMMAND *)NULL);
          }                                     /* endif find command. */
        }                                   /* endif parse macro name. */
      }                                        /* endif get arg value. */
    }                               /* endif parse line number string. */

    if (vb == (COMMAND *)NULL) return((COMMAND *)NULL);

    if (vb->macro != TRUE) {
      wipoutput(stderr,
        "Command %s is not a macro; cannot insert into a command.\n", vb->name);
      return((COMMAND *)NULL);
    }

    lineno = (arg == -1) ? vb->ncom : (NINT(arg) - 1);
    return(vb);
}

void wipinsert(COMMAND *command, Const char *string)
{
    if (command == (COMMAND *)NULL) {
      wipoutput(stderr, "Select a Macro first.\n");
      return;
    }

    insert_macro(command, string, lineno);
    lineno++;
}

/*  Returns 0 if successful; 1 on error. */
int wipdelete(char *string)
{
    int line1, line2;
    char *par;
    double arg;
    COMMAND *vb;

    vb = BUFFER;
    if ((par = wipparse(&string)) == (char *)NULL) {
      line1 = -1;
      line2 = -1;
    } else {
      if (wiparguments(&par, 1, &arg) != 1) return(1);
      line1 = NINT(arg);
      if ((par = wipparse(&string)) == (char *)NULL) {
        line2 = line1;
      } else {
        if (wiparguments(&par, 1, &arg) != 1) return(1);
        line2 = NINT(arg);
        if ((par = wipparse(&string)) != (char *)NULL) {
          if ((vb = find_command(BUFFER, par, 0)) == (COMMAND *)EOF) {
            wipoutput(stderr, "Macro %s not found in the command list.\n", par);
            return(1);
          }
        }
      }
    }

    if (vb == (COMMAND *)NULL) return(1);

    if (vb->macro != TRUE) {
      wipoutput(stderr, "Command %s is not a macro; nothing to delete.\n",
        vb->name);
      return(1);
    }

    line1 = (line1 == -1) ? vb->ncom : line1 ;
    line2 = (line2 == -1) ? vb->ncom : line2 ;
    line1 = MIN(MAX(line1, 1), vb->ncom);
    line2 = MIN(MAX(line2, 1), vb->ncom);

    if ((vb != BUFFER) && (line1 == 1) && (line2 == vb->ncom)) {
      delete_command(vb);
    } else {
      line1--;
      line2--;
      delete_macro(vb, line1, line2);
      if ((vb != BUFFER) && (vb->ncom <= 0)) {
        delete_command(vb);
      }
    }
    return(0);
}

COMMAND *wipstartmac(Const char *string)
{
    char *par, *ptr;
    char macro[BUFSIZ];

    ptr = Strcpy(macro, string);
    if ((par = wipparse(&ptr)) == (char *)NULL) {
      wipoutput(stderr, "A macro name is required.\n");
      return((COMMAND *)NULL);
    }

    return(create_macro(par));
}
