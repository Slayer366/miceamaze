UNAME := $(shell uname)

INSTALL_ICONS=0

PREFIX=.
BINDIR=${PREFIX}
DATAPATH=${PREFIX}/data
MANPATH=${PREFIX}/man

PIXPATH=${PREFIX}/pixmaps
MENUPATH=${PREFIX}
ICONPATH=${PREFIX}/icons

CXX      ?= g++
CXXFLAGS += -fsigned-char -Wall -DDATAPATH=\"${DATAPATH}\" $(shell sdl2-config --cflags)

LDLIBS = -fsigned-char -Wall $(shell sdl2-config --libs) -lSDL2_mixer -lSDL2_ttf

ifeq ($(UNAME), Darwin)
  LDLIBS += -framework OpenGL -lm
else
  LDLIBS += -lGL -lm
endif

OBJS := $(sort $(patsubst src/%.cpp,src/%.o,$(wildcard src/*.cpp)))

all: miceamaze miceamaze.6.gz

miceamaze: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

miceamaze.6.gz: man/miceamaze.6
	gzip -nc $^ > $@

clean:
	rm -f miceamaze miceamaze.6.gz src/*.o

install: all
	install -d -m 755 ${DESTDIR}${DATAPATH}
	cp -R -L data/* ${DESTDIR}${DATAPATH}/
	install -D -m 755 miceamaze ${DESTDIR}${BINDIR}/miceamaze
	install -D -m 644 miceamaze.6.gz ${DESTDIR}${MANPATH}/miceamaze.6.gz
ifeq ($(INSTALL_ICONS), 1)
	install -D -m 644 miceamaze.desktop ${DESTDIR}${MENUPATH}/miceamaze.desktop
	install -D -m 644 miceamaze.png ${DESTDIR}${ICONPATH}/miceamaze.png
	install -D -m 644 miceamaze.xpm ${DESTDIR}${PIXPATH}/miceamaze.xpm
endif

uninstall:
	rm -rf ${DESTDIR}${DATAPATH}
	rm -f ${DESTDIR}${BINDIR}/miceamaze
	rm -f ${DESTDIR}${MANPATH}/miceamaze.6.gz
ifeq ($(INSTALL_ICONS), 1)
	rm -f ${DESTDIR}${MENUPATH}/miceamaze.desktop
	rm -f ${DESTDIR}${ICONPATH}/miceamaze.png
	rm -f ${DESTDIR}${PIXPATH}/miceamaze.xpm
endif

re: clean all
