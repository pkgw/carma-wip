/*
	<find.c>
	04oct91 jm  Original code.
	06nov91 jm  Added new/freeitem code.
	16feb92 jm  Modified new/freeitem code to allow multiple declarations.
	01aug92 jm  Substantially modified to move all references to
		    vectors, variables, and strings variables to their
		    own private routines.  Also add "strings" to new and
		    free commands.
	23sep92 jm  Modified wipnewitem() to use routine which tests
		    for previously existing items.

Routines:
int wipnewitem ARGS(( Const char *string ));
int wipfreeitem ARGS(( Const char *string ));
int wipisuserfunc ARGS(( Const char *name ));
double wipuserfunc ARGS(( Const char *inword, double arg, LOGICAL *error ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/* Returns 0 if all items were defined; 1 if an error occured. */
#ifdef PROTOTYPE
int wipnewitem(Const char *string)
#else
int wipnewitem(string)
Const char *string;
#endif /* PROTOTYPE */
{
    char *par, *ptr, *psize, *next;
    char token[STRINGSIZE];
    char temp[BUFSIZ];
    int size;
    double arg;
    LOGICAL isavector, isastring;
    LOGICAL error;

    (void)Strcpy(temp, string);         /* Make a local copy of string. */
    if ((ptr = wipleading(temp)) == (char *)NULL) {    /* Nothing here. */
      wipoutput(stderr, "A new variable name is required.\n");
      return(1);
    }

    while ((par = wipgettoken(token, ptr, &next)) != (char *)NULL) {
      ptr = next;   /* Move ptr to character following the last token. */
      if ((*par == '-') || (BRACE(*par))) {
        wipoutput(stderr, "Cannot begin a variable name this way: %s.\n", par);
        return(1);
      } else if (*par == '"') {                  /* A string variable. */
        par[Strlen(par) - 1] = Null;      /* Remove the closing quote. */
        par++;                         /* Skip over the initial quote. */
        isastring = TRUE;
      } else {                                /* A vector or variable. */
        isastring = FALSE;
        for (psize = par; *psize != Null; psize++) /* Test for vector. */
          if (BRACE(*psize))
            break;
        if (*psize != Null) {                         /* Got a vector. */
          arg = wipevaluate(psize, &error);    /* Get the vector size. */
          if (error == TRUE) {
            wipoutput(stderr, "Trouble getting the vector size of %s.\n", par);
            return(1);
          }
          size = NINT(arg);
          *psize = Null;    /* par now only points to the vector name. */
          isavector = TRUE;
        } else {                            /* Just a simple variable. */
          isavector = FALSE;
        }
      } /* End of if statements. */

      /* Test to see if the name is already defined. */
      if (wipisstring(par) || wiptokenexists(par)) {
        wipoutput(stderr, "Ignoring [%s]; it is already defined!\n", par);
        continue;
      }

      if (isastring == TRUE) {
        if (wipNewStrVar(par)) {
          wipoutput(stderr, "Trouble defining string %s\n", par);
          return(1);
        }
      } else if (isavector == TRUE) {
        if (wipNewVector(par, size)) {
          wipoutput(stderr, "Trouble defining vector %s\n", par);
          return(1);
        }
      } else {
        if (wipNewVariable(par)) {
          wipoutput(stderr, "Trouble defining variable %s\n", par);
          return(1);
        }
      } /* End of if statements. */
    } /* End of while loop. */

    return(0);
}

/* Returns 0 if all items were removed; 1 if an error occured. */
#ifdef PROTOTYPE
int wipfreeitem(Const char *string)
#else
int wipfreeitem(string)
Const char *string;
#endif /* PROTOTYPE */
{
    char *par, *ptr, *next;
    char token[STRINGSIZE];
    char vecname[STRINGSIZE];
    char temp[BUFSIZ];

    (void)Strcpy(temp, string);         /* Make a local copy of string. */
    if ((ptr = wipleading(temp)) == (char *)NULL) {    /* Nothing here. */
      wipoutput(stderr, "wipfreeitem: An item name is required.\n");
      return(1);
    }

    while ((par = wipgettoken(token, ptr, &next)) != (char *)NULL) {
      ptr = next;   /* Move ptr to character following the last token. */
      if ((*par == '-') || (BRACE(*par))) {
        continue;
      } else if (*par == '"') {                  /* A string variable. */
        par[Strlen(par) - 1] = Null;      /* Remove the closing quote. */
        par++;                         /* Skip over the initial quote. */
      }

      (void)Strcpy(vecname, par);
      Strcat(vecname, "[1]");

      if (wipisstring(par)) {                    /* A string variable. */
        if (wipFreeString(par)) {
          wipoutput(stderr, "Trouble freeing string %s.\n", par);
          return(1);
        }
      } else if (wipisvar(par)) {                /* A simple variable. */
        if (wipFreeVariable(par)) {
          wipoutput(stderr, "Trouble freeing variable %s.\n", par);
          return(1);
        }
      } else if (wipisvec(vecname)) {                     /* A vector. */
        if (wipFreeVector(vecname)) {
          wipoutput(stderr, "Trouble freeing vector %s.\n", par);
          return(1);
        }
      } else {                                     /* An Unknown item. */
        wipoutput(stderr, "wipfreeitem: Ignoring unknown item: %s.\n", par);
        continue;
      }
    }

    return(0);
}

/* Returns 1 if user function; 0 otherwise. */
#ifdef PROTOTYPE
int wipisuserfunc(Const char *name)
#else
int wipisuserfunc(name)
Const char *name;
#endif /* PROTOTYPE */
{
    register char *ptr, *opbrac;
    char word[BUFSIZ];

    (void)Strcpy(word, name);
    if ((ptr = wipleading(word)) == (char *)NULL) return(0);

    /* End the string at the first open brace. */

    if ((opbrac = Strchr(ptr, '(')) != (char *)NULL) *opbrac = Null;
    if ((opbrac = Strchr(ptr, '[')) != (char *)NULL) *opbrac = Null;
    if ((opbrac = Strchr(ptr, '{')) != (char *)NULL) *opbrac = Null;

    /* ptr points to the function name without any arguments or any */
    /* braces (i.e. if name = userfunc(x), ptr points to userfunc). */

    /* NO USER FUNCTIONS YET... */
    return(0);
}

/* ARGSUSED */
#ifdef PROTOTYPE
double wipuserfunc(Const char *inword, double arg, LOGICAL *error)
#else
double wipuserfunc(inword, arg, error)
Const char *inword;  /* Unused for now... */
double arg;          /* Unused for now... */
LOGICAL *error;
#endif /* PROTOTYPE */
{
    /* NO USER FUNCTIONS YET... */
    *error = TRUE;
    return(0.0);
}
