/*
	<commands.h>
	28jul90 jm  Original code.
	06nov91 jm  Added echo, new, and free commands.
	03feb92 jm  Added wedge command.
	19feb92 jm  Made new and free commands insert==TRUE.
	03aug92 jm  Added aitoff and globe commands.
        09aug93 jm  Made vocab array static.
        05nov93 jm  Added the arrow command.
        22aug94 jm  Removed obsolete commands xlogarithm/ylogarithm.
        12dec94 jm  Added bgci, itf, and palette commands.
        12oct95 jm  Added beam command.
        25jun96 jm  Added plotfit and range commands.
        28oct96 jm  Added heq command.
*/

#ifdef WIP_COMMANDS
#undef WIP_COMMANDS

/* Initialize the COMMAND structure. */

/*  This next line is set up in wipinit()....
 * COMMAND *ENDIF = {"ENDIF", 0, FALSE, FALSE, FALSE, NULL, NULL, NULL};
 */

static COMMAND vocab[] = {
/*       "name", ncom, insert?,  macro?, delete?,  macptr,  help,   next */
 {     "aitoff",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "angle",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "arc",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "arrow",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "ask",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "autolevs",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "bar",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "beam",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "bgci",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "bin",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "box",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "color",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "connect",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "contour",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "cursor",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "data",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "define",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "delete",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "device",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "dot",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "draw",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "echo",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "ecolumn",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "end",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {"environment",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "erase",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "errorbar",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "etxt",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "expand",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "fill",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "fit",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "font",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "free",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "globe",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "halftone",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "hardcopy",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "header",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "help",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "heq",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "hi2d",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {  "histogram",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "hls",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {         "if",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {         "id",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "image",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 { "initialize",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "input",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "insert",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "itf",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "label",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "lcur",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "ldev",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "levels",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "limits",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "lines",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "list",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {  "logarithm",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "lookup",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "loop",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "lstyle",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "lwidth",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "macro",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "minmax",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "move",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "mtext",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "ncurse",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "new",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "olin",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "palette",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "panel",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "paper",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "pcolumn",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "phard",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "playback",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "plotfit",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "points",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "poly",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "putlabel",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "quarter",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "range",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "read",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "rect",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "reset",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "rgb",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "scale",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {        "set",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {       "show",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "slevel",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "string",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "subimage",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {  "submargin",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "symbol",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "ticksize",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "transfer",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "vector",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {   "viewport",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "vsize",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "vstand",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "wedge",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "winadj",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {      "write",   0,   FALSE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "xcolumn",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "xlabel",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {    "ycolumn",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "ylabel",   0,    TRUE,   FALSE,   FALSE,    NULL,   NULL,   NULL},
 {     "buffer",   0,   FALSE,    TRUE,   FALSE,    NULL,   NULL,   NULL}
/* "buffer" MUST be the LAST entry in this list!! */
};

#endif /* WIP_COMMANDS */
