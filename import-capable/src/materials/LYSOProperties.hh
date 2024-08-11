#pragma once

#include <map>
#include <vector>

namespace Materials {

namespace Lyso {
static const std::string NAME = "lyso";
static const double DENSITY = 7.1 * g/cm3;

// relative fractions, not by weight.
// need to be normalized
static const std::map<std::string, double>
FRACTIONS = {
    {"Si", 2},
    {"O", 5},
    {"Y", 0.199},
    {"Lu", 0.791},
    {"Ce", 0.01}
};

static const double RESOLUTION_SCALE = 1;
static const double SCINT_YIELD = 33000 / MeV;
static const double DECAY_TIME_CONSTANT = 36 * ns;

// computed with a fit to experimental data
// see documentation folder
static const std::vector<double> RINDEX_ENERGIES = {
    1.00*eV, 1.50*eV, 2.00*eV, 2.50*eV, 3.00*eV, 3.50*eV, 4.00*eV
};
static const std::vector<double> RINDEX = {
    1.79, 1.79, 1.80, 1.81, 1.83, 2.10, 2.10
};

static const std::vector<double> SCINT_INTENS = {
    0.00e+00, 0.00e+00, 5.05e-05, 1.75e-04, 2.75e-04, 4.98e-04, 7.29e-04, 1.06e-03,
    1.74e-03, 2.66e-03, 4.84e-03, 8.10e-03, 1.12e-02, 1.46e-02, 1.79e-02, 2.13e-02,
    2.51e-02, 2.90e-02, 3.22e-02, 3.53e-02, 3.93e-02, 4.25e-02, 4.58e-02, 4.90e-02,
    5.24e-02, 5.54e-02, 5.74e-02, 5.49e-02, 5.18e-02, 4.79e-02, 4.44e-02, 4.10e-02,
    3.69e-02, 3.33e-02, 2.97e-02, 2.60e-02, 2.30e-02, 1.85e-02, 1.47e-02, 1.22e-02,
    8.98e-03, 5.60e-03, 2.23e-03, 1.87e-04, 0.00e+00,
};

static const std::vector<double> SCINT_ENERGIES = {
    1.85*eV, 1.88*eV, 1.93*eV, 1.98*eV, 2.04*eV, 2.09*eV, 2.15*eV, 2.22*eV,
    2.29*eV, 2.36*eV, 2.44*eV, 2.50*eV, 2.54*eV, 2.58*eV, 2.60*eV, 2.63*eV,
    2.65*eV, 2.68*eV, 2.70*eV, 2.71*eV, 2.73*eV, 2.75*eV, 2.77*eV, 2.79*eV,
    2.81*eV, 2.84*eV, 2.91*eV, 2.99*eV, 3.03*eV, 3.06*eV, 3.08*eV, 3.11*eV,
    3.13*eV, 3.15*eV, 3.15*eV, 3.18*eV, 3.19*eV, 3.20*eV, 3.22*eV, 3.24*eV,
    3.25*eV, 3.28*eV, 3.33*eV, 3.43*eV, 3.60*eV,
};

const std::vector<double> ABS_LEN_ENERGIES = {
    1.55310 * eV, 1.56982 * eV, 1.58669 * eV, 1.60394 * eV, 1.62156 * eV, 1.63979 * eV, 1.66110 * eV,
    1.68046 * eV, 1.70051 * eV, 1.71961 * eV, 1.74135 * eV, 1.75864 * eV, 1.78215 * eV, 1.80393 * eV,
    1.82625 * eV, 1.84913 * eV, 1.87486 * eV, 1.89752 * eV, 1.92134 * eV, 1.95281 * eV, 1.97773 * eV,
    2.00362 * eV, 2.03720 * eV, 2.06365 * eV, 2.09256 * eV, 2.12265 * eV, 2.15250 * eV, 2.18821 * eV,
    2.21876 * eV, 2.25221 * eV, 2.28964 * eV, 2.32398 * eV, 2.36386 * eV, 2.40326 * eV, 2.44593 * eV,
    2.48764 * eV, 2.52977 * eV, 2.57282 * eV, 2.61791 * eV, 2.66690 * eV, 2.71597 * eV, 2.76627 * eV,
    2.81975 * eV, 2.87533 * eV, 2.92140 * eV, 2.95130 * eV, 2.97039 * eV, 2.98398 * eV, 2.99479 * eV,
    2.99696 * eV, 3.00495 * eV, 3.01079 * eV, 3.01518 * eV, 3.03066 * eV, 3.03214 * eV, 3.03214 * eV,
    3.04330 * eV, 3.04555 * eV, 3.05530 * eV, 3.05530 * eV, 3.05756 * eV, 3.06134 * eV, 3.06134 * eV,
    3.06740 * eV, 3.07120 * eV, 3.07272 * eV, 3.08495 * eV, 3.08726 * eV, 3.08726 * eV, 3.08726 * eV,
    3.09033 * eV, 3.09265 * eV, 3.09342 * eV, 3.09419 * eV, 3.10038 * eV, 3.10659 * eV, 3.10737 * eV,
    3.11440 * eV, 3.12067 * eV, 3.12618 * eV, 3.13408 * eV, 3.14202 * eV, 3.14362 * eV, 3.19629 * eV,
    3.26704 * eV, 3.34099 * eV, 3.41837 * eV, 3.49941 * eV, 3.58439 * eV, 3.67469 * eV, 3.76852 * eV,
    3.86726 * eV, 3.97131 * eV, 4.08649 * eV
};
const std::vector<double> ABS_LEN = {
    163.28 * mm, 163.70 * mm, 163.48 * mm, 163.50 * mm, 163.48 * mm, 163.08 * mm, 163.12 * mm,
    163.22 * mm, 163.00 * mm, 162.84 * mm, 162.76 * mm, 163.00 * mm, 162.82 * mm, 162.84 * mm,
    162.74 * mm, 162.82 * mm, 162.68 * mm, 162.64 * mm, 162.48 * mm, 162.80 * mm, 162.60 * mm,
    162.38 * mm, 162.04 * mm, 161.92 * mm, 161.66 * mm, 161.44 * mm, 161.36 * mm, 161.10 * mm,
    161.12 * mm, 161.16 * mm, 161.08 * mm, 161.16 * mm, 160.66 * mm, 160.90 * mm, 160.92 * mm,
    160.58 * mm, 160.36 * mm, 160.54 * mm, 160.28 * mm, 159.82 * mm, 159.34 * mm, 159.36 * mm,
    158.80 * mm, 157.60 * mm, 155.24 * mm, 151.78 * mm, 148.42 * mm, 144.42 * mm, 140.92 * mm,
    136.16 * mm, 131.00 * mm, 126.66 * mm, 122.50 * mm, 113.78 * mm, 118.90 * mm, 106.10 * mm,
    101.78 * mm, 109.40 * mm,  99.86 * mm,  89.50 * mm,  95.46 * mm,  82.56 * mm,  92.48 * mm,
     86.24 * mm,  74.04 * mm,  78.88 * mm,  64.30 * mm,  59.76 * mm,  52.22 * mm,  69.28 * mm,
     44.76 * mm,  56.06 * mm,  41.22 * mm,  48.86 * mm,  36.60 * mm,  27.06 * mm,  31.24 * mm,
     22.30 * mm,  17.78 * mm,  13.06 * mm,   9.10 * mm,   0.78 * mm,   6.04 * mm,   0.65 * mm,
      0.62 * mm,   0.75 * mm,   0.74 * mm,   0.75 * mm,   0.73 * mm,   0.74 * mm,   0.50 * mm,
      0.58 * mm,   0.56 * mm,   0.99 * mm
};

}

}
