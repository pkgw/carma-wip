/*
	<command.c> -- This file handles the COMMAND structure.
	19jul90 jm  Original code.
	16aug92 jm  Cleaned up code a bit.

Routines:
COMMAND *create_command ARGS(( void ));
void add_command ARGS(( COMMAND *vb ));
void delete_command ARGS(( COMMAND *vb ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

#ifdef PROTOTYPE
COMMAND *create_command(void)
#else
COMMAND *create_command()
#endif /* PROTOTYPE */
{
    COMMAND *vb;

    if ((vb = (COMMAND *)Malloc(sizeof(COMMAND))) == (COMMAND *)NULL) {
      wipoutput(stderr, "Could not allocate memory for a command item.\n");
      return((COMMAND *)NULL);
    }
    vb->next = (COMMAND *)NULL;
    return(vb);
}

#ifdef PROTOTYPE
void add_command(COMMAND *vb)
#else
void add_command(vb)
COMMAND *vb;
#endif /* PROTOTYPE */
{
    COMMAND *p;

    if (HEAD == (COMMAND *)NULL) {               /* Linked list empty? */
      HEAD = vb;
      return;
    }

    for (p = HEAD; p->next != (COMMAND *)NULL; p = p->next)
      /* NULL */ ;                /*  Find the end of the linked list. */
    p->next = vb;                              /* Append this COMMAND. */
    return;
}

#ifdef PROTOTYPE
void delete_command(COMMAND *vb)
#else
void delete_command(vb)
COMMAND *vb;
#endif /* PROTOTYPE */
{
    COMMAND *p;
    PCMACRO *ptr, *next;

    if (vb == (COMMAND *)NULL) return;               /* Pointer check. */

    if (vb->delete != TRUE) {   /* This command should not be removed. */
      wipoutput(stderr, "Cannot delete %s from the command list.\n", vb->name);
      return;
    }

    /* If help information is associated with this command, remove it. */

    if (vb->help != (PCMACRO *)NULL) {
      next = (PCMACRO *)NULL;
      for (ptr = vb->help; ptr != (PCMACRO *)NULL; ptr = next) {
        next = ptr->next;  /* Save a pointer to the next linked entry. */
        if (ptr->line != (char *)NULL) Free(ptr->line);
        Free(ptr);
      }
    }

    /* If this command is a macro, remove all commands in macro buffer. */

    if ((vb->macro == TRUE) && (vb->pcmac != (PCMACRO *)NULL))
      delete_macro(vb, 0, EOF);

    /* Find which command is to be removed from the linked list. */
    if (vb == HEAD) {
      HEAD = vb->next;
    } else {
      for (p = HEAD; ((p != (COMMAND *)NULL) && (p->next != vb)); p = p->next)
        /* NULL */ ;
      if (p == (COMMAND *)NULL) {
        wipoutput(stderr, "Cannot find %s in command list.\n", vb->name);
        return;
      }
      p->next = (p->next)->next;
    }

    /* Finally, free up the command's name space and it's structure. */

    if (vb->name != (char *)NULL) Free(vb->name);
    Free(vb);
    return;
}
