'''
Make a series of geometries to test the attenuation effect of tungsten filament
on a given radioactive source.

The radioactive sources we have in the lab are encapsulated,
thin films of evaporated metal salts.
The films are on the order of one micron thick and 3mm in diameter,
as per their data sheets.

The encapsulating material is kapton or aluminized mylar.
For testing the total attenuation, we neglect the encapsulation and 
treat the source as an isolated 3mm disc of diameter 1.5mm and thickness 1 micron.

We generate a series of metadata files and 3D objects which represent different
thicknesses of W filament slabs. Simulations are assumed to follow realistic radioactive
decay of different elements.

The detector location is a free parameter, but for this script we're putting it at (0, 0, 0)
and the vertical offset of the radioactive source is computed as follows:
    (fixed offset) + (thickness of slab) + (1.5mm)

The fixed offset is determined by the geometry of the measurement.
'''
import cadquery as cq
import json
import os

# IMPRESS-like
detector_rad = (37 / 2)
detector_thk = 5

source_rad = (3 / 2)

# Sum of: space in crystal housing,
# 3D printed compression cap,
# aluminum box lid thickness
space_above_crystal = 3.5 + 13.5 + 2.54

# We have several thicknesses of tungsten bricks
filament_side = 28
filament_thicks = [0.5] + list(range(1, 6))

# Finally, build up a bunch of geometries we want to look at.
crystal = cq.Workplane("XY").circle(radius=detector_rad).extrude(-detector_thk)

def save(obj, fn):
    cq.exporters.export(obj, fn, opt={'ascii': True})

save(crystal, 'crystal.stl')

num_gammas = 1000000
macro_source = f'''/gps/particle gamma
/gps/ene/mono 81 keV
/gps/ang/type iso
/gps/pos/type Plane

# Shape of the radiation source
/gps/pos/shape Circle
/gps/pos/radius {source_rad} mm

/gps/pos/centre 0 0 {{zloc}} mm
/run/printProgress {num_gammas // 100}
/run/beamOn {num_gammas}'''

common_meta = {
    "scale": 1,
    "translation": [0, 0, 0],
    "euler_rotation": [0, 0, 0]
}
for ft in filament_thicks:
    filament_slab = (
        cq.Workplane("XY")
        .rect(filament_side, filament_side)
        .extrude(ft)
        .translate((0, 0, space_above_crystal))
    )

    os.makedirs(direc_name := f'{ft}-slab', exist_ok=True)
    save(filament_slab, fil_fn := f'{direc_name}/slab.stl')
    meta = {
        "crystal": {
            "file": "tungsten-filament-slabs/crystal.stl",
            "type": "scintillator",
            "material": "cebr3",
            **common_meta
        },
        "slab": {
            "file": f"tungsten-filament-slabs/{fil_fn}",
            "type": "passive",
            "material": "tungsten-pla-filament",
            **common_meta
        }
    }
    with open(f"{direc_name}/meta.json", "w") as f:
        json.dump(meta, f)

    # Use this to generate an appropriate macro file
    # Offset a bit from the tungsten
    source_z_loc = space_above_crystal + ft + 2
    with open(f"{direc_name}/source_macro.mac", "w") as f:
        print(macro_source.format(zloc=source_z_loc), file=f)
