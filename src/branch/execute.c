/*
	<wipexecute.c>
	21jul90 jm  Original C code.
	20oct90 jm  Added a switch branch on the first character of
		    the command name to speed up the command search.
	1/26/91 jm  Added mgohalf command.
	4/10/91 jm  Substantial code rewriting.
	04nov91 jm  Modified heavily for addition of stack variables.
	19nov91 jm  Moved LOGICAL error from call to wipexecute(); now
	            the function returns an int=1 on error.
	03aug92 jm  Modified many function calls to reflect their changes.
	15aug92 jm  Added aitoff and bar functions.
	24aug92 jm  Added "hmin" and "hmax" statics to keep track of
		    the last halftone command arguments.  Also added fit
		    command and modified points to pass evec for colors.
	12jan93 jm  Modified calls to wipreadcol() so that when no points
		    are read, the "npts" element is properly reset.
	01aug93 jm  Added command logarithm.
	05nov93 jm  Added the arrow command.
	10nov93 jm  Switched opaque pointers from char* to Void*.  Also
		    added goto BADCOMMAND statements to the control flow
		    commands (rather than leaving them blank).
	27may94 jm  Fixed contour command.  It was clipping the last
		    (MAXCONTLEVEL) contour level.
	22aug94 jm  Removed depreciated commands xlogarithm/ylogarithm.
		    Also modified call to wipbar() to reflect its changes.
	12dec94 jm  Modified quite a bit for change to 2.0.
	25jun96 jm  Added beam, plotfit, and range command parsing.
	02oct96 jm  Modified for change to beam inputs.  Also flipped
		    min/max in call to cpgimag().
	28oct96 jm  Modified the image command to handle smoothing and
		    the halftone command to handle heq.
	13nov96 jm  Replaced null command for cursor; it is needed even
		    though wipprocess handles the command.


Routines:
int wipexecute ARGS(( Const char *commandname, char *line ));
*/

#include "wip.h"

/* Global variables for just this file */
/* Parameter STRINGSIZE defined in wip.h. */

#define MAXARG       10

static LOGICAL termidle = FALSE;      /* T if terminal; F if hardcopy. */

/* Code */

/* WIPEXECUTE returns 0 if no error; 1 otherwise. */
#ifdef PROTOTYPE
int wipexecute(Const char *commandname, char *line)
#else
int wipexecute(commandname, line)
Const char *commandname;
char *line;
#endif /* PROTOTYPE */
{
      Void *curimage;                /* Pointer to current image.      */
      char *par, *ptr;
      char infile[STRINGSIZE];       /* Storage for a input file name. */
      int location, narg;
      int nx, ny, npts;
      int j, argc;
      int cmin, cmax;
      int sx1, sx2, sy1, sy2;        /* Subimage index range.          */
      int nxsub, nysub;              /* Used by ticksize.              */
      float xtick, ytick;            /* Used by ticksize.              */
      float xfloat, yfloat;
      float xmin, xmax, ymin, ymax;
      float tr[6];                   /* Array of TRANSFER elements.    */
      float contlev[MAXCONTLEVEL];   /* Array of scaled contour levels.*/
      float *xvec;                   /* Array of X column values.      */
      float *yvec;                   /* Array of Y column values.      */
      float *evec;                   /* Array of ERR column values.    */
      float *pvec;                   /* Array of PSTYLE column values. */
      float *clevels;                /* Array of contour levels.       */
      float **impic;                 /* Pointer to image data.         */
      static float hmin, hmax;       /* Min/Max of last halftone call. */
      double arg[MAXARG];            /* Array of command input values. */
      LOGICAL error;

      error = FALSE;

      if ((commandname == (char *)NULL) || (*commandname == Null)) goto MISTAKE;
      argc = wipcountwords(line);

      switch ((int)(*commandname)) {
        case 'a':
          if (Strcmp(commandname, "aitoff") == 0) {
            if (argc > 0) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xfloat = arg[0]; xvec = &xfloat;
              yfloat = arg[1]; yvec = &yfloat;
              npts = 1;
            } else {
              xvec = wipvector("x", &nx, &npts);
              yvec = wipvector("y", &nx, &ny);
              npts = MIN(npts, ny);
            }
            if (npts > 0) wipaitoff(npts, xvec, yvec);
          } else if (Strcmp(commandname, "angle") == 0) {
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              wipsetangle(arg[0]);
            } else {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              wipsetslope(arg[0], arg[1], arg[2], arg[3]);
            }
          } else if (Strcmp(commandname, "arc") == 0) {
            xfloat = 360.0; yfloat = 0.0;
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            } else if (argc == 3) {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              xfloat = arg[2];
            } else {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              xfloat = arg[2]; yfloat = arg[3];
            }
            xmax = arg[0]; ymax = arg[1];
            ymin = wipgetvar("angle", &error);
            if (error == TRUE) goto MISTAKE;
            if (wiparc(xmax, ymax, xfloat, ymin, yfloat)) goto MISTAKE;
          } else if (Strcmp(commandname, "arrow") == 0) {
            xfloat = 45.0; yfloat = 0.3;
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            } else if (argc == 3) {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              xfloat = arg[2];
            } else {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              xfloat = arg[2]; yfloat = arg[3];
            }
            xmax = arg[0]; ymax = arg[1];
            wiparrow(xmax, ymax, xfloat, yfloat);
          } else if (Strcmp(commandname, "ask") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = (NINT(arg[0]) == 1) ? 1 : 0 ; /* Allow 1 or 0 only. */
            cpgask(narg);
          } else if (Strcmp(commandname, "autolevs") == 0) {
            curimage = wipimcur("curimage");
            if (wipimagexists(curimage) == 0) {
              wipoutput(stderr, "You must specify an image first!\n");
              goto MISTAKE;
            }
            wipimageminmax(curimage, &ymin, &ymax, 0);
            clevels = wipvector("levels", &nx, &ny);
            if (nx < 1) goto MISTAKE;
            ny = wipautolevs(line, clevels, nx, ymin, ymax);
            if (ny <= 0) goto MISTAKE;
            if (wipvectornpts("levels", ny)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'b':
          if (Strcmp(commandname, "bar") == 0) {
            narg = 1;                         /* Use the xfloat value. */
            xfloat = yfloat = 0;
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = 0;                 /* Don't use the xfloat value. */
            } else if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xfloat = arg[1];
            } else {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              xfloat = arg[1]; yfloat = arg[2];
            }
            location = NINT(arg[0]);
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) goto MISTAKE;
            evec = wipvector("err", &nx, &ny);
            if (wipbar(npts, xvec, yvec, ny, evec, location, narg, xfloat, yfloat))
              goto MISTAKE;
          } else if (Strcmp(commandname, "beam") == 0) {
            argc = (argc < 3) ? 3 : ((argc > 8) ? 8 : argc);
            if (wiparguments(&line, argc, arg) != argc) goto MISTAKE;
            narg = 15;                    /* Default fill color index. */
            xmin = -1;             /* Default is to do RA/Dec scaling. */
            ny = 0;            /* Default is to draw the bg rectangle. */
            xfloat = 0;                       /* Default is no offset. */
            switch (argc) {               /* Parse optional arguments. */
              case 8:    /* falls through */
                ny = NINT(arg[7]);          /* User specifies bg rect. */
              case 7:    /* falls through */
                narg = NINT(arg[6]);     /* User specifies fill color. */
              case 6:    /* falls through */
                xmin = arg[5];              /* User specifies scaling. */
              case 5:    /* falls through */
                yfloat = arg[4];     /* User specifies an offset in y. */
              case 4:
                xfloat = arg[3];     /* User specifies an offset in x. */
                break;
              default:                         /* No options supplied. */
                break;
            }
            if (argc < 5) yfloat = xfloat;       /* Default is y == x. */
            xmax = arg[0]; ymax = arg[1]; ymin = arg[2];
            if (wipbeam(xmax, ymax, ymin, xfloat, yfloat, narg, xmin, ny))
              goto MISTAKE;
          } else if (Strcmp(commandname, "bgci") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            wipsetbgci(narg);
          } else if (Strcmp(commandname, "bin") == 0) {
            narg = 1;                   /* Center bins on the X value. */
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);        /* User specifies centering. */
            } else if (argc > 1) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              narg = NINT(arg[0]);        /* User specifies centering. */
              xfloat = arg[1];                           /* Gap value. */
            }
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts <= 0) goto MISTAKE;
            if (argc > 1) {                      /* Use hline routine. */
              if (wiphline(npts, xvec, yvec, xfloat, narg)) goto MISTAKE;
            } else {                                    /* Use cpgbin. */
              cpgbin(npts, xvec, yvec, narg);
            }
            wipmove(xvec[npts-1], yvec[npts-1]);
          } else if (Strcmp(commandname, "box") == 0) {
            par = wipgetstring("xbox");
            ptr = wipgetstring("ybox");
            if (argc == 1) {
              par = wipparse(&line);
            } else if (argc > 1) {
              par = wipparse(&line);
              ptr = wipparse(&line);
            }
            wipgetick(&xtick, &nxsub, &ytick, &nysub);
            cpgtbox(par, xtick, nxsub, ptr, ytick, nysub);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'c':
          if (Strcmp(commandname, "color") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            wipcolor(narg);
          } else if (Strcmp(commandname, "connect") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts > 0) {
              cpgline(npts, xvec, yvec);
              wipmove(xvec[npts-1], yvec[npts-1]);
            }
          } else if (Strcmp(commandname, "contour") == 0) {
            curimage = wipimcur("curimage");
            if (wipimagexists(curimage) == 0) {
              wipoutput(stderr, "You must specify an image first!\n");
              goto MISTAKE;
            }
            wipimageminmax(curimage, &ymin, &ymax, 0);
            if ((par = wipgetstring("levtype")) == (char *)NULL) {
              wipoutput(stderr, "Can't get string variable `levtype'\n");
              goto MISTAKE;
            }
            arg[0] = wipgetvar("slevel", &error);
            xfloat = (error == TRUE) ? 0.0 : arg[0];
            clevels = wipvector("levels", &nx, &npts);
            if ((nx < 1) || (npts > MAXCONTLEVEL)) goto MISTAKE;
            for (nx = 0; nx < npts; nx++) contlev[nx] = clevels[nx];
            npts = wipscalevels(par, xfloat, ymin, ymax, contlev, npts);
            if (npts <= 0) {
              wipoutput(stderr, "You must specify contour levels first!\n");
              goto MISTAKE;
            }
            arg[0] = wipgetvar("nsig", &error);
            narg = (error == TRUE) ? 2 : NINT(arg[0]); error = FALSE;
            par = "s";                  /* Set up the default options. */
            if (argc > 0) {
              if ((par = wipparse(&line)) == (char *)NULL) goto MISTAKE;
              wiplower(par);
            }
            wipimagenxy(curimage, &nx, &ny);
            wipgetsub(&sx1, &sx2, &sy1, &sy2);
            wipgetr(tr);
            if (*par == '-') {        /* Handle the line style option. */
              npts = -npts;
              par++;
            }
            impic = wipimagepic(curimage);
            switch ((int)(*par)) {
              case 'b':
                xfloat = 0.0;
                if (argc > 1) {
                  if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
                  xfloat = arg[0];
                }
                cpgconb(*impic, nx, ny, sx1, sx2, sy1, sy2,
                      contlev, npts, tr, xfloat);
                break;
              case 'l':
                nxsub = 16; nysub = 8; /* maybe 20, 10? */
                if (argc == 2) {
                  if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
                  nxsub = NINT(arg[0]);
                } else if (argc > 2) {
                  if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
                  nxsub = NINT(arg[0]);
                  nysub = NINT(arg[1]);
                }
                cpgbbuf();
                cpgcont(*impic, nx, ny, sx1, sx2, sy1, sy2, contlev, npts, tr);
                for (j = 0; j < npts; j++) {
                  ptr = wipfpfmt(contlev[j], narg);
                  cpgconl(*impic, nx, ny, sx1, sx2, sy1, sy2,
                      contlev[j], tr, ptr, nxsub, nysub);
                }
                cpgebuf();
                break;
              case 's':
                cpgcons(*impic, nx, ny, sx1, sx2, sy1, sy2, contlev, npts, tr);
                break;
              case 't':
                cpgcont(*impic, nx, ny, sx1, sx2, sy1, sy2, contlev, npts, tr);
                break;
              default:
                wipoutput(stderr, "Incorrect type of contour selected.\n");
                goto MISTAKE;
            }
          } else if (Strcmp(commandname, "cursor") == 0) {
	    /* NULL */ ;    /*  This is handled by WIPPROCESS now....  */
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'd':
          if (Strcmp(commandname, "data") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              (void)Strcpy(infile, "stdin");
            } else {
              (void)Strcpy(infile, par);
            }
            if (wipopenfile(infile)) goto MISTAKE;
            if (wipsetstring("datafile", infile)) goto MISTAKE;
          } else if (Strcmp(commandname, "define") == 0) {
            goto BADCOMMAND;
          } else if (Strcmp(commandname, "delete") == 0) {
            if (wipdelete(line)) goto MISTAKE;
          } else if (Strcmp(commandname, "device") == 0) {
            if (wipdevice(line)) goto MISTAKE;
            nx = wipishard();          /* 1 if hardcopy; 0 if terminal. */
            termidle = (nx) ? FALSE : TRUE ;
            if (wipsetvar("hardcopy", (double)nx)) goto MISTAKE;
          } else if (Strcmp(commandname, "dot") == 0) {
            if (argc > 0) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xfloat = arg[0];
              yfloat = arg[1];
            } else {
              wipgetcxy(&xfloat, &yfloat);
            }
            arg[0] = wipgetvec("pstyle[1]", &error);
            ny = (error == TRUE) ? 0 : NINT(arg[0]); error = FALSE;
            cpgpt(1, &xfloat, &yfloat, ny);
          } else if (Strcmp(commandname, "draw") == 0) {
            if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            xfloat = arg[0];
            yfloat = arg[1];
            wipdraw(xfloat, yfloat);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'e':
          if (Strcmp(commandname, "echo") == 0) {
            wipecho(line);
          } else if (Strcmp(commandname, "ecolumn") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            evec = wipvector("err", &nx, &npts);
            if (nx < 1) goto MISTAKE;
            if ((nx = wipreadcol(evec, nx, narg)) == EOF) goto MISTAKE;
            if (wipvectornpts("err", nx)) goto MISTAKE;
          } else if (Strcmp(commandname, "end") == 0) {
            goto BADCOMMAND;
          } else if (Strcmp(commandname, "environment") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            wiprange(npts, xvec, &xmin, &xmax);
            wiprange(npts, yvec, &ymin, &ymax);
            nx = 0; ny = 0;
            if (argc > 0) {
              argc = (argc < 4) ? 4 : ((argc > 6) ? 6 : argc);
              if (wiparguments(&line, argc, arg) != argc) goto MISTAKE;
              switch (argc) {             /* Parse optional arguments. */
                case 6:    /* falls through */
                  ny = NINT(arg[5]);  /* User specifies axis argument. */
                case 5:
                  nx = NINT(arg[4]);  /* User specifies justification. */
                  break;
                default:                 /* No other options supplied. */
                  break;
              }
              if (arg[0] != arg[1]) {
                xmin = arg[0];
                xmax = arg[1];
              }
              if (arg[2] != arg[3]) {
                ymin = arg[2];
                ymax = arg[3];
              }
            }
            cpgenv(xmin, xmax, ymin, ymax, nx, ny);
            wiplimits();
          } else if (Strcmp(commandname, "erase") == 0) {
            cpgpage();
          } else if (Strcmp(commandname, "errorbar") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            location = NINT(arg[0]);
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            evec = wipvector("err", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            if (wiperrorbar(location, xvec, yvec, evec, npts)) goto MISTAKE;
          } else if (Strcmp(commandname, "etxt") == 0) {
            cpgetxt();
          } else if (Strcmp(commandname, "expand") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            wipexpand(arg[0]);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'f':
          if (Strcmp(commandname, "fill") == 0) {
            argc = (argc < 1) ? 1 : ((argc > 4) ? 4 : argc);
            if (wiparguments(&line, argc, arg) != argc) goto MISTAKE;
            xfloat = 45.0;                     /* Default hatch angle. */
            yfloat = 1.0;               /* Default hatch line spacing. */
            xmin = 0.0;                 /* Default is no phase offset. */
            switch (argc) {               /* Parse optional arguments. */
              case 4:    /* falls through */
                xmin = arg[3];         /* User specifies phase offset. */
              case 3:    /* falls through */
                yfloat = arg[2];            /* User specifies spacing. */
              case 2:
                xfloat = arg[1];        /* User specifies hatch angle. */
                break;
              default:                         /* No options supplied. */
                break;
            }
            narg = NINT(arg[0]);
            if (argc > 1) cpgshs(xfloat, yfloat, xmin);
            wipfill(narg);
          } else if (Strcmp(commandname, "fit") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            evec = wipvector("err", &nx, &ny);
            if (ny < 1) ny = 0;
            if (wipfit(line, npts, xvec, yvec, ny, evec)) goto MISTAKE;
          } else if (Strcmp(commandname, "font") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            wipfont(narg);
          } else if (Strcmp(commandname, "free") == 0) {
            if (wipfreeitem(line)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'g':
          if (Strcmp(commandname, "globe") == 0) {
            nx = 5; ny = 3;
            if (argc > 0) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              nx = NINT(arg[0]);
              ny = NINT(arg[1]);
            }
            wipaitoffgrid(nx, ny);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'h':
          if (Strcmp(commandname, "halftone") == 0) {
            curimage = wipimcur("curimage");
            if (wipimagexists(curimage) == 0) {
              wipoutput(stderr, "You must specify an image first!\n");
              goto MISTAKE;
            }
            wipimagenxy(curimage, &nx, &ny);
                                    /* bg     fg */
            wipimageminmax(curimage, &ymin, &ymax, 0);
            wipgetsub(&sx1, &sx2, &sy1, &sy2);
            wipgetr(tr);
            impic = wipimagepic(curimage);
            wipgetcir(&cmin, &cmax);
            narg = -1;
            xfloat = -99.0;
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              ymin = arg[0];
              ymax = arg[1];
            } else if (argc == 3) {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              ymin = arg[0];
              ymax = arg[1];
              narg = NINT(arg[2]);
            } else if (argc > 3) {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              ymin = arg[0];
              ymax = arg[1];
              narg = NINT(arg[2]);
              xfloat = arg[3];
            }
            if (narg <= 0) narg = cmax - cmin + 1;
            hmin = ymin;
            hmax = ymax;
            if (argc > 2)                /* Only do this if requested. */
              wipheq(nx, ny, impic, sx1, sx2, sy1, sy2, xfloat, ymin, ymax, narg);

            if ((cmin + 1) < cmax)
              cpgimag(*impic, nx, ny, sx1, sx2, sy1, sy2, ymin, ymax, tr);
            else
              cpggray(*impic, nx, ny, sx1, sx2, sy1, sy2, ymax, ymin, tr);
          } else if (Strcmp(commandname, "hardcopy") == 0) {
            if (termidle == FALSE) {
              if ((ptr = wipinquire("file")) == (char *)NULL) {
                wipoutput(stderr, "Trouble getting plot file name.\n");
                goto MISTAKE;
              }
              wipclose();
              if (wipspool(ptr)) goto MISTAKE;
            }
          } else if (Strcmp(commandname, "header") == 0) {
            par = wipgetstring("xheader");
            ptr = wipgetstring("yheader");
            if (argc == 1) {
              par = wipparse(&line);
              ptr = par;
            } else if (argc > 1) {
              par = wipparse(&line);
              ptr = wipparse(&line);
            }
            wipgetsub(&sx1, &sx2, &sy1, &sy2);
            if (wipheader(sx1, sy1, sx2, sy2, par, ptr)) goto MISTAKE;
          } else if (Strcmp(commandname, "help") == 0) {
            if (wiphelp(line)) goto MISTAKE;
          } else if (Strcmp(commandname, "hi2d") == 0) {
            curimage = wipimcur("curimage");
            if (wipimagexists(curimage) == 0) {
              wipoutput(stderr, "You must specify an image first!\n");
              goto MISTAKE;
            }
            wipimagenxy(curimage, &nx, &ny);
            wipgetsub(&sx1, &sx2, &sy1, &sy2);
            location = 0; /* no initial slant to successive y-elements. */
            narg = 1; /* center bins on x value. */
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            } else if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              location = NINT(arg[1]);
            } else {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              location = NINT(arg[1]);
              narg = NINT(arg[2]);
            }
            ymax = arg[0];  /* Required bias value. */
            nxsub = sx2 - sx1 + 1;
            xvec = vector(nxsub);
            yvec = vector(nxsub);
            if ((xvec == (float *)NULL) || (yvec == (float *)NULL)) {
              if (xvec) freevector(xvec);
              if (yvec) freevector(yvec);
              wipoutput(stderr, "Trouble allocating work arrays.\n");
              goto MISTAKE;
            }
            for (j = 0; j < nxsub; j++) xvec[j] = sx1 + j;
            impic = wipimagepic(curimage);
            cpghi2d(*impic, nx, ny, sx1, sx2, sy1, sy2,
                     xvec, location, ymax, narg, yvec);
            freevector(xvec);
            freevector(yvec);
          } else if (Strcmp(commandname, "histogram") == 0) {
            xvec = wipvector("x", &nx, &npts);
            if (npts < 1) goto MISTAKE;
            wiprange(npts, xvec, &xmin, &xmax);
            narg = 5; /* Number of bins. */
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xmin = arg[0]; xmax = arg[1];
            } else if (argc > 2) {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              xmin = arg[0]; xmax = arg[1];
              narg = NINT(arg[2]);
            }
            ny = 1;   /* PGFLAG = 1 ==> no pgenv called. */
            cpghist(npts, xvec, xmin, xmax, narg, ny);
            wipgetcxy(&xfloat, &yfloat);
            wipmove(xvec[npts-1], yfloat);
          } else if (Strcmp(commandname, "hls") == 0) {
            if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
            narg = NINT(arg[0]);
            xmin = arg[1]; xmax = arg[2]; ymin = arg[3];
            cpgshls(narg, xmin, xmax, ymin);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'i':
          if (Strcmp(commandname, "id") == 0) {
            cpgiden();
          } else if (Strcmp(commandname, "if") == 0) {
            goto BADCOMMAND;
          } else if (Strcmp(commandname, "image") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Input image filename\n");
              goto MISTAKE;
            }
            argc--;
            narg = 1;
            xfloat = -99.0;
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);
            } else if (argc > 1) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              narg = NINT(arg[0]);
              xfloat = arg[1];
            }
            wipfreeimage("curimage");
            if ((curimage = wipimage(par, narg, xfloat)) == (char *)NULL)
              goto MISTAKE;
            wipimsetcur("curimage", curimage);
            if (wipsetstring("imagefile", par)) error = TRUE;
            if (argc > 2) {                       /* Smoothing needed? */
              impic = wipimagepic(curimage);
              wipimagenxy(curimage, &nx, &ny);
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);
              wipsmooth(impic, nx, ny, narg, xfloat);
            }
          } else if (Strcmp(commandname, "initialize") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Initialize requires a vector name.\n");
              goto MISTAKE;
            }
            if (wiparguments(&line, 1, arg) != 1) {
              wipoutput(stderr, "Initialize requires a vector size.\n");
              goto MISTAKE;
            }
            narg = NINT(arg[0]);
            ptr = wipleading(line);
            if (wipvectorinit(par, narg, ptr)) goto MISTAKE;
          } else if (Strcmp(commandname, "input") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Input filename\n");
              goto MISTAKE;
            }
            if (wipreadinput(par)) goto MISTAKE;
          } else if (Strcmp(commandname, "insert") == 0) {
            goto BADCOMMAND;
          } else if (Strcmp(commandname, "itf") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            wipsetitf(narg);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'l':
          if (Strcmp(commandname, "label") == 0) {
            if (argc < 1) {
              wipoutput(stderr, "A label (string) is required.\n");
              goto MISTAKE;
            }
            wipputlabel(line, 0.0);
          } else if (Strcmp(commandname, "lcur") == 0) {
            xvec = wipvector("x", &nx, &npts); j = nx;
            yvec = wipvector("y", &nx, &ny); nx = MIN(nx, j);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            if (argc > 0) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              npts = NINT(arg[0]);
            }
            cpglcur(nx, &npts, xvec, yvec);
            if (npts <= 0) goto MISTAKE;
            if ((wipvectornpts("x", npts)) ||
                (wipvectornpts("y", npts))) goto MISTAKE;
          } else if (Strcmp(commandname, "ldev") == 0) {
            cpgldev();
          } else if (Strcmp(commandname, "levels") == 0) {
            clevels = wipvector("levels", &nx, &npts);
            if (nx < 1) goto MISTAKE;
            npts = wiplevels(line, clevels, nx);
            if (npts <= 0) goto MISTAKE;
            if (wipvectornpts("levels", npts)) goto MISTAKE;
          } else if (Strcmp(commandname, "limits") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            wiprange(npts, xvec, &xmin, &xmax);
            wiprange(npts, yvec, &ymin, &ymax);
            if (argc > 0) {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              if (arg[0] != arg[1]) {
                xmin = arg[0];
                xmax = arg[1];
              }
              if (arg[2] != arg[3]) {
                ymin = arg[2];
                ymax = arg[3];
              }
            }
            cpgswin(xmin, xmax, ymin, ymax);
            wiplimits();
          } else if (Strcmp(commandname, "lines") == 0) {
            if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            nx = NINT(arg[0]);
            ny = NINT(arg[1]);
            wiplines(nx, ny);
          } else if (Strcmp(commandname, "list") == 0) {
            nx = 1; ny = -1;
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              nx = NINT(arg[0]);
            } else if (argc > 1) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              nx = NINT(arg[0]);
              ny = NINT(arg[1]);
            }
            if (wiplist(nx, ny, line)) goto MISTAKE;
          } else if (Strcmp(commandname, "logarithm") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Input name of logarithm item.\n");
              goto MISTAKE;
            }
            xfloat = 1.0;
            if (argc > 1) {
              xfloat = wipevaluate(line, &error);
              if (error == TRUE) goto MISTAKE;
            }
            wiplower(par);
            (void)Strcpy(infile, par);
            Strcat(infile, "[1]");           /* Test name for vectors. */
            if (wipisvec(infile)) {
              yvec = wipvector(par, &ny, &npts);
              if ((yvec != (float *)NULL) && (npts > 0))
                wiplogarithm(yvec, npts, xfloat);
            } else if (Strcmp("image", par) == 0) {
              curimage = wipimcur("curimage");
              if (wipimagexists(curimage) == 0) {
                wipoutput(stderr, "You must specify an image first!\n");
                goto MISTAKE;
              }
              if (wipimlogscale(curimage, xfloat))
                goto MISTAKE;
            } else {
              wipoutput(stderr,
                "Specify either a vector name or 'image': [%s]\n", par);
              goto MISTAKE;
            }
          } else if (Strcmp(commandname, "lookup") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            evec = wipvector("err", &nx, &ny);
            npts = MIN(npts, ny);
            pvec = wipvector("pstyle", &nx, &narg);
            npts = MIN(npts, narg);
            if (npts < 1) goto MISTAKE;
            xfloat = 1.0;
            if (argc > 0) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              if (arg[0] < 0) xfloat = -1.0;
            }
            cpgctab(pvec, xvec, yvec, evec, npts, xfloat, 0.5);
          } else if (Strcmp(commandname, "loop") == 0) {
            goto BADCOMMAND;
          } else if (Strcmp(commandname, "lstyle") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            wipltype(narg);
          } else if (Strcmp(commandname, "lwidth") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            wiplw(narg);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'm':
          if (Strcmp(commandname, "macro") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Input filename\n");
              goto MISTAKE;
            }
            if (wipmacroinput(par)) goto MISTAKE;
          } else if (Strcmp(commandname, "minmax") == 0) {
            narg = 0;
            if (argc > 0) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);
            }
            curimage = wipimcur("curimage");
            if (wipimagexists(curimage) == 0) goto MISTAKE;
            wipimageminmax(curimage, &ymin, &ymax, narg);
            wipoutput(stdout, "Array Min = %G  Max = %G\n", ymin, ymax);
          } else if (Strcmp(commandname, "move") == 0) {
            if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            xfloat = arg[0];
            yfloat = arg[1];
            wipmove(xfloat, yfloat);
          } else if (Strcmp(commandname, "mtext") == 0) {
            if ((ptr = wipparse(&line)) == (char *)NULL) goto MISTAKE;
            if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
            xfloat = arg[0]; yfloat = arg[1]; xmin = arg[2];
            if (wipmtext(ptr, xfloat, yfloat, xmin, line)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'n':
          if (Strcmp(commandname, "ncurse") == 0) {
            xvec = wipvector("x", &nx, &npts); j = nx;
            yvec = wipvector("y", &nx, &ny); nx = MIN(nx, j);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            narg = npts;
            if (argc > 0) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);
            }
            if ((pvec = wipvector("pstyle", &ny, &npts)) == (float *)NULL)
              goto MISTAKE;
            ny = (npts > 0) ? NINT(pvec[0]) : 5;
            cpgncur(nx, &narg, xvec, yvec, ny);
            if (narg <= 0) goto MISTAKE;
            if ((wipvectornpts("x", narg)) ||
                (wipvectornpts("y", narg))) goto MISTAKE;
          } else if (Strcmp(commandname, "new") == 0) {
            if (wipnewitem(line)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'o':
          if (Strcmp(commandname, "olin") == 0) {
            xvec = wipvector("x", &nx, &npts); j = nx;
            yvec = wipvector("y", &nx, &ny); nx = MIN(nx, j);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            narg = npts;
            if (argc > 0) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);
            }
            if ((pvec = wipvector("pstyle", &ny, &npts)) == (float *)NULL)
              goto MISTAKE;
            ny = (npts > 0) ? NINT(pvec[0]) : 5;
            cpgolin(nx, &narg, xvec, yvec, ny);
            if (narg <= 0) goto MISTAKE;
            if ((wipvectornpts("x", narg)) ||
                (wipvectornpts("y", narg))) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'p':
          if (Strcmp(commandname, "palette") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            npts = -1;
            if (argc > 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              npts = NINT(arg[0]);
            }
            wippalette(narg, npts);
          } else if (Strcmp(commandname, "panel") == 0) {
            if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
            nx = NINT(arg[0]); ny = NINT(arg[1]); narg = NINT(arg[2]);
            wippanel(nx, ny, narg);
          } else if (Strcmp(commandname, "paper") == 0) {
            if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            xfloat = arg[0];
            yfloat = arg[1];
            cpgpap(xfloat, yfloat);
          } else if (Strcmp(commandname, "pcolumn") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            pvec = wipvector("pstyle", &nx, &npts);
            if (nx < 1) goto MISTAKE;
            if ((nx = wipreadcol(pvec, nx, narg)) == EOF) goto MISTAKE;
            if (wipvectornpts("pstyle", nx)) goto MISTAKE;
          } else if (Strcmp(commandname, "phard") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Device name required....\n");
              goto MISTAKE;
            }
            if (wipphard(par, line)) goto MISTAKE;
          } else if (Strcmp(commandname, "playback") == 0) {
            if (wipplayback(line)) goto MISTAKE;
          } else if (Strcmp(commandname, "plotfit") == 0) {
            xvec = wipvector("x", &nx, &npts);
            if (npts < 1) npts = 0;
            wipgetlimits(&xmin, &xmax, &ymin, &ymax);
            xfloat = 0.0;
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xmin = arg[0]; xmax = arg[1];
            } else if (argc > 2) {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              xmin = arg[0]; xmax = arg[1];
              xfloat = arg[2];
            }
            wipplotfit(xmin, xmax, xfloat, xvec, npts);
          } else if (Strcmp(commandname, "points") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) goto MISTAKE;
            pvec = wipvector("pstyle", &nx, &ny);
            if (ny < 1) ny = 0;
            evec = wipvector("err", &nx, &narg);
            if (narg < 1) narg = 0;
            if (argc < 1) narg = 0;
            if (wippoints(ny, pvec, npts, xvec, yvec, narg, evec)) goto MISTAKE;
          } else if (Strcmp(commandname, "poly") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) goto MISTAKE;
            cpgpoly(npts, xvec, yvec);
            wipmove(xvec[0], yvec[0]);
          } else if (Strcmp(commandname, "putlabel") == 0) {
            if (argc < 2) {
              wipoutput(stderr, "A label (string) is required.\n");
              goto MISTAKE;
            }
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            arg[0] = ABS(arg[0]);
            wipputlabel(line, arg[0]);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'q':
          if (Strcmp(commandname, "quarter") == 0) {
            narg = 0;
            if (argc > 0) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              narg = NINT(arg[0]);
            }
            if (wipquarter(narg)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'r':
          if (Strcmp(commandname, "range") == 0) {
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xmin = arg[0];
              xmax = arg[1];
              ymin = ymax = 0.0;
            } else {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              xmin = arg[0];
              xmax = arg[1];
              ymin = arg[2];
              ymax = arg[3];
            }
            wipfitrange(xmin, xmax, ymin, ymax);
          } else if (Strcmp(commandname, "read") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Read `filename'.\n");
              goto MISTAKE;
            }
            if (wipreadmac(par)) goto MISTAKE;
          } else if (Strcmp(commandname, "rect") == 0) {
            if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
            xmin = arg[0]; xmax = arg[1];
            ymin = arg[2]; ymax = arg[3];
            cpgrect(xmin, xmax, ymin, ymax);
          } else if (Strcmp(commandname, "reset") == 0) {
            wipreset();
          } else if (Strcmp(commandname, "rgb") == 0) {
            if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              narg = arg[0];
              xmin = xmax = ymin = arg[1];
            } else {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              narg = arg[0];
              xmin = arg[1]; xmax = arg[2]; ymin = arg[3];
            }
            cpgscr(narg, xmin, xmax, ymin);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 's':
          if (Strcmp(commandname, "scale") == 0) {
            narg = 2; /* world units per mm. */
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              xfloat = arg[0]; yfloat = xfloat;
            } else if (argc == 2) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xfloat = arg[0]; yfloat = arg[1];
            } else {
              if (wiparguments(&line, 3, arg) != 3) goto MISTAKE;
              xfloat = arg[0]; yfloat = arg[1];
              narg = NINT(arg[2]);
            }
            if (wipscale(xfloat, yfloat, narg)) goto MISTAKE;
          } else if (Strcmp(commandname, "set") == 0) {
            if (wipsetuser(line)) goto MISTAKE;
          } else if (Strcmp(commandname, "show") == 0) {
            wipshow(line);
          } else if (Strcmp(commandname, "slevel") == 0) {
            if ((ptr = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "A contour level type is required.\n");
              goto MISTAKE;
            }
            if (wipsetstring("levtype", ptr)) goto MISTAKE;
            if (argc > 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              if (wipsetvar("slevel", arg[0])) goto MISTAKE;
            }
          } else if (Strcmp(commandname, "string") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "A string variable name is required.\n");
              goto MISTAKE;
            }
            nx = ny = 0;
            if (argc == 2) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              nx = NINT(arg[0]);
            } else if (argc == 3) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              nx = NINT(arg[0]); ny = NINT(arg[1]);
            }
            if ((ptr = wipreadstr(nx, ny)) == (char *)NULL) goto MISTAKE;
            nx = wipsetstring(par, ptr);       /* Save the status int. */
            Free(ptr);                     /* Free the dynamic string. */
            if (nx != 0) goto MISTAKE;     /* Error in wipsetstring(). */
          } else if (Strcmp(commandname, "subimage") == 0) {
            curimage = wipimcur("curimage");
            if (wipimagexists(curimage) == 0) {
              wipoutput(stderr, "You must specify an image first!\n");
              goto MISTAKE;
            }
            if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
            wipimagenxy(curimage, &nx, &ny);
            sx1 = NINT(arg[0]); sx1 = MIN(nx, sx1); sx1 = MAX(1, sx1);
            sx2 = NINT(arg[1]); sx2 = MIN(nx, sx2); sx2 = MAX(1, sx2);
            sy1 = NINT(arg[2]); sy1 = MIN(ny, sy1); sy1 = MAX(1, sy1);
            sy2 = NINT(arg[3]); sy2 = MIN(ny, sy2); sy2 = MAX(1, sy2);
            if ((sx1 >= sx2) || (sy1 >= sy2)) {
              wipoutput(stderr, "Subimage range incorrect.  Check inputs.\n");
              goto MISTAKE;
            }
            wipsetsub(sx1, sx2, sy1, sy2);
          } else if (Strcmp(commandname, "submargin") == 0) {
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              xfloat = yfloat = arg[0];
            } else {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xfloat = arg[0]; yfloat = arg[1];
            }
            wipsetsubmar(xfloat, yfloat);
          } else if (Strcmp(commandname, "symbol") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            pvec = wipvector("pstyle", &nx, &ny);
            if (nx < 1) goto MISTAKE;
            if (wipsetvec("pstyle[1]", arg[0])) goto MISTAKE;
            if (wipvectornpts("pstyle", 1)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 't':
          if (Strcmp(commandname, "ticksize") == 0) {
            if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
            xtick = arg[0]; nxsub = NINT(arg[1]);
            ytick = arg[2]; nysub = NINT(arg[3]);
            wipsetick(xtick, nxsub, ytick, nysub);
          } else if (Strcmp(commandname, "transfer") == 0) {
            if (argc < 1) {
              for (j = 0; j < 6; j++) tr[j] = 0.0;
              tr[1] = 1.0;
              tr[5] = 1.0;
            } else {
              if (wiparguments(&line, 6, arg) != 6) goto MISTAKE;
              for (j = 0; j < 6; j++) tr[j] = arg[j];
            }
            wipsetr(tr);
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'v':
          if (Strcmp(commandname, "vector") == 0) {
            xfloat = 45.0; yfloat = 0.3;
            if (argc == 1) {
              if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
              xfloat = arg[0];
            } else if (argc > 1) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xfloat = arg[0];
              yfloat = arg[1];
            }
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            pvec = wipvector("pstyle", &nx, &ny);
            npts = MIN(npts, ny);
            evec = wipvector("err", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) goto MISTAKE;
            wipvfield(xvec, yvec, pvec, evec, npts, xfloat, yfloat);
          } else if (Strcmp(commandname, "viewport") == 0) {
            if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
            if (arg[0] == arg[1]) goto MISTAKE;
            if (arg[2] == arg[3]) goto MISTAKE;
            xmin = arg[0]; xmax = arg[1];
            ymin = arg[2]; ymax = arg[3];
            cpgsvp(xmin, xmax, ymin, ymax);
            wipviewport();
          } else if (Strcmp(commandname, "vsize") == 0) {
            if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
            if (arg[0] == arg[1]) goto MISTAKE;
            if (arg[2] == arg[3]) goto MISTAKE;
            xmin = arg[0]; xmax = arg[1];
            ymin = arg[2]; ymax = arg[3];
            cpgvsiz(xmin, xmax, ymin, ymax);
            wipviewport();
          } else if (Strcmp(commandname, "vstand") == 0) {
            cpgvstd();
            wipviewport();
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'w':
          if (Strcmp(commandname, "wedge") == 0) {
            if ((ptr = wipparse(&line)) == (char *)NULL) goto MISTAKE;
            if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
            xfloat = arg[0]; yfloat = arg[1]; xmin = hmin; xmax = hmax;
            par = "";
            if (argc > 3) {
              if (wiparguments(&line, 2, arg) != 2) goto MISTAKE;
              xmin = arg[0]; xmax = arg[1];
              par = line;
            }
            if ((Strchr(ptr, 'i') == (char *)NULL) &&
                (Strchr(ptr, 'I') == (char *)NULL) &&
                (Strchr(ptr, 'g') == (char *)NULL) &&
                (Strchr(ptr, 'G') == (char *)NULL)) {
              (void)Strcpy(infile, ptr);
              wipgetcir(&cmin, &cmax);
              if ((cmin + 1) < cmax)
                (void)Strcat(infile, "i");
              else
                (void)Strcat(infile, "g");
              ptr = infile;
            }
            if (wipwedge(ptr, xfloat, yfloat, xmin, xmax, par)) goto MISTAKE;
          } else if (Strcmp(commandname, "winadj") == 0) {
            xvec = wipvector("x", &nx, &npts);
            yvec = wipvector("y", &nx, &ny);
            npts = MIN(npts, ny);
            if (npts < 1) npts = 0;
            wiprange(npts, xvec, &xmin, &xmax);
            wiprange(npts, yvec, &ymin, &ymax);
            if (argc > 0) {
              if (wiparguments(&line, 4, arg) != 4) goto MISTAKE;
              if (arg[0] != arg[1]) {
                xmin = arg[0];
                xmax = arg[1];
              }
              if (arg[2] != arg[3]) {
                ymin = arg[2];
                ymax = arg[3];
              }
            }
            cpgwnad(xmin, xmax, ymin, ymax);
            wiplimits();
            wipviewport();
          } else if (Strcmp(commandname, "write") == 0) {
            if ((par = wipparse(&line)) == (char *)NULL) {
              wipoutput(stderr, "Write `filename' `macro1 macro2 ...'\n");
              goto MISTAKE;
            }
            if (wipwritemac(par, line)) goto MISTAKE;
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'x':
          if (Strcmp(commandname, "xcolumn") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            xvec = wipvector("x", &nx, &npts);
            if (nx < 1) goto MISTAKE;
            if ((npts = wipreadcol(xvec, nx, narg)) == EOF) goto MISTAKE;
            if (wipvectornpts("x", npts)) goto MISTAKE;
          } else if (Strcmp(commandname, "xlabel") == 0) {
            if (argc < 1) {
              wipoutput(stderr, "A label (string) is required.\n");
              goto MISTAKE;
            }
            if (wiplenc(line) <= 0) goto MISTAKE;
            wipdecode(line, infile, STRINGSIZE);
            cpglab(infile, "", "");
          } else {
            goto BADCOMMAND;
          }
          break;
        case 'y':
          if (Strcmp(commandname, "ycolumn") == 0) {
            if (wiparguments(&line, 1, arg) != 1) goto MISTAKE;
            narg = NINT(arg[0]);
            yvec = wipvector("y", &ny, &npts);
            if (ny < 1) goto MISTAKE;
            if ((npts = wipreadcol(yvec, ny, narg)) == EOF) goto MISTAKE;
            if (wipvectornpts("y", npts)) goto MISTAKE;
          } else if (Strcmp(commandname, "ylabel") == 0) {
            if (argc < 1) {
              wipoutput(stderr, "A label (string) is required.\n");
              goto MISTAKE;
            }
            if (wiplenc(line) <= 0) goto MISTAKE;
            wipdecode(line, infile, STRINGSIZE);
            cpglab("", infile, "");
          } else {
            goto BADCOMMAND;
          }
          break;
        default:
          goto BADCOMMAND;
      }

      if (error == TRUE) goto MISTAKE;
      if (termidle == TRUE) cpgupdt();
      return(0);

BADCOMMAND:
      wipoutput(stderr, " No code to execute for [%s].\n", commandname);
      /* Fall through to MISTAKE: */
MISTAKE:
      return(1);
}
