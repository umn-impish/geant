#pragma once

#include "QBBC.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4DecayPhysics.hh"
    
namespace ImpressForGrips
{
class PhysicsList : public QBBC
{ 
public:
    PhysicsList();
    virtual ~PhysicsList() override;
};
}

