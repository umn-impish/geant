"""
Construct a triangular crystal, triangular shell of a reflector,
a thin optical pad, and a SiPM on the end, for Geant or pvtrace simulations.

The readout face is at a 45 degree angle
"""

import cadquery as cq
import numpy as np
import os
import json

thick = 4
side = 33
vertices = ((side, 0), (0, side), (2 * side, side))

if __name__ == "__main__":
    crystal = (
        cq.Workplane("XY")
        .polyline(vertices)
        .close()
        .extrude(thick / 2, both=True)
        .faces(">Y")
        .workplane(offset=-thick / 2)
        .transformed(rotate=(45, 0, 0))
        .split(keepTop=False, keepBottom=True)
    )

    # Now we can do the same thing to make a nice shell
    air_gap = 10 / 1000
    esr_thick = 80 / 1000
    delta = air_gap
    shell_vertices = (
        (side, -delta),
        (-delta, side),
        (2 * side + delta, side),
    )

    # Compensate for air gap
    shell_thick = thick + 2 * delta

    shell = (
        cq.Workplane("XY")
        .polyline(shell_vertices)
        .close()
        .extrude(shell_thick / 2, both=True)
        .faces(">Y")
        .workplane(offset=-shell_thick / 2)
        .transformed(rotate=(45, 0, 0))
        .split(keepTop=False, keepBottom=True)
        .faces(">Y")
        .shell(esr_thick, kind="intersection")
    )

    # Now build up the optical pad
    pad_height, pad_length = 6, 68
    pad_depth = 1
    optical_pad = (
        cq.Workplane("XZ")
        .transformed(rotate=(-45, 0, 0))
        .box(pad_length, pad_height, pad_depth)
        # Move the pad to the edge, and then move it back a little
        # to put it on the crystal face
        .translate(
            (
                # Move to the center of the crystal
                side,
                (optical_pad_center := side + pad_depth / np.sqrt(2) - thick / 2),
                0,
            )
        )
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
        .translate(
            (
                side,
                optical_pad_center + pad_depth / np.sqrt(2) + sipm_depth / np.sqrt(2),
                0,
            )
        )
        # Translate to roughly center the crystal on the SiPMs
        .translate((0, -sipm_delta, -sipm_delta))
    )

    # Put a lead strip in front of the SiPMs to block weird events
    strip_width = 2 * side
    strip_length = 16
    strip_depth = 1
    strip = (
        cq.Workplane("XY")
        .box(strip_width, strip_length, strip_depth)
        .translate((side, side, thick * 1.2))
    )

    folder = "angled_triangle"
    os.makedirs(folder, exist_ok=True)
    cq.exporters.export(crystal, f"{folder}/crystal.stl", opt=dict(ascii=True))
    cq.exporters.export(shell, f"{folder}/esr_shell.stl", opt=dict(ascii=True))
    cq.exporters.export(optical_pad, f"{folder}/opticalpad.stl", opt=dict(ascii=True))
    cq.exporters.export(sipms, f"{folder}/sipms.stl", opt=dict(ascii=True))
    cq.exporters.export(strip, f"{folder}/lead_strip.stl", opt=dict(ascii=True))

    common = {"scale": 1, "translation": [0, 0, 0], "euler_rotation": [0, 0, 0]}
    # Extent along Z direction of largest face
    meta = {
        "crystal": {
            "file": f"{folder}/crystal.stl",
            "material": "lyso",
            "type": "scintillator",
            **common,
        },
        "esr": {
            "file": f"{folder}/esr_shell.stl",
            "material": "esr",
            "type": "specular_reflector",
            **common,
        },
        "opticalpad": {
            "file": f"{folder}/opticalpad.stl",
            "material": "pdms",
            "type": "passive",
            **common,
        },
        "sipm": {
            "file": f"{folder}/sipms.stl",
            "material": "G4_Si",
            "type": "optical_detector",
            **common,
        },
        "lead_strip": {
            "file": f"{folder}/lead_strip.stl",
            "material": "G4_W",
            "type": "passive",
            **common,
        },
    }

    with open(f"{folder}/meta.json", "w") as f:
        f.write(json.dumps(meta))
