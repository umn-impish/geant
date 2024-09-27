import cadquery as cq
import json
import numpy as np
import os

# 4 x 4cm crystal
x, y = 40, 40

thicks = np.arange(5, 200, 10)

output = 'crystals-varying-thickness'
os.makedirs(output, exist_ok=True)

for t in thicks:
    cr = (
        cq.Workplane('front')
        .box(x, y, t)
    )
    fn = f'{output}/{t}-crystal.stl'
    cq.exporters.export(cr, fn, opt={'ascii': True})

    meta_fn = f'{output}/{t}-meta.json'
    meta = {
        'crystal': {
            'file': fn,
            'material': 'lyso',
            'type': 'scintillator',
            'scale': 1,
            'translation': [0, 0, 0],
            'euler_rotation': [0, 0, 0],
            'color': [1, 0, 0.5, 0.5]
        }
    }
    print(json.dumps(meta))
    with open(meta_fn, 'w') as f:
        print(json.dumps(meta), file=f)
