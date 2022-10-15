all: deps build

DEPS=
APP=
INCLUDES=
LIBFLAGS=
CLEAN=

ifeq ($(OS),Windows_NT)
	DEPS=(cd vendor && make deps)
	APP=treefm.exe
	INCLUDES=-I.\vendor\pdcursesmod
	LIBFLAGS=.\vendor\pdcursesmod\wincon\pdcurses.a -lwinmm -lm
	CLEAN=del *.o $(APP)
else
	DEPS=(ls)
	APP=treefm
	INCLUDES=
	LIBFLAGS=-lncurses -lm
	CLEAN=rm *.o $(APP) 2> /dev/null || true
endif


deps:
	$(DEPS)

build: dirlist.o ui.o preview.o treefm.c
	gcc -Wall -g -o $(APP) $^ $(INCLUDES) $(LIBFLAGS)

dirlist.o: dirlist.c
	gcc -Wall -g -c $^ -o $@

ui.o: ui.c
	gcc -Wall -g -c $^ -o $@ $(INCLUDES)

preview.o: preview.c
	gcc -Wall -g -c $^ -o $@ $(INCLUDES)

clean:
	$(CLEAN)

.PHONY: all deps build clean
