# 	experimental Makefile for WIP - PJT - November 2000  (linux)
#       normally you would use 'domake' and 'makewip' - 
#       this file is intended for future Autoconf purposes

SRC = src/branch/execute.c src/branch/process.c src/branch/wipinit.c src/branch/wipmain.c \
      src/drivers/basic.c src/drivers/fits.c src/drivers/miriad.c \
      src/fit/fit.c src/fit/gaussfit.c src/fit/lsqfit.c src/fit/medfit.c src/fit/polyfit.c \
      src/images/extrema.c src/images/header.c src/images/heq.c src/images/image.c \
	src/images/smooth.c \
      src/interact/command.c src/interact/decode.c src/interact/help.c src/interact/ifxecute.c \
	src/interact/insert.c src/interact/interpret.c src/interact/list.c \
	src/interact/loopxecute.c src/interact/macros.c src/interact/maxecute.c \
	src/interact/parse.c src/interact/readinput.c \
      src/plot/aitoff.c src/plot/arc.c src/plot/array.c src/plot/arrow.c src/plot/cursor.c \
	src/plot/device.c src/plot/histo.c src/plot/imval.c src/plot/inquire.c \
	src/plot/levels.c src/plot/matrix.c src/plot/move.c src/plot/palette.c \
	src/plot/panel.c src/plot/phard.c src/plot/points.c src/plot/putlab.c \
	src/plot/quarter.c src/plot/reset.c src/plot/scale.c src/plot/set.c \
	src/plot/show.c src/plot/wedge.c \
      src/sysdep/filesize.c src/sysdep/inoutput.c src/sysdep/random.c src/sysdep/readata.c \
	src/sysdep/spool.c src/sysdep/unpack.c \
      src/variables/evaluate.c src/variables/find.c src/variables/pushpop.c \
	src/variables/str.c src/variables/var.c src/variables/vectors.c

CC = gcc
OPTS = -ansi -Dlinux
CFLAGS = -g 

CCMALLOC = -lccmalloc 

INC = -I./src/include -I$(MIRLIB)
HELP = \"wiphelp.dat\"

OBJ = $(SRC:.c=.o)

LIB = libwip.a

help:
	echo $(OBJ)

.c.o:
	$(CC) $(OPTS) $(CFLAGS) $(INC) -DHELPFILE=$(HELP) -c $< -o $*.o


all: wip

wip: $(LIB)
	$(CC) $(CFLAGS) -o wip libwip.a  -L$(MIRLIB) -L/usr/X11R6/lib \
	       -lcpgplot -lpgplot -lX11 -lg2c $(CCMALLOC) -lgcc -ldl -lm

# this doesn't work yet, wrong directory dependancies

depend:
	rm -f Makedepend
	${CC} -M $(OPTS) $(CFLAGS) $(INC) -DHELPFILE=$(HELP) $(SRC) > Makedepend


clean:
	rm -f libwip.a *.o wip

cleanall: clean
	rm -f src/*/*.o


$(LIB): $(OBJ)
	ar ruv $(LIB) $(OBJ)
