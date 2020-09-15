all: build

APP=
LIBFLAGS=
CLEAN=

ifeq ($(OS),Windows_NT)
	APP=treefm.exe
	LIBFLAGS=-I..\PDCursesMod ..\PDCursesMod\wincon\pdcurses.a -lwinmm
	CLEAN=del *.o $(APP)
else
	APP=treefm
	LIBFLAGS=-lncurses
	CLEAN=rm *.o $(APP) 2> /dev/null || true
endif

build: dirlist.o treefm.c
	gcc -Wall -g -o $(APP) $^ $(LIBFLAGS)

dirlist.o: dirlist.c
	gcc -Wall -g -c $^ -o $@

clean:
	$(CLEAN)

.PHONY: all build clean
