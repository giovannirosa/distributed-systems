all: vring clear

vring: vring.o smpl.o rand.o array.o
	$(LINK.c) -o $@ -Bstatic vring.o smpl.o rand.o array.o -lm

smpl.o: ../lib/smpl.c ../lib/smpl.h
	$(COMPILE.c) -g ../lib/smpl.c

array.o: ../lib/array.c ../lib/array.h
	$(COMPILE.c) -g ../lib/array.c

vring.o: src/vring.c src/vring.h
	$(COMPILE.c) -g src/vring.c

rand.o: ../lib/rand.c
	$(COMPILE.c) -g ../lib/rand.c

clear:
	$(RM) *.o

clean:
	$(RM) vring

