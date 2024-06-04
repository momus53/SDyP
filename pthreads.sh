#!/bin/bash

#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=XeonPHI
#SBATCH -o /nethome/sdyp8/pthreads/salidas/output.txt
#SBATCH -e /nethome/sdyp8/pthreads/salidas/errors.txt
source $ONEAPI_PATH/setvars.sh > /dev/null 2>&1
icc -pthread -o pthreads.o pthreads.c
./pthreads.o $1 $2
