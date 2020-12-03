all: tempo tarefa1 tarefa2 tarefa3 tarefa4 clear

tempo: tempo.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tempo.o smpl.o rand.o -lm

tarefa1: tarefa1.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa1.o smpl.o rand.o -lm

tarefa2: tarefa2.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa2.o smpl.o rand.o -lm

tarefa3: tarefa3.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa3.o smpl.o rand.o -lm

tarefa4: tarefa4.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa4.o smpl.o rand.o -lm

smpl.o: ../lib/smpl.c ../lib/smpl.h
	$(COMPILE.c) -g ../lib/smpl.c

tempo.o: src/tempo.c ../lib/smpl.h
	$(COMPILE.c) -g src/tempo.c

tarefa1.o: src/tarefa1.c ../lib/smpl.h
	$(COMPILE.c) -g src/tarefa1.c

tarefa2.o: src/tarefa2.c ../lib/smpl.h
	$(COMPILE.c) -g src/tarefa2.c

tarefa3.o: src/tarefa3.c ../lib/smpl.h
	$(COMPILE.c) -g src/tarefa3.c

tarefa4.o: src/tarefa4.c ../lib/smpl.h
	$(COMPILE.c) -g src/tarefa4.c

rand.o: ../lib/rand.c
	$(COMPILE.c) -g ../lib/rand.c

clear:
	$(RM) *.o

clean:
	$(RM) tarefa4 tarefa3 tarefa2 tarefa1 tempo

