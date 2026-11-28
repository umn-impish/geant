#!/bin/bash -l
#SBATCH --time=2:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu

outdir="$HOME/response-columns"
mkdir -p "$outdir"

cgen_dir="$HOME/payload-central-gen-completed"
hg_dir="$HOME/g4-histogrammed-runs"
ninc_payload=10000000
ninc_atm=10000000

# We've been using 1 keV bins throughout the whole process
for left_edge in {10..299}; do
    right_edge=$((left_edge + 1))
    hg_fn="$hg_dir/$left_edge.0-$right_edge.0.asdf"
    cgen_out_fn="$cgen_dir/completed-$left_edge.0-$right_edge.0.tar.gz"
    out_col_fn="$outdir/$left_edge-$right_edge-resp.asdf"
    sbatch generate-single-response-col.slurm $hg_fn $cgen_out_fn $ninc_atm $ninc_payload $out_col_fn
done
