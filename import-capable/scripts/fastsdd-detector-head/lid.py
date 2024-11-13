'''
CadQuery model of the Ni lid on the X-123
detector head, 25mm2 version in TO-8 package.

We also place a thin disc in front of the lid to
model the Be window.
The "miniscus" shape of the window is disregarded.
'''

import cadquery as cq
import numpy as np


# All units in mm
lid_width = 14
wall_thick = 0.25

bevel_dy = 1.92
# Computed from looking at Amptek docs
bevel_dx = 2.2
# the lip extends slightly in from the side of the bevel
lip_dx = 1.25

total_height = 9.65
side_wall_height = total_height - bevel_dy

# Sketches out a cross-sectional shape of the beveled lid
to_revolve_pts = (
    (0, 0),
    (0, side_wall_height),
    (bevel_dx, side_wall_height + bevel_dy),
    (bevel_dx + lip_dx, side_wall_height + bevel_dy),
    (bevel_dx + lip_dx, side_wall_height + bevel_dy - wall_thick),
    (bevel_dx, side_wall_height + bevel_dy - wall_thick),
    (wall_thick, side_wall_height),
    (wall_thick, 0),
)

# Shift the points from the origin by the appropriate amount
# so that we can revolve them about the Y axis to form the lid

total_width = 14
shift_delta_x = +(total_width / 2) - wall_thick / 2
to_revolve_pts = tuple(
    (x - shift_delta_x, y)
    for (x, y) in to_revolve_pts
)

def make_lid(translate: tuple[float]) -> dict[str, cq.Workplane]:
    '''
    Construct the nickel cover and the Be window
    for the X-123 FastSDD detector head
    '''
    lid = (
        cq.Workplane("YZ")
        .polyline(to_revolve_pts)
        .close()
        .revolve(
            360,
            (0, 0, 0),
            (0, 1, 0)
        )
        .translate(translate)
    )

    bbox = lid.combine().objects[0].BoundingBox()
    height = min(bbox.xlen, bbox.ylen, bbox.zlen)
    
    dx, dy, dz = translate
    window_rad = 9.525 / 2
    window_thk = 0.0127
    window = (
        cq.Workplane("XY")
          .cylinder(window_thk, window_rad)
          # Translate the window so it sits right on top of the lid
          .translate((dx, dy, dz + window_thk/2 + height))
    )

    return {'lid': lid, 'window': window}
