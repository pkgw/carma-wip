/*
	<wedge.c>
	03feb92 jm  Original code.
	18feb92 jm  Modified to restrict maximum number of points to 256.
	03aug92 jm  Modified to return status (void -> int) rather than
		    using a passed LOGICAL pointer.
	24aug92 jm  Substantial rewrite; added two input arguments which
		    define the values cooresponding to [0-255].
        10nov93 jm  Modified image variable to new opaque pointer type.
        06dec94 jm  Major revision (and simplification) due to the
		    addition of PGPLOT's cpgwedg().
        27nov95 jm  Copied almost all of PGPLOT's cpgwedg routine to
		    permit more functionality in box labels.
        23oct96 jm  Flipped sense of bg/fg so they are correct.
        13nov96 jm  Changed default action of label to draw a basic box.

Routines:
static int wipPwedge ARGS(( char *cside, float disp, float thick,
			    float bg, float fg, char *label ));
int wipwedge ARGS(( char *side, float disp, float thick,
		    float bg, float fg, char *label ));
*/

#include "wip.h"

/* Global Variables needed just for this file */

#define TXTFRC 0.6
#define TXTSEP 2.2
#define WDGPIX 100

typedef enum {NONE, LEFT, RIGHT, TOP, BOTTOM} SIDE;

/* Code */

/*
 *  This routine is essentially PGWEDG except it properly handles the
 *  min/max of the call to PGIMAG/PGGRAY and also includes the user
 *  defined tick arguments for the box.
 *
 *  Returns 0 on success; 1 on error.
 */
#ifdef PROTOTYPE
static int wipPwedge(char *cside, float disp, float thick, float bg, float fg, char *label)
#else
static int wipPwedge(cside, disp, thick, bg, fg, label)
char *cside;
float disp;
float thick;
float bg;
float fg;
char *label;
#endif /* PROTOTYPE */
{
    char *wlabel;
    char defLabel[6];
    char otherLabel[3];
    int i;
    int nxtick, nytick;
    float xtick, ytick;
    float wxa, wxb, wya, wyb;
    float vxa, vxb, vya, vyb;
    float xa, xb, ya, yb;
    float oldch, newch;
    float ndcsize;
    float wedwid, wdginc, vwidth, vdisp;
    float xch, ych, fg1, bg1;

    float tr[6];
    float wedgeArray[WDGPIX];
    SIDE side;
    LOGICAL horiz, image;

    tr[0] = tr[2] = tr[3] = tr[4] = 0.0;
    tr[1] = tr[5] = 1.0;

    if (cside == (char *)NULL)
      return(1);

    side = NONE;
    image = TRUE;
    while (*cside != Null) {
      switch(*cside++) {
        case 't': case 'T': side =    TOP; break;
        case 'l': case 'L': side =   LEFT; break;
        case 'r': case 'R': side =  RIGHT; break;
        case 'b': case 'B': side = BOTTOM; break;
        case 'i': case 'I': image =  TRUE; break;
        case 'g': case 'G': image = FALSE; break;
        default: break;
      }
    }
    if (side == NONE) {
      wipoutput(stderr, "Invalid SIDE argument to WEDGE.");
      return(1);
    }

    horiz = ((side == BOTTOM) || (side == TOP)) ? TRUE : FALSE ;

    /* Store the current world and viewport coords and the character height. */

    cpgbbuf();
    cpgqwin(&wxa, &wxb, &wya, &wyb);
    cpgqvp(0, &xa, &xb, &ya, &yb);
    cpgqch(&oldch);

    /* Determine the unit character height in NDC coords. */
    cpgsch(1.0);
    cpgqcs(0, &xch, &ych);
    ndcsize = (horiz == TRUE) ? ych : xch ;

    /* Convert 'WIDTH' and 'DISP' into viewport units. */

    vwidth = thick * ndcsize * oldch;
    vdisp  = disp * ndcsize * oldch;
    /*
     * Determine and set the character height required to fit the wedge
     * anotation text within the area allowed for it.
     */
    newch = TXTFRC * vwidth / (TXTSEP * ndcsize);
    cpgsch(newch);
    /*
     * Determine the width of the wedge part of the plot minus the anotation.
     * (NDC units).
     */
    wedwid = vwidth * (1.0 - TXTFRC);

    /* Determine the viewport coordinates for the wedge + annotation. */

    vxa = xa;
    vxb = xb;
    vya = ya;
    vyb = yb;
    switch (side) {
      case TOP:    vya = yb + vdisp; vyb = vya + wedwid; break;
      case LEFT:   vxb = xa - vdisp; vxa = vxb - wedwid; break;
      case RIGHT:  vxa = xb + vdisp; vxb = vxa + wedwid; break;
      case BOTTOM: vyb = ya - vdisp; vya = vyb - wedwid; break;
    }

    /* Set the viewport for the wedge. */

    cpgsvp(vxa, vxb, vya, vyb);
    fg1 = (bg > fg) ? bg : fg ;
    bg1 = (bg < fg) ? bg : fg ;

    /* Create the dummy wedge array to be plotted. */

    wdginc = (fg1 - bg1) / (WDGPIX - 1);
    for (i = 0; i < WDGPIX; i++)
      wedgeArray[i] = bg1 + (i * wdginc);

    wipgetick(&xtick, &nxtick, &ytick, &nytick);

    (void)Strcpy(otherLabel, "BC");        /* Set up a default border. */
    if ((wlabel = wipleading(label)) != (char *)NULL) {
      if (Strchr(wlabel, '0') != (char *)NULL)    /* No box requested. */
        otherLabel[0] = Null;
    } else {      /* No box label given; provide a reasonable default. */
      switch (side) {
        case TOP:    wlabel = Strcpy(defLabel, "BCMST"); break;
        case LEFT:   wlabel = Strcpy(defLabel, "BCNST"); break;
        case RIGHT:  wlabel = Strcpy(defLabel, "BCMST"); break;
        case BOTTOM: wlabel = Strcpy(defLabel, "BCNST"); break;
      }
    }

    /* Draw the wedge then change the world coordinates for labelling. */
    /* Also, draw a labelled frame around the wedge. */

    if (horiz == TRUE) {
      cpgswin(1.0, (float)WDGPIX, 0.9, 1.1);
      if (image == TRUE)
        cpgimag(wedgeArray, WDGPIX, 1, 1, WDGPIX, 1, 1, bg, fg, tr);
      else
        cpggray(wedgeArray, WDGPIX, 1, 1, WDGPIX, 1, 1, bg, fg, tr);
      cpgswin(bg1, fg1, 0.0, 1.0);
      cpgtbox(wlabel, xtick, nxtick, otherLabel, 0.0, 0);
    } else {
      cpgswin(0.9, 1.1, 1.0, (float)WDGPIX);
      if (image == TRUE)
        cpgimag(wedgeArray, 1, WDGPIX, 1, 1, 1, WDGPIX, bg, fg, tr);
      else
        cpggray(wedgeArray, 1, WDGPIX, 1, 1, 1, WDGPIX, bg, fg, tr);
      cpgswin(0.0, 1.0, bg1, fg1);
      cpgtbox(otherLabel, 0.0, 0, wlabel, ytick, nytick);
    }

    /* Reset the original viewport and world coordinates. */

    cpgsvp(xa, xb, ya, yb);
    cpgswin(wxa, wxb, wya, wyb);
    cpgsch(oldch);
    cpgebuf();

    return(0);
}

/*
 *  This routine will draw a wedge for use with halftone images.
 *  Side  describes whether the wedge should appear at the Bottom (B),
 *        Left (L), Top (T), or Right (R) edge of the viewport.
 *  Disp  is the displacement of the wedge from the specified edge of
 *        the viewport, measured outwards from the viewport in units
 *        of the character height. Use a negative value to write inside
 *        the viewport, a positive value to write outside.
 *  Thick is the total thickness of the wedge (including anotation)
 *        in character height units.
 *  Fg    The value which is to appear with shade 1 ("foreground").
 *  Bg    The value which is to appear with shade 0 ("background").
 *  Label Optional units for drawing the box.  If no label is desired,
 *        use "0".  If this is a null string, a default one is chosen.
 *
 *  Use the values of FG and BG that were used with the "halftone" command.
 *
 *  Returns 0 on success; 1 on error.
 */
#ifdef PROTOTYPE
int wipwedge(char *side, float disp, float thick, float bg, float fg, char *label)
#else
int wipwedge(side, disp, thick, bg, fg, label)
char *side;
float disp;
float thick;
float bg;
float fg;
char *label;
#endif /* PROTOTYPE */
{
    int nside;

    nside = wiplenc(side);         /* Also strips off trailing blanks. */
    if (nside < 1) {
      wipoutput(stderr, "Trouble interpreting side: [%s]\n", side);
      return(1);
    }

    if ((*side == 'P') || (*side == 'p')) {     /* Temporary add-on??? */
      side++;
      cpgwedg(side, disp, thick, fg, bg, label);
      return(0);
    }

    return(wipPwedge(side, disp, thick, bg, fg, label));
}

#ifdef TEST
#ifdef PROTOTYPE
main(int argc, char *argv[])
#else
main(argc, argv)
int argc;
char *argv[];
#endif /* PROTOTYPE */
{
    char ch;
    char device[80];
    float x, y;

    if (argc < 2) {
      (void)printf("Enter plot device: ");
      (void)scanf("%s", device);
    } else {
      (void)strcpy(device, argv[1]);
    }

    if (!cpgbeg(0, device, 1, 1)) {
      (void)printf("Trouble opening device [%s]\n", device);
      return(1);
    }

    cpgpage();
    cpgsvp(0.2, 0.8, 0.2, 0.8);
    cpgsci(2);
    cpgbox("bc", 0.0, 0, "bc", 0.0, 0);
    cpgsci(1);

    wippanel(2, 1, 1);

    (void)printf("Bottom gray wedge status = %d\n", 
       wipwedge("BG", -1.0, 1.0, 0.0, 1.0, "BCM"));

    (void)printf("Right gray wedge status = %d\n", 
       wipwedge("RG", -1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Top gray wedge status = %d\n", 
       wipwedge("TG", -1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Left gray wedge status = %d\n", 
       wipwedge("LG", -1.0, 1.0, 0.0, 1.0, ""));

    wippalette(0, 0); /* Reset palette the PGGRAY destroys. */

    (void)printf("Bottom image wedge status = %d\n", 
       wipwedge("BI", 1.0, 1.0, 0.0, 1.0, "BCN"));

    (void)printf("Right image wedge status = %d\n", 
       wipwedge("RI", 1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Top image wedge status = %d\n", 
       wipwedge("TI", 1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Left image wedge status = %d\n", 
       wipwedge("LI", 1.0, 1.0, 0.0, 1.0, ""));

    wippanel(2, 1, 2);

    (void)printf("Bottom gray wedge status = %d\n", 
       wipwedge("PBG", -1.0, 1.0, 0.0, 1.0, "BCM"));

    (void)printf("Right gray wedge status = %d\n", 
       wipwedge("PRG", -1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Top gray wedge status = %d\n", 
       wipwedge("PTG", -1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Left gray wedge status = %d\n", 
       wipwedge("PLG", -1.0, 1.0, 0.0, 1.0, ""));

    wippalette(0, 0); /* Reset palette the PGGRAY destroys. */

    (void)printf("Bottom image wedge status = %d\n", 
       wipwedge("PBI", 1.0, 1.0, 0.0, 1.0, "BCN"));

    (void)printf("Right image wedge status = %d\n", 
       wipwedge("PRI", 1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Top image wedge status = %d\n", 
       wipwedge("PTI", 1.0, 1.0, 0.0, 1.0, ""));

    (void)printf("Left image wedge status = %d\n", 
       wipwedge("PLI", 1.0, 1.0, 0.0, 1.0, ""));

    cpgupdt();

    (void)printf("Now in cursor...\n");
    (void)cpgcurs(&x, &y, &ch);

    cpgend();
    return(0);
}
#endif /* TEST */
