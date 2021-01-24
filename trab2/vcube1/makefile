all: vcube1 clear

vcube1: vcube1.o cisj.o smpl.o array.o rand.o
	$(LINK.c) -o $@ -Bstatic vcube1.o cisj.o smpl.o array.o rand.o -lm

vcube1.o: src/vcube1.c src/vcube1.h
	$(COMPILE.c) -g src/vcube1.c

cisj.o: ../../lib/cisj.c ../../lib/cisj.h
	$(COMPILE.c) -g ../../lib/cisj.c

smpl.o: ../../lib/smpl.c ../../lib/smpl.h
	$(COMPILE.c) -g ../../lib/smpl.c

array.o: ../../lib/array.c ../../lib/array.h
	$(COMPILE.c) -g ../../lib/array.c

rand.o: ../../lib/rand.c
	$(COMPILE.c) -g ../../lib/rand.c

clear:
	$(RM) *.o

clean:
	$(RM) vcube1

