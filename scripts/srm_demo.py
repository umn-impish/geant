"""
Generate a simple geometry to create a response matrix
"""

import json
import pathlib
import cadquery as cq

# Make two cylinders:
# one is the Si detector,
# and another is a sheet of Al

# Default unit = mm
radius = 10
det_thick = 0.5
detector = (
    cq.Workplane("XY")
      .circle(radius=radius)
      .extrude(until=-det_thick)
)

# Units in mm, but write it out in micron for clarity
filter_thick = (100 / 1000)
filter = (
    cq.Workplane("XY")
      .circle(radius=radius)
      .extrude(until=filter_thick)
)

# Write the metadata for the objects
meta = {
    "detector": {
        "file": "demo/det.stl",
        "material": "G4_Si",
        "type": "scintillator",
        "scale": 1,
        "translation": [0, 0, 0],
        "euler_rotation": [0, 0, 0],
        "color": [0, 0, 1, 0.5],
    },
    "filter": {
        "file": "demo/filter.stl",
        "material": "G4_Al",
        "type": "passive",
        "scale": 1,
        "translation": [0, 0, 0],
        "euler_rotation": [0, 0, 0],
        "color": [1, 0, 0, 0.5],
    },
}

base = pathlib.Path("demo")
base.mkdir(exist_ok=True)

cq.exporters.export(detector, str(base / "det.stl"), opt={"ascii": True})
cq.exporters.export(filter, str(base / "filter.stl"), opt={"ascii": True})

with open(base / "meta.json", "w") as f:
    json.dump(meta, f)