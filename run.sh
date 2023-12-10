rm *.o
mpicc -o exec.o jogodavidaMPI.c
# mpirun -np 8 ./exec.o
# mpirun -np 4 ./exec.o
# mpirun -np 2 ./exec.o
mpirun -np 1 ./exec.o