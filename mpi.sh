#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=4
#SBATCH -o /nethome/sdyp8/mpi/salidas/output.txt
#SBATCH -e /nethome/sdyp8/mpi/salidas/errors.txt
mpicc mpi.c -o mpi.o -lm
mpirun -np 8 mpi.o $1 #El valor de np se modifica en funcion de los valores de (-N * --task-per-node)
