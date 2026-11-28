#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --mem=4G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

for script in runme-*; do
    sbatch run-single-central-payload.slurm $script
done
