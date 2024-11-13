'''
The active region of the FastSDD detector head consists of:
  - A multi-layer collimator
    - The layer radial spacing of the collimator is estimated from the 
      datasheet's given field of view (74 degrees)
  - The Si drift detector (which we model as monolithic)
  - The Al2O3 substrate, which is approximately 1% gold
    (volume-averaged circuit traces, which are mostly on the edges)
    We don't need to worry that the whole volume has some gold in it;
    the tungsten collimator right behind the active area will block most
    X-rays which attempt to scatter out of that small gold content.
'''

import cadquery as cq
import numpy as np
import os
from tec import substrate_side

substrate_thk = 0.66

def make_active_region(translate: tuple[float]) -> dict[str, cq.Workplane]:
    '''
    Construct the active region of the X-123 given the
    translation vector (which shifts the center of the substrate)
    '''
    shapes: dict[str, cq.Workplane] = dict()
    # Make this a numpy array so we can add it more easily
    dx, dy, dz = translate
    shapes['substrate'] = (
        cq.Workplane("XY")
        .box(substrate_side, substrate_side, substrate_thk)
        .translate(translate)
        .edges("|Z")
        .chamfer(1)
    )

    si_side = 8.4
    si_thk = 1
    shapes['silicon'] = (
        cq.Workplane("XY")
        .box(si_side, si_side, si_thk)
        .translate((dx, dy, si_center := si_thk/2 + substrate_thk/2 + dz))
    )

    # The collimator is made up of W, Cr, Ti, Al
    # with the tungsten (presumably) closest to the silicon.
    # The inner diameter is given in the Amptek data sheet,
    # and we masured the outer diameter.
    coll_inner_rad = 4.67 / 2
    coll_outer_rad = 8 / 2
    # Based on the FOV given in the amptek documentation,
    # we can estimate the "radial step" of each
    # collimation layer to be about 56 micron.
    delta_radius = 56 / 1000

    # Material thicknesses, with the tungsten
    # being closest to the silicon itself.
    al_thk = 75 / 1000
    ti_thk = 15 / 1000
    cr_thk = 35 / 1000
    w_thk  = 100 / 1000

    def make_annulus(outer_radius, inner_radius, thickness) -> cq.Workplane:
        return (
            cq.Workplane("XY")
            .circle(outer_radius)
            .extrude(thickness)
            .faces(">Z")
            .circle(inner_radius)
            .cutThruAll()
        )

    # Build each collimation layer as an annulus,
    # slowly ticking up the inner radius by the average
    # step determined from Amptek documentation and geometry
    shapes['tungsten'] = make_annulus(
        coll_outer_rad, coll_inner_rad,
        w_thk
    ).translate((dx, dy, w_base := si_center + si_thk/2))

    shapes['chromium'] = make_annulus(
        coll_outer_rad,
        coll_inner_rad + delta_radius,
        cr_thk
    ).translate((dx, dy, cr_base := w_base + w_thk))

    shapes['titanium'] = make_annulus(
        coll_outer_rad,
        coll_inner_rad + 2*delta_radius,
        ti_thk
    ).translate((dx, dy, ti_base := cr_base + cr_thk))

    shapes['aluminum'] = make_annulus(
        coll_outer_rad,
        coll_inner_rad + 3*delta_radius,
        al_thk
    ).translate((dx, dy, ti_base + ti_thk))

    return shapes
