/*
	<declare.h>
	15jun91 jm  Original code.
	02nov91 jm  Added variables subdirectory and removed files
		    plot/variables and sysdep.../fillinit.
	01aug92 jm  Cleaned up and fixed entries due to changes in /variables.
	04oct95 jm  Added if-else for wipoutput declaration.
	13nov96 jm  Added wipDebugMode and wipsetQuiet.
	21jan98 jm  VMS reserves the readonly keyword, so it was changed
		    to justread in declaration of wipbegin().
*/

#ifndef WIP_DECLARE
#define WIP_DECLARE

/* Code in wip/branch */

extern     int   wipexecute ARGS(( Const char *cmdname, char *line ));

extern     int   wipprocess ARGS(( char *cmdline, int *mode, LOGICAL keep ));

extern     int   wipinit ARGS(( void ));

/* static void   usage ARGS(( Const char *filename )); */
/*        void   main ARGS(( int argc, char *argv[] )); */


/* Code in wip/images */

extern    void   wipextrema ARGS(( float **image, int nx, int ny, float *min, float *max ));

/*  static  void   wipimagefree ARGS(( Void *image )); */
/*  static WIPIMAGE *getimage ARGS(( Const char *file, int plane, float blank )); */
extern    Void  *wipimage ARGS(( Const char *file, int plane, float blank ));
extern    void   wipimagenxy ARGS(( Const Void *image, int *nx, int *ny ));
extern    void   wipimageminmax ARGS(( Void *image, float *min, float *max, int force ));
extern     int   wipimagexists ARGS(( Const Void *image ));
extern   float **wipimagepic ARGS(( Const Void *image ));
extern     int   wipimlogscale ARGS(( Void *image, float scale ));
extern     int   wipimsetminmax ARGS(( Void *image, float min, float max ));
extern     int   wipimgethead ARGS(( Const Void *image, int axis, double *crval, double *crpix, double *cdelt, char *ctype ));
extern     int   wipimctype ARGS(( Const Void *image, int axis, char *ctype ));
extern    char  *wipimtype ARGS(( Const Void *image ));
extern     int   wipimplane ARGS(( Const Void *image ));
extern     int   wipimhdprsnt ARGS(( Const Void *image, Const char *hdname ));
extern     int   wipimhdval ARGS(( Const Void *image, Const char *hdname, double *retval ));
extern     int   wipimhdstr ARGS(( Const Void *image, Const char *hdname, char *retval, size_t maxlen ));
extern    Void  *wipimcur ARGS(( Const char *imagename ));
extern    void   wipimsetcur ARGS(( Const char *imagename, Const Void *image ));
extern    void   wipfreeimage ARGS(( Const char *imagename ));

/* static  int   wipheadlim ARGS(( Const Void *image, Const char *xtype,
Const char *ytype, float *xscale, float *xoff, float *yscale, float *yoff )); */

extern     int   wipheader ARGS(( int blcx, int blcy, int trcx, int trcy, Const char *xtype, Const char *ytype ));

extern     int   wipsmooth ARGS(( float **array, int nx, int ny, int order, \
      float blank ));


/* Code in wip/interact */

extern COMMAND  *create_command ARGS(( void ));
extern    void   add_command ARGS(( COMMAND *cmd ));
extern    void   delete_command ARGS(( COMMAND *cmd ));

extern    void   wipdecode ARGS(( Const char *string, char *outstr, size_t maxout ));

/*  static  void   add_to_help ARGS(( COMMAND *cmd, char *helpline )); */
extern     int   wiphelp ARGS(( char *rest ));

extern COMMAND  *wipifxecute ARGS(( char **rest ));

extern    void   wipsaveline ARGS(( COMMAND *macro, Const char *line ));
extern COMMAND  *wipstartins ARGS(( char *rest ));
extern    void   wipinsert ARGS(( COMMAND *command, Const char *line ));
extern     int   wipdelete ARGS(( char *rest ));
extern COMMAND  *wipstartmac ARGS(( Const char *macname ));

extern COMMAND  *find_command ARGS(( COMMAND *start, Const char *cmdname, int exact ));
extern COMMAND  *wipinterpret ARGS(( char **cmdline ));

extern     int   wiplist ARGS(( int line1, int line2, Const char *rest ));

extern COMMAND  *wiploopxecute ARGS(( char **cmdline, int *ncount ));

extern COMMAND  *create_macro ARGS(( Const char *macname ));
extern    void   add_to_macro ARGS(( COMMAND *mac, Const char *cmdline ));
extern    void   insert_macro ARGS(( COMMAND *mac, Const char *cmdline, int before ));
extern    void   delete_macro ARGS(( COMMAND *mac, int first, int last ));

/*  static  char  *domacsubs ARGS(( char *string, Const char *line, char *macarg[] )); */
extern     int   wipmaxecute ARGS(( COMMAND *mac, int cnt, Const char *rest ));

extern    char  *wipparse ARGS(( char **line ));
extern     int   wipcountwords ARGS(( Const char *line ));
extern    void   wiplower ARGS(( char *line ));
extern    void   wipupper ARGS(( char *line ));
extern    char  *wipleading ARGS(( Const char *line ));
extern     int   wiplenc ARGS(( char *line ));
extern    char  *wipnewstring ARGS(( Const char *string ));
extern     int   wiparguments ARGS(( char **rest, int numarg, double arg[] ));

extern     int   wipreadinput ARGS(( Const char *rest ));
extern     int   wipwritemac ARGS(( Const char *file, Const char *macs ));
extern     int   wipplayback ARGS(( Const char *rest ));
extern     int   wipreadmac ARGS(( Const char *rest ));
extern     int   wipmacroinput ARGS(( Const char *file ));


/* Code in wip/fit */

/*  static float getfit ARGS(( float xval )); */
extern    void   wipplotfit ARGS(( float x1, float x2, float step, float x[], int nx ));
extern    void   wipfitrange ARGS(( float x1, float x2, float y1, float y2 ));
extern     int   wipfit ARGS(( Const char *rest, int nxy, float x[], \
      float y[], int ns, float sig[] ));

/*  static void fgauss ARGS(( float x, float a[], float *y, float dyda[], \
      int na )); */
/*  static void mrqcof ARGS(( float x[], float y[], float sig[], int ndata, \
      float a[], int ia[], int ma, float **alpha, float beta[], float *chisq, \
      void (*funcs) ARGS(( float, float [], float *, float [], int )) )); */
/*  static int gaussj ARGS(( float **a, int n, float **b, int m )); */
/*  static void covsrt ARGS(( float **covar, int ma, int ia[], int mfit )); */
/*  static int mrqmin ARGS(( float x[], float y[], float sig[], int ndata, \
      float a[], int ia[], int ma, float **covar, float **alpha, float *chisq,\
      void (*funcs) ARGS(( float, float [], float *, float [], int )), \
      float *alamda )); */
/*  static int gaussfit ARGS(( float x[], float y[], int ndata, float sig[], \
      int nsig, float a[], int ia[], int ma, float *chisq )); */
/*  static int moment ARGS(( float x[], float y[], int n, float *amp, \
      float *xmean, float *sdev )); */
extern     int   wipgaussfit ARGS(( char *rest, float x[], float y[], \
      int nxy, float sig[], int nsig, int ngauss, float terms[] ));

/*  static double gammln ARGS(( float x )); */
/*  static float gcf ARGS(( float a, float x )); */
/*  static float gser ARGS(( float a, float x )); */
/*  static float gammq ARGS(( float a, float x )); */
extern     int   lsqfit ARGS(( float x[], float y[], int ndata, float sig[], \
      int mwt, float *a, float *b, float *siga, float *sigb, \
      float *chi2, float *q ));

/*  static void hpsort ARGS(( unsigned long int n, float ra[] )); */
/*  static float rofunc ARGS(( float b, float x[], float y[], float arr[], \
      int npts, float *aa, float *abdev )); */
extern     int   medfit ARGS(( float x[], float y[], int ndata, float *a, \
      float *b, float *abdev ));


/* Code in wip/plot */

/*  static void setaitoff ARGS(( float *xscale, float *yscale, float *xorg, float *yorg)); */
/*  static void aitoffConvert ARGS(( float l, float b, float *x, float *y )); */
extern    void   wipaitoff ARGS(( int nxy, float x[], float y[] ));
extern    void   wipaitoffgrid ARGS(( int nlong, int nlats ));

extern     int   wiparc ARGS(( float majx, float majy, float arcangle, float angle, float start ));
extern     int   wipbeam ARGS(( float major, float minor, float posangle, \
      float offx, float offy, int fillcolor, float scale, int bgrect ));

extern    void   wiparrow ARGS(( float xp, float yp, float angle, float vent ));
extern    void   wipvfield ARGS(( float x[], float y[], float r[],
                   float phi[], int npts, float angle, float vent ));

extern    void   wiplogarithm ARGS(( float array[], int nxy, float scale ));
extern    void   wiprange ARGS(( int nx, float x[], float *xmin, float *xmax ));
extern     int   wiperrorbar ARGS(( int locat, float x[], float y[], float err[], int nxy ));

extern    void   wipcursor ARGS(( float *cx, float *cy, LOGICAL keep ));
extern    void   wipfixcurs ARGS(( Const char *cmd, char *rest, LOGICAL keep ));

extern     int   wipdevice ARGS(( Const char *devicename ));
extern    void   wipclose ARGS(( void ));

extern    void   wipheq ARGS(( int nx, int ny, float **image,  \
                               int x1, int x2, int y1, int y2, \
                               float blank, float min, float max, int nbins ));

extern     int   wiphline ARGS(( int npts, float x[], float y[], float gap, int center ));
extern     int   wipbar ARGS(( int npts, float x[], float y[], int nc, float color[], int location, int dolimit, float barlimit, float barwidth ));

extern   float   wipimval ARGS(( float **image, int nx, int ny, float cx, float cy, float tr[], LOGICAL *error ));
extern    char  *wipradecfmt ARGS(( float position ));

extern    char  *wipinquire ARGS(( Const char *item ));
extern     int   wipishard ARGS(( void ));

extern     int   wiplevels ARGS(( char *rest, float level[], int maxlev ));
extern     int   wipautolevs ARGS(( char *rest, float level[], int maxlev, float min, float max ));
extern     int   wipscalevels ARGS(( Const char *stype, float slev, float pmin, float pmax, float levels[], int nlev ));

extern   float  *vector ARGS(( int nx ));
extern   float **matrix ARGS(( int offx, int nx, int offy, int ny ));
extern    void   freevector ARGS(( float *vector ));
extern    void   freematrix ARGS(( float **matrix, int offx, int offy ));

extern    void   wipmove ARGS(( float x, float y ));
extern    void   wipdraw ARGS(( float x, float y ));
extern    void   wipgetcxy ARGS(( float *cx, float *cy ));

extern    void   wippalette ARGS(( int which, int levels ));

extern    void   wippanel ARGS(( int nx, int ny, int k ));

extern     int   wipphard ARGS(( Const char *device, Const char *rest ));

extern     int   wippoints ARGS(( int nstyle, float style[], int nxy, float x[], float y[], int nc, float c[] ));

extern    void   wipputlabel ARGS(( Const char *line, double justify ));
extern    char  *fixputlabel ARGS(( Const char *cmdname, char *rest, LOGICAL save ));
extern     int   wipmtext ARGS(( char *side, float disp, float coord, float just, char *line ));

extern     int   wipquarter ARGS(( int quadrant ));

extern    void   wipreset ARGS(( void ));

extern     int   wipscale ARGS(( float scalex, float scaley, int k ));

extern    void   wipcolor ARGS(( int color ));
extern    void   wipexpand ARGS(( double expand ));
extern    void   wipfill ARGS(( int fill ));
extern    void   wipfont ARGS(( int font ));
extern    void   wipltype ARGS(( int style ));
extern    void   wiplw ARGS(( int width ));
extern    void   wipsetbgci ARGS(( int color ));
extern    void   wipsetitf ARGS(( int type ));
extern    void   wipsetangle ARGS(( double angle ));
extern    void   wipsetslope ARGS(( double xa, double xb, double ya, double yb ));
extern    void   wiplimits ARGS(( void ));
extern    void   wipgetlimits ARGS(( float *x1, float *x2, float *y1, float *y2 ));
extern    void   wipviewport ARGS(( void ));
extern    void   wipgetvp ARGS(( float *x1, float *x2, float *y1, float *y2 ));
extern    void   wipsetr ARGS(( float tr[] ));
extern    void   wipgetr ARGS(( float tr[] ));
extern    void   wipsetsub ARGS(( int subx1, int subx2, int suby1, int suby2 ));
extern    void   wipgetsub ARGS(( int *subx1, int *subx2, int *suby1, int *suby2 ));
extern    void   wipsetick ARGS(( float xtick, int nxsub, float ytick, int nysub ));
extern    void   wipgetick ARGS(( float *xtick, int *nxsub, float *ytick, int *nysub ));
extern    void   wipsetsubmar ARGS(( float subx, float suby ));
extern    void   wipgetsubmar ARGS(( float *subx, float *suby ));
extern    void   wipsetcir ARGS(( int cmin, int cmax ));
extern    void   wipgetcir ARGS(( int *cmin, int *cmax ));
extern     int   wipDebugMode ARGS(( void ));


extern    void   wipshow ARGS(( Const char *rest ));
extern    char  *wipfpfmt ARGS(( float arg, int nsig ));
extern    char  *wipifmt ARGS(( float arg ));

extern     int   wipwedge ARGS(( char *side, float disp, float thick, float bg, float fg, char *label ));



/* Code in wip/sysdep */

extern long int  filesize ARGS(( FILE *fp ));

extern    void   wipsetQuiet ARGS(( int quiet ));
extern    void   wipbegin ARGS(( int disable, int justread ));
extern    void   wipexit ARGS(( int status ));
#ifndef NOVARARGS
extern    void   wipoutput ARGS(( FILE *fp, Const char *fmt, ... ));
#else
#define wipoutput (void)fprintf
#endif /* !NOVARARGS */
extern     int   wipinput ARGS(( FILE *fp, Const char *prompt, char *line, size_t maxsize ));

extern   float   wiprand ARGS(( long int *seed ));
extern   float   wipgaussdev ARGS(( long int *seed ));

extern    void   wiplines ARGS(( int first, int last ));
extern    void   wipgetlines ARGS(( int *first, int *last ));
extern    void   wipdatarray ARGS(( int nrow, int ncol ));
extern    void   wipinternaldata ARGS(( void ));
extern     int   wipopenfile ARGS(( Const char *file ));
extern     int   wipreadcol ARGS(( float array[], int maxsize, int ncol ));
extern    char  *wipreadstr ARGS(( int first, int second ));

extern     int   wipspool ARGS(( Const char *spoolfile ));
extern     int   wipcommand ARGS(( Const char *command ));

/*** Defined in image.h ***
extern    void   unpack16_c ARGS(( char *in, int *out, int n ));
extern    void   unpack32_c ARGS(( char *in, int *out, int n ));
extern    void   unpackr_c ARGS(( char *in, float *out, int n ));
extern    void   unpackd_c ARGS(( char *in, double *out, int n ));
 *** Defined in image.h ***/


/* Code in wip/variables */

/*  static   int   wipisop ARGS(( Const char *inword )); */
/*  static double  wipdoop ARGS(( Const char *op, double arg1, double arg2, LOGICAL *error )); */
/*  static   int   wipisfunction ARGS(( Const char *inword )); */
/*  static double  wipdofunc ARGS(( Const char *inword, double arg, LOGICAL *error )); */
extern     int   wipisnumber ARGS(( Const char *inword, double *retval ));
extern    void   wipecho ARGS(( Const char *input ));
extern     int   wipsetuser ARGS(( Const char *input ));
extern    char  *wipgettoken ARGS(( char *out, Const char *in, char **next ));
extern     int   wiptokenexists ARGS(( Const char *inword ));
extern  double   wipevaluate ARGS(( Const char *inword, LOGICAL *error ));
extern    char  *wipbracextract ARGS(( Const char *inword, char **left ));

extern     int   wipnewitem ARGS(( Const char *string ));
extern     int   wipfreeitem ARGS(( Const char *string ));
extern     int   wipisuserfunc ARGS(( Const char *name ));
extern  double   wipuserfunc ARGS(( Const char *inword, double arg, LOGICAL *error ));

extern    void   clear_stack ARGS(( void ));
extern     int   push_stack ARGS(( double value ));
extern     int   pop_stack ARGS(( double *value ));

/*  static     int   initString ARGS(( void )); */
/*  static WSTRINGS  *find_string ARGS(( Const char *name )); */
extern     int   wipisstring ARGS(( Const char *name ));
extern    char  *wipgetstring ARGS(( Const char *name ));
extern     int   wipsetstring ARGS(( Const char *name, Const char *value ));
extern     int   wipNewStrVar ARGS(( Const char *name ));
extern     int   wipFreeString ARGS(( Const char *name ));

/*  static     int   initVariable ARGS(( void )); */
/*  static VARIABLE *find_variable ARGS(( Const char *name )); */
extern     int   wipisvar ARGS(( Const char *name ));
extern  double   wipgetvar ARGS(( Const char *name, LOGICAL *error ));
extern     int   wipsetvar ARGS(( Const char *name, double value ));
extern     int   wipNewVariable ARGS(( Const char *name ));
extern     int   wipFreeVariable ARGS(( Const char *name ));

/*  static     int   initVector ARGS(( void )); */
/*  static VECTOR *find_vector ARGS(( Const char *name, int *indx )); */
extern     int   wipisvec ARGS(( Const char *name ));
extern  double   wipgetvec ARGS(( Const char *name, LOGICAL *error ));
extern     int   wipsetvec ARGS(( Const char *name, double value ));
extern   float  *wipvector ARGS(( Const char *name, int *maxsize, int *currentSize ));
extern     int   wipvectornpts ARGS(( Const char *name, int currentSize ));
extern     int   wipisvecfunc ARGS(( Const char *inword ));
extern  double   wipvecfunc ARGS(( Const char *inword, Const char *arg, LOGICAL *error ));
extern     int   wipvectorinit ARGS(( Const char *name, int npts, Const char *expression ));
extern     int   wipNewVector ARGS(( Const char *name, int size ));
extern     int   wipFreeVector ARGS(( Const char *name ));

#endif /* WIP_DECLARE */
