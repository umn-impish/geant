import numpy as np

'''
Short Python script to generate a Geant4 macro
which shoots gammas across an energy range to make a "pixellated"
response matrix.

Assumes the active surface is oriented towards +Z.

The user can specify the energy range, bin width, number of runs
per energy bin, and position/size of the gamma source.

In post-processing, you will have to scale your "measured"
photon energy depositions by the "expected" number to get
a true probability of photon conversion.

Don't use bins that are too large, or else you will miss important features,
or not be able to re-bin down to the count bins you want to use.

**IMPORTANTLY,** the energy spacing and boundaries you supply here
_define_ your photon model bins.
'''

# Energy parameters: start, end, step
ea = 1
eb = 80
de = 0.1

# How many runs per energy range
num_runs = 100000

# Set these if you'd like
# (in cm)
x_center, y_center, z_center = 0, 0, 2

# Orient the momentum direction how you'd like
# The easiest might be along -Z (how this
# script was originally written)
px, py, pz = 0, 0, -1

# Define the size of the plane in mm
# 8mm = large enough radius for X-123 detector head
# Need larger for other geometries
radius = 8

# We set up a gamma plane source in front of our detector
setup = f'''/gps/particle gamma

/gps/pos/centre {x_center} {y_center} {z_center} cm
/gps/pos/type Plane
/gps/pos/shape Circle
/gps/pos/radius {radius} mm

# Set particle momentum direction to be towards
# the active volume (defined by the user)
/gps/direction {px} {py} {pz}

# Linear distribution with no gradient,
# intercept 1 means flat distribution
/gps/ene/type Lin
/gps/ene/gradient 0
/gps/ene/intercept 1

# Print run information occasionally
/run/printProgress {num_runs // 2}'''

# Each run is a "flat" distribution across a narrow energy range.
# We later collect the data into a row (or column) of a response
# matrix to allow photon to count conversion.
run = '''/gps/ene/min {e0:.3f} keV
/gps/ene/max {e1:.3f} keV''' + f'\n/run/beamOn {num_runs}\n'

# Write out the configuration to a .mac file to be run
with open('srm.mac', 'w') as f:
    print(setup, file=f)
    print('', file=f)
    for energy in np.arange(ea, eb, de):
        this_run = run.format(e0=energy, e1=energy + de)
        print(this_run, file=f)
