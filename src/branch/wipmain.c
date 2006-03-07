/*
	<wipmain.c>
	18jul90 jm  Original code.
	01may91 jm  Rewrote argv section; added a few command line
		    options; checked for ENDIF return from process.
	02nov91 jm  Moved wipvarinit() into wipinit().
	01aug92 jm  Moved usage() to top of routine.  Also modified to
		    account for call change to wipcommand().
        03aug92 jm. Modified EOF to mean reset mode to mode-1.
	28sep92 jm  Added a clear flag to remove any EOF's when
		    encountered on stdin; this should keep WIP from
		    exiting except when a ^D is found at mode=1 level.
	21jul93 jm  Modified for syntax change of wipinput().  Also
		    modified to call new function wipbegin().
	25feb94 jm  Added system call ability to input files.
	22aug94 jm  Added command line option -r which allows users to
		    NOT write the READLINE history file.
	13nov96 jm  Added command line options -- and -q.  Changed debug
		    mode to be a routine rather than a global variable.
	09jan98 jm  Added bogus MAIN__ routine for some Linux compilers.
	21jan98 jm  VMS reserves the readonly keyword, so it was changed
		    to justread.
         9oct00 pjt no PROTOTYPE #define


Routines:
static void usage ARGS(( Const char *filename ));
void main ARGS(( int argc, char *argv[] ));
*/

#include "version.h"
#include "wip.h"

/* Global variables for just this file */
/* Parameter STRINGSIZE is defined in wip.h. */

#define MAXFILES 10

/* Code */

static void usage(Const char *name)
{
    char options[STRINGSIZE];

    (void)Strcpy(options,
      "[-d device] [--] [-x] [-r] [-b] [-g] [-q] [-h] [-?]");
    wipoutput(stderr, "Usage: %s %s plotfile ... [-e cmd [args]]\n",
      name, options);
    wipoutput(stderr, "Options:\n");
    wipoutput(stderr, "-?\t\t This help.\n");
    wipoutput(stderr, "-h\t\t This help.\n");
    wipoutput(stderr,
      "-g\t\t Enter debug mode (generates additional messages).\n");
    wipoutput(stderr,
      "-r\t\t Don't write a READLINE history file when exiting.\n");
    wipoutput(stderr, "-b\t\t Disable READLINE functionality.\n");
    wipoutput(stderr, "-x\t\t Exit after reading all input plot files.\n");
    wipoutput(stderr, "-d device\t Set default plot device to \"device\".\n");
    wipoutput(stderr, "--\t\t Include stdin as an input plotfile.\n");
    wipoutput(stderr, "-q\t\t Run in quiet mode.\n");
    wipoutput(stderr, "-e cmd [args]\t WIP executes the command after ");
    wipoutput(stderr, "loading all input files.\n");
    wipoutput(stderr,
      "\t\t This must be the last entry on the command line and\n");
    wipoutput(stderr, "\t\t it may include arguments.\n");
}

int main(int argc, char *argv[])
{
    char *ptr, *comm;                                     /* Pointers. */
    char *ifiles[MAXFILES];   /* Pointers to command line input files. */
    char prompt[10];                 /* String for interactive prompt. */
    char line[STRINGSIZE];              /* String to hold input lines. */
    register int j, k;                               /* Loop counters. */
    int debugMode = 0;             /* Level of debug messages; 0 none. */
    int disableRL = 0;   /* Is 0 if READLINE can be used; 1 otherwise. */
    int doclose;                   /* Is 0 if the input file is stdin. */
    int interactive = 1;     /* Exit after command line args are done. */
    int lastmode;             /* Save previous interactive mode value. */
    int mode = 1;                    /* Start out in interactive mode. */
    int nfiles = 0;             /* Number of command line input files. */
    int precommand = 0;     /* Number of command and args on cmd line. */
    int preopen = 1;                       /* Open the default device. */
    int quietMode = 0;       /* Is 0 if info-msgs written; 1 if quiet. */
    int justread = 0;    /* Is 0 if writing history file; 1 otherwise. */
    FILE *fp;                       /* Pointer for command line files. */

/*  Initialize the command file structure. */
    if (wipinit()) exit(1);   /* Returns 1 only on error; 0 otherwise. */

    for (k = 1; k < argc;  k++) {        /* Check for any input flags. */
      ptr = argv[k];
      if (*ptr++ != '-') {
        if ((nfiles + 1) >= MAXFILES) continue; /* Silently skip file. */
        ifiles[nfiles++] = argv[k];
      } else {
        while(*ptr) {
          switch(*ptr++) {
            case 'b':
            case 'B':
              disableRL = 1;
              break;
            case 'd':
            case 'D':
              if (++k < argc)
                (void)wipsetstring("device", argv[k]);
              break;
            case 'e':
            case 'E':
              if (++k < argc) {       /* Delay processing until later. */
                precommand = k;
                k = argc;
              }
              break;
            case 'g':
            case 'G':
              debugMode = 1;
              break;
            case 'q':
            case 'Q':
              quietMode = 1;
              break;
            case 'r':
            case 'R':
              justread = 1;
              break;
            case 'x':
            case 'X':
              interactive = 0;
              break;
            case '0':
              preopen = 0;
              break;
            case '-':
              if ((nfiles + 1) >= MAXFILES)
                continue;                       /* Silently skip file. */
              ifiles[nfiles++] = "stdin";
              interactive = 0;                 /* This is forced here! */
              break;
            case 'h': /* Also help. */
            case '?':
              usage(argv[0]);
              exit(0);
              break;
            default:
              usage(argv[0]);
              break;
          } /* end of switch */
        } /* end of while */
      } /* end of if */
    } /* end of for */

    wipsetvar("debugmode", (double)debugMode);
    wipsetQuiet(quietMode);

    if (preopen) {         /* Give the user the default output device. */
      if ((comm = wipgetstring("device")) != (char *)NULL) {
        wipoutput(stdout, "Setting up default device [%s]\n", comm);
        SPrintf(line, "device %s", comm);
        if (wipprocess(line, &mode, FALSE))
          wipoutput(stderr, "Trouble opening default device: [%s]\n", comm);
      }
    }
    /*
     * See if there are any command arguments left.  If there are, use
     * them as input files.
     */
    for (k = 0; k < nfiles;  k++) {
      doclose = (strcmp(ifiles[k], "stdin") != 0);
      if (doclose) {
        if ((fp = Fopen(ifiles[k], "r")) == (FILE *)NULL)
          wipoutput(stderr, "Cannot open [%s]\n", ifiles[k]);
      } else {
        fp = stdin;
      }
      if (fp != (FILE *)NULL) {
        wipoutput(stdout, "Reading commands from [%s]....\n", ifiles[k]);
        while ((j = wipinput(fp, (char *)NULL, line, STRINGSIZE)) != EOF) {
          if ((j == Null) || ((comm = wipleading(line)) == (char *)NULL))
            continue;                  /* Skip comment or empty lines. */
          if (*comm == '!') {           /* Spool system command lines. */
            comm++;                 /* Advance past the '!' character. */
            if (wipcommand(comm) != 0) break;                /* Error. */
          } else {
            if (wipprocess(comm, &mode, TRUE) > 0) break;    /* Error. */
          }
          if (mode == 0) break;    /* Finished reading and processing. */
        }
        if (doclose)
          Fclose(fp);
      }
      if (mode == 0) break;
    }

    if (precommand > 0) {      /* Command present on the command line? */
      (void)Strcpy(line, "");
      for (k = precommand; k < argc;  k++) {  /* Get the cmd and args. */
        (void)Strcat(line, argv[k]);
        (void)Strcat(line, " ");
      }
      if (wipprocess(line, &mode, TRUE))
        wipoutput(stderr, "Trouble executing initial command.\n");
    }

    if (interactive == 0) {         /* Command line force close check. */
           /* This section should not matter for non-hardcopy devices. */
	/* ...but it does! - mwp 03/07/2006 */
/*
      if (mode > 0) {
        (void)Strcpy(line, "hardcopy");
        if (wipprocess(line, &mode, FALSE))
          wipoutput(stderr, "Trouble spooling command.\n");
      }
*/
      mode = 0;
    }

    if (mode > 0) {   /* Announce the interactive part of the program. */
      wipsetQuiet(0);    /* Quiet mode is turned off when interactive. */
      wipoutput(stdout, "Welcome to WIP %s\n\n", WIP_VERSION);
      wipbegin(disableRL, justread);
    }

/*  Begin the interactive loop. */
    lastmode = 0;                           /* Force the first prompt. */
    while (mode > 0) {
      if (mode != lastmode) {
        switch (mode) {
          case  1: (void)Strcpy(prompt, "WIP> ");           break;
          case  2: (void)Strcpy(prompt, "DEFINE> ");        break;
          case  3: (void)Strcpy(prompt, "INSERT> ");        break;
          default: (void)Strcpy(prompt, "WIP> "); mode = 1; break;
        }
        lastmode = mode;
      }

      if ((j = wipinput(stdin, prompt, line, STRINGSIZE)) == EOF) {
        switch (mode) {
          case  1: mode = 0; break;
          case  2: /* Fall through to case 3. */
          case  3: mode = 1; break;
          default: mode = 0; break;
        }
        Clearerr(stdin);
      } else if (j != Null) {
        if ((comm = wipleading(line)) != (char *)NULL) {
          if (*comm == '!') {           /* Spool system command lines. */
            comm++;                 /* Advance past the '!' character. */
            (void)wipcommand(comm);         /* Returned value ignored. */
          } else {
            (void)wipprocess(comm, &mode, TRUE);     /* Ignore return. */
          }
        }
      }
    } /* end of while (mode > 0) loop */

    wipclose();
    wipexit(0);                   /* Status 0 means successful finish. */
    return 0;
}

	/* for some fortran implementations */

#if defined(MAIN__) || defined(linux)
MAIN__() {
  wipoutput(stderr, "MAIN__ called; some Fortran inconsistency.\n");
  exit(1);
}

MAIN_() {
  wipoutput(stderr, "MAIN_ called; some Fortran inconsistency.\n");
  exit(1);
}

#endif

