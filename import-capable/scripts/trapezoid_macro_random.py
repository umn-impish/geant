"""
Assume a 33mm x 66mm (bounding box) right trapezoid
has the "rectangular" corner centered at (0, 0) like this

-----------------------------
|                          /
|                         /
|                        /
|                       /
|______________________/
^ (0, 0)

And assume it's a prism centered on the XY plane with a depth

Here we generate a bunch of points randomly across (x, y, z)
"""

import numpy as np
from construct_trapezoid import thick, side

# mm from edge of the crystal we wanna be
padding = 0.2

num_y = 30
num_x = 30
num_z = 6
num_runs_per_pt = 1000

thickness = thick


# The edge of the crystal is a shifted y = x line
def x_edge(y):
    return y + side


# The edge of the crystal is also y(z) at the readout
def y_edge(z):
    return side - thickness + z


rng = np.random.default_rng(seed=0xD335)

if __name__ == "__main__":
    with (
        open("trapezoid/random_optical.mac", "w") as f,
        open("trapezoid/metadata.txt", "w") as mf,
    ):
        f.write(
            """# Set up the G4 GPS to fire monoenergetic opticals
# from a variety of points, isotropic across solid angle
/control/execute macros/optical-diagnose.mac
"""
        )
        for z in rng.uniform(-thickness / 2 + padding, thickness / 2 - padding, num_z):
            for y in rng.uniform(padding, y_edge(z) - padding, num_y):
                for x in rng.uniform(padding, x_edge(y) - padding, num_x):
                    f.write(
                        f"""
/gps/pos/centre {x:.4f} {y:.4f} {z:.4f} mm
/run/beamOn {num_runs_per_pt}"""
                    )
                    print(x, y, z, file=mf)
