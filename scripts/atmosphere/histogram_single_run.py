import argparse
import itertools
import pathlib

import asdf
import numpy as np


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Accepts a single output data folder of a Geant4 run "
            "along with some additional information to bin the run outputs "
            "into a momentum (energy) histogram. "
            "The histogram is written to an ASDF file."
        )
    )
    parser.add_argument(
        "-D",
        "--input-direc",
        type=str,
        required=True,
        help="Directory containiing G4 run",
    )
    parser.add_argument(
        "-e",
        "--e-min",
        type=float,
        required=True,
        help="The left energy bin edge for this G4 run",
    )
    parser.add_argument(
        "-E",
        "--e-max",
        type=float,
        required=True,
        help="The right energy bin edge for this G4 run",
    )
    parser.add_argument(
        "-n",
        "--num-momentum-bins",
        type=int,
        required=True,
        help="Number of momentum bins to span (-e_max, e_max); even number",
    )
    parser.add_argument(
        "-o",
        "--output-direc",
        type=str,
        required=True,
        help="The directory to which the .asdf histogram is saved",
    )
    args = parser.parse_args()

    if args.num_momentum_bins % 2 != 0:
        raise ValueError("Number of momentum bins must be an even number")

    print("Load momenta")
    all_momenta = []
    all_energies = []
    with open(pathlib.Path(args.input_direc) / "cryst-out.tab", "r") as f:
        for line in f:
            # Assumed data format:
            # - [evt|track x N] [(position, E) x N] [(momentum) x N]
            portions = line.strip().split()
            num_hits = sum("|" in p for p in portions)

            # Need the +1 because of the energy entry
            pos_iter, mom_iter = (
                itertools.islice(portions, num_hits + 1, 2 * num_hits + 1),
                itertools.islice(portions, 1 + 2 * num_hits, None),
            )
            # Energy is stored in the last entry of position.
            energies = [float(x[1:-1].split(",")[-1]) for x in pos_iter]
            momenta = [[float(x) for x in p[1:-1].split(",")] for p in mom_iter]
            all_momenta += momenta
            all_energies += energies

    all_momenta = np.array(all_momenta)
    all_energies = np.array(all_energies)
    print("Done loading; time to histogram")

    # Energy bins span from (-max --> +max)
    # with uniform delta E, and the origin is at the edge of a bin
    # by construction.
    # The momentum is in units of energy (E = pc with c = 1)
    emin, emax = args.e_min, args.e_max
    half_num = args.num_momentum_bins // 2
    energy_bins = np.concatenate(
        (np.linspace(-emax, 0, half_num), np.linspace(0, emax, half_num))
    )

    # Bin the momenta weighted by (1 / z-axis angle cosine)
    # See: Shultis, J. K., & Faw, R. E. (2011). An MCNP primer, Sec 3.6.2
    pz = all_momenta[:, 2]
    # All momentum vectors are unit vectors
    cosine = pz / 1.0
    momentum_hg, *_ = np.histogramdd(
        (all_momenta.T * all_energies).T,
        bins=(energy_bins, energy_bins, energy_bins),
        weights=(1 / np.abs(cosine)),
    )

    out_dir = pathlib.Path(args.output_direc)
    out_dir.mkdir(exist_ok=True, parents=True)
    print("Done histogramming; time to save")
    af = asdf.AsdfFile(tree={"histogram": momentum_hg, "bin_edges": energy_bins})
    af.write_to(out_dir / f"{emin}-{emax}.asdf", all_array_compression="bzp2")


if __name__ == "__main__":
    main()
