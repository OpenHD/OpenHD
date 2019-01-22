ifdef RPISDK
	CROSS_COMPILE ?= arm-linux-gnueabihf-
endif

CC = $(CROSS_COMPILE)gcc
CCOPTS =  -Wall -Wextra -Wdouble-promotion -Wshadow

ifdef DEBUG
CCOPTS += -O0 -g
else
CCOPTS = -O2
endif

INCLUDEFLAGS=-I$(RPISDK)/opt/vc/include -I$(RPISDK)/opt/vc/include/interface/vmcs_host/linux -I$(RPISDK)/opt/vc/include/interface/vcos/pthreads -I..
LIBFLAGS= -Wl,-rpath=$(RPISDK)/opt/vc/lib -L$(RPISDK)/opt/vc/lib -lEGL -lGLESv2 -lbcm_host -pthread -ljpeg -lpng -lfreetype -lfontconfig -lz

all: shapedemo hellovg mouse-hellovg particles clip ptest dot

shapedemo:	shapedemo.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o shapedemo shapedemo.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

test:	shapedemo
	./shapedemo demo 5

newfont: newfont.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  newfont newfont.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

chars:	chars.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  chars chars.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

hellovg:	hellovg.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  hellovg hellovg.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

mouse-hellovg:	mouse-hellovg.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  mouse-hellovg mouse-hellovg.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

particles:	particles.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  particles particles.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

clip:	clip.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  clip clip.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

screensize:	screensize.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  screensize screensize.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

indent:
	indent -linux -c 60 -brf -l 132 shapedemo.c hellovg.c mouse-hellovg.c particles.c screensize.c

ptest:	ptest.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  ptest ptest.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

fontlist:	fontlist.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o  fontlist fontlist.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

fonttest:	fonttest.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o fonttest fonttest.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

image:	image.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o image image.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

dot:	dot.c ../libshapes.o ../oglinit.o ../fontsystem.o
	$(CC) $(CCOPTS) $(INCLUDEFLAGS) -o dot dot.c ../libshapes.o ../oglinit.o ../fontsystem.o $(LIBFLAGS)

