#include "PhysicsList.hh"
#include "G4Radioactivation.hh"

namespace ImpressForGrips
{
PhysicsList::PhysicsList() :
    QBBC()
{
    // updates to FTFP_BERT lists
    ReplacePhysics(new G4EmStandardPhysics_option4);
    RegisterPhysics(new G4OpticalPhysics);
    RegisterPhysics(new G4DecayPhysics);
    // RegisterPhysics(new G4RadioactiveDecayPhysics);
    // RegisterPhysics(new G4Radioactivation);
} 
    
PhysicsList::~PhysicsList()
{ }   
}
