import argparse
import io
import pathlib
import tarfile

import asdf
import numpy as np
from astropy import units as u

# Photon energy binning assumed to be 1 keV
DE = 1


def main():
    args = parse_args()

    tracks = None
    proj_areas = None
    print("Load from gzip")
    with tarfile.open(args.g4_run_gzip, "r:*") as tf:
        for m in tf.getmembers():
            if m.name.endswith("cryst-out.tab"):
                f = tf.extractfile(m)
                if not isinstance(f, io.BufferedReader):
                    raise ValueError("Can't load deposited hits from tarfile")
                print("Load in hits")
                tracks = load_hits(f)
                print("Done loading hits")
            elif m.name.endswith("spec-in.tab"):
                f = tf.extractfile(m)
                if not isinstance(f, io.BufferedReader):
                    raise ValueError("Can't load incident photons from tarfile")
                print("Load in areas")
                proj_areas = load_projected_areas(f, args.num_incident_payload)
                print("Done loading areas")
    print("Done loading")

    # Did these get loaded?
    assert tracks is not None
    assert proj_areas is not None

    input_fn = pathlib.Path(args.g4_run_gzip).name
    emin = float(input_fn.split("-")[1])
    emax = emin + DE
    response_cols = compute_drm_columns(tracks, proj_areas, args, emax)

    # The energy bin is encoded in the input file name
    response_cols["energy_bin"] = [emin, emax] << u.keV
    af_out = asdf.AsdfFile(tree=response_cols)
    print("Write out response columns")
    # No compression: the file shouldn't be very large
    af_out.write_to(pathlib.Path(args.output_file).resolve())


def compute_drm_columns(
    tracks: np.ndarray,
    proj_areas: np.ndarray,
    args: argparse.Namespace,
    max_energy: float,
) -> dict[str, np.ndarray]:
    # Histogram the effective areas based on position
    ids_ = tracks[:, 0].astype(int)
    areas = proj_areas[ids_]
    x, y = tracks[:, 2], tracks[:, 3]
    filters = {
        "collimated LYSO (D0)": (y > 0) & (x > -70),
        "uncollimated YAP (D1)": (y < 0) & (x < -70),
        "collimated YAP (D2)": (y > 0) & (x < -70),
        "collimated YAP (D3)": (y < 0) & (x > -70),
    }
    response_cols = {}

    af = asdf.open(pathlib.Path(args.input_histogram))
    # The total flux scaling through the atmosphere,
    # required for normalizing the effective areas
    flux_scaling = af["histogram"].sum() / args.num_incident_atm

    ebins = np.arange(0, max_energy + DE, DE)
    print("Histogram/normalize effective areas")
    for k, filt in filters.items():
        filt_energies = tracks[:, 1][filt]
        filt_areas = areas[filt]
        col, _ = np.histogram(filt_energies, bins=ebins, weights=filt_areas)
        col = (flux_scaling / args.num_incident_payload) * col
        response_cols[k] = col.copy() << (u.cm**2 * u.ct / u.ph)
    print("Done histogramming")
    return response_cols


def load_hits(f: io.BufferedReader) -> np.ndarray:
    hits = []
    for line in f:
        *trackids, energy, pos = line.split()
        id_ = int(trackids[0].split(b"|")[0])
        hits.append([id_, float(energy)] + [float(x) for x in pos[1:-1].split(b",")])
    return np.array(hits)


def load_projected_areas(f: io.BufferedReader, ninc: int) -> np.ndarray:
    proj_areas = np.zeros(ninc, dtype=float)
    for line in f:
        idx, _, area = (float(x) for x in line.strip().split())
        idx = int(idx)
        proj_areas[idx] = area
    return proj_areas


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="""
Given a single (gzipped) directory of name format "completed-[ea]-[eb].tar.gz,
along with the input momentum histogram file at float altitude,
properly bin Geant4 counts into a response matrix column of units (cm2 ct/ph).
"""
    )
    p.add_argument(
        "--input-histogram",
        type=str,
        required=True,
        help="The input Cartesian histogram ASDF file used for the CentralGenerator",
    )
    p.add_argument(
        "--g4-run-gzip",
        type=str,
        required=True,
        help="The G4 run as a gzipped file, containing the standard output directory",
    )
    p.add_argument(
        "--num-incident-atm",
        type=int,
        required=True,
        help="The number of primaries launched from outer space (for normalization)",
    )
    p.add_argument(
        "--num-incident-payload",
        type=int,
        required=True,
        help="The number of primaries launched from the CentralEventGenerator (for normalization)",
    )
    p.add_argument(
        "--output-file",
        type=str,
        required=True,
        help="File to write out response matrix columns (ASDF format)",
    )
    return p.parse_args()


if __name__ == "__main__":
    main()
