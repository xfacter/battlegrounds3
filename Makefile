BINDIR = ./bin

BUILD_PRX = 1
TARGET = $(BINDIR)/Battlegrounds3
EXTRA_TARGETS = $(BINDIR)/EBOOT.PBP
PSP_EBOOT = $(BINDIR)/EBOOT.PBP
PSP_EBOOT_SFO = $(BINDIR)/PARAM.SFO
PSP_EBOOT_TITLE = Battlegrounds3

CFLAGS = -g -Wall -O3
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpspgum_vfpu -lpspvfpu -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lm
LDFLAGS =

OBJS = main.o xlib/xlib.o xlib/xmem.o xlib/xlog.o xlib/xmath.o xlib/xtime.o xlib/xctrl.o xlib/xgraphics.o xlib/xtexture.o xlib/xtext.o xlib/xsound.o xlib/xobj.o xlib/xheightmap.o xlib/xini.o xlib/xparticle.o xlib/xbuffer.o values.o bg3_util.o map.o astar.o base.o game.o menu.o

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

*.o: xlib/xconfig.h

xlib/xlib.o: xlib/xlib.c xlib/xlib.h

xlib/xmem.o: xlib/xmem.c xlib/xmem.h

xlib/xlog.o: xlib/xlog.c xlib/xlog.h

xlib/xmath.o: xlib/xmath.c xlib/xmath.h

xlib/xtime.o: xlib/xtime.c xlib/xtime.h

xlib/xctrl.o: xlib/xctrl.c xlib/xctrl.h

xlib/xgraphics.o: xlib/xgraphics.c xlib/xgraphics.h

xlib/xtexture.o: xlib/xtexture.c xlib/xtexture.h

xlib/xtext.o: xlib/xtext.c xlib/xtext.h

xlib/xsound.o: xlib/xsound.c xlib/xsound.h

xlib/xobj.o: xlib/xobj.c xlib/xobj.h

xlib/xheightmap.o: xlib/xheightmap.c xlib/xheightmap.h

xlib/xini.o: xlib/xini.c xlib/xini.h

xlib/xparticle.o: xlib/xparticle.c xlib/xparticle.h

xlib/xbuffer.o: xlib/xbuffer.c xlib/xbuffer.h

values.o: values.c values.h

bg3_util.o: bg3_util.c bg3_util.h

map.o: map.c map.h

astar.o: astar.c astar.h

base.o: base.c base.h map.o values.o bg3_util.o

game.o: game.c game.h base.o values.o bg3_util.o

menu.o: menu.c menu.h base.o values.o bg3_util.o

main.o: main.c xlib/xlib.o xlib/xmem.o xlib/xlog.o xlib/xmath.o xlib/xtime.o xlib/xctrl.o xlib/xgraphics.o xlib/xtexture.o xlib/xtext.o xlib/xsound.o xlib/xobj.o xlib/xheightmap.o xlib/xini.o xlib/xparticle.o xlib/xbuffer.o values.o bg3_util.o map.o astar.o base.o game.o menu.o

EBOOT.PBP: main.o
