/*
	<vars.c>
	03oct91 jm  Original code.
	31jul92 jm  Code combined from multiple source files into a
		    single file.  This is to hide as much structure
		    as possible.  Also changed the order in which
		    new stack items are loaded (at the front rather
		    than at the end); this should mean more recent
		    additions will be found quicker.  Also, sort the
		    initially defined elements such that more commonly
		    accessed items are closer to the end of the list.
	01oct92 jm  Changed initVariable() to a static function called
		    once by find_variable().

Routines:
static int initVariable ARGS(( void ));
static VARIABLE *find_variable ARGS(( Const char *inname ));
int wipisvar ARGS(( Const char *name ));
double wipgetvar ARGS(( Const char *inword, LOGICAL *error ));
int wipsetvar ARGS(( Const char *inword, double value ));
int wipNewVariable ARGS(( Const char *name ));
int wipFreeVariable ARGS(( Const char *name ));
*/

#define WIP_VARIABLES
#include "wip.h"
#include "variables.h"

/* Global variables for just this file */

static VARIABLE *VARHEAD = (VARIABLE *)NULL;

/* Code */

/*  Always returns 0, but function needs to return a status int. */
#ifdef PROTOTYPE
static int initVariable(void)
#else
static int initVariable()
#endif /* PROTOTYPE */
{
    register int j, number;
    VARIABLE *vb;

    number = sizeof(initialVarArray) / sizeof(initialVarArray[0]);

    VARHEAD = (VARIABLE *)NULL;
    for (j = 0; j < number; j++) {
      vb = &initialVarArray[j];
      vb->next = VARHEAD;
      VARHEAD = vb;
    }

    return(0);
}

/*
 *  Returns a pointer to the VARIABLE structure if "inname" is defined
 *  as a variable; a pointer to NULL otherwise.
 */
#ifdef PROTOTYPE
static VARIABLE *find_variable(Const char *inname)
#else
static VARIABLE *find_variable(inname)
Const char *inname;
#endif /* PROTOTYPE */
{
    char *par, *ptr;
    char word[STRINGSIZE];
    static LOGICAL FirstTime = TRUE;
    VARIABLE *vb;

    if (FirstTime == TRUE) {   /* Initialize the predefined variables. */
      FirstTime = FALSE;
      if (initVariable() != 0) {
        wipoutput(stderr, "Trouble initializing the variables!\n");
        wipoutput(stderr, "Some variables will be undefined!\n");
      }
    }

    ptr = Strncpy(word, inname, STRINGSIZE);     /* Make a local copy. */
    word[STRINGSIZE-1] = Null;          /* Make sure it is terminated. */
    if ((par = wipparse(&ptr)) == (char *)NULL)       /* Nothing here! */
      return((VARIABLE *)NULL);
    wiplower(par);                              /* Make it lower case! */

    for (vb = VARHEAD; vb != (VARIABLE *)NULL; vb = vb->next)
      if (Strcmp(par, vb->name) == 0)                     /* Found it. */
        return(vb);

    return((VARIABLE *)NULL);                            /* Not found. */
}

/* Returns 1 if "name" is defined as a variable; 0 otherwise. */
#ifdef PROTOTYPE
int wipisvar(Const char *name)
#else
int wipisvar(name)
Const char *name;
#endif /* PROTOTYPE */
{
    return(find_variable(name) != (VARIABLE *)NULL);
}

/*
 *  Returns, if the variable exists, the current value of the variable
 *  and sets error to FALSE; otherwise, it returns 0 and sets error to TRUE.
 */
#ifdef PROTOTYPE
double wipgetvar(Const char *inword, LOGICAL *error)
#else
double wipgetvar(inword, error)
Const char *inword;
LOGICAL *error;
#endif /* PROTOTYPE */
{
    VARIABLE *vb;

    if ((vb = find_variable(inword)) == (VARIABLE *)NULL) {
      wipoutput(stderr, "Unknown variable - %s\n", inword);
      *error = TRUE;
      return(0);
    }

    *error = FALSE;
    return(vb->value);
}

/* Returns 0 if the variable exists and was set; 1 if an error occured. */
#ifdef PROTOTYPE
int wipsetvar(Const char *inword, double value)
#else
int wipsetvar(inword, value)
Const char *inword;
double value;
#endif /* PROTOTYPE */
{
    VARIABLE *vb;

    if ((vb = find_variable(inword)) == (VARIABLE *)NULL) {
      wipoutput(stderr, "Unknown variable - %s\n", inword);
      return(1);
    }

    vb->value = value;
    return(0);
}

/* Returns 0 if all went well; 1 if an error occured. */
#ifdef PROTOTYPE
int wipNewVariable(Const char *name)
#else
int wipNewVariable(name)
Const char *name;
#endif /* PROTOTYPE */
{
    char *ptr;
    VARIABLE *vb;

    if (wipisstring(name) || wiptokenexists(name)) {
      wipoutput(stderr, "Variable name [%s] already reserved.\n", name);
      return(1);
    }

    if ((ptr = wipnewstring(name)) == (char *)NULL) {
      wipoutput(stderr, "A variable name is required.\n");
      return(1);
    }

    if ((vb = (VARIABLE *)Malloc(sizeof(VARIABLE))) == (VARIABLE *)NULL) {
      wipoutput(stderr, "Could not allocate memory for the variable.\n");
      Free(ptr);
      return(1);
    }

    wiplower(ptr);

    vb->name = ptr;
    vb->value = 0;
    vb->delete = TRUE;
    vb->next = VARHEAD;
    VARHEAD = vb;

    return(0);
}

#ifdef PROTOTYPE
int wipFreeVariable(Const char *name)
#else
int wipFreeVariable(name)
Const char *name;
#endif /* PROTOTYPE */
{
    register VARIABLE *p;
    VARIABLE *vb;

    if ((vb = find_variable(name)) == (VARIABLE *)NULL) {
      wipoutput(stderr, "Cannot find the variable %s!\n", name);
      return(1);
    }

    /* Do nothing if this variable should not be removed. */

    if (vb->delete != TRUE) {
      wipoutput(stderr, "Cannot remove [%s] from the variable list.\n",
        vb->name);
      return(0);                     /* Do not consider this an error. */
    }

    /* Find the variable to be removed. */

    if (vb == VARHEAD) {
      VARHEAD = vb->next;
    } else {
      for (p = VARHEAD; p != (VARIABLE *)NULL; p = p->next)
        if (p->next == (VARIABLE *)vb)
          break;

      if (p == (VARIABLE *)NULL) {
        wipoutput(stderr, "Cannot find [%s] in variable list.\n", vb->name);
        return(1);
      }
      p->next = (p->next)->next;
    }

    /* Remove all allocated entries in the structure. */

    if (vb->name != (char *)NULL) Free(vb->name);
    Free(vb);

    return(0);
}
