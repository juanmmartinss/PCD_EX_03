rm *.o
rm *.btr
mpicc -o exec.o jogodavidaMPI.c
mpirun -np 8 ./exec.o