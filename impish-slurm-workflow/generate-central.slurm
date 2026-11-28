#!/bin/bash -l
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --mem=128G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

source ../.venv/bin/activate
outdir="$SHARED/impish-geant/g4-real-full-response"
rm -rf $outdir
mkdir -p $outdir/data-out

projdir="$HOME/impish-geant"
python generate_many_central_gen.py \
    --output-directory "$outdir" \
    --momentum-hg-direc "$HOME/g4-histogrammed-runs" \
    --config-fn $projdir/simulation.config.file \
    --geant-executable $projdir/build/cadmesh-wrapper \
    --geometry-directory "$HOME/payload"\
    --polar-rotation -67.0
