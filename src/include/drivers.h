/*
 *	<drivers.h> --- Image include file.
 *	26oct91 jm  Original code.
 *	10nov93 jm  Modified declarations to usage of Void.
 *
 */

/* ARGS(alist), FLOAT, and Void are defined in imageP.h */

extern Void *miropen ARGS(( Const char *name, int naxis, int axes[] ));
extern void mirclose ARGS(( Void *op ));
extern int mirread ARGS(( Void *op, int indx, FLOAT *array, FLOAT badpixel ));
extern int mirsetpl ARGS(( Void *op, int naxis, int axes[] ));
extern void mirrdhdd ARGS(( Void *op, Const char *key,double *val,double def ));
extern void mirrdhdr ARGS(( Void *op, Const char *key, FLOAT *val,FLOAT rdef ));
extern void mirrdhdi ARGS(( Void *op, Const char *key, int *val, int defval ));
extern void mirrdhda ARGS(( Void *op, Const char *key, char *val, Const char *defval, size_t maxlen ));
extern int mirhdprsnt ARGS(( Void *op, Const char *key ));

extern Void *fitopen ARGS(( Const char *name, int naxis, int axes[] ));
extern void fitclose ARGS(( Void *op ));
extern int fitread ARGS(( Void *op, int indx, FLOAT *data, FLOAT badpixel ));
extern int fitsetpl ARGS(( Void *op, int naxis, int axes[] ));
extern void fitrdhdd ARGS(( Void *op, Const char *key,double *val,double def ));
extern void fitrdhdr ARGS(( Void *op, Const char *key, FLOAT *val,FLOAT rdef ));
extern void fitrdhdi ARGS(( Void *op, Const char *key, int *val, int defval ));
extern void fitrdhda ARGS(( Void *op, Const char *key, char *val, Const char *defval, size_t maxlen ));
extern int fithdprsnt ARGS(( Void *op, Const char *key ));

extern Void *basopen ARGS(( Const char *name, int naxis, int axes[] ));
extern void basclose ARGS(( Void *op ));
extern int basread ARGS(( Void *op, int indx, FLOAT *array, FLOAT badpixel ));
extern int bassetpl ARGS(( Void *op, int naxis, int axes[] ));
extern void basrdhdd ARGS(( Void *op, Const char *key,double *val,double def ));
extern void basrdhdr ARGS(( Void *op, Const char *key, FLOAT *val,FLOAT rdef ));
extern void basrdhdi ARGS(( Void *op, Const char *key, int *val, int defval ));
extern void basrdhda ARGS(( Void *op, Const char *key, char *val, Const char *defval, size_t maxlen ));
extern int bashdprsnt ARGS(( Void *op, Const char *key ));

#ifdef WIP_DRIVERS
#undef WIP_DRIVERS

/*
 *  Structure in which to store image reading commands.  Contains:
 *  imtype    : What kind of file is this.
 *  imopen    : Test and open routine.
 *  imclose   : Close the image file.
 *  imread    : Read selected plane.
 *  imsetpl   : Select plane number.
 *  imrdhdd   : Get double precision header item.
 *  imrdhdr   : Read a real header item.
 *  imrdhdi   : Get an integer header.
 *  imrdhda   : Read a character (string) header item.
 *  imhdprsnt : Returns 1 if the header is present; 0 otherwise.
 */
typedef struct {
    char   *imtype;
    Void *(*imopen)  ARGS(( Const char *name, int naxis, int axes[] ));
    void  (*imclose) ARGS(( Void *op ));
     int  (*imread)  ARGS(( Void *op, int indx, FLOAT *array, FLOAT badpixel ));
     int  (*imsetpl) ARGS(( Void *op, int naxis, int axes[] ));
    void  (*imrdhdd) ARGS(( Void *op, Const char *key, double *val, double defval ));
    void  (*imrdhdr) ARGS(( Void *op,Const char *key,FLOAT *val,FLOAT defval ));
    void  (*imrdhdi) ARGS(( Void *op, Const char *key, int *val, int defval ));
    void  (*imrdhda) ARGS(( Void *op, Const char *key, char *val, Const char *defval, size_t maxlen ));
     int  (*imhdprsnt) ARGS(( Void *op, Const char *key ));
} IMFORMAT;

static IMFORMAT Format_Table[] = {
  {"miriad", miropen, mirclose, mirread, mirsetpl,
             mirrdhdd, mirrdhdr, mirrdhdi, mirrdhda, mirhdprsnt},
  {  "fits", fitopen, fitclose, fitread, fitsetpl,
             fitrdhdd, fitrdhdr, fitrdhdi, fitrdhda, fithdprsnt},
  { "basic", basopen, basclose, basread, bassetpl,
             basrdhdd, basrdhdr, basrdhdi, basrdhda, bashdprsnt},
};

#else

typedef char IMFORMAT;

#endif /* WIP_DRIVERS */
