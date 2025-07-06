#include <configs.hh>

#include <G4Exception.hh>

#include <fstream>
#include <stdexcept>
#include <mutex>
#include <thread>

const std::string GlobalConfigs::DEFAULT_CFG_FN = "simulation.config.file";
const std::string GlobalConfigs::NO_FILE = "____NO_FILE_BLAH";

const GlobalConfigs& GlobalConfigs::instance(const std::string& fn)
{
    static GlobalConfigs igc(fn);
    if (fn != NO_FILE && fn != igc.currentFileName)
        throw std::runtime_error("filename mismatch for GlobalConfigs");
    return igc;
}

GlobalConfigs::GlobalConfigs(const std::string& fn) : 
    currentFileName(fn)
{
    loadConfig(fn);
}

GlobalConfigs::~GlobalConfigs()
{ }

void GlobalConfigs::loadConfig(const std::string& fn)
{
    std::ifstream ifs(fn);
    if (!ifs) {
        throw std::runtime_error(
            "can't load " + fn + "! make sure it's in the executible directory.");
    }

    std::string line, key, type;
    while (std::getline(ifs, line)) {
        try {
            static constexpr char COMMENT_MARKER = '#';
            if (line.size() == 0 || line[0] == COMMENT_MARKER)
                continue;

            std::stringstream ss(line);
            ss >> key >> type;
            if (type == "bool") {
                bool b;
                ss >> b;
                configMap[key] = b;
            } else if (type == "double") {
                double d;
                ss >> d;
                configMap[key] = d;
            } else if (type == "string") {
                std::string st;
                ss >> st;
                configMap[key] = st;
            }
            else {
                throw std::out_of_range(
                    "got a value type that can't be parsed in GlobalConfigs: "
                    + key + " of type " + type
                );
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
