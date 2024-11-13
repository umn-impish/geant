import astropy.units as u
import numpy as np

# keV
emin, emax = 10, 200

start = f'''
/control/execute macros/collimator.mac

# Flat energy distribution
/gps/ene/type Lin
/gps/ene/min {emin} keV
/gps/ene/max {emax} keV

# Zero slope, constant intercept --> flat across E
/gps/ene/gradient 0
/gps/ene/intercept 1

# Turn off visualization if in interactive mode
/vis/disable

'''

macro_fn = 'collimator_diagnose.mac'

# the angles we wanna test
d_theta = 0.25
th0, th1 = 0, 70
angles = np.arange(th0, th1+d_theta, d_theta) << u.deg

num_runs = 100000
with open('collimation-test.mac', 'w') as f:
    print(start, file=f)
    for th in angles:
        # We slowly point off of the Z-axis
        # towards positive X
        print(
            f'/gps/direction {np.sin(th):.5f} 0 {-np.cos(th):.5f}',
            file=f
        )
        print(f'/run/beamOn {num_runs}', file=f)
        print('', file=f)
    print('# angle info (t0, t1 inclusive, dt):\n#', th0, th1, d_theta, file=f)
    print('# energy info (min, max):\n', emin, emax, file=f)
