#pragma once

/*
    Includes short classes that facilitate detection.
    Excluded is the SteppingAction because the code is long.
*/

#include <memory>
#include <random>
#include <variant>

#include <G4GeneralParticleSource.hh>
#include <G4GenericIon.hh>
#include <G4OpBoundaryProcess.hh>
#include <G4ParticleGun.hh>
#include <G4UserEventAction.hh>
#include <G4UserRunAction.hh>
#include <G4UserSteppingAction.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <nlohmann/json.hpp>

#include <hits.hh>

using json = nlohmann::json;

class G4PVPlacement;
class G4Event;
class G4Run;

class ActionInitialization : public G4VUserActionInitialization {
public:
  ActionInitialization();
  virtual ~ActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;
};

/**
  CentralEventGenerator: fire events from a fixed radius at a
  detector/instrument volume. Allows you to offset a random amount from the
  point by |dx, dy, dz|. The particles launched are directed towards the origin
  offset by that amount. The momentum histogram is assumed to be a cube with
  equal binning along px, py, pz. The particle is always a photon; the momentum
  is equivalent to energy. The momentum units are in keV.
*/
struct CentralEventGenerator : public G4ParticleGun {
  // How far the beam can move off the shell along (x, y, z)
  const G4double beam_dx, beam_dy, beam_dz;
  // The sphere radius from which particles are nominally launched,
  // towards the origin
  const G4double radius;
  // Momentum bin edges (1D; assumes cube of momentum)
  const std::vector<double> momentum_bins;
  // How many momentum elements per axis of cube
  const uint64_t entries_per_axis;

  static std::unique_ptr<CentralEventGenerator>
  from(const std::string &config_file);
  void GeneratePrimaryVertex(G4Event *) override final;
  CentralEventGenerator() = delete;
  ~CentralEventGenerator() = default;
  CentralEventGenerator(const G4double dx, const G4double dy, const G4double dz,
                        const G4double radius,
                        const std::vector<double> &momentum_bins,
                        const std::vector<uint64_t> &momentum_hg);

private:
  std::mt19937 random_gen;
  // The weights are the number of hits per momentum bin.
  std::discrete_distribution<uint64_t> momentum_dist;
  // An offset to apply to initial positions of rays emitting from the spherical edge
  std::uniform_real_distribution<double> pos_unif;
};

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
  using gen_t = std::variant<std::unique_ptr<G4GeneralParticleSource>,
                             std::unique_ptr<CentralEventGenerator>>;
  PrimaryGeneratorAction();
  virtual ~PrimaryGeneratorAction();
  virtual void GeneratePrimaries(G4Event *) override;
  G4double GetParticleEnergy();

private:
  gen_t gen;
};

class RunAction : public G4UserRunAction {
public:
  RunAction();
  ~RunAction();

  virtual void BeginOfRunAction(const G4Run *) override;
  virtual void EndOfRunAction(const G4Run *) override;
};

class EventAction : public G4UserEventAction {
public:
  EventAction();
  ~EventAction();
  void EndOfEventAction(const G4Event *event) override final;
  void BeginOfEventAction(const G4Event *event) override final;

  void addScintillatedPhotons(size_t add);

private:
  G4Cache<size_t> scintillatedPhotonsPerEvent;
};

class SteppingAction : public G4UserSteppingAction {
public:
  SteppingAction();
  virtual ~SteppingAction();
  virtual void UserSteppingAction(const G4Step *);

private:
  void trackScintillation(const G4Step *step);
  void processOptical(const G4Step *step);
  void processDetected(const G4VPhysicalVolume *, const G4VPhysicalVolume *,
                       const G4Step *);
};
