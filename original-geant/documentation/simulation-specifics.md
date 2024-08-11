# Simulation setup and configuration

## Optical properties
### Scintillator crystals
Optical properties of all scintillators should use [Davis look-up table](
	https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#davis-look-up-tables-lutdavis) models.
These were computed using atomic force microscope measurements of crystalline surfaces.
I trust them.

A rough crystal is finished with the `Rough_LUT` option.
A polished crystal face is finished with the `Polished_LUT` option.

### Optical reflectors around scintillator crystal
All reflectors are approximated.
Teflon is a perfectly
	[Lambertian](https://en.wikipedia.org/wiki/Lambertian_reflectance) reflector.
Shiny film (a la
	[ESR](https://www.3m.com/3M/en_US/p/d/b5005047091/)) is approximated as a
	[specular](https://en.wikipedia.org/wiki/Specular_reflection) reflector.
It has a "5% defect probability" meaning that 5% of the time an interacting optical
	photon will undergo Lambertian reflection inistead of specular reflection.

#### NB: reflector ordering and thickness
The ordering of the volumes used to define optical border surfaces matters.
That is, the transition from volume 1 to volume 2 is different from the transition
from volume 2 to volume 1.
To take this into account,
the transition from _crystal_ to _air gap_ is defined, but not vice versa.
This is achieved by making "dummy" volumes of vacuum around each crystal side.

Also, the thickness of these dummy volumes matters.
They are defined to be 1/2 the air gap depth.
If they take up the whole air gap, weird things happen.

### Miscellaneous optical parameters
N/A at the moment
