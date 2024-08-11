#pragma once

#include <limits>
#include <cassert>
#include <string>

#include "G4Exception.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4String.hh"
#include "G4OpticalParameters.hh"
#include "G4PhysicalConstants.hh"
#include "G4Scintillation.hh"
#include "G4SystemOfUnits.hh"

#include "AluminumProperties.hh"
#include "BerylliumProperties.hh"
#include "CeBr3Properties.hh"
#include "QuartzProperties.hh"
#include "SiProperties.hh"
#include "TeflonProperties.hh"
#include "LYSOProperties.hh"
#include "GaGGProperties.hh"
#include "ESRProperties.hh"
#include "PDMSProperties.hh"

/*
 * Empirical data sources
 * CeBr3 scintillation information
 *     Quarati et al doi:10.1016/j.nima.2013.08.005
 * CeBr3 index of refraction, absorption length info
 *     Li et al doi:10.1016/j.ssc.2007.08.040
 * Al refractive index info
 *     Rakic 1995 doi:10.1364/AO.34.004755
 */

namespace ImpressForGrips
{
namespace Materials
{
    std::string selectScintillator(const std::string& choice);
    G4Material* selectReflectorMaterial(const std::string& choice);

    static const G4String kVACUUM = "vacuum";
    static const G4double VACUUM_MOLAR_DENSITY = 1.01 * g / mole;
    static const G4double VACUUM_ATOMIC_NUMBER = 1.0;
    static const G4double VACUUM_TEMPERATURE = 2.73 * kelvin;
    static const G4double VACUUM_PRESSURE = 1.322e-11 * pascal;

    static const G4double SATELLITE_TEMP = 283 * kelvin;
    void makeMaterials();

    // scintillation
    static const G4String kSCINT_YIELD = "SCINTILLATIONYIELD";
    static const G4String kONLY_SCINT_COMPONENT = "SCINTILLATIONCOMPONENT1";
    static const G4String kONLY_TIME_CONSTANT = "SCINTILLATIONTIMECONSTANT1";
    static const G4String kRESOLUTION_SCALE = "RESOLUTIONSCALE";

    // optical photon stuff
    static const G4String kREFR_IDX = "RINDEX";
    static const G4String kREFR_IDX_REAL = "REALRINDEX";
    static const G4String kREFR_IDX_IMAG = "IMAGINARYRINDEX";
    static const G4String kABSORPTION_LEN = "ABSLENGTH";
    
    static const G4String kOP_DET_EFF = "EFFICIENCY";
    static const G4String kREFLECTIVITY = "REFLECTIVITY";
    static const G4String kTRANSMITTANCE = "TRANSMITTANCE";
}
}
