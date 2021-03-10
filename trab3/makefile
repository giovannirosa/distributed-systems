all: bebcast clear

bebcast: bebcast.o vcube2.o cisj.o smpl.o array.o rand.o
	$(LINK.c) -o $@ -Bstatic bebcast.o vcube2.o cisj.o smpl.o array.o rand.o -lm

bebcast.o: src/bebcast.c src/bebcast.h
	$(COMPILE.c) -g src/bebcast.c

vcube2.o: ../lib/vcube2.c ../lib/vcube2.h
	$(COMPILE.c) -g ../lib/vcube2.c

cisj.o: ../lib/cisj.c ../lib/cisj.h
	$(COMPILE.c) -g ../lib/cisj.c

smpl.o: ../lib/smpl.c ../lib/smpl.h
	$(COMPILE.c) -g ../lib/smpl.c

array.o: ../lib/array.c ../lib/array.h
	$(COMPILE.c) -g ../lib/array.c

rand.o: ../lib/rand.c
	$(COMPILE.c) -g ../lib/rand.c

clear:
	$(RM) *.o

clean:
	$(RM) bebcast

