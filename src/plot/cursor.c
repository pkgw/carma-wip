/*
	<cursor.c>
	24apr91 jm  Original code.
	28oct91 jm  Modified for new image variable.
	10nov93 jm  Modified image variable to new opaque pointer type.
	29mar95 jm  Modified dot command to use position.
	12oct95 jm  Modified fix routine to call wipgetcxy.

Routines:
void wipcursor ARGS(( float *cx, float *cy, LOGICAL keep ));
void wipfixcurs ARGS(( Const char *cmdname, char *rest, LOGICAL keep ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

/* Interactive cursor routine. */
#ifdef PROTOTYPE
void wipcursor(float *cx, float *cy, LOGICAL keep)
#else
void wipcursor(cx, cy, keep)
float *cx, *cy;
LOGICAL keep;
#endif /* PROTOTYPE */
{
      Void *curimage;
      char ch[2];
      char full[80];
      int firstime;
      int nx, ny, nsig;
      int mode, nsymb;
      float xp, yp;
      float intens;
      float tr[6];
      float **impic;
      double darg;
      LOGICAL error;
      LOGICAL save, dosave;

      xp = *cx;
      yp = *cy;
      dosave = keep;
      firstime = 1;

      if (dosave != TRUE) {  /* Special non-interactive case.  No looping. */
        if (cpgcurs(&xp, &yp, &ch[0]) != 0) {
          *cx = xp;
          *cy = yp;
        }
        return;
      }

      curimage = wipimcur("curimage");
      if (wipimagexists(curimage)) {
        wipimagenxy(curimage, &nx, &ny);
        wipgetr(tr);
        darg = wipgetvar("nsig", &error);
        nsig = (error == TRUE) ? 6 : NINT(darg);
      }

      darg = wipgetvar("cursmode", &error);
      mode = (error == TRUE) ? 0 : NINT(darg);

      darg = wipgetvec("pstyle[1]", &error);
      nsymb = (error == TRUE) ? 0 : NINT(darg);

      while (cpgband(mode, 1, *cx, *cy, &xp, &yp, &ch[0]) != 0) {
        ch[1] = Null;          /* Convert the character into a string. */
        save = FALSE;
        wiplower(ch);
        wipoutput(stdout, "%s\r", "                                         ");
        switch ((int)ch[0]) {    /* Some compilers require an integer. */
          case 'a':
            if (wipimagexists(curimage)) {
              impic = wipimagepic(curimage);
              intens = wipimval(impic, nx, ny, xp, yp, tr, &error);
              if (error != TRUE) {
                wipoutput(stdout, "Array value at (%f, %f)", xp, yp);
                wipoutput(stdout, " = %s\n", wipfpfmt(intens, nsig));
                break;
              }
            }
            wipoutput(stdout, "Cursor at (%f, %f)\n", xp, yp);
            break;
          case 'd':
            save = dosave;
            wipdraw(xp, yp);
            SPrintf(full, "draw %f %f", xp, yp);
            wipoutput(stdout, "Draw to (%f, %f)\r", xp, yp);
            break;
          case 'm':
            save = dosave;
            wipmove(xp, yp);
            SPrintf(full, "move %f %f", xp, yp);
            wipoutput(stdout, "Move to (%f, %f)\r", xp, yp);
            break;
          case 'p':
            save = dosave;
            cpgpt(1, &xp, &yp, nsymb);
            SPrintf(full, "dot %f %f", xp, yp);
            wipoutput(stdout, "Dot at (%f, %f)\r", xp, yp);
            break;
          case 'r':
            if (wipimagexists(curimage)) {
              impic = wipimagepic(curimage);
              intens = wipimval(impic, nx, ny, xp, yp, tr, &error);
              if (error != TRUE) {
                wipoutput(stdout, "Array value at (%s,", wipradecfmt(xp));
                wipoutput(stdout, " %s)", wipradecfmt(yp));
                wipoutput(stdout, " = %s\n", wipfpfmt(intens, nsig));
                break;
              }
            }
            wipoutput(stdout, "Cursor at (%f, %f)\n", xp, yp);
            break;
          case 's':
            dosave = (dosave == TRUE) ? FALSE : TRUE ;
            wipoutput(stdout, "%save commands.\r",
              (dosave == TRUE) ? "S" : "Don't s" );
            break;
          case 'x':
            *cx = xp;
            *cy = yp;
            goto EOLOOP;
          case '?':
            wipoutput(stdout, "\n");
            SPrintf(full, "cursor");
            if (wiphelp(full)) goto EOLOOP;
              break;
          default :
              break;
        } /* End of switch. */

        Fflush(stdout);
        if (save == TRUE) {
          wipsaveline(BUFFER, full);
          firstime = 0;
        }
        *cx = xp;
        *cy = yp;
      } /* End of while loop. */

EOLOOP:
      if ((firstime) && (dosave == TRUE)) {
        SPrintf(full, "move %f %f", *cx, *cy);
        wipsaveline(BUFFER, full);
      }
      return;
}

#ifdef PROTOTYPE
void wipfixcurs(Const char *cmdname, char *rest, LOGICAL keep)
#else
void wipfixcurs(cmdname, rest, keep)
Const char *cmdname;
char *rest;
LOGICAL keep;
#endif /* PROTOTYPE */
{
      char *par, *ptr;
      char save[STRINGSIZE];
      float xp, yp;
      double darg;

      /* Make some special allowances for the CURSOR command. */

      if (Strcmp(cmdname, "cursor")) return;

      wipgetcxy(&xp, &yp);

      ptr = Strcpy(save, rest);
      if ((par = wipparse(&ptr)) != (char *)NULL) {
        if (wiparguments(&par, 1, &darg) == 1) {
          xp = darg;
          if ((par = wipparse(&ptr)) != (char *)NULL) {
            if (wiparguments(&par, 1, &darg) == 1) {
              yp = darg;
            }
          }
        }
      }

      wipcursor(&xp, &yp, keep);
      wipmove(xp, yp);
      return;
}
