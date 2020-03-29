all: build

APP=treefm

build: dirlist.o treefm.c
	gcc -Wall -lncurses -o $(APP) $^

dirlist.o: dirlist.c
	gcc -Wall -c $^ -o $@

clean:
	rm *.o $(APP)
