#pragma once

#include <fstream>
#include <unordered_map>

#include "G4Accumulable.hh"
#include "G4String.hh"
#include "G4VAnalysisManager.hh"

class G4Event;
class G4Run;
class G4VHitsCollection;

namespace ImpressForGrips
{

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
        void updateFlareId(const G4String& fid);

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

        void initFiles(G4bool isMaster);
        void saveFiles(G4bool isMaster);
        void saveEvent(const G4Event* evt);

        void addEvents(std::size_t nEvts)
        { totalEvents += nEvts; };

        void addIncidentEnergy(long double e);
        void saveScintillated(std::size_t num);

        void updateFlareIdentifier(const std::string& fid);

        Analysis(const Analysis&) =delete;
        void operator=(const Analysis&) =delete;
    private:
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

        G4Accumulable<std::size_t> totalEvents;
        bool saveSiEnergies;
        bool saveSiPositions;
        bool saveSiTrackLengths;

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
}
