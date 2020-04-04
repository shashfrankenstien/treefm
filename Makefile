all: build

APP=treefm

build: dirlist.o treefm.c
	gcc -Wall -g -lncurses -o $(APP) $^

dirlist.o: dirlist.c
	gcc -Wall -g -c $^ -o $@

clean:
	rm *.o $(APP)
