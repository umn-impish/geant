#pragma once

#include <fstream>
#include <unordered_map>

#include "G4Accumulable.hh"
#include "G4String.hh"
#include "G4VAnalysisManager.hh"

class G4Event;
class G4Run;
class G4VHitsCollection;

class CrystalHit;
class VirtualHit;

class AnalysisFileWrapper
{
    public:
        AnalysisFileWrapper(
            const G4String& prefix, bool isBinary);
        ~AnalysisFileWrapper();

        std::ofstream& file()
        { return outf; }

        void reset(std::uint64_t newTimePfx);

        G4String buildFilename();
    private:
        std::uint64_t timePfx;
        bool isBinary;
        G4String fileNamePrefix;
        std::ofstream outf;
};

class Analysis
{
    public:
        static Analysis& instance();
        static std::uint32_t currentRunNumber();

        static void setEventId(G4int);
        static G4int getEventId();

        void initFiles(G4bool isMaster);
        void saveFiles(G4bool isMaster);
        void saveEvent(const G4Event* evt);

        void addIncidentEnergy(long double e);
        void saveScintillated(std::size_t num);

        Analysis(const Analysis&) =delete;
        void operator=(const Analysis&) =delete;
    private:
        static G4ThreadLocal G4int currentEventId;
        static std::uint32_t runNumber;

        Analysis();
        ~Analysis();
        void processEvent(const G4Event* evt);
        void processHitCollection(const G4VHitsCollection* hc);
        void saveCrystalHits(const std::vector<VirtualHit*>* vec);
        void saveIncidentSpectrumChunk();
        void saveConfig();

        void printSiHits(const std::vector<VirtualHit*>* vec);
        void saveSiHits(const std::vector<VirtualHit*>* vec);

        bool saveSiEnergies;
        bool saveSiPositions;

        std::vector<long double> incidentEnergiesChunk;

        using fw = AnalysisFileWrapper;
        fw crystOut;
        fw specIn;
        fw siOut;
        fw siEngOut;
        fw scintOut;
        fw cfgOut;
        std::vector<fw*> wrappers;
};
