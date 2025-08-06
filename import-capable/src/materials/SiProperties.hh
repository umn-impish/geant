#pragma once

#include <G4String.hh>
#include <G4Types.hh>
#include <G4SystemOfUnits.hh>

namespace Materials
{
static const G4double SI_DENSITY = 2.33 * g / cm3;

const std::vector<G4double> SI_DET_EFF_ENERGIES = {
    1.*eV, 5.*eV
};

const std::vector<G4double> SI_DET_EFF = std::vector<double>(SI_DET_EFF_ENERGIES.size(), 1);
// set to zero so that we either detect or reflect
const std::vector<G4double> SI_TRANSMITTANCE = std::vector<double>(SI_DET_EFF_ENERGIES.size(), 0);

const std::vector<G4double> SI_REFR_IDX_ENERGY = {
    0.1*eV, 8*eV
};

const std::vector<G4double> SI_REFR_IDX_REAL = {
    1.6, 1.6
};
}
