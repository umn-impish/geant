import argparse
import os
import pathlib
from collections import defaultdict
from typing import cast

import asdf
import multiprocess as mp
import numpy as np
from scipy.spatial.transform import Rotation

"""
A series of functions for processing Geant4 runs into 3D momentum histograms
which can be resampled to produce an accurate response for IMPISH,
or other balloon-borne instruments.

The exact process behind why we are computing things in this way is rather complicated,
so please read the IMPISH instrument paper for more details than what are present here.
"""


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Take a set of Geant4 directories with a 'passthrough' detector "
            "and process the momenta into histograms which can be sampled later. "
            "Outputs an asdf file."
        )
    )
    parser.add_argument(
        "-D",
        "--direc",
        type=str,
        required=True,
        help="The directory containing all of the G4 runs data",
    )
    parser.add_argument(
        "-e",
        "--e-min",
        type=float,
        required=True,
        help="The minimum energy (lowest energy bin edge) of the G4 runs",
    )
    parser.add_argument(
        "-E",
        "--e-max",
        type=float,
        required=True,
        help="The maximum energy (highest energy bin edge) of the G4 runs",
    )
    parser.add_argument(
        "-d",
        "--delta-e",
        type=float,
        required=True,
        help="The energy delta for each model energy bin per G4 run",
    )
    parser.add_argument(
        "-o",
        "--max-ord",
        type=int,
        required=True,
        help="The maximum scattering order to include in the atmosphere matrix",
    )
    parser.add_argument(
        "-f",
        "--output-file",
        type=str,
        required=True,
        help="The ASDF file to which we write out all of the histograms",
    )
    parser.add_argument(
        "-a",
        "--elevation-angle",
        type=float,
        required=True,
        help="The elevation angle (in degrees) above the X axis in the XZ plane used during simulation.",
    )

    program_args = parser.parse_args()

    e_min: float = program_args.e_min
    de: float = program_args.delta_e

    def energy_bin_from_direc(p: pathlib.Path) -> tuple[float, float]:
        start = e_min
        run = next(part for part in reversed(p.stem.split("-")) if "run" in part)
        run_num = int(run[3:])
        high = start + run_num * de
        return (high - de, high)

    max_momentum_order = program_args.max_ord

    out_fn = program_args.output_file
    if not out_fn.endswith(".asdf"):
        out_fn += ".asdf"

    with asdf.AsdfFile() as af:
        af.write_to(out_fn)

    direcs = sorted(pathlib.Path(program_args.direc).iterdir())
    ncpu = cast(int, os.cpu_count())
    splits: list[np.ndarray] = np.array_split(ary=direcs, indices_or_sections=ncpu)  # pyright: ignore[reportCallIssue, reportArgumentType]
    args = [
        (i, s, out_fn, program_args.elevation_angle) for (i, s) in enumerate(splits)
    ]

    def work(args: tuple[int, np.ndarray, str, float]) -> None:
        i, direcs, fn_base, elev = args
        out_fn = f"temp{i}-" + fn_base
        with asdf.AsdfFile() as af:
            af.write_to(out_fn)

        for d in direcs:
            min_e, max_e = energy_bin_from_direc(d)
            k = str((min_e, max_e))
            with asdf.open(out_fn, mode="r") as af:
                if k in af.tree:  # pyright: ignore[reportOperatorIssue]
                    # Already have this bin computed
                    continue

            by_track = load_momenta_per_track(direc=d, rotation_angle=elev)
            by_order = split_momenta_by_scatter_order(
                by_track, max_order=max_momentum_order
            )

            # Number of momentum space bins in the resulting histograms, one per dimension
            num_bins = 64
            cartesian_bin_edges = np.linspace(-max_e, max_e, num_bins + 1)
            momenta_hist = histogram_momenta(
                edges=cartesian_bin_edges, momenta=by_order
            )

            with asdf.open(out_fn, mode="rw") as af:
                af.tree[k] = {  # pyright: ignore[reportIndexIssue]
                    "bin_edges": cartesian_bin_edges,
                } | momenta_hist
                af.update(all_array_compression="bzp2")

            print(f"done {min_e, max_e}")

    with mp.Pool(ncpu) as p:
        p.map(work, args)

    # Merge the temporary files
    for f in pathlib.Path(".").glob("temp*"):
        with asdf.open(f, mode="r") as af_tmp, asdf.open(out_fn, mode="rw") as af:
            af.tree |= af_tmp.tree.copy()  # pyright: ignore[reportOperatorIssue, reportAttributeAccessIssue]
            af.update(all_array_compression="bzp2")
        f.unlink(missing_ok=True)
        print("done merging", f)


def load_momenta_per_track(
    direc: pathlib.Path,
    rotation_angle: float,
) -> dict[tuple[int, int], list[np.ndarray]]:
    """Given a path to some Geant4 run outputs which were recorded by a passthrough detector,
    load the momenta per detector hit into a dictionary which sorts them by the unique track identfiers.

    This lets us keep track of (ha ha) which hits correspond to which tracks so that we don't double count.

    For example: say a single particle track passes through the detector altitude (~40 km),
    backscatters once, passes back through the detector, scatters again, passes through, and then dies.
    There will be ae total of three hits corresponding to a single track.
    Each hit might be at a different energy if the scattering process was inelastic.
    No matter what, we want to track the momentum and energy of the particle at each hit,
    but keep them separate.

    The entry for this track would look something like this:

        `(123, 0): [p_a, p_b, p_c]`

    where pa, pb, pc are the momenta at each point, normalized such that norm(p_i) = E.

    The **rotation angle** is the elevation above the horizon from which photons emanated.
    The whole momentum coordinate system is rotated so that that nominal direction
    aligns with the Z axis.
    This is th simplify the creation of response matrices later.
    """
    momenta_by_track = defaultdict(list)
    rot_mat = Rotation.from_euler("y", angles=rotation_angle - 90, degrees=True).as_matrix()

    with open(direc / "cryst-out.tab") as f:
        for line in f:
            these_events = []
            split = line.strip().split()
            for part in split:
                if "|" not in part:
                    # We finished getting event/track IDs
                    break
                these_events.append(tuple(int(x) for x in part.split("|")))

            # We have the number of events now
            n_evt = len(these_events)
            collapsed = split[n_evt:]
            pos, mom = collapsed[:n_evt], collapsed[n_evt:]
            pos = [tuple(float(x) for x in p[1:-1].split(",")) for p in pos]
            # Rotate each momentum so that its nominal axis (along the LoS) aligns with the Z axis
            mom = [rot_mat @ np.array(tuple(float(x) for x in m[1:-1].split(","))) for m in mom]

            for k, x, p in zip(these_events, pos, mom):
                # Ignore the position stored in x[:2]
                momenta_by_track[k].append(
                    # The norm of the momentum vector is the energy of the photon
                    # Energy is in the -1th element of position. Whatever.
                    x[-1] * np.array(p)
                )

    return momenta_by_track


def split_momenta_by_scatter_order(
    momenta: dict[tuple[int, int], list[np.ndarray]], max_order: int
) -> list[list[np.ndarray]]:
    """Take momentum vectors which have been grouped by track ID and split them instead by scattering order.
    The scattering order describes how many times a particle scatters back into a detector volume before being absorbed
    elsewhere. Order zero means the first time it hits the detector. Order one means a single backscatter. Order two means it scattered again
    after backscattering, etc."""
    evts_by_order = []
    for ord_idx in range(max_order):
        evts_by_order.append([])
        for v in momenta.values():
            try:
                evts_by_order[ord_idx].append(v[ord_idx])
            except IndexError:
                pass

    return evts_by_order


def sample_nd_histogram(
    hist: np.ndarray, edges: list[np.ndarray], num_samples: int
) -> np.ndarray:
    """Draw random samples from a multidimensional histogram.
    (thanks, Gemini)"""
    # Normalize histogram to form a valid probability distribution
    prob_density = hist.ravel() / hist.sum()

    # Randomly choose flat bin indices based on the bin weights
    flat_indices = np.random.choice(prob_density.size, size=num_samples, p=prob_density)

    # Unravel flat indices into D-dimensional subscripts
    multi_indices = np.unravel_index(flat_indices, hist.shape)

    # For each sample, pick a uniform random point within the chosen bin edges
    samples = np.zeros((num_samples, len(edges)))

    for d in range(len(edges)):
        bin_idx = multi_indices[d]
        low_edges = edges[d][bin_idx]
        high_edges = edges[d][bin_idx + 1]

        # Uniform random draw between low and high edge for dimension d
        samples[:, d] = np.random.uniform(low_edges, high_edges, size=num_samples)

    return samples


def histogram_momenta(
    edges: np.ndarray, momenta: list[list[np.ndarray]]
) -> dict[str, np.ndarray]:
    """Given a set of momentum vectors, whose norms are particle energy,
    make a 3D histogram across a sensible set of bin edges.
    The idea is that you can take draws from this histogram at a given energy bin to
    approximate the effect of the atmosphere.

    The histograms are split by scattering order. The total counts per histogram
    is analogous to the total probability that a given photon will originate from that
    scattering order.
    Say scattering order 0 has 90 counts, and order 1 has 10 counts, and higher orders have 0 counts.
    The relative sampling probabilities are then

        `[0.9. 0.1, 0]`

    where the list index corresponds to the scattering order.
    """
    # Scatter order after which we truncate the distributions
    histograms_by_order = []
    total_counts = []
    for ord_ in range(len(momenta)):
        momentum_order = np.array(momenta[ord_])
        try:
            momentum_hg, _ = np.histogramdd(
                sample=momentum_order, bins=(edges, edges, edges)
            )
            histograms_by_order.append(momentum_hg)
        except ValueError:
            # There are no hits to histogram
            histograms_by_order.append(
                np.zeros((edges.size - 1, edges.size - 1, edges.size - 1))
            )
        # The total counts will weight the first roll: what order do we use?
        total_counts.append(momentum_order.shape[0])
    return {
        "momentum_histograms": np.array(histograms_by_order, dtype=int),
        "total_counts": np.array(total_counts, dtype=int),
    }


if __name__ == "__main__":
    main()
