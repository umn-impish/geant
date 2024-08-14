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
import sipms

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

ret = sipms.make_sipms_pad(*crystal_dims[:2])
optical_readout = ret['sipms']
pad = ret['pad']

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
meta = {
    'crystal': generate_meta(
        fn=save_to('crystal.stl'),
        mat='lyso',
        type_='scintillator',
    ),
    'reflector': generate_meta(
        fn=save_to('reflector.stl'),
        mat='esr',
        type_='specular_reflector',
        trans=(0, 0, refl_thick + air_gap)
    ),
    'pad': generate_meta(
        fn=save_to('pad.stl'),
        mat='pdms',
        type_='optically_active',
        trans=(0, 0, (pad_loc := -crystal_dims[2]/2 - sipms.optical_pad_thickness/2))
    ),
    'readout': generate_meta(
        fn=save_to('optical_readout.stl'),
        mat='G4_Si',
        type_='optical_detector',
        trans=(0, 0, pad_loc - sipms.optical_pad_thickness/2 - sipms.sipm_thick/2)
    )
}

with open(save_to('rect_meta.json'), 'w') as f:
    print(json.dumps(meta), file=f)
