/*
	<readata.c>
	24jul90 jm  Original C code.
	13apr91 jm  Modified wipreadcol() to return EOF on failure.
	28feb92 jm  Increased dimension of character array "line" in
		    wipreadcol() from STRINGSIZE to BUFSIZ.
	02aug92 jm  Modified wipopenfile() to return status (void -> int)
		    rather than using a passed LOGICAL pointer.
	16aug92 jm  Added wipreadstr() routine.
	28sep92 jm  Modified wipreadcol() to test if a file exists only
		    if the column to read is greater than zero.  Added a
		    clear flag to remove any EOF's when data is read
		    from stdin; this should keep WIP from exiting.  Also
		    added a prompt when data is read from stdin.  Changed
		    temporary file usage from tmpfile() to tmpnam().
	12jan93 jm  Modified wipopenfile() to check that the opened file
		    does have something inside it.  Modified wipreadcol()
		    to save a copy of input line to use in error message.
        21jul93 jm  Modified for syntax change of wipinput().
	29mar95 jm  Modified LINE2 to allow an arbitrary maximum size.
		    See the comments below for more details.  Also
		    modified wipopenfile() to drop maxsize requirement.
	18nov97 jm  Modified wipreadcol() to warn user when number of
	            points is larger than (and truncated to) maxsize.
	16nov04 pjt add suggestion that's an FAQ
        14apr10 pjt no more prototypes

Routines:
void wiplines ARGS(( int first, int last ));
void wipgetlines ARGS(( int *first, int *last ));
int wipopenfile ARGS(( Const char *name ));
int wipreadcol ARGS(( float array[], int maxsize, int nc ));
char *wipreadstr ARGS(( int first, int second ));
*/

#include "wip.h"

/* Global variables for just this file */

static FILE *datafp = (FILE *)NULL;

/*
 *  LINE1 is the first data line; lines 1 to LINE1-1 are skipped
 *  before reading data.  LINE2 is the last line to read.  If LINE2
 *  is less than LINE1, it is treated as if LINE2 were infinite.
 */
static int LINE1;
static int LINE2;

/* Code */

void wiplines(int first, int last)
{
    LINE1 = first;
    LINE2 = last;
    return;
}

void wipgetlines(int *first, int *last)
{
    *first = LINE1;
    *last = LINE2;
    return;
}

/*  Returns 1 on error; 0 otherwise. */
int wipopenfile(Const char *name)
{
    char *ptr;
    char enddata[8];
    char line[STRINGSIZE];
    int inflag;
    static char *tmpName = (char *)NULL;   /* Save the temp file name. */

    if (datafp != (FILE *)NULL) {   /* datafp is file global variable. */
      Fclose(datafp);
      if (tmpName != (char *)NULL) {
        if (Remove(tmpName) != 0) {    /* Only remove temporary files. */
          wipoutput(stderr, "Could not remove temporary file [%s].\n", tmpName);
        }
        tmpName = (char *)NULL;
      }
    }

    ptr = Strncpy(line, name, STRINGSIZE);       /* Make a local copy. */
    line[STRINGSIZE-1] = Null;            /* Insure a Null at the end. */
    wiplower(ptr);                              /* Make it lower case. */
    /*
     *  According to ANSI-C, tmpfile() opens a temporary file in binary
     *  mode (wb+).  To avoid any reading conficts later, use tmpnam()
     *  and hope the system allows calls to remove() (which is ANSI-C).
     */
    if (Strcmp("stdin", ptr) != 0) {              /* An external file. */
      ptr = (char *)name;            /* Point to un-lowered file name. */
    } else {                                       /* Read from stdin. */
      tmpName = Tmpnam((char *)NULL);    /* Get a temporary file name. */
      if ((datafp = Fopen(tmpName, "w")) == (FILE *)NULL) {
        wipoutput(stderr, "Trouble opening a scratch file... \n");
        wipoutput(stderr, "\t...is the temporary file [%s] writeable?\n",
          tmpName);
        return 1;
      }

      wipoutput(stdout,
        "Enter data terminated by the word `enddata' on a line by itself.\n");

      while ((inflag = wipinput(stdin, "Data Input> ", line, STRINGSIZE))
                     != (int)EOF) {
        if ((inflag == (int)Null) || ((ptr = wipleading(line)) == (char *)NULL))
          continue;
        (void)Strncpy(enddata, ptr, 7);
        enddata[7] = Null;
        wiplower(enddata);
        if (Strncmp(enddata, "enddata", 7) == 0) break;
        wipoutput(datafp, "%s\n", line);
      }
      Clearerr(stdin);               /* Remove any EOF or error flags. */
      Fclose(datafp);      /* Close it from writing; open for reading. */
      ptr = tmpName;              /* Point to the temporary file name. */
    }

    if ((datafp = Fopen(ptr, "r")) == (FILE *)NULL) {
      wipoutput(stderr, "Error opening file [%s] for reading.\n", ptr);
      return 1;
    }

    if (filesize(datafp) <= 0L) {
      wipoutput(stderr, "No data within file [%s] to read.\n", ptr);
      return 1;
    }

    wiplines(1, 0);
    return 0;
}

/* Returns the number of elements read or EOF if an error occured. */
int wipreadcol(float array[], int maxsize, int nc)
{
    char *ptr;
    char line[BUFSIZ];
    char copy[BUFSIZ];
    int i, j, k, nxy;
    double vars;

    nxy = 0;
    if (nc < 0) {                /* Special load case; no file needed. */
      j = 1;
      i = ABS(nc) + 1;
      if (i > maxsize) {
        wipoutput(stderr,
          "Error: Number of requested points too large [%d > %d] for\n",
          i, maxsize);
        wipoutput(stderr,
          "\tinternal storage.  Setting number of points to maximum.\n");
        wipoutput(stderr,"Or try setting:    set maxarray %d\n",i);
        wipoutput(stderr,"in your ~/.wipinit file\n");
        i = maxsize;
      }
      while (j < i) array[nxy++] = j++;
      return(nxy);
    }

    if (datafp == (FILE *)NULL) {
      wipoutput(stderr, "Error: No data file has been opened yet!\n");
      return(EOF);
    }

    if (LINE1 > 1) { /* Skip lines up to LINE1. */
      for (j = 1; j < LINE1; j++) {
        if (wipinput(datafp, (char *)NULL, line, BUFSIZ) == (int)EOF)
          goto END_OF_FILE;
      }
    }

    if (LINE2 < LINE1) {
      i = maxsize;
    } else {
      i = LINE2 - LINE1 + 1;
      if (i > maxsize) {
        wipoutput(stderr,
          "Error: Number of requested points too large [%d > %d] for\n",
          i, maxsize);
        wipoutput(stderr,
          "\tinternal storage.  Setting number of points to maximum.\n");
        i = maxsize;
      }
    }
    for (j = 0; j < i; j++) {
      if ((k = wipinput(datafp, (char *)NULL, line, BUFSIZ)) == (int)EOF)
        goto END_OF_FILE;
      if (k == (int)Null) continue;               /* Skip blank lines. */
      if (nc == 0) {
        wipoutput(stdout, "%s\n", line);
      } else { /* (nc > 0) */
        if ((ptr = wipleading(line)) == (char *)NULL)
          continue;                                /* Skip comments. */
        (void)Strcpy(copy, line);
        for (k = 1; ((k < nc) && (wipparse(&ptr) != (char *)NULL)); k++)
          /* NULL */ ;
        if ((k < nc) || (wiparguments(&ptr, 1, &vars) != 1)) {
          wipoutput(stderr, "Error reading data on line number %d\n",
            (j + LINE1));
          wipoutput(stdout, "%s\n", copy);
          Rewind(datafp);
          return(EOF);
        }
        array[nxy++] = vars;
      }
    }
END_OF_FILE:
    Rewind(datafp);
    return(nxy);
}

/*
 *  Reads from the current file (set at LINE1), a string bounded
 *  (inclusively) by word "first" and "second".  If "first" is 0,
 *  the entire line is returned.  If "first" is > 0, this identifies
 *  the first word to load (1-based index).  The integer "second"
 *  identifies the length of the line when "first" is greater than 0.
 *  If "second" is 0, the remainder of the line is loaded.  If
 *  "second" is less than 0, the next abs("second") words are loaded
 *  (up to the end of the string).  If "second" is greater than 0,
 *  the next "second" words are loaded (up to the EOS).  If "second"
 *  is less than "first" (but positive), then only the "first" word
 *  is loaded.
 *
 *  Returns the requested string (as a dynamically allocated string)
 *  or NULL if an error occured.  The caller should free the string
 *  when finished with it.
 */
char *wipreadstr(int first, int second)
{
    char *par, *ptr;
    char line[BUFSIZ];
    char string[BUFSIZ];
    register int j;

    if (datafp == (FILE *)NULL) {
      wipoutput(stderr, "Error: No data file has been opened yet!\n");
      return((char *)NULL);
    }

    if (LINE1 > 1) {                        /* Skip lines up to LINE1. */
      for (j = 1; j < LINE1; j++)
        if (wipinput(datafp, (char *)NULL, line, BUFSIZ) == (int)EOF)
          goto END_OF_FILE;
    }

    if ((j = wipinput(datafp, (char *)NULL, line, BUFSIZ)) != (int)EOF) {
      Rewind(datafp);  /* Make sure to rewind file before any returns. */

      if (j == (int)Null)                             /* Empty string. */
        return((char *)NULL);

      ptr = line;
      if (first > 0) {                   /* Skip to the starting word. */
        for (j = 1; ((j < first) && (wipparse(&ptr) != (char *)NULL)); j++)
          /* NULL */ ;
        if (j < first) { /* ie. (wipparse(&ptr) == NULL). */
          wipoutput(stderr, "Line too short to read up to the [%d] word.\n",
            first);
          return((char *)NULL);
        }

        if (second > 0) {  /* Return a string up to the "second" word. */
          par = Strcpy(string, ptr);
          j = second - first + 1;                 /* Determine offset. */
          while ((j-- > 0) && (wipparse(&par) != (char *)NULL))
            /* NULL */ ;
          if (par != (char *)NULL) /* Terminate the original string... */
            ptr[par-string-1] = Null;           /* ...if words remain. */
        } else if (second < 0) { /* Return the next abs(second) words. */
          par = Strcpy(string, ptr);
          j = second;
          while ((j++ < 0) && (wipparse(&par) != (char *)NULL))
            /* NULL */ ;
          if (par != (char *)NULL) /* Terminate the original string... */
            ptr[par-string-1] = Null;           /* ...if words remain. */
        }
      }
      if (wiplenc(ptr) < 1)              /* Strip off trailing blanks. */
        return((char *)NULL);               /* Nothing left to return. */

      par = wipnewstring(ptr);    /* Get a dynamic copy of the string. */
      return(par);
    }
END_OF_FILE:
    Rewind(datafp);
    return((char *)NULL);
}
