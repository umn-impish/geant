#pragma once

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4ParticleGun.hh"

class G4GeneralParticleSource;
class G4Event;
class G4Box;

namespace ImpressForGrips
{
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();    
    virtual ~PrimaryGeneratorAction();
    virtual void GeneratePrimaries(G4Event*) override;

  private:
    std::unique_ptr<G4GeneralParticleSource> gps;
};
}
