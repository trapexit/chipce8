HUC=huc
PCEAS=pceas
OPTS=-t -O2 -fno-recursive -msmall
FILES=chipce8.c emulator.c bcd.c screen.c menu.c font.c joypad.c sprite.c psg.c *.inc *.asm

all: chipce8.pce

chipce8.pce: ${FILES}
	$(HUC) $(OPTS) chipce8.c

iso: ${FILES}
	$(HUC) -scd $(OPTS) chipce8.c

clean:
	rm -f chipce8.pce
	rm -f chipce8.iso
	rm -f chipce8.s
	rm -f chipce8.sym

run: chipce8.pce
	mednafen chipce8.pce

roms:
	./tools/convert-roms

.PHONY: roms
