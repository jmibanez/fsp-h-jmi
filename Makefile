CC = gcc
C_FLAGS = -DNOSTDLIB -DHAVE_XPM -Wall -I/usr/X11R6/include -m386 -falign-loops=2 -falign-jumps=2 -falign-functions=2
L_FLAGS = -nostdlib -O1 -Xlinker -s -L/usr/X11R6/lib -lX11 -lXpm
PROGNAME = fspanel

$(PROGNAME): Makefile fspanel.c fspanel.h icon.xpm
	$(CC) $(C_FLAGS) $(L_FLAGS) fspanel.c -o $(PROGNAME)
	@ls -l $(PROGNAME)

clean: 
	rm -f core *.o $(PROGNAME) nohup.out

install: $(PROGNAME)
	cp $(PROGNAME) /usr/local/bin
