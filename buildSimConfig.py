def buildSimConfig(file_name: str,scint_name: str,build_cladd: int,reflector: str,finish: int,x_dim: int,y_dim: int, z_dim: int):
	FILE_NAME = 'simulation.config.file'
	cnfg_fi = open(FILE_NAME,'w')

	cnfg_fi.write('#prefix the folders saved\n')
	cnfg_fi.write("save-prefix " + file_name + "\n")
	cnfg_fi.write("attenuator-choice nothing\n")
	cnfg_fi.write("optical-draw-frequency 1\n")
	cnfg_fi.write("\n")
	cnfg_fi.write("scintillator-material " + scint_name + "\n")
	cnfg_fi.write("save-each-cryst-hit-energy 0\n")
	cnfg_fi.write("save-scintillator-positions 1\n")
	cnfg_fi.write("save-sipm-positions 1\n")
	cnfg_fi.write("save-sipm-energies 0\n")

	line_str = "\nsipms-per-row " + str((x_dim+2)//6) + "\nnum-sipm-rows 1\n"
	cnfg_fi.write(line_str + "\n")

	cnfg_fi.write("#default finish types:\n")
	cnfg_fi.write("#  rough bare crystal = Rough_LUT\n")
	cnfg_fi.write("#  polished bare crystal = Polished_LUT\n")
	cnfg_fi.write("# types that may be used WITH CARE\n")
	cnfg_fi.write("#  see the table here: https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#id20\n")
	cnfg_fi.write("# some more info\n")
	cnfg_fi.write("#  +z is where detectors attach\n")

	if finish == 1:
		cnfg_fi.write("scintillator-minus-z-face-finish Rough_LUT" + "\n" + "scintillator-xz-faces-finish Rough_LUT" + "\n" + "scintillator-yz-faces-finish Rough_LUT" + "\n")
	elif finish == 2:
		cnfg_fi.write("scintillator-minus-z-face-finish Rough_LUT" + "\n" + "scintillator-xz-faces-finish Rough_LUT" + "\n" + "scintillator-yz-faces-finish Polished_LUT" + "\n")
	elif finish == 3:
		cnfg_fi.write("scintillator-minus-z-face-finish Polished_LUT" + "\n" + "scintillator-xz-faces-finish Polished_LUT" + "\n" + "scintillator-yz-faces-finish Polished_LUT" + "\n")
	elif finish == 4:
		cnfg_fi.write("scintillator-minus-z-face-finish Polished_LUT" + "\n" + "scintillator-xz-faces-finish Polished_LUT" + "\n" + "scintillator-yz-faces-finish Rough_LUT" + "\n")

	cnfg_fi.write("\n\n")

	cnfg_fi.write("#unit = nanosecond\n")	
	cnfg_fi.write("scintillator-time-const 50\n")

	cnfg_fi.write("\n\n")
	
	cnfg_fi.write('# allowed types: (teflon, esr)\n')

	if reflector == 'ESR':
		cnfg_fi.write("scintillator-cladding-type esr\n")
	elif reflector == 'TEF':
		cnfg_fi.write("scintillator-cladding-type teflon\n")

	cnfg_fi.write("build-scintillator-cladding " + str(build_cladd) + "\n")

	cnfg_fi.write("\n\n")

	cnfg_fi.write("# unit = millimeter\n")
	cnfg_fi.write("light-guide-thickness 1\n")
	cnfg_fi.write("sipm-spacing 0.05\n")
	cnfg_fi.write("sipm-side-length 6" + "\n")

	x = str(x_dim)
	y = str(y_dim)
	z = str(z_dim)

	cnfg_fi.write("scintillator-dx " + x + "\n")
	cnfg_fi.write("scintillator-dy " + y + "\n")
	cnfg_fi.write("scintillator-dz " + z + "\n")

	cnfg_fi.write("\n\n")

	cnfg_fi.write("#unit = micron\n")
	cnfg_fi.write("scintillator-cladding-air-gap-thickness 50\n")
	cnfg_fi.write("sipm-air-gap-thickness 0\n")

	cnfg_fi.close()
