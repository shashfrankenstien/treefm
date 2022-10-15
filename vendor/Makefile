DEPS=
WINCON=
CLEAN=

ifeq ($(OS),Windows_NT)
	DEPS=pdcursesmod
	WINCON=git clone https://github.com/Bill-Gray/PDCursesMod.git $(DEPS) && cd $(DEPS)\wincon && make
	CLEAN=rmdir /s /q $(DEPS)
endif


all: $(DEPS)


$(DEPS):
	$(WINCON)


clean:
	$(CLEAN)


.PHONY: all clean
