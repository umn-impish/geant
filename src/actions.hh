#pragma once

/*
    Includes short classes that facilitate detection.
    Excluded is the SteppingAction because the code is long.
*/

#include <memory>

#include <G4GeneralParticleSource.hh>
#include <G4GenericIon.hh>
#include <G4OpBoundaryProcess.hh>
#include <G4UserRunAction.hh>
#include <G4UserEventAction.hh>
#include <G4VUserActionInitialization.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4UserSteppingAction.hh>

#include <hits.hh>

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

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();    
    virtual ~PrimaryGeneratorAction();
    virtual void GeneratePrimaries(G4Event*) override;

private:
    std::unique_ptr<G4GeneralParticleSource> gps;
};

class RunAction : public G4UserRunAction {
public:
    RunAction();
    ~RunAction();

    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;
};

class EventAction : public G4UserEventAction {
public:
    EventAction();
    ~EventAction();
    void EndOfEventAction(const G4Event* event) override final;
    void BeginOfEventAction(const G4Event* event) override final;

    void addScintillatedPhotons(size_t add);
private:
    G4Cache<size_t> scintillatedPhotonsPerEvent;
};

class SteppingAction : public G4UserSteppingAction {
public:
    SteppingAction();
    virtual ~SteppingAction();
    virtual void UserSteppingAction(const G4Step*);

private:
    void trackScintillation(const G4Step* step);
    void processOptical(const G4Step* step);
    void processDetected(const G4VPhysicalVolume*, const G4VPhysicalVolume*, const G4Step*);
};

