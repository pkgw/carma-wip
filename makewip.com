$! DCL command procedure to compile WIP for
$! OpenVMS VAX and OpenVMS Alpha (AXP) systems.
$!
$! Requires one input argument: the WIP root directory.
$! This should be run in the directory where the PGPLOT
$! include files and libraries reside.  The WIP.OPT may
$! need to be modified depending on the drivers selected.
$!
$! To be able to specify command-line options, define
$! WIP as a "foreign command":
$!   $ WIP == "$ECC1:[TJP.PGPLOT]WIP"
$! (remember to substitute the correct disk and directory,
$! but keep the $ sign).
$!
$! History:
$! 20jan98 jm  Modified from PGPLOT's sys_vms MAKE_CPG.COM.
$!
$!----------------------------------------------------------------------
$ DELETE = "DELETE/NOLOG/NOCONFIRM"
$ PURGE  = "PURGE/NOLOG/NOCONFIRM"
$ ECHO   = "WRITE SYS$OUTPUT"
$!
$ ON WARNING THEN EXIT
$ PROC     = P1
$ IF PROC.EQS."" THEN PROC = "[]"
$ WIP      = F$PARSE(PROC,,,"DEVICE","SYNTAX_ONLY") + -
             F$PARSE(PROC,,,"DIRECTORY","SYNTAX_ONLY")
$!
$ SRC      = WIP - "]" + ".SRC]"
$ INC      = SRC - "]" + ".INCLUDE]"
$ BRANCH   = SRC - "]" + ".BRANCH]"
$ DRIVERS  = SRC - "]" + ".DRIVERS]"
$ FIT      = SRC - "]" + ".FIT]"
$ IMAGES   = SRC - "]" + ".IMAGES]"
$ INTERACT = SRC - "]" + ".INTERACT]"
$ PLOT     = SRC - "]" + ".PLOT]"
$ SYSDEP   = SRC - "]" + ".SYSDEP]"
$ VARIABLE = SRC - "]" + ".VARIABLES]"
$ WIPHELP  = WIP + "WIPHELP.DAT"
$!
$! Check for VMS or AXP.
$!
$ ON WARNING THEN GOTO VAX
$ MACHINE=F$GETSYI("ARCH_NAME")
$ IF MACHINE .EQS. "AXP" THEN GOTO AXP
$ IF MACHINE .EQS. "Alpha" THEN GOTO AXP
$ GOTO VAX
$VAX:
$  ECHO "OpenVMS VAX"
$  MACHINE="VAX"
$  CCOMP = "CC"
$  GOTO START
$AXP:
$  ECHO "OpenVMS AXP"
$  CCOMP = "CC/STANDARD=VAXC"
$  GOTO START
$START:
$ CCOMPILE = CCOMP + /NODEBUG + /INCLUDE_DIRECTORY=('INC',[])
$!
$ ON WARNING THEN GOTO EXIT
$!
$! Check that the cpg routines exist.
$!
$ IF F$SEARCH("cpgplot.h") .EQS. ""
$ THEN
$    ECHO "Install CPG first!"
$    EXIT
$ ENDIF
$!
$! Check that the main file exists.
$!
$ MAINFILE = F$SEARCH(BRANCH+"WIPMAIN.C")
$ IF MAINFILE .EQS. ""
$ THEN
$    ECHO "WIP Directory structure is invalid!"
$    EXIT
$ ENDIF
$!
$! Create the object-module library.
$!
$ ECHO "Creating object-module library"
$ LIBRARY/CREATE=(BLOCKS:200)/NOLOG TEMP.OLB 
$!
$! Compile the WIP routines and insert them in the library.
$!
$ ECHO "Compiling WIP library routines from ", SRC
$!
$ CALL COMPILE 'BRANCH'
$ CALL COMPILE 'DRIVERS'
$ CALL COMPILE 'FIT'
$ CALL COMPILE 'IMAGES'
$ CALL COMPILE 'INTERACT'
$ CALL COMPILE 'PLOT'
$ CALL COMPILE 'SYSDEP'
$ CALL COMPILE 'VARIABLE'
$!
$ RENAME TEMP.OLB LIBWIP.OLB
$ SET FILE/PROTECTION=(S:RWED,O:RWED,G:RE,W:RE) LIBWIP.OLB;*
$ PURGE LIBWIP.OLB
$!
$! Create an option list.  This may need to be
$! modified depending on the devices installed.
$!
$ CREATE WIP.OPT
pgplot_dir:LIBWIP.OLB/lib
pgplot_dir:CPGPLOT.OLB/lib
pgplot_dir:GRPCKG.OLB/lib
SYS$SHARE:DECW$XLIBSHR.EXE/share
SYS$SHARE:DECC$SHR.EXE/share
$!
$! Compile the main program.
$!
$ ECHO "Compiling main program WIP.EXE"
$ CCOMPILE 'BRANCH'WIPMAIN.C /DEFINE=(WIPVMS)
$ LINK/EXEC=WIP.EXE WIPMAIN, WIP.OPT/OPT
$ DELETE WIPMAIN.OBJ;*
$ SET PROTECTION=(S:RWED,O:RWED,G:RE,W:RE) WIP.EXE
$ PURGE WIP.EXE
$!
$! All done.
$!
$EXIT:
$ EXIT
$!
$! Subroutine called to compile the files in each directory.
$!
$COMPILE:
$ SUBROUTINE
$   ECHO "Compiling WIP library routines in ", P1
$   FILES = P1 + "*.C"
$LOOP:
$     FILE = F$SEARCH(FILES)
$     IF FILE .EQS. ""       THEN GOTO ENDLOOP
$     IF FILE .EQS. MAINFILE THEN GOTO LOOP
$     NAME = F$PARSE(FILE,,,"NAME","SYNTAX_ONLY")
$     ECHO NAME
$       CCOMPILE 'FILE' /DEFINE=(WIPVMS,"HELPFILE=""''WIPHELP'""")
$       LIBRARY/REPLACE TEMP 'NAME'.OBJ
$       DELETE 'NAME'.OBJ;*
$     GOTO LOOP
$   ENDLOOP:
$ ENDSUBROUTINE
$! End of script
