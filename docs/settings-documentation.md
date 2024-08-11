# Documentation for all settings available in the config file
**Settings are given one per line. They are lower-case strings separated by hyphens. See `simulation.config.file` for an example.**
**You can write comments by proceeding with a hash mark. Comments must be on their own line. The name is separated from the value with whitespace. Blank lines are ok.**

**Update 13 February 2023: you can input a settings file name via command line arguments. The order is this: `./executable [settings file] [macro name]`. Both arguments are optional but must occur in that order.**
| Name | Description | Unit | Value type |
|:----:|:-----------:|:----:|:----------:|
| save-prefix | prefix for folders saved into data-out. | none | string
| attenuator-choice | currently unused. choose the attenuator to put in front of the detector. | none | string
| scintillator-material | the type of scintillator material. choose from lyso, gagg, or cebr3 | none | string
| scintillator-minus-z-face-finish | surface finish for minus-z scintillator face, i.e. opposite the SiPMs. choose from options [here](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#id20); for now do not use the ones that include Teflon or ESR in their name. just use the bare crystal finishes. | none | string
| scintillator-xz-faces-finish | surface finish for xz-plane scintillator faces. choose from options [here](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#id20); for now do not use the ones that include Teflon or ESR in their name. just use the bare crystal finishes. | none | string
| scintillator-yz-faces-finish | surface finish for yz-plane scintillator faces. choose from options [here](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#id20); for now do not use the ones that include Teflon or ESR in their name. just use the bare crystal finishes. | none | string
| scintillator-cladding-type | what type of reflector (teflon or esr) | none | string
| save-each-cryst-hit-energy | whether you want to save the energy of each mini hit within the scintillator crystal from ionized electrons. useful for debugging. | none | 1 or 0
| save-scintillator-positions | whether you want to save the positions of hits within the scintillator from X-rays, electrons, etc. useful for some analyses. | none | 1 or 0
| save-sipm-positions | whether or not you want to save the positions of optical photon hits on the SiPMs. useful for illumination analysis. | none | 1 or 0
| save-sipm-energies | whether or not you want to save the energies of optical photons that hit the SiPMs. useful as a sanity check sometimes. | none | 1 or 0
| save-sipm-track-lengths | whether or not you want to save the track lengths of scintillated opticals after they hit the SiPM. | none | 1 or 0
| build-scintillator-cladding | whether or not to build the reflector around the scintillator | none | 1 or 0
| build-scintillator-cladding | scintillator reflector thickness | millimeter | double
| optical-draw-frequency | how often to draw an optical photon. a value of 1 means all. a value of 10 means each tenth photon gets drawn | none | int
| sipms-per-row | number of SiPMs per row | none | int
| num-sipm-rows | number of rows of SiPMs | none | int
| scintillator-time-const | time constant of the scintillator | nanosecond | double
| light-guide-thickness | thickness of light guide between scintillator and SiPMs | mm | double
| sipm-spacing | spacing between SiPMs | mm | double
| sipm-side-length | side length of SiPM | mm | double
| scintillator-dx | x-length of scintillator | mm | double
| scintillator-dy | y-length of scintillator | mm | double
| scintillator-dz | z-length of scintillator | mm | double
| scintillator-cladding-air-gap-thickness | air gap between scintillator and reflector. | micron | double
| sipm-air-gap-thickness | currently unused. the gap between the light guide and sipm. | micron | double
