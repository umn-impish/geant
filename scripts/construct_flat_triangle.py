"""
Construct a triangular crystal, triangular shell of a reflector,
a thin optical pad, and a SiPM on the end, for Geant or pvtrace simulations.

The readout face is flat
"""

import cadquery as cq
import numpy as np
import json
import os

thick = 4 * np.sqrt(2)
side = 33
vertices = ((side, 0), (0, side), (2 * side, side))

if __name__ == "__main__":
    crystal = (
        cq.Workplane("XY").polyline(vertices).close().extrude(thick / 2, both=True)
    )

    # Make a box for the air gap which we will base other stuff on
    air_gap = 10 / 1000
    t = air_gap
    gap_vertices = (
        (side, -t),
        (-t, side),
        (2 * side + t, side),
    )
    gap_box = (
        cq.Workplane("XY")
        .polyline(gap_vertices)
        .close()
        .extrude(thick / 2 + air_gap, both=True)
    )

    # Compensate for air gap
    esr_thick = 80 / 1000
    shell_thick = thick + 2 * air_gap

    shell = gap_box.faces(">Y").shell(esr_thick, kind="intersection")

    # Now build up the optical pad
    pad_height, pad_length = 6, 68
    pad_depth = 0.5
    optical_pad = (
        cq.Workplane("XZ")
        .box(pad_length, pad_height, pad_depth)
        # Move the pad to the edge, and then move it back a little
        # to put it on the crystal face
        .translate(
            (
                # Move to the center of the crystal
                side,
                (optical_pad_center := side + pad_depth / 2),
                0,
            )
        )
    )

    # Finally, we can put the SiPMs
    sipm_height, sipm_length, sipm_depth = 7, 70, 1
    sipms = (
        cq.Workplane("XZ")
        .box(sipm_length, sipm_height, sipm_depth)
        .translate((side, optical_pad_center + pad_depth / 2 + sipm_depth / 2, 0))
    )

    direc = "flat_triangle"
    os.makedirs(direc, exist_ok=True)
    cq.exporters.export(crystal, f"{direc}/crystal.stl", opt=dict(ascii=True))
    cq.exporters.export(shell, f"{direc}/esr_shell.stl", opt=dict(ascii=True))
    cq.exporters.export(optical_pad, f"{direc}/opticalpad.stl", opt=dict(ascii=True))
    cq.exporters.export(sipms, f"{direc}/sipms.stl", opt=dict(ascii=True))

    common = {"scale": 1, "translation": [0, 0, 0], "euler_rotation": [0, 0, 0]}
    meta = {
        "crystal": {
            "file": f"{direc}/crystal.stl",
            "material": "lyso",
            "type": "scintillator",
            "color": [0, 0.5, 0, 0.1],
            **common,
        },
        "esr": {
            "file": f"{direc}/esr_shell.stl",
            "material": "esr",
            "type": "specular_reflector",
            "color": [0, 0, 1, 0.1],
            **common,
        },
        "opticalpad": {
            "file": f"{direc}/opticalpad.stl",
            "material": "pdms",
            "type": "passive",
            "color": [0, 1, 0, 0.1],
            **common,
        },
        "sipm": {
            "file": f"{direc}/sipms.stl",
            "material": "G4_Si",
            "type": "optical_detector",
            "color": [1, 0, 0, 0.1],
            **common,
        },
    }

    with open(f"{direc}/meta.json", "w") as f:
        f.write(json.dumps(meta))
