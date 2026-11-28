#!/bin/bash -l
#SBATCH --time=2:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=24
#SBATCH --mem=8G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

set -euo pipefail

# Of the form runme-XX-YY
script=$1
IFS="-" read -ra energies <<< "$script"
ea="${energies[1]}"
eb="${energies[2]}"

cores=24
GEANT4_NUM_CORES=$cores ./$script

cd data-out
direc=$ea-$eb-*
tar -czvf "completed-$ea-$eb.tar.gz" $direc
rm -rf $direc
