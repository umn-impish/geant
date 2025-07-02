import math
import cadquery as cq

optical_pad_thickness = 0.5
sipm_thick = 0.7
sipm_side_len = 7


def make_sipms_pad(readout_face_x, readout_face_y):
    # The optical pad is a thin piece of silicone elastomer
    # which sits right at the end of the crystal.
    # Translation will be performed in Geant after import
    pad = cq.Workplane("front").box(
        readout_face_x, readout_face_y, optical_pad_thickness
    )

    # For SiPMs we are using onsemi C-series.
    # As a simplifying assumption, we take the
    # readout face to be one big piece of active
    # area, which is not really accurate--
    # there will be small gaps between them,
    # but it should be good enough for qualitative analysis.

    # Dimensions from C-series documentation
    # https://www.onsemi.com/download/data-sheet/pdf/microc-series-d.pdf
    num_sipms = math.ceil(readout_face_x / sipm_side_len)
    print("optical readout is equivalent to", num_sipms, "C-series")
    optical_readout = cq.Workplane("front").box(
        num_sipms * sipm_side_len, sipm_side_len, sipm_thick
    )

    return {"pad": pad, "sipms": optical_readout}
