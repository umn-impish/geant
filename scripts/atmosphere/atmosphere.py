"""
Generate a series of slabs going from ~100 km above the surface
to ~40km. use nrlmsis to compute density fractions etc. and create slabs
of appropriate thicknesses.

Assumes the slabs are all 1km thick.
"""

import json
import pathlib
import sys
from typing import cast

import numpy as np
from astropy import table as atab
from astropy import units as u

# Calculated using pymsis results
try:
    fracs_file = pathlib.Path(sys.argv[1])
    slab_data = cast(atab.QTable, atab.QTable.read(fracs_file))
except IndexError:
    print("Supply the mass fractions .asdf as the sole script argument")
    sys.exit(1)

# The altitudes themselves might not be in delta = 1km,
# but we should assume that the LOS is in 1km steps.
slab_thick = 1e6
slab_side = 50e6
alts = np.arange(cast(u.Quantity, slab_data["altitude"]).size) * slab_thick

# Generate the metadata we need for a bunch of slabs to be layered
simulation_z_size = 800
placements: np.ndarray = round(-(alts.max() - alts.min()) / 2) + (alts - alts.min())

elements: list[str] = [c for c in slab_data.columns if c not in {"altitude", "density"}]
meta = {}
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
        "translation": [0, 0, placements[i]],
        "color": [0.8, 0.8, 0.8, 0.8],
    }

# The sensitive detector volume is placed at the bottom of the stack
meta["detector"] = {
    "primitive_type": "box",
    "halfx": slab_side / 2,
    "halfy": slab_side / 2,
    "halfz": slab_thick / 2,
    "type": "perfect_detector",
    "material": "G4_W",
    "euler_rotation": [0, 0, 0],
    "translation": [0, 0, (placements.min() - slab_thick)],
    "color": [0.5, 0, 0, 1],
}

out_dir = pathlib.Path("atmosphere-" + fracs_file.stem)
out_dir.mkdir(exist_ok=True)
with open(out_dir / "meta.json", "w") as f:
    json.dump(meta, f)
