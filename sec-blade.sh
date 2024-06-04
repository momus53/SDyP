#!/bin/bash

#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH -o /nethome/sdyp8/secuencial/salidas/output.txt
#SBATCH -e /nethome/sdyp8/secuencial/salidas/errors.txt
gcc -o secuencial secuencial.c
./secuencial $1
