'''
Combine the "sub-shapes" into one big one and export all of the
properly-translated objects
'''

import cadquery as cq
import os

import active_region
import base
import lid
import tec

# The base defines the origin.
# It's a circle extruded from Z = 0
# so anything we build on top of it needs
# to get shifted by at least the thickness of the base,
# perhaps more.
objects = {'base': base.make_base((0, 0, 0))}

# After building the base, we can
# attach the TEC on top of it.
# The TEC starts by building a ceramic "box"
# so we need to translate away from the base
# by its thickness and 1/2 the ceramic thickness
dz = base.base_thick + tec.ceramic_thick/2
tec_objs = tec.build_tec((0, 0, dz))
objects |= tec_objs

# I don't really know how tall everything is
# at this point so let's just compute the
# total Z height of the TEC stack
tec_height = 0
for layer in tec_objs.values():
    bbox = layer.combine().objects[0].BoundingBox()
    tec_height += bbox.zlen

# The active region gets built above the TEC and the base,
# and it starts using a box of "substrate"
active_objs = active_region.make_active_region(
    (0, 0, tec_height + base.base_thick + active_region.substrate_thk/2)
)
objects |= active_objs

# The lid needs to just go on top of everything.
# Its origin is a little funny but we should be able
# to just shift it up by the base thickness and call it good
objects |= lid.make_lid(
    (0, 0, base.base_thick)
)


direc = 'x123-parts'
os.makedirs(direc, exist_ok=True)
# Export all of the objects
for (k, v) in objects.items():
    cq.exporters.export(v, f'{direc}/{k}.stl', opt={'ascii': True})