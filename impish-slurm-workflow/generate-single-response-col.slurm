#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=8G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

source ../.venv/bin/activate
python generate_response_column.py \
    --input-histogram "$1" \
    --g4-run-gzip "$2" \
    --num-incident-atm "$3" \
    --num-incident-payload "$4" \
    --output-file "$5"
