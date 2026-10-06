"""
Generate a series of slabs going from ~100 km above the surface
to ~40km. use nrlmsis to compute density fractions etc. and create slabs
of appropriate thicknesses.

Assumes the slabs are all 1km thick.

The detector is a "passthrough" volume, meaning it is a very thin, very wide slab
which does not stop particles. It simply records the energy and momentum of the
_incoming_ particles, but the outgoing particles are ignored.
This permits measurement of multi-order scattering along the same track.
"""

import inspect
import json
import pathlib
import sys
from typing import cast

import numpy as np
from astropy import table as atab
from astropy import units as u


def generate_meta(
    fracs_file: pathlib.Path, target_altitude: float, elevation: u.Quantity[u.deg]
):
    slab_data = cast(atab.QTable, atab.QTable.read(fracs_file))
    slab_side = 800e6
    alts: np.ndarray = cast(
        np.ndarray, cast(u.Quantity, slab_data["altitude"]).to_value(u.mm)
    )
    slab_thick = alts[1] - alts[0]
    print(slab_data["altitude"])

    det_thick = 10

    # Generate the metadata we need for a bunch of slabs to be layered
    extent = alts[-1] - alts[0]
    placements = (alts - alts[0]) - extent / 2

    out_dir = pathlib.Path("backscatter-atmosphere-" + fracs_file.stem)
    out_dir.mkdir(exist_ok=True)
    generate_macro(
        ((placements[-1] - placements[0]) / 1e6 - target_altitude) << u.km,
        slab_side=(slab_side / 1e6) << u.km,
        elevation=elevation,
        out_direc=out_dir,
    )

    elements: list[str] = [
        c for c in slab_data.columns if c not in {"altitude", "density"}
    ]
    meta = {}

    slab_alts = np.array(cast(u.Quantity, slab_data["altitude"]).to_value(u.km))
    target_idx = np.argmin(np.abs(slab_alts - target_altitude))
    print("float altitude is at index", target_idx)

    for i, row in enumerate(slab_data):  # pyright: ignore[reportArgumentType]
        fractions = {k: float(row[k]) for k in elements}
        norm = sum(fractions.values())
        components = {}
        for k, v in fractions.items():
            components[k] = v / norm

        k = f"slab{i}"
        meta[k] = {
            "primitive_type": "box",
            "type": "passive",
            "halfx": slab_side / 2,
            "halfy": slab_side / 2,
            "halfz": slab_thick / 2,
            "material": k,
            k: {
                "density": float(row["density"].to_value(u.g / u.cm**3)),
                "components": components,
            },
            "euler_rotation": [0, 0, 0],
            # Add detection slab thickness onto slabs above it
            "translation": [0, 0, placements[i] + (det_thick if i > target_idx else 0)],
            "color": [0.8, 0.8, 0.8, 0.01],
        }

    # The detector is offset from the target slab
    # and is very very low density (vacuum)
    special = meta[f"slab{target_idx}"]
    meta["detector"] = {
        "material": "G4_Galactic",
        "type": "passthru_detector",
        "primitive_type": "box",
        "halfx": slab_side / 2,
        "halfy": slab_side / 2,
        "halfz": det_thick / 2,
        "euler_rotation": special["euler_rotation"],
        "translation": [
            0,
            0,
            special["translation"][2] + slab_thick / 2 + det_thick / 2,
        ],
        "color": [1, 0, 1, 0.4],
    }

    with open(out_dir / "meta.json", "w") as f:
        json.dump(meta, f, indent=2)


def generate_macro(
    atmosphere_extent: u.Quantity[u.km],
    slab_side: u.Quantity[u.km],
    elevation: u.Quantity[u.deg],
    out_direc: pathlib.Path,
):
    zloc = cast(float, (atmosphere_extent / 2).to_value(u.km)) + 10
    zdir = -float(np.sin(elevation))
    xdir = -float(np.cos(elevation))
    xloc = cast(float, slab_side.to_value(u.km)) / 4
    macro_base = f"""/gps/particle gamma
                    # Need to update this depending on the geometry
                    /gps/pos/centre {xloc:.0f} 0 {zloc:.0f} km
                    /gps/pos/type Point

                    /gps/direction {xdir} 0 {zdir}

                    /gps/ene/type Lin
                    /gps/ene/gradient 0
                    /gps/ene/intercept 1
                    /run/printProgress 50000
                    """
    macro_base = inspect.cleandoc(macro_base)
    with open(out_direc / "macro.mac", "w") as f:
        print(macro_base, file=f)
        for energy in np.arange(10, 300):
            print(f"/gps/ene/min {energy}", file=f)
            print(f"/gps/ene/max {energy + 1}", file=f)
            print("/run/beamOn 100000", file=f)


if __name__ == "__main__":
    # Calculated using pymsis results
    try:
        fracs_file = pathlib.Path(sys.argv[1])
    except IndexError:
        print("Supply the mass fractions .asdf as the sole script argument")
        sys.exit(1)

    generate_meta(fracs_file, 40, elevation=30 << u.deg)
