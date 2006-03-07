/*
	"version.h"
	1.0 15jun91 Initial version.
	1.1 26jul91 Corrected some minor bugs found when writing manual.
	1.2 02aug91 Made declarations ANSI standard.
	1.3 02nov91 Made variables a structure rather than global items.
		    Added image drivers for MIRIAD and FITS.
	1.5 01mar92 Substantial structure and manual upgrade.
	1.6 01oct92 Continued structure change and manual upgrade.
	1.7 10nov93 Minor modification to clean up compiler warnings and
		    add checks against empty data files.  Added Gaussian
		    fitting and logarithm command.  Added Void opaque
		    pointer and ANSI-fied driver code.  Added READLINE
		    ability.  Reorganized image include section.  Also
		    updated the manual.
	2.0 01jan95 Major modifications.  Also modified for PGPLOT 5.0
		    with new cpgplot library.
	2.1 30oct96 Minor modifications and additions.
	2.2 13nov96 Minor modifications and image bug fix.
	2.3 22jan98 Minor modifications, bug fixes, and added VMS code.

        2.4 ?  12sep99 Fixed various linux problems with mask files
        2.3.2  22sep00 More variables (MAXVAR) in evaluate.c
        2.3.3  10oct00 include fix for cd1_1/2_2 for cdelt in image.c
        2.3.4  30nov00 more stringspace, fixed  FWHM reporting in Gaussfit
        2.3.5  21sep02 mask reading in linux corrected
        2.3.7  07mar06 removed "hardcopy" command if -x on command line.
*/
#ifndef WIP_VERSION
#define WIP_VERSION "Version: 2.3.7 07mar06"
#endif /* WIP_VERSION */
