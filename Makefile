all: build

APP=
INCLUDES=
LIBFLAGS=
CLEAN=

ifeq ($(OS),Windows_NT)
	APP=treefm.exe
	INCLUDES=-I..\PDCursesMod
	LIBFLAGS=..\PDCursesMod\wincon\pdcurses.a -lwinmm -lm
	CLEAN=del *.o $(APP)
else
	APP=treefm
	INCLUDES=
	LIBFLAGS=-lncurses -lm
	CLEAN=rm *.o $(APP) 2> /dev/null || true
endif

build: dirlist.o ui.o treefm.c
	gcc -Wall -g -o $(APP) $^ $(INCLUDES) $(LIBFLAGS)

dirlist.o: dirlist.c
	gcc -Wall -g -c $^ -o $@

ui.o: ui.c
	gcc -Wall -g -c $^ -o $@ $(INCLUDES)

clean:
	$(CLEAN)

.PHONY: all build clean
