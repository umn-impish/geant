#pragma once

#define ASSIGN_KEY(n, s) const std::string GlobalConfigs::n = s;

const std::unordered_map<std::string, G4double> GlobalConfigs::ATTENUATOR_THICKNESSES = {
    // these need to be changed!
};

// static specified in .hh
ASSIGN_KEY(kATTENUATOR_CHOICE,                       "attenuator-choice")
ASSIGN_KEY(kOPTICAL_DRAW_FREQUENCY,                  "optical-draw-frequency")
ASSIGN_KEY(kSAVE_PREFIX,                             "save-prefix")
ASSIGN_KEY(kSAVE_EACH_SCINTILLATOR_HIT_ENERGY,       "save-each-cryst-hit-energy")
ASSIGN_KEY(kSAVE_SCINTILLATOR_POSITIONS,             "save-scintillator-positions")

ASSIGN_KEY(kSCINTILLATOR_MATERIAL,                   "scintillator-material")
ASSIGN_KEY(kSCINTILLATOR_TIME_CONST,                 "scintillator-time-const")
ASSIGN_KEY(kSCINTILLATOR_DX,                         "scintillator-dx")
ASSIGN_KEY(kSCINTILLATOR_DY,                         "scintillator-dy")
ASSIGN_KEY(kSCINTILLATOR_DZ,                         "scintillator-dz")
ASSIGN_KEY(kSCINTILLATOR_MINUS_Z_FACE_FINISH,        "scintillator-minus-z-face-finish")
ASSIGN_KEY(kSCINTILLATOR_XZ_FACES_FINISH,            "scintillator-xz-faces-finish")
ASSIGN_KEY(kSCINTILLATOR_YZ_FACES_FINISH,            "scintillator-yz-faces-finish")
ASSIGN_KEY(kSCINTILLATOR_CLADDING_TYPE,              "scintillator-cladding-type")
ASSIGN_KEY(kSCINTILLATOR_CLADDING_AIR_GAP_THICKNESS, "scintillator-cladding-air-gap-thickness")
ASSIGN_KEY(kBUILD_SCINTILLATOR_CLADDING,             "build-scintillator-cladding")
ASSIGN_KEY(kREFLECTOR_THICKNESS,                     "reflector-thickness")

ASSIGN_KEY(kNUM_SIPM_ROWS,                           "num-sipm-rows")
ASSIGN_KEY(kSIPMS_PER_ROW,                           "sipms-per-row")
ASSIGN_KEY(kSIPM_SPACING,                            "sipm-spacing")
ASSIGN_KEY(kSIPM_SIDE_LENGTH,                        "sipm-side-length")
ASSIGN_KEY(kSAVE_SIPM_POSITIONS,                     "save-sipm-positions")
ASSIGN_KEY(kSAVE_SIPM_TRACK_LENGTHS,                 "save-sipm-track-lengths")
ASSIGN_KEY(kSAVE_SIPM_ENERGIES,                      "save-sipm-energies")
ASSIGN_KEY(kSIPM_AIR_GAP_THICKNESS,                  "sipm-air-gap-thickness")

ASSIGN_KEY(kLIGHT_GUIDE_THICKNESS,                   "light-guide-thickness")

namespace { using pt = GlobalConfigs::ParseType; }
const std::unordered_map<std::string, pt> GlobalConfigs::KEY_TYPE_PARSE = {
    {kATTENUATOR_CHOICE,                       pt::vSTRING},
    {kOPTICAL_DRAW_FREQUENCY,                  pt::vINT},
    {kSAVE_PREFIX,                             pt::vSTRING},
    {kSAVE_EACH_SCINTILLATOR_HIT_ENERGY,       pt::vBOOL},
    {kSAVE_SCINTILLATOR_POSITIONS,             pt::vBOOL},

    {kSCINTILLATOR_MATERIAL,                   pt::vSTRING},
    {kSCINTILLATOR_TIME_CONST,                 pt::vDOUBLE},
    {kSCINTILLATOR_DX,                         pt::vDOUBLE},
    {kSCINTILLATOR_DY,                         pt::vDOUBLE},
    {kSCINTILLATOR_DZ,                         pt::vDOUBLE},
    {kSCINTILLATOR_MINUS_Z_FACE_FINISH,        pt::vSTRING},
    {kSCINTILLATOR_XZ_FACES_FINISH,            pt::vSTRING},
    {kSCINTILLATOR_YZ_FACES_FINISH,            pt::vSTRING},
    {kSCINTILLATOR_CLADDING_TYPE,              pt::vSTRING},
    {kSCINTILLATOR_CLADDING_AIR_GAP_THICKNESS, pt::vDOUBLE},
    {kBUILD_SCINTILLATOR_CLADDING,             pt::vBOOL},
    {kREFLECTOR_THICKNESS,                     pt::vDOUBLE},

    {kSIPMS_PER_ROW,                           pt::vINT},
    {kNUM_SIPM_ROWS,                           pt::vINT},
    {kSIPM_SPACING,                            pt::vDOUBLE},
    {kSIPM_SIDE_LENGTH,                        pt::vDOUBLE},
    {kSAVE_SIPM_POSITIONS,                     pt::vBOOL},
    {kSAVE_SIPM_TRACK_LENGTHS,                 pt::vBOOL},
    {kSAVE_SIPM_ENERGIES,                      pt::vBOOL},
    {kSIPM_AIR_GAP_THICKNESS,                  pt::vDOUBLE},

    {kLIGHT_GUIDE_THICKNESS,                   pt::vDOUBLE}
};
