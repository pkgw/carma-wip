/*
	<evaluate.c>
	09jul90 jm  Original code.
	29oct91 jm  Modified for new stack structure.
	18nov91 jm  Added a check for the end of a string to bracextract.
	24feb92 jm  Added routine wipgettoken().
	01aug92 jm  Reordered code so statics appear at the top of the file.
		    Also added a bounds check for USERVAR.
        03aug92 jm  Modified wipsetuser() to return status (void -> int)
		    rather than using a passed LOGICAL pointer.
        23sep92 jm  Added wiptokenexists() test routine.
        09aug93 jm  Added wipisnumber() function.
        10nov93 jm  Modified image variable to new opaque pointer type.
        02jul96 jm  Added rand and gasdev functions.

Routines:
static int wipisop ARGS(( Const char *inword ));
static double wipdoop ARGS(( Const char *op, double arg1, double arg2, LOGICAL *error ));
static int wipisfunction ARGS(( Const char *inword ));
static double wipdofunc ARGS(( Const char *inwrd, double arg, LOGICAL *error ));
int    wipisnumber ARGS(( Const char *inword, double *retval ));
void   wipecho ARGS(( Const char *input ));
int    wipsetuser ARGS(( Const char *input ));
char  *wipgettoken ARGS(( char *output, Const char *input, char **next ));
int wiptokenexists ARGS(( Const char *inword ));
double wipevaluate ARGS(( Const char *inword, LOGICAL *error ));
char  *wipbracextract ARGS(( Const char *inword, char **left ));
*/

#include "wip.h"

/* Global variables for just this file */

static double USERVAR[21]; /* USERVAR is an array of user setable variables. */

/* Code */

/* Returns 1 if "inword" is a predefined operation; 0 otherwise. */
#ifdef PROTOTYPE
static int wipisop(Const char *inword)
#else
static int wipisop(inword)
Const char *inword;
#endif /* PROTOTYPE */
{
    char *ptr;

    if ((ptr = wipleading(inword)) == (char *)NULL)
      return(0);

    if ((Strcmp("+",   ptr) == 0) || (Strcmp("-",   ptr) == 0) ||
        (Strcmp("*",   ptr) == 0) || (Strcmp("/",   ptr) == 0) ||
        (Strcmp("%",   ptr) == 0) || (Strcmp("**",  ptr) == 0) ||
        (Strcmp("max", ptr) == 0) || (Strcmp("min", ptr) == 0) ||
        (Strcmp("==",  ptr) == 0) || (Strcmp("!=",  ptr) == 0) ||
        (Strcmp("<",   ptr) == 0) || (Strcmp(">",   ptr) == 0) ||
        (Strcmp("<=",  ptr) == 0) || (Strcmp(">=",  ptr) == 0) ||
        (Strcmp("||",  ptr) == 0) || (Strcmp("&&",  ptr) == 0) ||
        (Strcmp("^",   ptr) == 0)) {
      return(1);
    } else if ((*ptr == ESC) && (Strlen(ptr) == 1)) {
      return(1);
    }
    return(0);
}

/* Returns the result of the operation (arg1 op arg2). */
#ifdef PROTOTYPE
static double wipdoop(Const char *inword, double arg1, double arg2, LOGICAL *error)
#else
static double wipdoop(inword, arg1, arg2, error)
Const char *inword;
double arg1, arg2;
LOGICAL *error;
#endif /* PROTOTYPE */
{
    char *ptr;
    double arg;

    *error = TRUE;
    if ((ptr = wipleading(inword)) == (char *)NULL)
      return(0);

    if (Strcmp(ptr, "+") == 0) {
      arg = arg1 + arg2;
    } else if (Strcmp(ptr,   "-") == 0) {
      arg = arg1 - arg2;
    } else if (Strcmp(ptr,   "*") == 0) {
      arg = arg1 * arg2;
    } else if (Strcmp(ptr,   "/") == 0) {
      arg = arg1 / arg2;
    } else if (Strcmp(ptr,   "%") == 0) {
      arg = arg1 - (arg2 * INT(arg1 / arg2));
    } else if (*ptr == ESC) {
      arg = INT(arg1 / arg2);
    } else if (Strcmp(ptr, "max") == 0) {
      arg = MAX(arg1, arg2);
    } else if (Strcmp(ptr, "min") == 0) {
      arg = MIN(arg1, arg2);
    } else if (Strcmp(ptr,  "**") == 0) {
      if (arg1 <= 0.0) {
        arg = 0.0;
        if (arg1 == 0.0) {
          if (arg2 <= 0.0) {
            wipoutput(stderr, "Cannot take exponent of negative value.\n");
            return(0);
          }
        } else {
          long tmpint;
          tmpint = arg2;
          if (tmpint != arg2) {
            wipoutput(stderr, "Cannot take exponent of negative value.\n");
            return(0);
          }
          arg = EXP(arg2 * LOG(-arg1)); /* Fudge. */
          if (tmpint & 1) arg = -arg;
        }
      } else {
        arg = EXP(arg2 * LOG(arg1)); /* Fudge. */
      }
    } else if (Strcmp(ptr,   "<") == 0) {
      arg = arg1 < arg2;
    } else if (Strcmp(ptr,   ">") == 0) {
      arg = arg1 > arg2;
    } else if (Strcmp(ptr,  "<=") == 0) {
      arg = arg1 <= arg2;
    } else if (Strcmp(ptr,  ">=") == 0) {
      arg = arg1 >= arg2;
    } else if (Strcmp(ptr,  "==") == 0) {
      arg = arg1 == arg2;
    } else if (Strcmp(ptr,  "!=") == 0) {
      arg = arg1 != arg2;
    } else if (Strcmp(ptr,  "&&") == 0) {
      arg = arg1 && arg2;
    } else if (Strcmp(ptr,  "||") == 0) {
      arg = arg1 || arg2;
    } else if (Strcmp(ptr,   "^") == 0) {
      arg = !(arg1 && arg2) && (arg1 || arg2);
    } else {
      wipoutput(stderr, "Unrecognized arithmetic operator: %s\n", inword);
      return(0);
    }
    *error = FALSE;
    return(arg);
}

/*  Returns 1 if "inword" is a predefined (standard) function; 0 otherwise. */
#ifdef PROTOTYPE
static int wipisfunction(Const char *inword)
#else
static int wipisfunction(inword)
Const char *inword;
#endif /* PROTOTYPE */
{
    register char *ptr, *opbrac;
    char word[BUFSIZ];

    (void)Strcpy(word, inword); /* Input string is already in lower case. */
    if ((ptr = wipleading(word)) == (char *)NULL) return(0);

    /* End the string at the first open brace. */

    if ((opbrac = Strchr(ptr, '(')) != (char *)NULL) *opbrac = Null;
    if ((opbrac = Strchr(ptr, '[')) != (char *)NULL) *opbrac = Null;
    if ((opbrac = Strchr(ptr, '{')) != (char *)NULL) *opbrac = Null;
 
    /* ptr points to the function name without any arguments or any */
    /* braces (i.e. if name = thisfunc(x), ptr points to thisfunc). */

    if ((Strcmp("sin",  ptr) == 0) || (Strcmp("sind",  ptr) == 0) ||
        (Strcmp("asin", ptr) == 0) || (Strcmp("asind", ptr) == 0) ||
        (Strcmp("cos",  ptr) == 0) || (Strcmp("cosd",  ptr) == 0) ||
        (Strcmp("acos", ptr) == 0) || (Strcmp("acosd", ptr) == 0) ||
        (Strcmp("tan",  ptr) == 0) || (Strcmp("tand",  ptr) == 0) ||
        (Strcmp("atan", ptr) == 0) || (Strcmp("atand", ptr) == 0) ||
        (Strcmp("sqrt", ptr) == 0) || (Strcmp("ln",    ptr) == 0) ||
        (Strcmp("log",  ptr) == 0) || (Strcmp("log10", ptr) == 0) ||
        (Strcmp("exp",  ptr) == 0) || (Strcmp("abs",   ptr) == 0) ||
        (Strcmp("int",  ptr) == 0) || (Strcmp("nint",  ptr) == 0) ||
        (Strcmp("rand", ptr) == 0) || (Strcmp("gasdev", ptr) == 0)) {
      return(1);
    }
    return(0);
}

/* Returns the result of the operation (F(arg)). */
#ifdef PROTOTYPE
static double wipdofunc(Const char *inword, double arg, LOGICAL *error)
#else
static double wipdofunc(inword, arg, error)
Const char *inword;
double arg;
LOGICAL *error;
#endif /* PROTOTYPE */
{
    long int narg;
    static long int randarg = (-911);
    double value;

    *error = TRUE;
    if (Strcmp("sin", inword) == 0) {
      value = SIN(arg);
    } else if (Strcmp("sind", inword) == 0) {
      arg *= RPDEG;
      value = SIN(arg);
    } else if (Strcmp("asin", inword) == 0) {
      if (ABS(arg) > 1.0) goto BADVALUE;
      value = ASIN(arg);
    } else if (Strcmp("asind", inword) == 0) {
      if (ABS(arg) > 1.0) goto BADVALUE;
      value = ASIN(arg) / RPDEG;
    } else if (Strcmp("cos", inword) == 0) {
      value = COS(arg);
    } else if (Strcmp("cosd", inword) == 0) {
      arg *= RPDEG;
      value = COS(arg);
    } else if (Strcmp("acos", inword) == 0) {
      if (ABS(arg) > 1.0) goto BADVALUE;
      value = ACOS(arg);
    } else if (Strcmp("acosd", inword) == 0) {
      if (ABS(arg) > 1.0) goto BADVALUE;
      value = ACOS(arg) / RPDEG;
    } else if (Strcmp("tan", inword) == 0) {
      value = TAN(arg);
    } else if (Strcmp("tand", inword) == 0) {
      arg *= RPDEG;
      value = TAN(arg);
    } else if (Strcmp("atan", inword) == 0) {
      value = ATAN(arg);
    } else if (Strcmp("atand", inword) == 0) {
      value = ATAN(arg) / RPDEG;
    } else if (Strcmp("sqrt", inword) == 0) {
      if (arg < 0.0) goto BADVALUE;
      value = SQRT(arg);
    } else if (Strcmp("ln", inword) == 0) {
      if (arg <= 0.0) goto BADVALUE;
      value = LOG(arg);
    } else if ((Strcmp("log", inword) == 0) || (Strcmp("log10", inword) == 0)) {
      if (arg <= 0.0) goto BADVALUE;
      value = LOG10(arg);
    } else if (Strcmp("exp", inword) == 0) {
      value = EXP(arg);
    } else if (Strcmp("abs", inword) == 0) {
      value = ABS(arg);
    } else if (Strcmp("int", inword) == 0) {
      value = INT(arg);
    } else if (Strcmp("nint", inword) == 0) {
      value = NINT(arg);
    } else if (Strcmp("rand", inword) == 0) {
      narg = NINT(arg);
      if (narg >= 0) narg = randarg;
      value = wiprand(&narg);
      randarg = narg;
    } else if (Strcmp("gasdev", inword) == 0) {
      narg = NINT(arg);
      if (narg >= 0) narg = randarg;
      value = wipgaussdev(&narg);
      randarg = narg;
    } else {
      wipoutput(stderr, "Unrecognized arithmetic operator: %s\n", inword);
      return(0);
    }
    *error = FALSE;
    return(value);

BADVALUE:
    wipoutput(stderr, "Illegal operation: %s %G\n", inword, arg);
    return(0);
}

/*
 *  This routine will test if the input string is just a simple number.
 *  This takes into account integers, floating point numbers, and numbers
 *  presented in scientific notation (including Fortran's D-format).
 *  This routine returns 0 if the input string could not be formatted
 *  entirely into a number and the value of "retval" will be undefined;
 *  otherwise, the number will be assigned to "retval" and 1 will be returned.
 */
#ifdef PROTOTYPE
int wipisnumber(Const char *inword, double *retval)
#else
int wipisnumber(inword, retval)
Const char *inword;
double *retval;
#endif /* PROTOTYPE */
{
    char *ptr;
    char temp[50];
    int dummy, expon;
    double arg;

    if ((ptr = wipleading(inword)) == (char *)NULL)
      return(0);                                /* Nothing to process. */

    SPrintf(temp, "%s~~1", ptr);           /* Fudge to make test work. */
    if ((SScanf(temp, "%lg~~%d", &arg, &dummy) == 2) && (dummy == 1)) {
      *retval = arg;                /* Token was just a simple number. */
    } else if ((SScanf(temp, "%lg%*[dD]%d~~%d", &arg, &expon, &dummy) == 3) &&
               (dummy == 1)) {          /* Token was a Fortran double. */
      *retval = arg;
      if (expon != 0)
        *retval *= EXP(expon * LOG(10.0));
    } else {                         /* Token was not a simple number. */
      return(0);
    }

    return(1);
}

/*
 *  This function allows the user to print messages to the standard
 *  output including the evaluation of expressions and the value of
 *  string variables.  The syntax is that the input string should
 *  contain any literal text enclosed in double quotes (") and
 *  multiple expressions to be evaluated enclosed in braces (see
 *  definition of BRACE in wip.h).  Single string variables, user
 *  variables, or vectors may appear by themselves; all other items
 *  produce an error message.
 */
#ifdef PROTOTYPE
void wipecho(Const char *input)
#else
void wipecho(input)
Const char *input;
#endif /* PROTOTYPE */
{
    char *ptr, *op, *token, *next, *tmpnext;
    char sval[50];                        /* This size should be ample. */
    char tstring[STRINGSIZE];              /* Storage for token string. */
    char word[BUFSIZ];       /* Storage for local copy of input string. */
    double arg1, arg2, result;
    LOGICAL error;

    ptr = Strcpy(word, input);           /* Make a local copy of input. */

    while ((token = wipgettoken(tstring, ptr, &next)) != (char *)NULL) {
      if (*token == '"') {                                    /* Quote. */
        token[Strlen(token) - 1] = Null;       /* Remove closing quote. */
        token++;                            /* Skip over initial quote. */
        wipoutput(stdout, "%s", token);            /* Print the string. */
      } else if (wipisstring(token)) {         /* User string variable. */
        wipoutput(stdout, "%s", wipgetstring(token));     /* Print var. */
      } else {            /* Either a simple variable or an expression. */
        result = wipevaluate(token, &error);           /* Get variable. */
        if (error == TRUE) {                           /* Check syntax. */
          wipoutput(stderr, "Error evaluating expression: %s\n", token);
        } else {         /* Now test if an operation follows first arg. */
          token = wipgettoken(tstring, next, &tmpnext);
          if (token != (char *)NULL) {
            while (wipisop(token)) {               /* Binary operation. */
              arg1 = result;        /* Move result into first argument. */
              op = Strcpy(sval, token);        /* Save operation token. */
              next = tmpnext;               /* Advance pointer past op. */
              token = wipgettoken(tstring, next, &tmpnext);
              if (token == (char *)NULL) {
                wipoutput(stderr, "Improper operator format: [%s].\n", ptr);
                next = tmpnext;          /* Advance pointer past error. */
                break;                                /* Abort op loop. */
              }
              next = tmpnext;         /* Advance pointer past argument. */
              arg2 = wipevaluate(token, &error);   /* Get 2nd argument. */
              if (error == TRUE) {                     /* Check syntax. */
                wipoutput(stderr, "Error evaluating expression: %s\n", token);
                break;                                /* Abort op loop. */
              }
              result = wipdoop(op, arg1, arg2, &error);/* Do binary op. */
              if (error == TRUE) {                  /* Check op syntax. */
                wipoutput(stderr, "Error evaluating operation: [%s]\n", ptr);
                break;                                /* Abort op loop. */
              }
              token = wipgettoken(tstring, next, &tmpnext);
              if (token == (char *)NULL)
                break;
            }
          }
          wipoutput(stdout, " %g", result);
        }
      }
      ptr = next;    /* Move ptr to character following the last token. */
    }                                 /* End of scan string while loop. */
    wipoutput(stdout, "\n");
    return;
}

/*
 *  Syntax calls for two "arguments": PAR EXPRESSION.  The EXPRESSION
 *  may be a multi-word expression explained below.
 *
 *  PAR is the string/variable/vector being set and EXPRESSION is
 *  either a replacement string or a simple syntax statement to be
 *  evaluated.
 *
 *  To set a string variable, the variable should be enclosed in
 *  double quotes (").  For string variables, EXPRESSION is not
 *  evaluated, but is stored as the value for the variable.
 *
 *  The remainder of this discussion describes how variable and vector
 *  variables are set.
 *
 *  When evaluating the EXPRESSION, precedence is ALWAYS left to right
 *  with the inner most set of parenthesis evaluated first.  Hence,
 *  the expression (5 * 3 + 2) IS NOT the same as (2 + 3 * 5) but
 *  ((5 * 3) + 2) IS the same as (2 + (3 * 5)).  Currently, a maximum
 *  of 20 nested levels of parenthesis is permitted.
 *
 *  At the most basic level, EXPRESSION may just be an item that is
 *  either a number, a predefined user variable or vector element.
 *  More complex EXPRESSION's may be generated using the operators
 *  and functions listed below.
 *
 *  If an item within the EXPRESSION begins with '-', it is treated
 *  as a unary minus sign.
 *
 *  Currently, the binary operations that are defined are:
 *     +          add |  /         divide | max               maximum
 *     -     subtract |  %  modulo divide | min               minimum
 *     *     multiply |  \ integer divide |  **        exponentiation
 *    ==     equal to |  >   greater than |  >= greater than or equal
 *    != not equal to |  <      less than |  <=    less than or equal
 *    ||   logical OR | &&    logical AND |   ^           logical XOR
 *  Note that min and max are used as (a min b) and (a max b).
 *  Also, note that logical operations return 1 if true; 0 if false.
 *
 *  Current pre-defined functions (ie. f(x)) are:
 *   sqrt(x)          square root |    abs(x)        absolute value 
 *    int(x)   integer truncation |   nint(x)           nearest int
 *     ln(x)    natural logarithm |    log(x)           log base 10
 *    exp(x) base-e antilogarithm |     10(x) base-10 antilogarithm
 *    sin(x)      sine in radians |   sind(x)       sine in degrees
 *   asin(x)   arcsine in radians |  asind(x)    arcsine in degrees
 *   ... etc. for the four functions of cos and tan;
 *   Note that log(x) or log10(x) may be used for the log base 10.
 *
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wipsetuser(Const char *rest)
#else
int wipsetuser(rest)
Const char *rest;
#endif /* PROTOTYPE */
{
    Void *curimage;                  /* Pointer to current image item. */
    char *ptr, *token, *next;
    char tokenstring[STRINGSIZE];        /* Storage for current token. */
    char word[BUFSIZ];                  /* Local copy of input string. */
    int varindex;
    float pmin, pmax;
    double arg, arg1;
    LOGICAL error;

    ptr = Strcpy(word, rest);        /* Point "ptr" to the local copy. */
    if ((token = wipgettoken(tokenstring, ptr, &next)) == (char *)NULL) {
      wipoutput(stderr, "Incorrect set format: [%s].\n", rest);
      return(1);                                      /* Nothing here. */
    }

    if (*token == '"') {                           /* String variable. */
      token[Strlen(token) - 1] = Null;        /* Remove closing quote. */
      token++;                             /* Skip over initial quote. */
      if (wipsetstring(token, next)) goto MISTAKE;
    } else if (wipisstring(token)) {               /* String variable. */
      if (wipsetstring(token, next)) goto MISTAKE;
    } else {
      wiplower(token);       /* Force the variable name to lower case. */
      arg = wipevaluate(next, &error);         /* Evaluate EXPRESSION. */
      if (error == TRUE) goto MISTAKE;

      if (wipisvar(token)) {
        if (wipsetvar(token, arg)) goto MISTAKE;
      } else if (wipisvec(token)) {
        if (wipsetvec(token, arg)) goto MISTAKE;
      } else if (*token == ESC) {
        token++;
        if (wiparguments(&token, 1, &arg1) != 1) goto MISTAKE;
        varindex = NINT(arg1);
        if ((varindex >= 0) && (varindex < 21))
          USERVAR[varindex] = arg;
      } else if (wipimagexists(curimage = wipimcur("curimage"))) {
        if (Strcmp(token, "immin") == 0) {
          wipimageminmax(curimage, &pmin, &pmax, 0);
          pmin = arg;
          if (wipimsetminmax(curimage, pmin, pmax)) goto MISTAKE;
        } else if (Strcmp(token, "immax") == 0) {
          wipimageminmax(curimage, &pmin, &pmax, 0);
          pmax = arg;
          if (wipimsetminmax(curimage, pmin, pmax)) goto MISTAKE;
        } else {
          goto MISTAKE;
        }
      } else {
        goto MISTAKE;
      }
    }

/* good value found */
    return(0);

MISTAKE:
    wipoutput(stderr, "Error setting variable: %s\n", token);
    return(1);
}

#ifdef PROTOTYPE
char *wipgettoken(char *output, Const char *input, char **next)
#else
char *wipgettoken(output, input, next)
char *output;
Const char *input;
char **next;
#endif /* PROTOTYPE */
{
/*
 * This function parses the input string and returns the next token and
 * a pointer to the character following the current token (address relative
 * to the input string).  The syntax of the input string can contain any
 * literal text enclosed in double quotes ("), single user variables or
 * string variables, and multiple expressions to be evaluated that are
 * enclosed in braces (see definition of BRACE in wip.h).
 *
 * NOTE: that the returned token is located in the string OUTPUT and this
 * should be large enough to hold the token.  No test for this is done!
 * The pointer returned is the first character of OUTPUT.
 */
    char *ptr, *par, *current, *ptrbegin;
    char *openbrace, *closebrace;

    if ((ptr = wipleading(input)) == (char *)NULL)       /* Nothing here. */
      return((char *)NULL);
    current = par = output;                   /* Initialize the pointers. */

    if (*ptr == '"') {               /* Special case for a quoted string. */
      *par++ = *ptr++;                  /* Copy the open quote character. */
      while ((*ptr != Null) && (*ptr != '"'))
        *par++ = *ptr++;                  /* Copy over the quoted string. */
      *par++ = '"'; /* Put a final double quote at the end of the string. */
      *par++ = Null;                             /* Terminate the string. */
      if (*ptr == '"') ptr++;  /* Skip the quote character, if necessary. */
      *next = ptr;                            /* Identify where we ended. */
      return(current);                    /* All done if this is a quote. */
    }

    if ((*ptr == '-') && (!WHITE(*(ptr+1))))              /* Unary minus. */
      *par++ = *ptr++;               /* Copy the minus sign and continue. */

    if (BRACE(*ptr)) {                /* Is this character an open brace? */
      closebrace = wipbracextract(ptr, &openbrace);   /* Find it's match. */
      if (ptr != openbrace) {                /* Check that all went well. */
        wipoutput(stderr, "Format error: %s\n", input);
        return((char *)NULL);
      }
      if (closebrace != (char *)NULL) {        /* A closebrace was found. */
        closebrace++;                         /* Move past closing brace. */
        while (ptr != closebrace) *par++ = *ptr++;  /* Copy braced token. */
        *par++ = Null;                           /* Terminate the string. */
      } else {       /* No closebrace found; copy to end of input string. */
        while (*ptr != Null) *par++ = *ptr++;  /* Copy over the string... */
        switch (*openbrace) {               /* Include a closing brace... */
          case '{': *par++ = '}'; break;
          case '[': *par++ = ']'; break;
          default : *par++ = ')'; break;
        }
        *par++ = Null;                    /* ...and terminate the string. */
      }
    } else {      /* What remains is either a function or a simple token. */
      ptrbegin = ptr;
      while ((*ptr != Null) && (!WHITE(*ptr)))
        *par++ = *ptr++;             /* First, parse a simple word token. */
      *par = Null;        /* Terminate the token and test for a function. */
      if (*ptr != Null) {                    /* Still more in the string? */
        closebrace = wipbracextract(current, &openbrace);  /* A function? */
        if ((openbrace != (char *)NULL) && (closebrace == (char *)NULL)) {
                                                           /* A function. */
          closebrace = wipbracextract(ptrbegin, &openbrace);  /* Get end. */
          if (closebrace != (char *)NULL) closebrace++;
                                           /* Get past the closing brace. */
          while (ptr != closebrace) *par++ = *ptr++;/* Copy braced token. */
          *par++ = Null;                         /* Terminate the string. */
        } else {                                          /* No function. */
          ptr++;                     /* Advance past the end of the word. */
        }
      }
    }
    *next = ptr;                              /* Identify where we ended. */
    return(current);
}

/*
 *  Returns 1 if "inword" is already a defined user or vector variable,
 *  standard, user, or vector function, any operation, or literal number;
 *  0 otherwise.  This does not test for string variables or non-predefined
 *  image header items and ignores quoted strings and items beginning with
 *  a minus sign or open brace; these items cause an immediate return
 *  with a value of 0.
 */
#ifdef PROTOTYPE
int wiptokenexists(Const char *inword)
#else
int wiptokenexists(inword)
Const char *inword;
#endif /* PROTOTYPE */
{
    char *par, *ptr, *next;
    char tokenstring[STRINGSIZE];           /* Storage for current token. */
    char vecname[STRINGSIZE];       /* Storage for temporary vector name. */
    char word[BUFSIZ];                     /* Local copy of input string. */
    double dummy;

    par = Strcpy(word, inword);         /* point par to the private copy. */
    wiplower(par);    /* par now is a lowercase copy of the input string. */

    if ((ptr = wipgettoken(tokenstring, par, &next)) == (char *)NULL)
      return(0);                            /* Nothing here to work with. */

    if (*ptr == '"')                                   /* Ignore strings. */
      return(0);

    if ((*ptr == '-') || (BRACE(*ptr)))        /* Incorrect token format. */
      return(0);

    (void)Strcpy(vecname, par);
    Strcat(vecname, "[1]");                 /* Use this name for vectors. */

    if (wipisnumber(ptr, &dummy)) {                   /* A simple number? */
      return(1);                        /* Token is just a simple number. */
    } else if ((wipisop(ptr))      ||          /* A predefined operation? */
              (wipisfunction(ptr)) ||             /* A standard function? */
              (wipisvecfunc(ptr))  ||            /* Vector name function? */
              (wipisuserfunc(ptr)) ||         /* A user defined function? */
              (wipisvar(ptr))      ||            /* Simple user variable? */
              (wipisvec(vecname))  ||     /* Simple user vector variable? */
              (*ptr == ESC)) {           /* Simple user storage variable? */
      return(1);
    } else if ((Strcmp("immin", ptr) == 0) ||           /* Image minimum? */
               (Strcmp("immax", ptr) == 0) ||           /* Image maximum? */
               (Strcmp("nx", ptr) == 0)    ||    /* # of points in x-dir? */
               (Strcmp("ny", ptr) == 0)) {  /* Number of points in y-dir? */
      return(1);
    }
    return(0);
}

#ifdef PROTOTYPE
double wipevaluate(Const char *inword, LOGICAL *error)
#else
double wipevaluate(inword, error)
Const char *inword;
LOGICAL *error;
#endif /* PROTOTYPE */
{
    Void *curimage;                     /* Pointer to current image item. */
    char *par, *ptr, *op, *next;
    char *openbrace, *closebrace;
    char opstring[50];            /* Storage for binary operation string. */
    char tempstring[50];                         /* Size should be ample. */
    char tokenstring[STRINGSIZE];           /* Storage for current token. */
    char word[BUFSIZ];                     /* Local copy of input string. */
    int nstack;     /* Contains the current number of items on the stack. */
    int opending;     /* 1 if a binary operation is pending; 0 otherwise. */
    int sign;   /* 1 if result is positive; -1 for unary minus operation. */
    int nx, ny, indx;                      /* Integer function arguments. */
    float pmin, pmax;                         /* Real function arguments. */
    double result;                /* The final result of this evaluation. */
    double arg1, arg2;                     /* Binary operation arguments. */

    par = Strcpy(word, inword);         /* point par to the private copy. */
    wiplower(par);    /* par now is a lowercase copy of the input string. */

    nstack = 0;                              /* Stack is initially empty. */
    opending = 0;                        /* No pending binary operations. */
    while ((ptr = wipgettoken(tokenstring, par, &next)) != (char *)NULL) {

      *error = FALSE;
      if (*ptr == '"') {              /* A quoted string is skipped here. */
        par = next;                         /* Set pointer to next token. */
        continue;                                 /* Continue processing. */
      }

      sign = 1;
      if ((*ptr == '-') && (*(ptr+1) != Null)) {          /* Unary minus. */
        ptr++;
        sign = -1;
      }

      if (BRACE(*ptr)) {              /* Is this character an open brace? */
        ptr[Strlen(ptr) - 1] = Null;             /* Remove closing brace. */
        ptr++;                                /* Skip over opening brace. */
        result = wipevaluate(ptr, error);
      } else {                                       /* No opening brace. */

      /* ptr now points to either a simple word or something like f(...). */

        if (wipisnumber(ptr, &arg2)) { /* Token was just a simple number. */
          result = arg2;
        } else if (wipisop(ptr)) {
          if (nstack < 1) {
            wipoutput(stderr, "Operator appears out of order.\n");
            wipoutput(stderr, "Recheck command: %s\n", inword);
            goto ANYERROR;
          }
          opending = 1;                          /* Set a flag for later. */
          op = Strcpy(opstring, ptr);             /* Save this operation. */
          par = next;                       /* Set pointer to next token. */
          continue;                           /* Get the second argument. */
        } else if (wipisfunction(ptr)) {          /* A standard function? */
          closebrace = wipbracextract(ptr, &openbrace);
          if (openbrace == (char *)NULL) {
            wipoutput(stderr, "Are you sure [%s] is a standard function?\n",
              ptr);
            goto ANYERROR;
          }
          *openbrace++ = Null;   /* openbrace now points at the argument. */
          *closebrace = Null; /* ptr now looks at just the function name. */
          arg1 = wipevaluate(openbrace, error);      /* Get the argument. */
          if (*error == TRUE) goto ANYERROR;
          result = wipdofunc(ptr, arg1, error); /* Do the basic function. */
        } else if (wipisvecfunc(ptr)) {          /* Vector name function? */
          closebrace = wipbracextract(ptr, &openbrace);
          if (openbrace == (char *)NULL) {
            wipoutput(stderr, "Are you sure [%s] is a vector function?\n", ptr);
            goto ANYERROR;
          }
          *openbrace++ = Null;   /* openbrace now points at the argument. */
          *closebrace = Null; /* ptr now looks at just the function name. */
          (void)Strcpy(tempstring, openbrace);    /* Get a name to use... */
          Strcat(tempstring, "[1]");     /* ...to test if it is a vector. */
          if (wipisvec(tempstring) == 0)     /* Is operand a vector name? */
            goto ANYERROR;
          result = wipvecfunc(ptr, openbrace, error);
        } else if (wipisuserfunc(ptr)) {      /* A user defined function? */
          closebrace = wipbracextract(ptr, &openbrace);
          if (openbrace == (char *)NULL) {
            wipoutput(stderr, "Are you sure [%s] is a vector function?\n", ptr);
            goto ANYERROR;
          }
          *openbrace++ = Null;   /* openbrace now points at the argument. */
          *closebrace = Null; /* ptr now looks at just the function name. */
          arg1 = wipevaluate(openbrace, error);      /* Get the argument. */
          if (*error == TRUE) goto ANYERROR;
          result = wipuserfunc(ptr, arg1, error);
        } else if (wipisvar(ptr)) {              /* Simple user variable. */
          result = wipgetvar(ptr, error);
        } else if (wipisvec(ptr)) {       /* Simple user vector variable. */
          result = wipgetvec(ptr, error);
        } else if (*ptr == ESC) {        /* Simple user storage variable. */
            ptr++;         /* ptr now points the storage variable number. */
            result = wipevaluate(ptr, error);
            if (*error == TRUE) goto ANYERROR;
            indx = NINT(result);
            if ((indx < 0) || (indx >= 21)) goto ANYERROR;
            result = USERVAR[indx];
        } else if (wipimagexists(curimage = wipimcur("curimage"))) {
          if (Strcmp(ptr, "immin") == 0) {              /* Image minimum. */
            wipimageminmax(curimage, &pmin, &pmax, 0);
            result = pmin;
          } else if (Strcmp(ptr, "immax") == 0) {       /* Image maximum. */
            wipimageminmax(curimage, &pmin, &pmax, 0);
            result = pmax;
          } else if (Strcmp(ptr, "nx") == 0) {  /* Number of points in X. */
            wipimagenxy(curimage, &nx, &ny);
            result = nx;
          } else if (Strcmp(ptr, "ny") == 0) {  /* Number of points in Y. */
            wipimagenxy(curimage, &nx, &ny);
            result = ny;
          } else if (wipimhdprsnt(curimage, ptr)) {  /* Any image header. */
            if (wipimhdval(curimage, ptr, &result)) {
              wipoutput(stderr, "Trouble getting image item: %s\n", ptr);
              goto ANYERROR;
            }
          } else {                                /* No image item found. */
            wipoutput(stderr, "Unknown item: %s\n", ptr);
            goto ANYERROR;
          }
        } else {               /* Not a proper item and no image defined. */
          wipoutput(stderr, "Unknown item: %s\n", ptr);
          goto ANYERROR;
        }
      }

      if (*error == TRUE) goto ANYERROR;
      result *= sign;                        /* Include unary minus sign. */
      if ((nstack = push_stack(result)) < 1) {    /* Push onto the stack. */
        wipoutput(stderr, "No more room in internal stack.\n");
        goto ANYERROR;
      }

      if (opending == 1) {               /* Is a binary function pending? */
        if (nstack < 2) {                  /* Are there enough arguments? */
          wipoutput(stderr, "Improper operator format: [%s].\n", inword);
          goto ANYERROR;
        }
        if ((pop_stack(&arg2) < 0) || (pop_stack(&arg1) < 0)) {
          wipoutput(stderr, "Trouble reading stack.\n");
          goto ANYERROR;
        }
        result = wipdoop(op, arg1, arg2, error);      /* Binary function. */
        if (*error == TRUE) goto ANYERROR;
        if ((nstack = push_stack(result)) < 1) {          /* Save result. */
          wipoutput(stderr, "No more room in internal stack.\n");
          goto ANYERROR;
        }
      }
      opending = 0;               /* Remove binary function pending flag. */
      par = next;                           /* Set pointer to next token. */
    }

    *error = FALSE;
    if (pop_stack(&result) < 0) goto ANYERROR;
    return(result);

ANYERROR:
    clear_stack();
    *error = TRUE;
    return(0.0);
}

#ifdef PROTOTYPE
char *wipbracextract(Const char *inword, char **left)
#else
char *wipbracextract(inword, left)
Const char *inword;
char **left;
#endif /* PROTOTYPE */
{
    char *ptr;
    int level, chopen, chclose;
 
    *left = (char *)NULL;
    ptr = (char *)inword;
    while ((*ptr != Null) && (*ptr != '#')) {
      if ((*ptr == '(') || (*ptr == '[') || (*ptr == '{')) break;
      ptr++;
    }
    if (*ptr == Null) return((char *)NULL);

    *left = ptr;
    chopen = *ptr++;
    switch (chopen) {
      case '(': chclose = ')'; break;
      case '[': chclose = ']'; break;
      case '{': chclose = '}'; break;
      default: *left = (char *)NULL; return((char *)NULL);
    }
    level = 1;
    while ((*ptr != Null) && (*ptr != '#') && (level > 0)) {
      if (*ptr == chopen) level++;
      if (*ptr == chclose) level--;
      if (level) ptr++;
    }
    if (*ptr == Null) return((char *)NULL); /* End of string. */
    if (*ptr == '#') return((char *)NULL); /* Comment character. */
    return(ptr);
}

#ifdef TEST
/*
 * The remainder of the file provides code to test the callable
 * routines in this file.  Compile the code with -DTEST and link
 * it with libwip.a (necessary for the parsing routines).
 */
#define VERSION_ID "1.0"

#ifdef PROTOTYPE
main(int argc, char *argv[])
#else
main(argc, argv)
int argc;
char *argv[];
#endif /* PROTOTYPE */
{
    char *par;
    char intext[BUFSIZ];

    (void)printf("%s Version %s\n\n", argv[0], VERSION_ID);
    (void)printf("Test of echo and sub-routines.\n");

    while ((void)printf("Enter a string to echo: "),
           ((par = gets(intext)) != (char *)NULL)) {
      (void)printf("Calling echo with command: [%s].\n", par);
      wipecho(par);
    }

    (void)printf("TEST: Finished.\n");
}
#endif /* TEST */
