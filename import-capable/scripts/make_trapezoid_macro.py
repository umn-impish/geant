'''
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
and launch photons from the grid points.
'''

import numpy as np

side = 33
# mm from edge of the crystal we wanna be
padding = 0.2

num_y = 10
num_x = 10
num_runs_per_pt = 1000

thickness = 4
num_z = 5

# The edge of the crystal is a shifted y = x line
x_edge = lambda y: y + side
# The edge of the crystal is also y(z) at the readout
y_edge = lambda z: side - thickness + z

if __name__ == '__main__':
    with open('trapezoid/optical.mac', 'w') as f, open('trapezoid/metadata.txt', 'w') as mf:
        f.write(
    '''
# Set up the G4 GPS to fire monoenergetic opticals
# from a variety of points, isotropic across solid angle
    /control/execute macros/optical-diagnose.mac
    '''
        )
        for z in np.linspace(-thickness/2 + padding, thickness/2 - padding, num_z):
            for y in np.linspace(padding, y_edge(z) - padding, num_y):
                for x in np.linspace(padding, x_edge(y) - padding, num_x):
                    f.write(
    f'''
    /gps/pos/centre {x:.4f} {y:.4f} {z:.4f} mm
    /run/beamOn {num_runs_per_pt}

    ''')
                    print(x, y, z, file=mf)
