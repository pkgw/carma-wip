/*
	<pushpop.c> -- This file handles the push and pop stack commands.
	30oct91 jm  Original code.
	01aug92 jm  Modified only the internal variable names.
         9oct00 pjt no more PROTOTYPE

Routines:
void clear_stack ARGS(( void ));
int push_stack ARGS(( double value ));
int pop_stack ARGS(( double *value ));
*/

#include "wip.h"

/* Global variables for just this file */

#define MAXSTACK 20

static    int  stackCount = -1;
static double *stackTop;
static double  stackList[MAXSTACK];

/* Code */

void clear_stack(void)
{
      stackTop = stackList;
      stackCount = 0;
}

/* Returns 0 if error; stack_count (1 <--> MAXSTACK-1) after push otherwise. */
int push_stack(double value)
{
      if (stackCount < 0) clear_stack();
      if ((stackCount + 1) == MAXSTACK) return(0);
      *stackTop++ = value;
      stackCount++;
      return(stackCount);
}

/* Returns -1 if error; stack_count (0 <--> MAXSTACK) after pop otherwise. */
int pop_stack(double *value)
{
      if (stackCount < 0) clear_stack();
      if (stackCount == 0) return(-1);
      stackTop--;
      stackCount--;
      *value = *stackTop;
      return(stackCount);
}
