# Things that need to get done
## Overall simulation goal as of Feb 2023: figure out what scintillator, optical finishes, reflector, SiPMs are best given design constraints.

## Specific questions and features that need to be answered and implemented
### C++ additions
- [ ] A reflective ring around the light guide. Teflon or ESR?
- [ ] Small air gaps (5-10 µm) between the crystal, light guide, SiPMs to simulate imperfect coupling.

### Run with macros/Python helpers and perform analysis
- [ ] What surface finish is best?
	- [ ] Try bare crystal finishes (`Rough_LUT` and `Polished_LUT`) with reflector
	- [ ] Try pre-computed finishes (like `RoughTeflon_LUT`) with no reflector
	- [ ] Quantify differences.
	- [ ] What should we ultimately use in the model?
- [ ]  Similar question: what reflector material is best?
	- [ ] Teflon (diffuse)?
	- [ ] ESR (specular)?
- [ ] What scintillator dimensions are best?
	- [ ] Plate
	- [ ] Rod
	- [ ] Cube (maybe not feasible, but good sanity check)
	- [ ] Something else?
- [ ] What SiPM dimensions are best?
	- [ ] Slightly bigger than crystal/light guide?
	- [ ] Slightly smaller but multiple covering the crystal?
	- [ ] How close can we pack the SiPMs in reality?
- [ ] Related question: how much SiPM over-coverage do we need?
	- [ ] Is 1mm over-coverage good enough?
	- [ ] How does e.g. 1mm over-coverage compare with others?
- [ ] What kind of scintillator is best?
	- [ ] Things to consider: SiPM efficiency, time constant, light yield (~ energy resolution)
- [ ] How is our signal affected if SiPMs only cover part of the detector surface?
