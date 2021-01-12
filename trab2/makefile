all: cisj clear

cisj: cisj.o
	$(LINK.c) -o $@ -Bstatic cisj.o -lm

cisj.o: src/cisj.c
	$(COMPILE.c) -g src/cisj.c

clear:
	$(RM) *.o

clean:
	$(RM) cisj

