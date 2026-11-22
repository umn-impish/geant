"""
Generate a very simple geometry for testing the CentralEventGenerator.

Detector meta: a rectangular prism
Momentum bins: uniform across a sphere
"""

import json
import pathlib
import zlib

import msgpack
import numpy as np

outdir = pathlib.Path("basic-central-test")
outdir.mkdir(exist_ok=True)
central_meta_fn = "central-meta.mpack"
dx = 100
dy = 200
dz = 300


def main():
    generate_momentum()
    generate_geometry()
    generate_config()
    generate_macro()


def generate_macro():
    with open(outdir / "macro.mac", "w") as f:
        print("/run/beamOn 100000", file=f)


def generate_config():
    cfg = f"""data-dir string data-out
save-prefix string basic-central-generator
generator-type string central-generator
central-generator-config string {outdir.resolve() / central_meta_fn!s}

save-each-cryst-hit-energy bool 0
save-each-cryst-hit-momentum bool 1
save-all-scintillator-positions bool 1
save-sipm-positions bool 0
save-sipm-energies bool 0

# unit = nanosecond
scintillator-time-const double 25
enable-scintillation bool 0

lyso-attenuation-length double 50
specular-sigma-alpha-deg double 2

# World volume parameters (in units of mm, aka native G4 units)
world-x double 1000
world-y double 1000
world-z double 1000

# Whether or not we want to save the track and event ids
# for hits that are made
save-track-identifiers bool 0"""
    with open(outdir / "config.file", "w") as f:
        print(cfg, file=f)


def generate_geometry():
    with open(outdir / "geometry.json", "w") as f:
        json.dump({
            "det": {
                "type": "perfect_detector",
                "primitive_type": "box",
                "halfx": dx / 2,
                "halfy": dy / 2,
                "halfz": dz / 2,
                "material": "G4_Galactic",
                "euler_rotation": [0, 0, 0],
                "translation": [0, 0, 0],
                "color": [50 / 255, 130 / 255, 40 / 255, 0.8]
            }
        }, f, indent=1)


def generate_momentum():
    pmax = 10
    dp = 0.1
    bins = np.arange(-pmax, pmax + dp, dp)
    mids = bins[:-1] + np.diff(bins) / 2
    dist_sq = mids**2
    # From https://stackoverflow.com/a/16170977
    # Make a distance**2 array, extend it along 2D, then along 3D
    mask_ary = dist_sq + dist_sq[:, None] + dist_sq[:, None, None]
    elts_per_axis = mids.size
    hist = np.zeros((elts_per_axis, elts_per_axis, elts_per_axis))
    hist[mask_ary < pmax**2] = 1

    output = {
        "bounding_dx_mm": dx,
        "bounding_dy_mm": dy,
        "bounding_dz_mm": dz,
        "polar_rotation_degrees": 0,
        "momentum_bins_kev": bins.tolist(),
        "momentum_hg_kev": zlib.compress(hist.ravel().astype("<u4").tobytes()),
    }
    with open(outdir / central_meta_fn, "wb") as f:
        msgpack.pack(output, f)


if __name__ == "__main__":
    main()