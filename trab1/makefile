all: vring clear

vring: vring.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic vring.o smpl.o rand.o -lm

smpl.o: ../lib/smpl.c ../lib/smpl.h
	$(COMPILE.c) -g ../lib/smpl.c

vring.o: src/vring.c ../lib/smpl.h
	$(COMPILE.c) -g src/vring.c

rand.o: ../lib/rand.c
	$(COMPILE.c) -g ../lib/rand.c

clear:
	$(RM) *.o

clean:
	$(RM) vring

