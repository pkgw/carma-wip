/*
	<macros.c> -- This file handles the PCMACRO structure.
	23jul90 jm  Original code.
	31jul92 jm  Modified to use wipnewstring() command.
	02aug92 jm  Checked cast of returns and cleaned up code.
	06aug93 jm  Fixed omission in create_macro() which didn't force
		    new macro names to be saved in lower case.
         9oct00 pjt no more PROTOTYPEs


Routines:
COMMAND *create_macro ARGS(( Const char *name ));
void add_to_macro ARGS(( COMMAND *vb, Const char *macline ));
void insert_macro ARGS(( COMMAND *vb, Const char *macline, int before ));
void delete_macro ARGS(( COMMAND *vb, int first, int last ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

COMMAND *create_macro(Const char *name)
{
    char *nameptr;
    COMMAND *com;

    if ((nameptr = wipnewstring(name)) == (char *)NULL) {
      wipoutput(stderr, "A macro name is required.\n");
      return((COMMAND *)NULL);
    }
    wiplower(nameptr);              /* Make the macro name lower case. */

    if ((com = find_command(HEAD, nameptr, 1)) != (COMMAND *)EOF) {
      if ((com != (COMMAND *)NULL) && (com->macro == TRUE) &&
          (com->delete == TRUE)) {
        wipoutput(stdout,
          "Macro [%s] definition is going to overwrite previous definition.\n",
          com->name);
        delete_command(com);
      }

      if ((com = find_command(HEAD, nameptr, 1)) != (COMMAND *)EOF) {
        wipoutput(stderr, "Macro name [%s] is not unique.\n", nameptr);
        Free(nameptr);
        return((COMMAND *)NULL);
      }
    }

    /* Try to get room for storing a new COMMAND and the command name. */

    if ((com = create_command()) == (COMMAND *)NULL) {
      wipoutput(stderr, "Failed to allocate memory for the command.\n");
      Free(nameptr);
      return((COMMAND *)NULL);
    }

    /* Initialize some of the COMMAND structure values. */

    com->name = nameptr;
    com->ncom = 0;
    com->insert = TRUE;
    com->macro = TRUE;
    com->delete = TRUE;
    com->pcmac = (PCMACRO *)NULL;
    com->help = (PCMACRO *)NULL;
    add_command(com);
    return(com);
}

void add_to_macro(COMMAND *vb, Const char *macline)
{
    int addAtTheEnd = -1;

    insert_macro(vb, macline, addAtTheEnd);
    return;
}

/*
 *  Inserts a command line "macline" in the PCMACRO "vb" just before
 *  the line number "before".
 */
void insert_macro(COMMAND *vb, Const char *macline, int before)
{
    register int j;
    char *lineptr;
    PCMACRO *mac, *ptr;

    if (vb == (COMMAND *)NULL) {
      wipoutput(stderr, "A macro must be defined first.\n");
      return;
    }

    /* Make a private copy of the input macro string. */
    if ((lineptr = wipnewstring(macline)) == (char *)NULL) {
      wipoutput(stderr, "Could not allocate memory for the command line.\n");
      return;
    }

    /* Try to get room for storing a new PCMACRO and the command line. */

    if ((mac = (PCMACRO *)Malloc(sizeof(PCMACRO))) == (PCMACRO *)NULL) {
      wipoutput(stderr, "Could not allocate memory for the macro.\n");
      Free(lineptr);
      return;
    }
    mac->line = lineptr;
    mac->next = (PCMACRO *)NULL;

    /* If this PCMACRO is new, initialize it with the command and return. */

    if (vb->pcmac == (PCMACRO *)NULL) {
      vb->pcmac = mac;
      vb->ncom = 1;
      return;
    }
    vb->ncom++;

    if (before == 0) {
      mac->next = vb->pcmac;
      vb->pcmac = mac;
      return;
    }

    /* before == -1 is a special case used to find the end of the macro. */
    if (before < 0) before = vb->ncom;

    /* Go to the PCMACRO entry just before the one desired (or end of */
    /* the list) and then add the new command. */

    ptr = vb->pcmac;
    for (j = 1; ((ptr->next != (PCMACRO *)NULL) && (j < before)); j++)
      ptr = ptr->next;

    mac->next = ptr->next;
    ptr->next = mac;
    return;
}

void delete_macro(COMMAND *vb, int first, int last)
{
    int j = 0;
    PCMACRO *ptr, *next;

    if (vb->pcmac == (PCMACRO *)NULL) return;
    if (vb->macro != TRUE) {
      wipoutput(stderr, "Command [%s] is not a macro.\n", vb->name);
      return;
    }

/*************************************
      if (vb->delete != TRUE) {
        wipoutput(stderr, "Command [%s] cannot be deleted.\n", vb->name);
        return;
      }
*************************************/

    if ((last != EOF) && (last < first)) {
      wipoutput(stderr, "Invalid delete range: %d to %d\n", first, last);
      return;
    }

    ptr = vb->pcmac;
    first--;
    while ((j++ < first) && (ptr != (PCMACRO *)NULL))
      ptr = ptr->next;

    if (ptr == (PCMACRO *)NULL) return;

    for (next = ptr->next; next != (PCMACRO *)NULL; next = ptr->next) {
      if ((last != EOF) && (j++ > last)) break;
      ptr->next = next->next;
      if (next->line) Free(next->line);
      Free(next);
      vb->ncom--;
    }

    if (first < 0) {
      vb->pcmac = ptr->next;
      if (ptr->line) Free(ptr->line);
      Free(ptr);
      vb->ncom--;
    }
    return;
}
