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

Here we generate a grid of points inside that trapezoid,
and put 176Lu ions at the points to simulate its emission spectrum.
"""

import numpy as np
from construct_trapezoid import thick, side

# mm from edge of the crystal we wanna be
padding = 0.2

num_y = 10
num_x = 10
num_runs_per_pt = 10000

thickness = thick
num_z = 2


# The edge of the crystal is a shifted y = x line
def x_edge(y):
    return y + side


# The edge of the crystal is also y(z) at the readout
def y_edge(z):
    return side - thickness + z


if __name__ == "__main__":
    with (
        open("trapezoid/176lu.mac", "w") as f,
        open("trapezoid/lu-metadata.txt", "w") as mf,
    ):
        f.write(
            """# Set up the G4 GPS to put Lu ions
/process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year
/gps/particle ion
/gps/ion 71 176

/gps/pos/centre 0 0 0 cm
/gps/pos/type Point
/gps/ang/type iso

/gps/polarization 1 0 0

/gps/ene/type Mono
/gps/ene/mono 0 eV

"""
        )
        for z in np.linspace(-thickness / 2 + padding, thickness / 2 - padding, num_z):
            for y in np.linspace(padding, y_edge(z) - padding, num_y):
                for x in np.linspace(padding, x_edge(y) - padding, num_x):
                    f.write(
                        f"""
/gps/pos/centre {x:.4f} {y:.4f} {z:.4f} mm
/run/beamOn {num_runs_per_pt}
"""
                    )
                    print(x, y, z, file=mf)
