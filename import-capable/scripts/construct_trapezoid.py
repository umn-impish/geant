'''
Construct a trapezoidal crystal, trapezoidal shell of a reflector,
a thin optical pad, and a SiPM on the end, for Geant or pvtrace simulations.
'''
import cadquery as cq
import numpy as np
import os

thick = 4
side = 33
vertices = (
    (0, 0),
    (0, side),
    (2 * side, side),
    (side, 0)
)

if __name__ == '__main__':
    crystal = (
    cq.Workplane("XY")
      .polyline(vertices)
      .close()
      .extrude(thick/2, both=True)
      .faces(">Y")
      .workplane(offset=-thick/2)
      .transformed(rotate=(45, 0, 0))
      .split(keepTop=False, keepBottom=True)
    )

# Now we can do the same thing to make a nice shell
    air_gap = 10/1000
    esr_thick = 80/1000
    delta = air_gap # + esr_thick
    shell_vertices = (
        (0, 0),
        (0, side + delta),
        (2 * (side + delta), side + delta),
        (side + delta, 0)
    )

# We need to translate the shell to exactly encapsulate
# the crystal
    shell_vertices = tuple(
        (x - delta, y - delta)
        for (x, y) in shell_vertices
    )


# Compensate for air gap
    shell_thick = thick + 2*delta

    shell = (
    cq.Workplane("XY")
      .polyline(shell_vertices)
      .close()
      .extrude(shell_thick/2, both=True)
      .faces(">Y")
      .workplane(offset=-shell_thick/2)
      .transformed(rotate=(45, 0, 0))
      .split(keepTop=False, keepBottom=True)
      .faces(">Y")
      .shell(esr_thick, kind='intersection')
    )

# Now build up the optical pad
    pad_height, pad_length= 6, 68
    pad_depth = 1
    optical_pad = (
    cq.Workplane("XZ")
      .transformed(rotate=(-45, 0, 0))
      .box(pad_length, pad_height, pad_depth)
      # Move the pad to the edge, and then move it back a little
      # to put it on the crystal face
      .translate((
          # Move to the center of the crystal
          side,
          (optical_pad_center := side + pad_depth/np.sqrt(2) - thick/2),
          0
      ))
      # Center it slightly on the crystal
      .translate((0, -0.25, -0.25))
    )

# Finally, we can put the SiPMs
    sipm_delta = 1
    sipm_height, sipm_length, sipm_depth = 7, 70, 1
    sipms = (
    cq.Workplane("XZ")
      .transformed(rotate=(-45, 0, 0))
      .box(sipm_length, sipm_height, sipm_depth)
      .translate((side, optical_pad_center + pad_depth/np.sqrt(2) + sipm_depth/np.sqrt(2), 0))
      # Translate to roughly center the crystal on the SiPMs
      .translate((0, -sipm_delta, -sipm_delta))
    )

    os.makedirs('trapezoid', exist_ok=True)
    cq.exporters.export(crystal, 'trapezoid/crystal.stl', opt=dict(ascii=True))
    cq.exporters.export(shell, 'trapezoid/esr_shell.stl', opt=dict(ascii=True))
    cq.exporters.export(optical_pad, 'trapezoid/opticalpad.stl', opt=dict(ascii=True))
    cq.exporters.export(sipms, 'trapezoid/sipms.stl', opt=dict(ascii=True))
