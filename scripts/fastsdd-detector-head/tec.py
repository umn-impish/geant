'''
Build an approximation of the thermoelectric cooler
inside the FastSDD detector head.
It's a two-stage cooler which consists of a stack of three
ceramic-like plates connected with P- and N-type doped silicon rods,
like in the following picture:
https://ars.els-cdn.com/content/image/1-s2.0-S0017931017319142-gr1.jpg

We measured the ceramic thicknesses and the silicon rod sizes.
The silicon rods have approximately 50% fill factor where they are present,
so we model those areas as "average" blocks where the density should be set to be
1/2 that of normal crystalline Si.

The copper strips on the pads also therefore have about 50% fill factor.
We assume they have the same (X, Y) extent as the silicon rods, but that
the thickness is only 1/10 that of the ceramic components.

The effect of the copper pads is captured in the ceramic density.
We say the ceramic is 97% Al2O3 by volume and 3% Cu by volume.
'''

import cadquery as cq
import os

ceramic_thick = 0.5
layer_zero_side = 6.43
layer_one_side = layer_zero_side
layer_two_side = 4.8

si_rod_height = 1.17
si_zero_one_width = 0.9 * layer_one_side
si_one_two_width = 0.9 * layer_two_side

shield_side = 7.6
shield_thick = 0.4

substrate_side = 9.6

def build_tec(translate: tuple[float]) -> dict[str, cq.Workplane]:
    shapes: dict[str, cq.Workplane] = dict()

    dx, dy, dz = translate

    shapes['first_ceramic'] = (
        cq
        .Workplane("XY")
        # First, make the "zeroth layer" ceramic with
        .box(layer_zero_side, layer_zero_side, ceramic_thick)
        .translate(translate)
    )

    shapes['first_silicon'] = (
        cq.Workplane("XY")
        # Next, make a box around the ceramic which
        # will have 1/2 the average crystalline Si density
        # representing the conenctions between the TEC sections
        .box(si_zero_one_width, si_zero_one_width, si_rod_height)
        .translate((dx, dy, first_si_pos := si_rod_height/2 + ceramic_thick/2 + dz))
    )

    shapes['second_ceramic'] = (
        cq.Workplane("XY")
        # Now build the second ceramic layer
        .box(layer_one_side, layer_one_side, ceramic_thick)
        .translate((dx, dy, second_ceramic_pos := first_si_pos + si_rod_height/2 + ceramic_thick/2))
    )

    shapes['second_silicon'] = (
        cq.Workplane("XY")
        # Second Si layer is just a "ring"
        .box(si_one_two_width, si_one_two_width, si_rod_height)
        .faces("|Z")
        # Negative = shell "inwards" i.e. keep
        # the width the same as we want but 0.1x is the wall thickness
        .shell(-0.1 * si_one_two_width)
        .translate((dx, dy, second_silicon_pos := second_ceramic_pos + si_rod_height/2 + ceramic_thick/2))
    )

    # The third ceramic is the final one before the active volume
    shapes['third_ceramic'] = (
        cq.Workplane("XY")
        # Now build the second ceramic layer
        .box(layer_two_side, layer_two_side, ceramic_thick)
        .translate((dx, dy, third_ceramic_pos := second_silicon_pos + si_rod_height/2 + ceramic_thick/2))
    )

    # The shield is slightly off-center so we need
    # to adjust for that here
    even_distribution = (substrate_side - shield_side) / 2
    # One side we measured the shield to be 1.2mm from the edge
    long_shift = 1.2
    x_shift = long_shift - even_distribution

    # Finally, we have a thin tungsten shield resting on top of
    # the final ceramic portion. It's secured with some kind of
    # glue but we're gonna ignore that for now.
    shapes['shield'] = (
        cq.Workplane("XY")
        .box(shield_side, shield_side, shield_thick)
        # We need to translate this in the Z direction,
        # but the shield is also slightly off-center
        # relative to the rest of the detector. Strange.
        .translate((x_shift + dx, dy, third_ceramic_pos + ceramic_thick/2 + shield_thick/2))
    )

    return shapes
