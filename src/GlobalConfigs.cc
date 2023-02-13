#include <GlobalConfigs.hh>

#include <G4Exception.hh>

#include <fstream>
#include <stdexcept>

#define ASSIGN_KEY(n, s) const std::string GlobalConfigs::n = s;

namespace ImpressForGrips {

const std::string GlobalConfigs::DEFAULT_CFG_FN = "simulation.config.file";

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

ASSIGN_KEY(kNUM_SIPM_ROWS,                           "num-sipm-rows")
ASSIGN_KEY(kSIPMS_PER_ROW,                           "sipms-per-row")
ASSIGN_KEY(kSIPM_SPACING,                            "sipm-spacing")
ASSIGN_KEY(kSIPM_SIDE_LENGTH,                        "sipm-side-length")
ASSIGN_KEY(kSAVE_SIPM_POSITIONS,                     "save-sipm-positions")
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

    {kSIPMS_PER_ROW,                           pt::vINT},
    {kNUM_SIPM_ROWS,                           pt::vINT},
    {kSIPM_SPACING,                            pt::vDOUBLE},
    {kSIPM_SIDE_LENGTH,                        pt::vDOUBLE},
    {kSAVE_SIPM_POSITIONS,                     pt::vBOOL},
    {kSAVE_SIPM_ENERGIES,                      pt::vBOOL},
    {kSIPM_AIR_GAP_THICKNESS,                  pt::vDOUBLE},

    {kLIGHT_GUIDE_THICKNESS,                   pt::vDOUBLE}
};

GlobalConfigs& GlobalConfigs::instance()
{
    static GlobalConfigs igc;
    return igc;
}

GlobalConfigs::GlobalConfigs()
{
    loadConfig(DEFAULT_CFG_FN);
}

GlobalConfigs::~GlobalConfigs()
{ }

void GlobalConfigs::reload(const std::string& newFileName)
{
    loadConfig(newFileName);
}

void GlobalConfigs::loadConfig(const std::string& fn)
{
    std::ifstream ifs(fn);
    if (!ifs) {
        throw std::runtime_error(
            "can't load " + fn + "! make sure it's in the executible directory.");
    }

    std::string line, key;
    while (std::getline(ifs, line)) {
        try {
            static const char COMMENT_MARKER = '#';
            if (line.size() == 0 || line[0] == COMMENT_MARKER)
                continue;

            std::stringstream ss(line);
            ss >> key;
            const auto& loadType = KEY_TYPE_PARSE.at(key);
            switch (loadType) {
                case pt::vBOOL: {
                    bool b;
                    ss >> b;
                    configMap[key] = b;
                    break;
                } case pt::vDOUBLE: {
                    double d;
                    ss >> d;
                    configMap[key] = d;
                    break;
                } case pt::vINT: {
                    int i;
                    ss >> i;
                    configMap[key] = i;
                    break;
                } case pt::vSTRING: {
                    std::string st;
                    ss >> st;
                    configMap[key] = st;
                    break;
                } default: {
                    throw std::out_of_range("got a value type that can't be parsed in GlobalConfigs");
                    break;
                }
            }
        }
        catch (const std::out_of_range& e) {
            auto errStr = std::string(e.what()) + " -> key was " + key;
            G4Exception(
                "GlobalConfigs/loadConfig", "lcfg0", RunMustBeAborted,
                errStr.c_str());
        }
    }
}

std::ostream& operator<<(std::ostream& os, const GlobalConfigs& igf) {
    for (const auto& [k, v] : igf.configMap) {
        os << k << ' ';
        const auto& loadType = igf.KEY_TYPE_PARSE.at(k);
        switch (loadType) {
            case pt::vBOOL: {
                os << igf.configOption<bool>(k);
                break;
            } case pt::vDOUBLE: {
                os << igf.configOption<double>(k);
                break;
            } case pt::vINT: {
                os << igf.configOption<int>(k);
                break;
            } case pt::vSTRING: {
                os << igf.configOption<std::string>(k);
                break;
            } default: {
                throw std::out_of_range("got a value type that can't be parsed in GlobalConfigs");
                break;
            }
        }
        os << std::endl;
    }

    return os;
}
}
