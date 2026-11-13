#include <memory>
#include <random>

#include <G4ParticleGun.hh>

#include <configs.hh>

/**
  CentralEventGenerator: fire events from a fixed radius at a
  detector/instrument volume. Allows you to offset a random amount from the
  point by |dx, dy, dz|. The particles launched are directed towards the origin
  offset by that amount. The momentum histogram is assumed to be a cube with
  equal binning along px, py, pz. The particle is always a photon; the momentum
  is equivalent to energy (E = pc). The momentum units are in keV.
*/
struct CentralEventGenerator : public G4ParticleGun {
  // How far the beam can move off the shell along (x, y, z)
  const G4double beam_dx, beam_dy, beam_dz;
  // The sphere radius from which particles are nominally launched,
  // towards the origin
  const G4double radius;
  // Momentum bin edges (1D; assumes cube of momentum)
  const std::vector<double> momentum_bins;
  // How many momentum elements per axis of cube
  const uint64_t entries_per_axis;

  static std::unique_ptr<CentralEventGenerator>
  from(const std::string &config_file);

  void GeneratePrimaryVertex(G4Event *) override final;

  CentralEventGenerator() = delete;
  ~CentralEventGenerator() = default;
  CentralEventGenerator(const G4double dx, const G4double dy, const G4double dz,
                        const G4double radius,
                        const std::vector<double> &momentum_bins,
                        const std::vector<uint16_t> &momentum_hg);

private:
  static thread_local std::random_device rd;
  static thread_local std::mt19937 random_gen;
  // The weights are the number of hits per momentum bin.
  std::discrete_distribution<uint16_t> momentum_dist;
  // An offset to apply to initial positions of rays emitting from the spherical
  // edge
  std::uniform_real_distribution<double> pos_unif;
};