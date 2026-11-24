#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

#include "G4String.hh"

class G4Event;
class G4Run;
class G4VHitsCollection;

class CrystalHit;
class VirtualHit;

class AnalysisFileWrapper {
public:
  AnalysisFileWrapper(const G4String &prefix, bool isBinary);
  ~AnalysisFileWrapper();

  std::ofstream &file() { return outf; }

  void reset(std::uint64_t newTimePfx);

  G4String buildFilename();

private:
  std::uint64_t timePfx;
  bool isBinary;
  G4String fileNamePrefix;
  std::ofstream outf;
};

class Analysis {
public:
  static Analysis &instance();
  static std::uint32_t currentRunNumber();

  void initFiles(G4bool isMaster);
  void saveFiles(G4bool isMaster);
  void saveEvent(const G4Event *evt);

  void addIncidentEnergy(long double e);
  void addProjectedArea(double);
  void addEventId(G4int);
  void saveScintillated(std::size_t num);

  Analysis(const Analysis &) = delete;
  void operator=(const Analysis &) = delete;

private:
  static std::uint32_t runNumber;

  Analysis();
  ~Analysis();
  void processEvent(const G4Event *evt);
  void processHitCollection(const G4VHitsCollection *hc);
  void saveCrystalHits(const std::vector<VirtualHit *> *vec);
  void saveIncidentSpectrumChunk();
  void saveConfig();

  void printSiHits(const std::vector<VirtualHit *> *vec);
  void saveSiHits(const std::vector<VirtualHit *> *vec);

  bool saveSiEnergies;
  bool saveSiPositions;

  std::vector<long double> incidentEnergiesChunk;
  std::vector<double> projAreasChunk;
  std::vector<G4int> eventIdChunk;

  using fw = AnalysisFileWrapper;
  fw crystOut;
  fw specIn;
  fw siOut;
  fw siEngOut;
  fw scintOut;
  fw cfgOut;
  std::vector<fw *> wrappers;
};
