/*
	<help.c>
	04aug90 jm  Original code.
	31jul92 jm  Modified to use wipnewstring() command.  Also moved
		    add_to_help() up to the top of the file.
        03aug92 jm  Modified wiphelp() to return status (void -> int)
		    rather than using a passed LOGICAL pointer.
        21jul93 jm  Modified wiphelp() to handle different call syntax
		    to wipinput().
        13jun95 jm  Added a help message (at the end of the one line
		    list) to user if no arguments are given.

Routines:
static void add_to_help ARGS(( COMMAND *cmd, char *helpline ));
int wiphelp ARGS(( char *line ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global Variables needed just for this file */

static char dash[] = "---------------------------------------";

/* Code */

#ifdef PROTOTYPE
static void add_to_help(COMMAND *cmd, char *helpline)
#else
static void add_to_help(cmd, helpline)
COMMAND *cmd;
char *helpline;
#endif /* PROTOTYPE */
{
    PCMACRO *mac, *ptr;

    if (cmd == (COMMAND *)NULL) {
      wipoutput(stderr, "Define the command before assigning help to it.\n");
      return;
    }

    if ((mac = (PCMACRO *)Malloc(sizeof(PCMACRO))) == (PCMACRO *)NULL) {
      wipoutput(stderr, "Could not allocate memory for the help text.\n");
      return;
    }

    if ((mac->line = wipnewstring(helpline)) == (char *)NULL) {
      wipoutput(stderr, "Could not allocate memory for the help string.\n");
      Free(mac);
      return;
    }

    mac->next = (PCMACRO *)NULL;

    if (cmd->help == (PCMACRO *)NULL) {
      cmd->help = mac;
    } else {                          /* Find the end of the help list. */
      for (ptr = cmd->help; ptr->next != (PCMACRO *)NULL; ptr = ptr->next)
        /* NULL */ ;
      ptr->next = mac;
    }
    return;
}

/*  Returns 0 if successful; 1 on error. */
#ifdef PROTOTYPE
int wiphelp(char *line)
#else
int wiphelp(line)
char *line;
#endif /* PROTOTYPE */
{
    char *ptr, *pstr;
    char string[STRINGSIZE], save[STRINGSIZE];
    static char *lastname = (char *)NULL;
    int inflag;
    FILE *fp;
    COMMAND *vb;
    PCMACRO *pc;
    LOGICAL trash;

    if ((ptr = wipgetstring("helpfile")) == (char *)NULL) {
      wipoutput(stderr, "Trouble finding help file string variable.\n");
      return(1);
    }

    if (lastname == (char *)NULL) /* Force a mismatch below. */
      lastname = wipnewstring("?");

    if (Strcmp(lastname, ptr) != 0) {        /* Read the new help file. */
      wipoutput(stdout, "Reading contents of help file...\n");
      Free(lastname);                   /* Release storage of old name. */
      if ((fp = Fopen(ptr, "r")) == (FILE *)NULL) {
        wipoutput(stderr, "Trouble opening help file %s.\n", ptr);
        lastname = (char *)NULL;
        return(1);
      }

      lastname = wipnewstring(ptr);      /* Save the current file name. */
      if (lastname == (char *)NULL) {
        wipoutput(stderr,
          "Trouble allocating local storage for the help file name.\n");
        return(1);
      }

      trash = TRUE;
      while ((inflag = wipinput(fp, (char *)NULL, string, STRINGSIZE)) != EOF) {
        if (inflag == Null) continue;
        pstr = string;
        if ((*pstr) != '\t') {
          ptr = Strcpy(save, pstr);
          trash = FALSE;
          if ((vb = wipinterpret(&ptr)) == (COMMAND *)NULL) trash = TRUE;
        }
        if (trash == TRUE) {
          wipoutput(stderr, "Help ?? %s\n", pstr);
        } else {
          add_to_help(vb, pstr);
        }
      }
      Fclose(fp);
      wipoutput(stdout, "...done.\n");
    }

    if ((ptr = wipparse(&line)) == (char *)NULL) {
      for (vb = HEAD; vb != (COMMAND *)NULL; vb = vb->next) {
        if ((vb->macro != TRUE) && (vb->help != (PCMACRO *)NULL)) {
          wipoutput(stdout, " %s\n", (vb->help)->line);
        } else if (vb->macro != TRUE) {
          wipoutput(stdout, " %s (No help for this command)\n", vb->name);
        } else {
          wipoutput(stdout, " %s (macro)\n", vb->name);
        }
      }
      wipoutput(stderr, "Use `Help ?' or `Help command'; List a `macro'.\n");
    } else if (Strcmp(ptr, "?") == 0) {
      wipoutput(stdout, "%s%s\n", dash, dash);
      wipoutput(stdout, "Commands:\n");
      wipoutput(stdout, "%s%s\n", dash, dash);
      inflag = 1;
      for (vb = HEAD; vb != BUFFER; vb = vb->next) {
        if (vb->name == (char *)NULL) continue; /* No command name??? */
        inflag += 16;
        if (inflag > 80) {
          wipoutput(stdout, "%-15s\n", vb->name);
          inflag = 1;
        } else {
          wipoutput(stdout, "%-16s", vb->name);
        }
      }
      if (inflag > 1) wipoutput(stdout, "\n");
      wipoutput(stdout, "%s%s\n", dash, dash);
      wipoutput(stdout, "Macros:\n");
      wipoutput(stdout, "%s%s\n", dash, dash);
      inflag = 1;
      for (vb = BUFFER; vb != (COMMAND *)NULL; vb = vb->next) {
        if (vb->name == (char *)NULL) continue; /* No command name??? */
        inflag += 16;
        if (inflag > 80) {
          wipoutput(stdout, "%-15s\n", vb->name);
          inflag = 1;
        } else {
          wipoutput(stdout, "%-16s", vb->name);
        }
      }
      if (inflag > 1) wipoutput(stdout, "\n");
      wipoutput(stdout, "%s%s\n", dash, dash);
    } else {
      if (((vb = wipinterpret(&ptr)) == (COMMAND *)NULL) ||
          ((vb->macro == TRUE) && (Strcmp(vb->name, "buffer")))) {
        wipoutput(stderr, "Use `Help ?' or `Help command'; List a `macro'.\n");
        return(1);
      }
      wipoutput(stdout, "%s%s\n", dash, dash);
      for (pc = vb->help; pc != (PCMACRO *)NULL; pc = pc->next) {
        wipoutput(stdout, " %s\n", pc->line);
      }
      wipoutput(stdout, "%s%s\n", dash, dash);
    }

    return(0);
}
