#!/bin/bash -l
#SBATCH --time=2:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=8G
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=sette095@umn.edu
#SBATCH --tmp=30G

# The energy of this run is the only positional argument
e=$1

set -euo pipefail

# Write data out into a node-local directory
stem="g4-run-$e-$(date -u +'%Y-%m-%dT%H-%M-%SZ')"
mydir="/scratch.local/$stem"
echo "local scratch dir is $mydir"
rm -rf $mydir
mkdir $mydir

config="data-dir string $mydir
save-prefix string atm-resp-$e
generator-type string gps
central-generator-config string momentum-meta.json
save-each-cryst-hit-momentum bool 1
save-all-scintillator-positions bool 1
save-track-identifiers bool 1

save-each-cryst-hit-energy bool 0
save-sipm-positions bool 0
save-sipm-energies bool 0
enable-scintillation bool 0

scintillator-time-const double 25.0
lyso-attenuation-length double 50.0
specular-sigma-alpha-deg double 2.0

world-x double 1000e6
world-y double 1000e6
world-z double 1000e6
"
echo "$config" > $mydir/config.file

macro="/gps/particle gamma
# Need to update this depending on the geometry
/gps/pos/centre 200 0 85 km
/gps/pos/type Point

/gps/direction -0.866025 0 -0.5

/gps/ene/type Lin
/gps/ene/gradient 0
/gps/ene/intercept 1
/run/printProgress 500000
/gps/ene/min $e keV
/gps/ene/max $((e + 1)) keV
/run/beamOn 10000000"
echo "$macro" > $mydir/macro.mac

export GEANT4_NUM_CORES=16
./cadmesh-wrapper "$mydir/config.file" meta.json "$mydir/macro.mac"

# After run is done, copy output data back onto main network storage
cd $mydir
echo "compressing g4 output"
outfn="$stem.tar.gz"
tar -czvf "$outfn" *
rsync -av "$outfn" /users/6/sette095/geant4-outputs/
