'''
Generate the metadata required to load this model into the
CADMesh Geant wrapper

There are a few custom materials that need to get added:
 - sdd_substrate (a mix of Al2O3 and a bit of gold for the traces)
 - tec_ceramic (a mix of AlN and a little copper)
 - kovar (a standard nickel/cobalt alloy)
'''
import json
import os

direc = 'x123-parts'
def generate_meta(fn, mat, type_='passive', scale=1, trans=(0, 0, 0), rot=(0, 0, 0)):
    return {
        fn.split('.')[0]: {
            'file': f'{direc}/{fn}',
            'material': mat,
            'type': type_,
            'scale': scale,
            'translation': trans,
            'euler_rotation': rot
        }
    }

output = dict()

# Window and lid cover the assembly
output |= generate_meta('window.stl', 'G4_Be')
output |= generate_meta('lid.stl', 'G4_Ni')
output |= generate_meta('shield.stl', 'G4_W')

# These are the collimator materials
output |= generate_meta('tungsten.stl', 'G4_W')
output |= generate_meta('chromium.stl', 'G4_Cr')
output |= generate_meta('titanium.stl', 'G4_Ti')
output |= generate_meta('aluminum.stl', 'G4_Al')

# Custom material: 99% Al2O3, 1% gold (traces on the edges)
output |= generate_meta('substrate.stl', 'sdd_substrate')

# the detector is (stupidly) named "silicon"
output |= generate_meta('silicon.stl', 'G4_Si', type_='scintillator')
output |= generate_meta('silicon_edge.stl', 'G4_Si', type_='passive')

# Custom material: mostly AlN, some copper
output |= generate_meta('first_ceramic.stl', 'tec_ceramic')
output |= generate_meta('second_ceramic.stl', 'tec_ceramic')
output |= generate_meta('third_ceramic.stl', 'tec_ceramic')
# The "first" and "second" silicon are layers between the
# TEC ceramics
output |= generate_meta('first_silicon.stl', 'half_silicon')
output |= generate_meta('second_silicon.stl', 'half_silicon')

# Kovar is a cobalt/nickel alloy
output |= generate_meta('base.stl', 'kovar')

# Give keys
os.makedirs('x123-parts', exist_ok=True)
with open('x123-parts/fastsdd_head_meta.json', 'w') as f:
    json.dump(output, f, indent=2)
