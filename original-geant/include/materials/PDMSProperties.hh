#pragma once


namespace ImpressForGrips {namespace Materials
{
static const G4String kPDMS = "pdms";
static constexpr G4double PDMS_DENSITY = 965 * kg / m3;

const std::vector<G4double> PDMS_REFR_IDX_ENERGIES = {
    1.37760e+00*eV, 1.42675e+00*eV, 1.48129e+00*eV, 1.53827e+00*eV, 1.60186e+00*eV, 1.66870e+00*eV, 1.74380e+00*eV, 1.82598e+00*eV,
    1.91334e+00*eV, 2.01273e+00*eV, 2.11939e+00*eV, 2.24203e+00*eV, 2.37518e+00*eV, 2.53029e+00*eV, 2.70708e+00*eV, 2.90361e+00*eV,
    3.13884e+00*eV, 3.40616e+00*eV, 3.73446e+00*eV, 4.13281e+00*eV
};

const std::vector<G4double> PDMS_REFR_IDXS = {
    1.40611e+00, 1.40665e+00, 1.40726e+00, 1.40792e+00, 1.40869e+00, 1.40953e+00, 1.41052e+00, 1.41164e+00,
    1.41289e+00, 1.41438e+00, 1.41605e+00, 1.41808e+00, 1.42039e+00, 1.42324e+00, 1.42669e+00, 1.43076e+00,
    1.43596e+00, 1.44227e+00, 1.45059e+00, 1.46146e+00
};    

} }
