/*
	<str.c>
	04oct91 jm  Original code.
	31jul92 jm  Modified to use wipnewstring() command.
	01aug92 jm  Code combined from multiple source files into a
		    single file.  This is to hide as much structure
		    as possible.  Also changed the order in which
		    new stack items are loaded (at the front rather
		    than at the end); this should mean more recent
		    additions will be found quicker.  Also, sort the
		    initially defined elements such that more commonly
		    accessed items are closer to the end of the list.
	11sep92 jm  Added test for an empty value in wipsetstring().
	01oct92 jm  Changed initString() to a static function called
		    once by find_string().
         9oct00 pjt no more PROTOTYPE

Routines:
static int initString ARGS(( void ));
static WSTRINGS *find_string ARGS(( Const char *inname ));
int wipisstring ARGS(( Const char *name ));
char *wipgetstring ARGS(( Const char *inword ));
int wipsetstring ARGS(( Const char *input, Const char *value ));
int wipNewStrVar ARGS(( Const char *string ));
int wipFreeString ARGS(( Const char *string ));
*/

#define WIP_STRING
#include "wip.h"
#include "str.h"

/* Global variables for just this file */

static WSTRINGS *STRHEAD = (WSTRINGS *)NULL;

/* Code */

/*  Always returns 0, but function needs to return a status int. */
static int initString(void)
{
    register int j, number;
    WSTRINGS *vb;

    number = sizeof(initialStrArray) / sizeof(initialStrArray[0]);

    STRHEAD = (WSTRINGS *)NULL;
    for (j = 0; j < number; j++) {
      vb = &initialStrArray[j];
      vb->next = STRHEAD;
      STRHEAD = vb;
    }

    return(0);
}

/*
 *  Returns a pointer to the WSTRINGS structure if "inname" is defined
 *  as a string variable; a pointer to NULL otherwise.
 */
static WSTRINGS *find_string(Const char *inname)
{
    char *par, *ptr;
    char word[STRINGSIZE];
    static LOGICAL FirstTime = TRUE;
    WSTRINGS *vb;

    if (FirstTime == TRUE) {/* Initialize the predefined string stack. */
      FirstTime = FALSE;
      if (initString() != 0) {
        wipoutput(stderr, "Trouble initializing the string variables!\n");
        wipoutput(stderr, "Some string variables will be undefined!\n");
      }
    }

    ptr = Strncpy(word, inname, STRINGSIZE);     /* Make a local copy. */
    word[STRINGSIZE-1] = Null;          /* Make sure it is terminated. */
    if ((par = wipparse(&ptr)) == (char *)NULL)       /* Nothing here! */
      return((WSTRINGS *)NULL);
    wiplower(par);                              /* Make it lower case! */

    for (vb = STRHEAD; vb != (WSTRINGS *)NULL; vb = vb->next)
      if (Strcmp(par, vb->name) == 0)                     /* Found it! */
        return(vb);

    return((WSTRINGS *)NULL);                             /* No match! */
}

/* Returns 1 if "string" is defined as a string variable; 0 otherwise. */
int wipisstring(Const char *name)
{
    return(find_string(name) != (WSTRINGS *)NULL);
}

/*
 *  Returns a pointer to the string value if the string variable exists;
 *  a pointer to NULL if not found or is set to an empty string.
 */
char *wipgetstring(Const char *inword)
{
    WSTRINGS *vb;

    if ((vb = find_string(inword)) == (WSTRINGS *)NULL) {
      wipoutput(stderr, "Unknown string - %s\n", inword);
      return((char *)NULL);
    }

    return(vb->value);
}

/* Returns 0 if the string variable exists and was set; 1 on error. */
int wipsetstring(Const char *input, Const char *value)
{
    char *ptr;
    WSTRINGS *vb;

    if ((vb = find_string(input)) == (WSTRINGS *)NULL) {
      wipoutput(stderr, "Unknown string variable: %s\n", input);
      return(1);
    }

    if ((ptr = wipleading(value)) == (char *)NULL) {
      (void)Strcpy(vb->value, "");    /* Set value to an empty string. */
    } else {
      (void)Strncpy(vb->value, ptr, STRINGSIZE);           /* Copy it. */
      vb->value[STRINGSIZE-1] = Null;                 /* Terminate it. */
      (void)wiplenc(vb->value);          /* Strip off trailing blanks. */
    }

    return(0);
}

/* Returns 0 if all went well; 1 if an error occured. */
int wipNewStrVar(Const char *name)
{
    char *ptr;
    WSTRINGS *vb;

    if (wipisstring(name) || wiptokenexists(name)) {
      wipoutput(stderr, "String name [%s] already reserved.\n", name);
      return(1);
    }

    if ((ptr = wipnewstring(name)) == (char *)NULL) {
      wipoutput(stderr, "A string name is required.\n");
      return(1);
    }

    if ((vb = (WSTRINGS *)Malloc(sizeof(WSTRINGS))) == (WSTRINGS *)NULL) {
      wipoutput(stderr, "Could not allocate memory for the string variable.\n");
      Free(ptr);
      return(1);
    }

    wiplower(ptr);

    vb->name = ptr;
    (void)Strcpy(vb->value, "");
    vb->delete = TRUE;
    vb->next = STRHEAD;
    STRHEAD = vb;
    return(0);
}

/* Returns 0 if all went well; 1 if an error occured. */
int wipFreeString(Const char *name)
{
    register WSTRINGS *p;
    WSTRINGS *vb;

    if ((vb = find_string(name)) == (WSTRINGS *)NULL) {
      wipoutput(stderr, "Could not find the string variable %s!\n", name);
      return(1);
    }

    /* Do nothing if this string variable should not be removed. */

    if (vb->delete != TRUE) {
      wipoutput(stderr, "Cannot delete [%s] from the string variable list.\n",
        vb->name);
      return(0);                     /* Do not consider this an error. */
    }

    /* Find the variable to be removed. */

    if (vb == STRHEAD) {
      STRHEAD = vb->next;
    } else {
      for (p = STRHEAD; p != (WSTRINGS *)NULL; p = p->next)
        if (p->next != (WSTRINGS *)vb)
          break;

      if (p == (WSTRINGS *)NULL) {
        wipoutput(stderr, "Cannot find [%s] in the string variable list.\n",
          vb->name);
        return(1);
      }
      p->next = (p->next)->next;
    }

    /* Remove all allocated entries in the structure. */

    if (vb->name != (char *)NULL) Free(vb->name);
    Free(vb);

    return(0);
}
