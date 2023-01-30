#pragma once

#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
    
namespace ImpressForGrips
{
class PhysicsList : public FTFP_BERT
{ 
    public:
        PhysicsList();
        virtual ~PhysicsList() override;
    private:

};
}

