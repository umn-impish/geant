#pragma once

#include <map>
#include <vector>

namespace Materials {

namespace Gagg {
static const std::string NAME = "gagg:ce";
static const double DENSITY = 6.63 * g/cm3;

// relative fractions, not by weight.
// need to be normalized
static const std::map<std::string, double>
FRACTIONS = {
    {"Gd", 2.99},
    {"Ce", 0.01},
    {"Al", 2},
    {"Ga", 1},
    {"O", 12},
};


static const double RESOLUTION_SCALE = 1;
static const double SCINT_YIELD = 40 / keV;
static const double DECAY_TIME_CONSTANT = 50 * ns;

static const std::vector<double> SCINT_ENERGIES = {
    1.58*eV, 1.60*eV, 1.62*eV, 1.63*eV, 1.65*eV, 1.67*eV, 1.69*eV, 1.71*eV, 1.73*eV,
    1.75*eV, 1.77*eV, 1.79*eV, 1.81*eV, 1.84*eV, 1.86*eV, 1.88*eV, 1.91*eV, 1.93*eV, 1.95*eV,
    1.97*eV, 1.98*eV, 2.00*eV, 2.01*eV, 2.03*eV, 2.04*eV, 2.06*eV, 2.07*eV, 2.09*eV, 2.10*eV,
    2.11*eV, 2.13*eV, 2.14*eV, 2.16*eV, 2.17*eV, 2.20*eV, 2.22*eV, 2.24*eV, 2.26*eV, 2.30*eV,
    2.33*eV, 2.37*eV, 2.40*eV, 2.41*eV, 2.43*eV, 2.44*eV, 2.46*eV, 2.47*eV, 2.48*eV, 2.49*eV,
    2.50*eV, 2.50*eV, 2.51*eV, 2.52*eV, 2.53*eV, 2.53*eV, 2.54*eV, 2.56*eV, 2.57*eV, 2.58*eV,
    2.60*eV, 2.62*eV, 2.65*eV, 2.69*eV, 2.74*eV,
};

static const std::vector<double> SCINT_INTENS = {
    3.044e-04, 5.458e-04, 6.695e-04, 7.818e-04, 8.829e-04, 9.279e-04, 1.079e-03, 1.304e-03, 1.602e-03, 1.939e-03,
    2.327e-03, 2.786e-03, 3.355e-03, 4.046e-03, 4.844e-03, 5.787e-03, 6.821e-03, 8.136e-03, 9.564e-03, 1.089e-02,
    1.220e-02, 1.364e-02, 1.508e-02, 1.654e-02, 1.798e-02, 1.931e-02, 2.068e-02, 2.218e-02, 2.369e-02, 2.508e-02,
    2.649e-02, 2.779e-02, 2.918e-02, 3.071e-02, 3.210e-02, 3.335e-02, 3.479e-02, 3.631e-02, 3.721e-02, 3.759e-02,
    3.712e-02, 3.575e-02, 3.416e-02, 3.257e-02, 3.095e-02, 2.918e-02, 2.730e-02, 2.531e-02, 2.347e-02, 2.179e-02,
    1.977e-02, 1.806e-02, 1.631e-02, 1.472e-02, 1.258e-02, 1.052e-02, 8.878e-03, 7.132e-03, 5.556e-03, 4.059e-03,
    2.468e-03, 1.040e-03, 5.290e-04, 2.820e-04,
};

static const std::vector<double> RINDEX_ENERGIES = {
    1.26*eV, 1.27*eV, 1.27*eV, 1.28*eV, 1.29*eV, 1.30*eV, 1.31*eV, 1.31*eV, 1.32*eV, 1.33*eV,
    1.34*eV, 1.36*eV, 1.37*eV, 1.38*eV, 1.39*eV, 1.40*eV, 1.41*eV, 1.43*eV, 1.44*eV, 1.45*eV,
    1.46*eV, 1.48*eV, 1.49*eV, 1.50*eV, 1.52*eV, 1.53*eV, 1.55*eV, 1.56*eV, 1.57*eV, 1.59*eV,
    1.61*eV, 1.62*eV, 1.64*eV, 1.65*eV, 1.67*eV, 1.69*eV, 1.70*eV, 1.72*eV, 1.74*eV, 1.76*eV,
    1.78*eV, 1.80*eV, 1.82*eV, 1.84*eV, 1.86*eV, 1.88*eV, 1.90*eV, 1.92*eV, 1.94*eV, 1.97*eV,
    1.99*eV, 2.01*eV, 2.04*eV, 2.07*eV, 2.09*eV, 2.12*eV, 2.15*eV, 2.17*eV, 2.20*eV, 2.23*eV,
    2.26*eV, 2.29*eV, 2.33*eV, 2.36*eV, 2.39*eV, 2.42*eV, 2.45*eV, 2.48*eV, 2.50*eV, 2.54*eV,
    2.58*eV, 2.60*eV, 2.62*eV, 2.67*eV, 2.71*eV, 2.75*eV, 2.79*eV, 2.83*eV, 2.88*eV, 2.93*eV,
    2.94*eV, 2.98*eV, 3.02*eV, 3.03*eV, 3.06*eV, 3.09*eV, 3.09*eV, 3.12*eV, 3.18*eV, 3.20*eV,
    3.25*eV, 3.31*eV, 3.31*eV, 3.34*eV, 3.39*eV, 3.45*eV, 3.51*eV, 3.51*eV, 3.51*eV, 3.53*eV,
    3.53*eV, 3.54*eV, 3.58*eV, 3.63*eV, 3.68*eV, 3.74*eV, 3.76*eV, 3.81*eV, 3.83*eV, 3.86*eV,
    3.90*eV, 3.92*eV, 3.92*eV, 3.98*eV, 4.00*eV, 4.04*eV, 4.09*eV, 4.14*eV, 4.17*eV, 4.20*eV,
    4.22*eV, 4.26*eV, 4.31*eV, 4.36*eV, 4.39*eV, 4.44*eV, 4.49*eV, 4.52*eV, 4.52*eV, 4.56*eV,
    4.59*eV, 4.62*eV, 4.65*eV, 4.67*eV, 4.70*eV, 4.73*eV, 4.76*eV, 4.79*eV, 4.80*eV, 4.87*eV,
    4.88*eV, 4.89*eV, 4.91*eV, 4.95*eV, 4.96*eV, 4.97*eV, 5.00*eV, 5.05*eV, 5.05*eV, 5.08*eV,
};

static const std::vector<double> RINDEX = {
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00,
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00,
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00,
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00,
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00,
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00,
    1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.87e+00, 1.88e+00,
    1.87e+00, 1.87e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00,
    1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00, 1.88e+00,
    1.88e+00, 1.88e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00,
    1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.89e+00, 1.90e+00, 1.90e+00, 1.90e+00, 1.90e+00,
    1.90e+00, 1.90e+00, 1.90e+00, 1.90e+00, 1.90e+00, 1.91e+00, 1.91e+00, 1.91e+00, 1.91e+00, 1.91e+00,
    1.92e+00, 1.92e+00, 1.92e+00, 1.92e+00, 1.92e+00, 1.92e+00, 1.93e+00, 1.93e+00, 1.93e+00, 1.93e+00,
    1.93e+00, 1.94e+00, 1.94e+00, 1.94e+00, 1.94e+00, 1.94e+00, 1.94e+00, 1.95e+00, 1.95e+00, 1.95e+00,
    1.96e+00, 1.95e+00, 1.96e+00, 1.96e+00, 1.96e+00, 1.96e+00, 1.97e+00, 1.97e+00, 1.97e+00, 1.97e+00,
};

const std::vector<double> ABS_LEN_ENERGIES = SCINT_ENERGIES;
const std::vector<double> ABS_LEN(ABS_LEN_ENERGIES.size(), 100. * m);

}

}
