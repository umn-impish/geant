#!/bin/bash -l
#SBATCH --time=24:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=1G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

for e in {10..299}; do
    sbatch histogram-single-g4-run $e
done
