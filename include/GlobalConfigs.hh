#pragma once

#include <G4SystemOfUnits.hh>
#include <G4String.hh>

#include <any>
#include <ostream>
#include <unordered_map>
#include <string>

/******************************************************
 * To add a new config setting:
 *      - add a new key (like "kAIR_GAP_THICKNESS")
 *      - specify the conversion type in KEY_TYPE_PARSE
 ******************************************************/

namespace ImpressForGrips {

class GlobalConfigs
{
    public:
        static const GlobalConfigs& instance(const std::string& fn = NO_FILE);
        const std::string& fileName() const { return currentFileName; }

        ~GlobalConfigs();

        template<class T>
        T configOption(const std::string& key) const
        {
            try {
                return std::any_cast<T>(configMap.at(key));
            } catch (std::out_of_range& e) {
                std::cerr
                    << "key was: " << key << std::endl
                    << "emsg:    " << e.what() << std::endl;
                throw;
            }
        }
        
        static const std::string DEFAULT_CFG_FN;

        // see .cc for what these are
        static const std::string
            kENABLE_SCINTILLATION,
            kMAKE_ONLY_CRYSTAL,
            kATTENUATOR_CHOICE,
            kOPTICAL_DRAW_FREQUENCY,
            kSAVE_PREFIX,
            kSAVE_EACH_SCINTILLATOR_HIT_ENERGY,
            kSAVE_SCINTILLATOR_POSITIONS,

            kSCINTILLATOR_MATERIAL,
            kSCINTILLATOR_TIME_CONST,
            kSCINTILLATOR_DX,
            kSCINTILLATOR_DY,
            kSCINTILLATOR_DZ,
            kSCINTILLATOR_MINUS_Z_FACE_FINISH,
            kSCINTILLATOR_XZ_FACES_FINISH,
            kSCINTILLATOR_YZ_FACES_FINISH,
            kSCINTILLATOR_CLADDING_TYPE,
            kBUILD_SCINTILLATOR_CLADDING,
            kREFLECTOR_THICKNESS,
            kSCINTILLATOR_CLADDING_AIR_GAP_THICKNESS,

            kNUM_SIPM_ROWS,
            kSIPMS_PER_ROW,
            kSIPM_SPACING,
            kSIPM_SIDE_LENGTH,
            kSAVE_SIPM_POSITIONS,
            kSAVE_SIPM_ENERGIES,
            kSAVE_SIPM_TRACK_LENGTHS,
            kSIPM_AIR_GAP_THICKNESS,

            kLIGHT_GUIDE_THICKNESS;

        enum class ParseType {vBOOL, vDOUBLE, vINT, vSTRING};
        static const std::unordered_map<std::string, ParseType> KEY_TYPE_PARSE;

        // from analytical model
        // round to nice numbers
        // last modified 24 dec 2021 (now using sunxspex + CHIANTI)
        static const std::unordered_map<std::string, G4double> ATTENUATOR_THICKNESSES;
    private:
        GlobalConfigs(const std::string& fn);
        GlobalConfigs() =delete;
        std::string currentFileName;
        void loadConfig(const std::string& fn);

        std::unordered_map<std::string, std::any> configMap;
        friend std::ostream& operator<<(std::ostream& os, const GlobalConfigs& igf);

        static const std::string NO_FILE;
};

}

