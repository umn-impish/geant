'''
An approximation of the FastSDD detector head base
made of Kovar in a TO-8 package. Made to mount to the 
lid and TEC etc. in other scripts
'''
import cadquery as cq

# All units mm
base_diam = 15.24
base_thick = 1.6

mount_stud_diam = 2.84
mount_stud_length = 7

pin_radius = 0.46
inside_pin_height = 4
outer_pin_height = 7.87

# There are 12 pins, three per side,
# and they're spaced "evenly" in 0.1"
# increments
# 0.1" in mm
delta = (25.4 / 10)
modified = (-delta, 0, delta)
pin_points = [(x, 2*delta) for x in modified]
pin_points += [(-2*delta, y) for y in modified]
pin_points += [(2*delta, y) for y in modified]
pin_points += [(x, -2*delta) for x in modified]


def make_base(translate: tuple[float]) -> cq.Workplane:
    # Build the base as just a cylinder
    # and then push on points for the electrical pins
    base = (
        cq.Workplane("XY")
          .circle(base_diam / 2)
          .extrude(base_thick)
          .tag("base")
          .faces("+Z")
          .pushPoints(pin_points)
          .circle(pin_radius)
          # Not sure why we need the thickness but it makes it work
          .extrude(inside_pin_height + base_thick)

          .faces("-Z", tag="base")
          .pushPoints(pin_points)
          .circle(pin_radius)
          # Negative so it goes "down" (minus Z)
          .extrude(-outer_pin_height)

          # Finally, we can extrude the stud out of the base
          .faces("-Z", tag="base")
          .circle(mount_stud_diam/2)
          .extrude(-mount_stud_length)
    )

    return base