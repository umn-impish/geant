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
	[ESR](https://www.3m.com/3M/en_US/p/d/b5005047091/)) is approximated as a perfectly
	[specular](https://en.wikipedia.org/wiki/Specular_reflection) reflector.

### Miscellaneous optical parameters
There is a minimum air-gap between all optical interfaces of 50nm.
This is enforced in the code.
Any additional code should follow this guideline.

