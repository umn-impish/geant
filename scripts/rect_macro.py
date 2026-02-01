"""
Generate a macro file which fires photons from a grid of points
inside the crystal defined by `rectangular.py`
"""

import numpy as np

import rectangular as rect

# Z direction is normal to the SiPMs
num_along_x = 10
num_along_y = 10
num_along_z = 20

# We built the geometry to make the crystal centered at (0, 0, 0),
# so we can make a simple grid with this in mind.
x, y, z = rect.crystal_dims

# we want the first and last grid points
# to be offset from the edges by some amount.
# otherwise weird stuff might happen
offset_from_edge = 0.2


def make_pts(pt, num):
    return np.linspace(-pt + offset_from_edge, pt - offset_from_edge, num)


xp = make_pts(x / 2, num_along_x)
yp = make_pts(y / 2, num_along_y)
zp = make_pts(z / 2, num_along_z)

# now we can write the macro file, given the defined grid
num_runs_per_point = 1000

if __name__ == "__main__":
    with (
        open("rectangular/optical_macro.mac", "w") as f,
        open("rectangular/metadata.txt", "w") as metaf,
    ):
        # Set up the GPS to emit isotropically and at 3 eV (near what we want)
        print("/control/execute macros/optical-diagnose.mac", file=f)
        for xx in xp:
            for yy in yp:
                for zz in zp:
                    print(f"/gps/pos/centre {xx} {yy} {zz} mm", file=f)
                    print(f"/run/beamOn {num_runs_per_point}", file=f)

                    # Save this out for use in analysis later
                    print(xx, yy, zz, file=metaf)
