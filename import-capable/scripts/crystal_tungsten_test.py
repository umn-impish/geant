'''
Make an approximate experimental setup we used when testing the tungsten
3D printed filament for a Geant4 simulation.

The crystal was in an aluminum box, and the tungsten
filament prism was placed in front of an Am241 point source to block
most of its flux from reaching the crystal.
A lead brick was behind the source, about 2" behind it.
'''
import json
import os

import cadquery as cq

mm_per_inch = 25.4

rod = cq.Workplane("XY").box(5, 5, 40)

box = (
    cq.Workplane("XY")
    .box(
        (box_side := 3.55 * mm_per_inch), box_side, box_side
    )
    # .faces(">Z")
    .shell(0.1 * mm_per_inch, kind='arc')
    .rotateAboutCenter((0, 0, 1), 45)
)

tungsten = (
    cq.Workplane("YZ")
    .box(12.3, 12.3, 2)
    .translate((7.75 * mm_per_inch, 0, 0))
    .rotateAboutCenter((1, 0, 0), 45)
)

lead_brick = (
    cq.Workplane("YZ")
    .box(6 * mm_per_inch, 3 * mm_per_inch, 1.5 * mm_per_inch)
    .translate((10 * mm_per_inch, 0, 0))
)

os.makedirs(direc := 'tungsten-filament-experiment', exist_ok=True)
cq.exporters.export(rod, rod_fn := f'{direc}/rod.stl', opt={'ascii': True})
cq.exporters.export(box, box_fn := f'{direc}/al_box.stl', opt={'ascii': True})
cq.exporters.export(tungsten, tung_fn := f'{direc}/tung.stl', opt={'ascii': True})
cq.exporters.export(lead_brick, brick_fn := f'{direc}/brick.stl', opt={'ascii': True})

common = {"scale": 1, "translation": [0, 0, 0], "euler_rotation": [0, 0, 0]}
meta = {
    "crystal": {
        "file": rod_fn,
        "material": "lyso",
        "type": "scintillator",
        **common
    },
    "box": {
        "file": box_fn,
        "material": "G4_Al",
        "type": "passive",
        **common
    },
    "tung": {
        "file": tung_fn,
        "material": "tungsten-pla-filament",
        "type": "passive",
        **common
    },
    "brick": {
        "file": brick_fn,
        "material": "G4_Pb",
        "type": "passive",
        **common
    }
}

with open(f'{direc}/meta.json', 'w') as f:
    json.dump(meta, f)