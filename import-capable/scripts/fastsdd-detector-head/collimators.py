import cadquery as cq

'''
Collimators for the CubiXSS detectas
'''

collimator_thickness = 1

def tungsten_disk(center: tuple[float], inner: float = 1):
    # Inner and outer diameters
    outer = 16 / 2

    shape = (
      cq.Workplane("XY", origin=center)
        .cylinder(height=collimator_thickness, radius=outer)
        .circle(inner/2)
        .cutThruAll()
    )

    return shape

# Cadmesh needs ASCII to parse
opt = {'ascii': True}

coll1 = tungsten_disk((0, 0, first_center := 12))
cq.exporters.export(coll1, "x123-parts/closer_collimator.stl", opt=opt)

coll2 = tungsten_disk((0, 0, first_center + 60), inner=7)
cq.exporters.export(coll2, "x123-parts/further_collimator.stl", opt=opt)