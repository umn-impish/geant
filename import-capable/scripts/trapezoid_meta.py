import json

common = {
    "scale": 1,
    "translation": [0, 0, 0],
    "euler_rotation": [0, 0, 0]
}
# Extent along Z direction of largest face
meta = {
    "crystal": {
        "file": "trapezoid/crystal.stl",
        "material": "lyso",
        "type": "scintillator",
        **common
    },
    "esr": {
        "file": "trapezoid/esr_shell.stl",
        "material": "esr",
        "type": "specular_reflector",
        **common
    },
    "opticalpad": {
        "file": "trapezoid/opticalpad.stl",
        "material": "pdms",
        "type": "passive",
        **common
    },
    "sipm": {
        "file": "trapezoid/sipms.stl",
        "material": "G4_Si",
        "type": "optical_detector",
        **common
    }
}

with open('trapezoid/meta.json', 'w') as f:
    f.write(json.dumps(meta))
