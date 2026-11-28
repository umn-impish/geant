#!/bin/bash -l
#SBATCH --time=24:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --tmp=10G
#SBATCH --cpus-per-task=4
#SBATCH --mem=8G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

set -euo pipefail

lefte=$1

# Make output directory if not exist
outdir="$HOME/g4-histogrammed-runs"
mkdir -p $outdir

export TMPDIR="/scratch.local/"
workdir=$(mktemp -d)
cd $workdir
# Only grab the first file with the specified left edge, in case
# there are duplicates for some reason
target_file=$(ls -1 $HOME/geant4-outputs/g4-run-$lefte-* | head -n1)
tar -xzvf $target_file

source $HOME/impish-geant/scripts/.venv/bin/activate
python $HOME/impish-geant/scripts/atmosphere/histogram_single_run.py \
    --input-direc atm-resp-$lefte*/ \
    --e-min $lefte \
    --e-max $((lefte + 1)) \
    --num-momentum-bins 100 \
    -o .

cp *.asdf $outdir

# If there are multiple files in the scratch dir (?),
# we copied them
all=$(ls *.asdf | tr "\n" " ")
echo "copied $all to $outdir"
