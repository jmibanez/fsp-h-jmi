CC = gcc
C_FLAGS = -Wall -I/usr/X11R6/include
L_FLAGS = -O1 -Xlinker -s -L/usr/X11R6/lib -lX11 -lXpm
PROGNAME = fspanel

$(PROGNAME): Makefile fspanel.c fspanel.h icon.xpm
	$(CC) $(C_FLAGS) $(L_FLAGS) fspanel.c -o $(PROGNAME)
	@ls -l $(PROGNAME)

clean: 
	rm -f core *.o $(PROGNAME) nohup.out

install: $(PROGNAME)
	cp $(PROGNAME) /usr/local/bin
