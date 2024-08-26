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

    private:
        GlobalConfigs(const std::string& fn);
        GlobalConfigs() =delete;
        std::string currentFileName;
        void loadConfig(const std::string& fn);

        std::unordered_map<std::string, std::any> configMap;
        static const std::string NO_FILE;
};

