all: build

APP=treefm

build: dirlist.o treefm.c
	gcc -Wall -g -o $(APP) $^ -lncurses

dirlist.o: dirlist.c
	gcc -Wall -g -c $^ -o $@

clean:
	rm *.o $(APP) 2> /dev/null || true

.PHONY: all build clean
