/*
	<readinput.c>
	03aug90 jm  Original code.
	04may91 jm  Modified readinput to interatively call itself and
	            to make use of process.  Also changed playback so
		    that it calls maxecute.
	07jun91 jm  Added MACRO command (wipmacroinput).
	14jul91 jm  Modified wipreadinput and wipreadmac.
	19nov91 jm  Modified playback for change of call to maxecute.
	04may92 jm  Modified wipreadinput and wipmacroinput to not print
		    error messages for comment lines.
        03aug92 jm  Modified all the routines to return status
		    (void -> int) rather than using a passed LOGICAL pointer.
        21jul93 jm  Modified for syntax change of wipinput().
        25feb94 jm  Modified to permit system calls in input files and
		    skip them in macro files.
         9oct00 pjt no more PROTOTYPEs

Routines:
int wipreadinput ARGS(( Const char *rest ));
int wipwritemac ARGS(( Const char *file, Const char *macs ));
int wipplayback ARGS(( Const char *line ));
int wipreadmac ARGS(( Const char *rest ));
int wipmacroinput ARGS(( Const char *filename ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */
/* STRINGSIZE is a global parameter defined in wip.h. */

/* #define MAXLEVEL 5 see wip.h */

static int inplevel = -1;
static int inpmode[MAXLEVEL];
static char inpfile[MAXLEVEL][STRINGSIZE];

/* Code */

/*  Returns 0 if successful; 1 on errors. */
int wipreadinput(Const char *rest)
{
      char *ptr, *par;
      char string[STRINGSIZE], full[STRINGSIZE];
      int ret, inflag;
      FILE *fp;
      LOGICAL trash;
      COMMAND *vb;

      if ((inplevel + 1) >= MAXLEVEL) {
        wipoutput(stderr, "Input level nested too deep.\n");
        return(1);
      }

      inplevel++;
      inflag = Null;
      inpmode[inplevel] = 1;
      (void)Strcpy(inpfile[inplevel], rest);

/* Open the file. */
      if ((fp = Fopen(inpfile[inplevel], "r")) == (FILE *)NULL) {
        wipoutput(stderr, "Error opening input file %s.\n", inpfile[inplevel]);
        inplevel--;
        return(1);
      }
      trash = FALSE;

/* Retrieve the commands from the file. */
      while ((inflag = wipinput(fp, (char *)NULL, string, STRINGSIZE)) != EOF) {
        if (inflag == Null) continue;
        if ((ptr = wipleading(string)) == (char *)NULL)   /* Skip comments. */
          continue;
        if (*ptr == '!') {          /* Go ahead and honor any system calls. */
          if (!((inpmode[inplevel] == 2) && (trash == TRUE)))
            (void)wipcommand(ptr+1);               /* Ignore status return. */
          continue;
        }
        par = Strcpy(full, string);    /* Make a local copy of the command. */
        vb = wipinterpret(&ptr);        /* I need to know the command name. */

/* If trash is set, ignore everything but an END command; then reset trash. */
        if ((inpmode[inplevel] == 2) && (trash == TRUE)) {
          if ((vb != (COMMAND *)NULL) && (Strcmp(vb->name, "end") == 0)) {
            inpmode[inplevel] = 1;
            trash = FALSE;
          }
          continue;
        }

        if (vb == (COMMAND *)NULL) {   /* Print out bad command lines. */
          wipoutput(stderr, "Skipping text: [%s]\n", full);
          wipoutput(stderr, "from input file %s.\n", inpfile[inplevel]);
          break;
        }

/* Now execute the command. */
        ret = wipprocess(par, &inpmode[inplevel], FALSE);

/* Check return status for a couple of special cases. */
        if ((ret > 0) && (Strcmp(vb->name, "define") != 0)) {
          wipoutput(stderr, "Ignoring macro definition...\n");
          inpmode[inplevel] = 2; /* Doesn't get set by wipprocess. */
          trash = TRUE;
          continue;
        }

        if (ret < 0) continue;              /* ENDIF from an "if" command. */
        if (ret > 0) break;                 /* Error in executing command. */
        if (inpmode[inplevel] <= 0) break;  /* End of commands. */

      } /* End of while loop (no more info in current input file). */

      if ((inflag != EOF) && (inpmode[inplevel] > 0)) trash = TRUE;
      if (fp != (FILE *)NULL) Fclose(fp);
      inplevel--;

      return(trash == TRUE);
}

/*  Returns 0 if no error; 1 otherwise. */
int wipwritemac(Const char *file, Const char *macs)
{
    char *ptr, *mac;
    char buffer[STRINGSIZE];
    FILE *fp;
    COMMAND *vb;
    PCMACRO *pc;

    if ((fp = Fopen(file, "w+")) == (FILE *)NULL) {
      wipoutput(stderr, "Error opening output file [%s].\n", file);
      return(1);
    }

    if ((ptr = wipleading(macs)) == (char *)NULL)
      ptr = Strcpy(buffer, "buffer");
    else
      ptr = Strcpy(buffer, ptr);

    wiplower(ptr);

    while ((mac = wipparse(&ptr)) != (char *)NULL) {
      if (Strcmp(mac, "all") == 0) {
        for (vb = BUFFER->next; vb != (COMMAND *)NULL; vb = vb->next) {
          wipoutput(fp, "define %s\n", vb->name);
          for (pc = vb->pcmac; pc != (PCMACRO *)NULL; pc = pc->next) {
            wipoutput(fp, "%s\n", pc->line);
          }
          wipoutput(fp, "end\n\n");
        }
        vb = BUFFER;
        for (pc = vb->pcmac; pc != (PCMACRO *)NULL; pc = pc->next) {
          wipoutput(fp, "%s\n", pc->line);
        }
      } else if (Strcmp(mac, "macros") == 0) {
        for (vb = BUFFER->next; vb != (COMMAND *)NULL; vb = vb->next) {
          wipoutput(fp, "define %s\n", vb->name);
          for (pc = vb->pcmac; pc != (PCMACRO *)NULL; pc = pc->next) {
            wipoutput(fp, "%s\n", pc->line);
          }
          wipoutput(fp, "end\n\n");
        }
      } else {
        if ((vb = wipinterpret(&mac)) == (COMMAND *)NULL) {
          wipoutput(stderr,
            "Write `filename' `macro name' or `all' or 'macros'.\n");
          Fclose(fp);
          return(1);
        }
        if (vb != BUFFER) wipoutput(fp, "define %s\n", vb->name);
        for (pc = vb->pcmac; pc != (PCMACRO *)NULL; pc = pc->next) {
          wipoutput(fp, "%s\n", pc->line);
        }
        if (vb != BUFFER) wipoutput(fp, "end\n\n");
      }
    }

    Fclose(fp);

    return(0);
}

/*  Returns 0 if no error; 1 otherwise. */
int wipplayback(Const char *macro)
{
    char *ptr, *mac;
    char macstr[STRINGSIZE];
    COMMAND *comm;

    if ((ptr = wipleading(macro)) == (char *)NULL)
      mac = Strcpy(macstr, "buffer");
    else
      mac = Strcpy(macstr, ptr);

    if (((ptr = wipparse(&mac)) == (char *)NULL) ||
        ((comm = wipinterpret(&ptr)) == (COMMAND *)NULL)) {
      wipoutput(stderr, "Error interpreting macro [%s].\n", macro);
      wipoutput(stderr, "Playback `macro name'\n");
      return(1);
    }

    if ((comm == (COMMAND *)NULL) || (comm->macro != TRUE)) {
      wipoutput(stderr, "Playback `macro name'\n");
      return(1);
    }

    return(wipmaxecute(comm, 1, mac));
}

/*  Returns 0 if no error; 1 otherwise. */
int wipreadmac(Const char *rest)
{
    char *name;
    char *ptr, *par;
    char string[STRINGSIZE], full[STRINGSIZE];
    int inflag;
    FILE *fp;
    COMMAND *vb;
    LOGICAL inmacro;

    ptr = Strcpy(string, rest);
    if ((name = wipparse(&ptr)) == (char *)NULL) {
      wipoutput(stderr, "Read `filename'.\n");
      return(1);
    }

    if ((fp = Fopen(name, "r")) == (FILE *)NULL) {
      wipoutput(stderr, "Error opening input file %s.\n", name);
      return(1);
    }

    inmacro = FALSE;

/* Retrieve the commands from the file. */

    while ((inflag = wipinput(fp, (char *)NULL, string, STRINGSIZE)) != EOF) {
      if (inflag == Null) continue;               /* Skip blank lines. */
      ptr = Strcpy(full, string);                /* Make a local copy. */
      if ((par = wipparse(&ptr)) == (char *)NULL)    /* Skip comments. */
        continue;
      wiplower(par);
      if ((vb = find_command(HEAD, par, 0)) == (COMMAND *)EOF) {
        wipoutput(stderr, "Unknown command skipped: [%s]\n", string);
        wipoutput(stderr, "from input file %s.\n", name);
        continue;                      /* Warn about unknown commands. */
      }

/* If we are reading a macro definition, skip until end of definition. */

      if (inmacro == TRUE) {
        if ((vb != (COMMAND *)NULL) && (Strcmp(vb->name, "end") == 0))
          inmacro = FALSE;
        continue;
      }

/* If an ambiguous command name, warn the user and quit reading. */

      if (vb == (COMMAND *)NULL) {
        wipoutput(stderr, "Error interpreting line: [%s]\n", string);
        wipoutput(stderr, "from input file %s.\n", name);
        Fclose(fp);
        return(1);
      }

/* If user is defining a macro, warn them and set up skip flag. */

      if (Strcmp(vb->name, "define") == 0) {
        wipoutput(stderr, "Macro [%s] will be ignored.\n", ptr);
        wipoutput(stderr,
          "Use the command 'macro %s' to load this macro.\n", name);
        inmacro = TRUE;
        continue;
      }

/* Save the full command name and any arguments. */

      (void)Strcpy(string, vb->name);
      if (*ptr) {
        Strcat(string, " ");
        Strcat(string, ptr);
      }
      wipsaveline(BUFFER, string);

    } /* End of while loop (no more lines to read in current file). */

    Fclose(fp);

    return(0);
}

/*  Returns 0 if no error; 1 otherwise. */
int wipmacroinput(Const char *filename)
{
    char *ptr, *par;
    char string[STRINGSIZE], full[STRINGSIZE];
    int ret, inflag;
    int macromode;
    FILE *fp;
    LOGICAL trash, inmacro;
    COMMAND *vb;

    /* Open the file. */

    if ((fp = Fopen(filename, "r")) == (FILE *)NULL) {
      wipoutput(stderr, "Error opening macro file [%s].\n", filename);
      return(1);
    }

    inmacro = FALSE;
    trash = FALSE;
    macromode = 1;

    /* Retrieve the commands from the file. */

    while ((inflag = wipinput(fp, (char *)NULL, string, STRINGSIZE)) != EOF) {
      if (inflag == Null) continue;               /* Skip blank lines. */
      ptr = Strcpy(full, string);                /* Make a local copy. */
      if ((par = wipparse(&ptr)) == (char *)NULL)    /* Skip comments. */
        continue;
      wiplower(par);                            /* Make it lower case. */
      if (*par == '!')                        /* Skip system commands. */
        continue;
      if ((vb = find_command(HEAD, par, 0)) == (COMMAND *)EOF) {
        wipoutput(stderr, "Unknown command skipped: [%s]\n", string);
        wipoutput(stderr, "from input file %s.\n", filename);
        continue;                      /* Warn about unknown commands. */
      }
      par = string;

      if (inmacro != TRUE) { /* Loop until a "define" comand is found. */
        if ((vb == (COMMAND *)NULL) || (Strcmp(vb->name, "define") != 0))
          continue;
      }
    /*
     *  At this point, either the command is "define" or we are already
     *  in the "define" state.  If "trash" is set to TRUE, then ignore
     *  everything but the "END" command and then reset "trash".
     */
      if ((inmacro == TRUE) && (trash == TRUE)) {
        if ((vb != (COMMAND *)NULL) && (Strcmp(vb->name, "end") == 0)) {
          inmacro = FALSE;
          trash = FALSE;
        }
        continue;
      }

      if (vb == (COMMAND *)NULL) {     /* Print out bad command lines. */
        wipoutput(stderr, "Skipping text: [%s]\n", string);
        wipoutput(stderr, "from macro file %s.\n", filename);
        break;
      }

    /* Now execute the command and check for macro status. */

      ret = wipprocess(par, &macromode, FALSE);
      if ((inmacro != TRUE) && (macromode == 2)) inmacro = TRUE;
      if ((inmacro == TRUE) && (macromode == 1)) inmacro = FALSE;

    /* Check return status for a couple of special cases. */

      if ((ret > 0) && (Strcmp(vb->name, "define") == 0)) {
        inmacro = TRUE;
        trash = TRUE;
        continue;
      }

      if (ret < 0) continue;            /* ENDIF from an "if" command. */
      if (ret > 0) break;               /* Error in executing command. */
      if (macromode <= 0) break;                   /* End of commands. */

    }       /* End of while loop (no more info in current macro file). */

    if ((inflag != EOF) && (macromode > 1)) trash = TRUE;
    Fclose(fp);
    return(trash == TRUE);
}
