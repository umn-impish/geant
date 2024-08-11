'''
Script generates a few pieces for a rectangular scintillator crystal.
It is entirely parametric.

Orientation is:
    - readout face on +Z
    - (x, y) plane faces the X-ray source

Pieces produced:
    - scintillation crystal
    - reflector around it
    - optical pad
    - optical readout--approximated to be monolithic 

.stl files are output into the specified directory
'''
import json
import math
import os
import cadquery as cq

# All dimensions in mm

# (x, y, z)
# just a box for now
# z-axis is oriented along the
# X-ray arrival direction
crystal_dims = (40, 5, 40)

crystal = (
    cq.Workplane('front')
    .box(*crystal_dims)
)


# The reflector is just a thin shell around the
# crystal, with a small air gap
refl_thick = 0.05
# air gap between reflector and crytsal
air_gap = 0.001
refl_dims = (
    # Need factor of 2 because it's on both sides
    crystal_dims[0] + air_gap + 2*refl_thick,
    crystal_dims[1] + air_gap + 2*refl_thick,
    # No factor of 2 here: only on one side
    crystal_dims[2] + refl_thick + air_gap
)
reflector = (
    cq.Workplane('front')
    .box(*refl_dims)
    .faces('|Z and -Z')
    .shell(refl_thick)
)


# The optical pad is a thin piece of silicone elastomer
# which sits right at the end of the crystal.
# Translation will be performed in Geant after import
optical_pad_thickness = 0.1
# The optical pad has the same (x, y) dims as the crystal
pad = (
    cq.Workplane('front')
    .box(*crystal_dims[:2], optical_pad_thickness)
)


# For SiPMs we are using onsemi C-series.
# As a simplifying assumption, we take the
# readout face to be one big piece of active
# area, which is not really accurate--
# there will be small gaps between them,
# but it should be good enough for qualitative analysis.

# Dimensions from C-series documentation
# https://www.onsemi.com/download/data-sheet/pdf/microc-series-d.pdf
sipm_side_len = 7
sipm_thick = 0.7
num_sipms = math.ceil(crystal_dims[0] / sipm_side_len)
print('optical readout is equivalent to', num_sipms, 'C-series')
optical_readout = (
    cq.Workplane('front')
    .box(num_sipms * sipm_side_len, sipm_side_len, sipm_thick)
)


output_dir = 'rectangular'
os.makedirs(output_dir, exist_ok=True)
save_to = lambda f: f'{output_dir}/{f}'
saveit = lambda obj, fn: cq.exporters.export(
    obj,
    save_to(fn),
    opt=dict(ascii=True)
)

things = (
    'crystal', 'reflector',
    'pad', 'optical_readout'
)

for t in things:
    saveit(eval(t), f'{t}.stl')

# Now we output the metadata required for Geant to position things correctly
def generate_meta(fn, mat, type_, scale=1, trans=(0, 0, 0), rot=(0, 0, 0)):
    return {
        'file': fn,
        'material': mat,
        'type': type_,
        'scale': scale,
        'translation': trans,
        'euler_rotation': rot
    }

# Metadata given to Geant to parse the solids into a
# real detector
meta = [
    generate_meta(
        fn=save_to('crystal.stl'),
        mat='lyso',
        type_='scintillator',
    ),
    generate_meta(
        fn=save_to('reflector.stl'),
        mat='esr',
        type_='specular_reflector',
        trans=(0, 0, refl_thick + air_gap)
    ),
    generate_meta(
        fn=save_to('pad.stl'),
        mat='pdms',
        type_='optically_active',
        trans=(0, 0, (pad_loc := -crystal_dims[2]/2 - optical_pad_thickness/2))
    ),
    generate_meta(
        fn=save_to('optical_readout.stl'),
        mat='G4_Si',
        type_='optical_detector',
        trans=(0, 0, pad_loc - optical_pad_thickness/2 - sipm_thick/2)
    )
]

with open(save_to('rect_meta.json'), 'w') as f:
    print(json.dumps(meta, indent=2), file=f)
