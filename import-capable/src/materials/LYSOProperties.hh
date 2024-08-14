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
    0.1*eV, 6*eV
};
const std::vector<double> ABS_LEN = {
    50*cm, 50*cm
};

}

}
