#!/bin/bash -l
#SBATCH --time=24:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=12
#SBATCH --mem=128G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

source ../.venv/bin/activate
python atmosphere_to_histograms.py --direc ./30deg-multi-scatter -e 10 -E 300 -d 1 --output-direc veryfine-momentum-histograms.d --elevation-angle 30
