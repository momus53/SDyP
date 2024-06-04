#!/bin/bash

#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=XeonPHI
#SBATCH -o /nethome/sdyp8/secuencial/salidas/output.txt
#SBATCH -e /nethome/sdyp8/secuencial/salidas/errors.txt
source $ONEAPI_PATH/setvars.sh > /dev/null 2>&1
icc -o secuencial secuencial.c
./secuencial $1
