#include <memory>
#include <random>

#include <G4ParticleGun.hh>

#include <configs.hh>

/**
  CentralEventGenerator: fire events from an arbitrary momentum distribution
  towards a random point in an instrument volume.
  The particles launched are directed towards a randomly-generated point
  within [-d\vec{r}/2, d\vec{r}/2].
  The momentum histogram is assumed to be a cube with
  equal binning along px, py, pz. The particle is always a photon; the momentum
  is equivalent to energy (E = pc). The momentum units are in keV.
*/
struct CentralEventGenerator : public G4ParticleGun {
  // Dimensions which bound the instrument under investigation,
  // assuming it's centered at the origin
  const G4double bounding_dx, bounding_dy, bounding_dz;
  // How far to rotate the momentum distribution along the polar angle.
  // Useful if you want to simulate some pointing vs. source elevation
  const G4double polar_rotation;
  // Momentum bin edges (1D; assumes cube of momentum)
  const std::vector<double> momentum_bins;
  // How many momentum elements per axis of cube
  const uint64_t entries_per_axis;

  static std::unique_ptr<CentralEventGenerator>
  from(const std::string &config_file);

  void GeneratePrimaryVertex(G4Event *) override final;
  // Projected area of current vertex
  double projected_area();

  CentralEventGenerator() = delete;
  ~CentralEventGenerator() = default;
  CentralEventGenerator(const G4double dx, const G4double dy, const G4double dz,
                        const G4double rotation,
                        const std::vector<double> &momentum_bins,
                        const std::vector<uint32_t> &momentum_hg);

private:
  static thread_local std::random_device rd;
  static thread_local std::mt19937 random_gen;
  // The weights are the number of hits per momentum bin.
  std::discrete_distribution<uint32_t> momentum_dist;
  // An offset to apply to initial positions of rays emitting from the spherical
  // edge
  std::uniform_real_distribution<double> pos_unif;

  // The projected area of the current photon
  double atot;
};
