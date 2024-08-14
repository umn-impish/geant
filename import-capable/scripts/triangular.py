import json, math, os, cadquery as cq
import sipms

# Start from a plate of given dims.
# and cut it down given the constraints we set

# 4mm thick, 66mm on a side
# x, z plane is flat, y is the vertical (thickness) direction
square_dims = dx, dy, dz = (66, 4, 66)
start_pt = (-dx/2, -dz/2)

crystal = (
    cq.Workplane('front')
    .moveTo(*start_pt)
    .line(dx, 0)
    .line(0, dz)
    .line(-dx, -dz)
    .close()
    .extrude(dy)
)

dx, dy = 66, 5.7
ret = sipms.make_sipms_pad(dx, dy)

cq.exporters.export(
    ret['pad'],
    'iso-triangle/pad.stl',
    opt=dict(ascii=True)
)

cq.exporters.export(
    ret['sipms'],
    'iso-triangle/sipms.stl',
    opt=dict(ascii=True)
)

common = {
    "scale": 1,
    # Need to put the final Z translation on X because of the rotation
    # To perfectly center this geometry we need to account for the full shape
    # "bounding box", so the relevant dimension is the projected SiPM dim.
    "translation": [-sipms.sipm_side_len/math.sqrt(2) / 2, 0, -dx/4],
    "euler_rotation": [0, 0, 3*math.pi / 2]
}
# Extent along Z direction of largest face
meta = [{
    "file": "iso-triangle/iso-90deg-allparts.obj",
    "crystal": {
        "material": "lyso",
        "type": "passive",
        **common
    },
    "reflector": {
        "material": "esr",
        "type": "specular_reflector",
        **common
    },
    "pad": {
        "material": "pdms",
        "type": "passive",
        **common
    },
    "sipms": {
        "material": "pdms",
        "type": "optical_detector",
        **common
    }
}]

with open('iso-triangle/tri_meta.json', 'w') as f:
    print(json.dumps(meta, indent=1), file=f)
