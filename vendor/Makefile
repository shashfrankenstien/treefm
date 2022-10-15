all: pdcursesmod


pdcursesmod:
	git clone https://github.com/Bill-Gray/PDCursesMod.git $@


wincon: pdcursesmod
	cd $^\$@ && make


deps: wincon


clean:
	rmdir /s /q pdcursesmod


.PHONY: all deps clean wincon
