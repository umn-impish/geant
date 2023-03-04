#include <GlobalConfigs.hh>

#include <G4Exception.hh>

#include <fstream>
#include <stdexcept>

namespace ImpressForGrips {

const std::string GlobalConfigs::DEFAULT_CFG_FN = "simulation.config.file";

#include <ConfigCustomization.hh>

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
