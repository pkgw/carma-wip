/*
	<show.c>
	14apr91 jm  Original code.
	28oct91 jm  Modified for new image variable.
	19jul91 jm  Changed show() to reflect change in variable syntax.
	28feb92 jm  Changed show() to allow selectable items to be displayed.
		    This meant that the call sequence was changed to
		    only include the "rest" string.  All else is retrievable.
	30sep93 jm  Added EPS to value in log10 expression when value < 1
		    in wipfpfmt() so that 0.1 & 0.2 both work out correctly.
	10nov93 jm  Modified image variable to new opaque pointer type.
	12oct95 jm  Modified to use wipgetcxy.
	24apr96 jm  Modified to use wipgetcir.

Routines:
void wipshow ARGS(( Const char *rest ));
char *wipfpfmt ARGS(( float arg, int nfig ));
char *wipifmt ARGS(( float arg ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/* Presents some general information to the user. */
#ifdef PROTOTYPE
void wipshow(Const char *rest)
#else
void wipshow(rest)
Const char *rest;
#endif /* PROTOTYPE */
{
    Void *curimage;
    char *ptr, *fmt;
    char user[4];                    /* Storage for storage register name. */
    char format[STRINGSIZE];             /* Local storage for rest string. */
    int doall = 1;    /* 1 if no format present in rest; 0 if rest exists. */
    int nfig, nsig;    /* Sets the number of significant figures to print. */
    int narg1, narg2;
    int maxsize, npts;
    float arg1, arg2;
    float *vecptr;                             /* Pointer to vector array. */
    float tr[6];
    double darg;
    LOGICAL error;

    ptr = Strcpy(format, rest);  /* Make a local copy of the input string. */
    if ((fmt = wipleading(ptr)) != (char *)NULL) {
      doall = 0;
      wiplower(ptr);
    }

    darg = wipgetvar("nsig", &error);         /* User specified formatter. */
    nsig = (error == TRUE) ? 3 : NINT(darg);    /* Number of sig. figures. */

/* Device/Printer items... */
    if ((doall) || (Strchr(fmt, 'd') != (char *)NULL)) {
      wipoutput(stdout, "device/type.............................. \"%s\"\n",
        wipgetstring("device"));

      wipoutput(stdout, "Print command............................ \"%s\"\n",
        wipgetstring("print"));

      if (((ptr = wipinquire("device")) == (char *)NULL) || (*ptr == '?'))
        ptr = wipinquire("dev/type");
      wipoutput(stdout, "device/type.............................. %s\n",
        ((ptr == (char *)NULL) ? "(none)" : ptr));

      ptr = wipgetstring("datafile");
      wipoutput(stdout, "Current data file............................ %s\n",
        ((*ptr == Null) ? "(none)" : ptr));

      wipgetlines(&narg1, &narg2);
      wipoutput(stdout,
        "Data file lines.............................. (%d-%d)", narg1, narg2);
      vecptr = wipvector("x", &maxsize, &npts);
      wipoutput(stdout, "   # of Points %d\n", npts);

      wipoutput(stdout, "\n");
    }

/* Coordinate systems... */
    if ((doall) || (Strchr(fmt, 'c') != (char *)NULL)) {
      nfig = nsig;
      darg = wipgetvar("x1", &error);
      arg1 = (error == TRUE) ? 0.0 : darg;
      darg = wipgetvar("x2", &error);
      arg2 = (error == TRUE) ? 0.0 : darg;
      wipoutput(stdout, " (X1, X2)  World (User) x-limits......... ");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      nfig = nsig;
      darg = wipgetvar("y1", &error);
      arg1 = (error == TRUE) ? 0.0 : darg;
      darg = wipgetvar("y2", &error);
      arg2 = (error == TRUE) ? 0.0 : darg;
      wipoutput(stdout, " (Y1, Y2)  World (User) y-limits......... ");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      nfig = nsig;
      darg = wipgetvar("vx1", &error);
      arg1 = (error == TRUE) ? 0.0 : darg;
      darg = wipgetvar("vx2", &error);
      arg2 = (error == TRUE) ? 0.0 : darg;
      wipoutput(stdout, "(VX1, VX2) Viewport (Device) x-limits.... ");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      nfig = nsig;
      darg = wipgetvar("vy1", &error);
      arg1 = (error == TRUE) ? 0.0 : darg;
      darg = wipgetvar("vy2", &error);
      arg2 = (error == TRUE) ? 0.0 : darg;
      wipoutput(stdout, "(VY1, VY2) Viewport (Device) y-limits.... ");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      nfig = nsig;
      wipgetcxy(&arg1, &arg2);
      wipoutput(stdout, " (CX, CY)  Current Pen Position.......... ");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      wipoutput(stdout, "\n");
    }

/* Graphical attributes. */
    if ((doall) || (Strchr(fmt, 'g') != (char *)NULL)) {
      nfig = 1;
      vecptr = wipvector("pstyle", &maxsize, &npts);
      arg1 = (npts > 0) ? vecptr[0] : 0.0;
      arg2 = npts;
      wipoutput(stdout, "(PSTYLE[1]) Symbol number......");
      wipoutput(stdout, "%s;            ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "# of Symbols..... %s\n", wipifmt(arg2));

      nfig = 2;
      darg = wipgetvar("expand", &error);
      arg1 = (error == TRUE) ? 0 : darg;
      wipoutput(stdout, "(EXPAND) Character size...%s;            ",
        wipfpfmt(arg1, nfig));
      darg = wipgetvar("fill", &error);
      narg1 = (error == TRUE) ? 0 : NINT(darg);
      wipoutput(stdout, "Fill type... %d (1: solid; 2: hollow)\n", narg1);

      darg = wipgetvar("lwidth", &error);
      narg1 = (error == TRUE) ? 0 : NINT(darg);
      wipoutput(stdout, "(LWIDTH) Line width....... %d;              ", narg1);
      darg = wipgetvar("lstyle", &error);
      narg1 = (error == TRUE) ? 0 : NINT(darg);
      wipoutput(stdout, "(LSTYLE)  Line style...... %d\n", narg1);

      nfig = 1;
      darg = wipgetvar("color", &error);
      narg1 = (error == TRUE) ? 0 : NINT(darg);
      wipoutput(stdout, "(COLOR) Current color..... %d;              ", narg1);
      darg = wipgetvar("angle", &error);
      arg1 = (error == TRUE) ? 0 : darg;
      wipoutput(stdout, "(ANGLE) Current angle.....%s\n", wipfpfmt(arg1, nfig));

      darg = wipgetvar("font", &error);
      narg1 = (error == TRUE) ? 0 : NINT(darg);
      wipoutput(stdout, "(FONT) Current font....... %d ", narg1);
      wipoutput(stdout, "(1: normal; 2: roman; 3: italic; 4: script)\n");

      wipgetcir(&narg1, &narg2);
      wipoutput(stdout,
        "(CMIN-CMAX)  Color index range............... %d-%d\n", narg1, narg2);

      wipoutput(stdout, "\n");
    }

    if ((doall) || (Strchr(fmt, 'b') != (char *)NULL)) {
      if ((ptr = wipgetstring("xbox")) == (char *)NULL)
        ptr = wipgetstring("boxdef");
      wipoutput(stdout, "(XBOX) Default argument for the X axis box... %s\n",
        ((*ptr == Null) ? "(none)" : ptr));

      if ((ptr = wipgetstring("ybox")) == (char *)NULL)
        ptr = wipgetstring("boxdef");
      wipoutput(stdout, "(YBOX) Default argument for the Y axis box... %s\n",
        ((*ptr == Null) ? "(none)" : ptr));

      nfig = nsig;
      darg = wipgetvar("xtick", &error);
      arg1 = (error == TRUE) ? 0 : darg;
      darg = wipgetvar("ytick", &error);
      arg2 = (error == TRUE) ? 0 : darg;
      wipoutput(stdout, "(XTICK, YTICK) Major tick mark interval......");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      darg = wipgetvar("nxsub", &error);
      arg1 = (error == TRUE) ? 0 : darg;
      darg = wipgetvar("nysub", &error);
      arg2 = (error == TRUE) ? 0 : darg;
      wipoutput(stdout, "(NXSUB, NYSUB) Number of minor tick marks....");
      wipoutput(stdout, "%s, ", wipifmt(arg1));
      wipoutput(stdout, "%s\n", wipifmt(arg2));

      nfig = nsig;
      darg = wipgetvar("xsubmar", &error);
      arg1 = (error == TRUE) ? 0 : darg;
      darg = wipgetvar("ysubmar", &error);
      arg2 = (error == TRUE) ? 0 : darg;
      wipoutput(stdout,
        "(XSUBMAR, YSUBMAR) Panel gap in character heights..........");
      wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
      wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

      wipoutput(stdout, "\n");
    }

    if ((doall) || (Strchr(fmt, 'r') != (char *)NULL)) {
      nfig = nsig;
      wipoutput(stdout, "(\\#) Register variables.......\n");
      user[0] = ESC;
      user[3] = Null;
      for (narg1 = 0; narg1 < 21; narg1++) {
        narg2 = (narg1 + 1) % 6;
        if (narg1 == 20) narg2 = 0;
        user[1] = '0' + (narg1 / 10);
        user[2] = '0' + (narg1 % 10);
        darg = wipevaluate(user, &error);
        arg1 = (error == TRUE) ? 0.0 : darg;
        wipoutput(stdout, " %2d: %s%c", narg1, wipfpfmt(arg1, nfig),
          (narg2 > 0) ? ',' : '\n');
      }

      wipoutput(stdout, "\n");
    }

    if ((doall) || (Strchr(fmt, 'i') != (char *)NULL)) {
      curimage = wipimcur("curimage");
      if (wipimagexists(curimage)) {
        wipoutput(stdout, "Current image name............... %s\n",
          wipgetstring("imagefile"));

        wipimagenxy(curimage, &narg1, &narg2);
        wipoutput(stdout, "(NX, NY) Image size... %dx%d;", narg1, narg2);
        arg1 = wipimplane(curimage);
        wipoutput(stdout, "                Plane # %s\n", wipifmt(arg1));

        nfig = nsig;
        wipimageminmax(curimage, &arg1, &arg2, 0);
        wipoutput(stdout, "(IMMIN, IMMAX) Image minimum/maximum.......... ");
        wipoutput(stdout, "%s, ", wipfpfmt(arg1, nfig));
        wipoutput(stdout, "%s\n", wipfpfmt(arg2, nfig));

        wipoutput(stdout, "(SUBX1-SUBX2, SUBY1-SUBY2) Subindex range..... ");
        darg = wipgetvar("subx1", &error);
        narg1 = (error == TRUE) ? 0 : NINT(darg);
        darg = wipgetvar("subx2", &error);
        narg2 = (error == TRUE) ? 0 : NINT(darg);
        wipoutput(stdout, "%d-%d, ", narg1, narg2);
        darg = wipgetvar("suby1", &error);
        narg1 = (error == TRUE) ? 0 : NINT(darg);
        darg = wipgetvar("suby2", &error);
        narg2 = (error == TRUE) ? 0 : NINT(darg);
        wipoutput(stdout, "%d-%d\n", narg1, narg2);

        ptr = wipgetstring("xheader");
        wipoutput(stdout, "(XHEADER, YHEADER) header default arguments... (%s",
          ((*ptr == Null) ? "(none)" : ptr));
        ptr = wipgetstring("yheader");
        wipoutput(stdout, ", %s)\n", ((*ptr == Null) ? "(none)" : ptr));

        nfig = nsig;
        wipoutput(stdout, "(TRANSFER[1-6]) Transfer function...");
        wipgetr(tr);
        for (narg1 = 0; narg1 < 6; narg1++)
          wipoutput(stdout, " %s%s", wipfpfmt(tr[narg1], nfig),
            (narg1 < 5) ? "," : "\n");

        nfig = nsig;
        vecptr = wipvector("levels", &maxsize, &npts);
        wipoutput(stdout, "Number of contour levels..... %d\n", npts);
        for (narg1 = 0; narg1 < npts; narg1++) {
          narg2 = (narg1 + 1) % 10;
          if (narg1 == npts - 1) narg2 = 0;
          wipoutput(stdout, " %s%s", wipfpfmt(vecptr[narg1], nfig),
            (narg2 > 0) ? "," : "\n");
        }
        if ((npts > 0) && (narg2 > 0)) wipoutput(stdout, "\n");
      }
    }
}

#ifdef PROTOTYPE
char *wipfpfmt(float arg, int nfig)
#else
char *wipfpfmt(arg, nfig)
float arg;
int nfig;
#endif /* PROTOTYPE */
{
      int ndec, iexpo, nchar;
      char *ptr, fspec;
      char fmt[20];
      static char field[80];
      float val, expo;

      val = ABS(arg);
      if (val == 0.0) {
        ndec = nfig;
        nchar = nfig + 3;
        fspec = 'f';
      } else if ((val < 1.0e6) && (val > 1.0e-4)) {
        if (val < 0.9) val += 1.0e-5;
        expo = LOG10(val);
        iexpo = expo;
        if (expo >= 0.0) iexpo++;
        ndec = MAX(nfig - iexpo, 0);
        nchar = ndec + 2 + MAX(iexpo, 0);
        fspec = 'f';
      } else {
        ndec = nfig - 1;
        nchar = nfig + 6;
        fspec = 'E';
      }
      SPrintf(fmt, "%%%d.%d%c", nchar, ndec, fspec);
      SPrintf(field, fmt, arg);
      ptr = field;
      return(ptr);
}

#ifdef PROTOTYPE
char *wipifmt(float arg)
#else
char *wipifmt(arg)
float arg;
#endif /* PROTOTYPE */
{
      int iexpo, nchar;
      char *ptr;
      char fmt[20];
      static char field[80];
      float val, expo;

      val = ABS(arg);
      if (val == 0.0) {
        nchar = 2;
      } else {
        expo = LOG10(val);
        iexpo = expo;
        nchar = iexpo + 2;
      }
      SPrintf(fmt, "%%%dd", nchar);
      SPrintf(field, fmt, NINT(arg));
      ptr = field;
      return(ptr);
}
