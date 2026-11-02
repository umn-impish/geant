#include <fstream>

#include <nlohmann/json.hpp>
#include <G4Gamma.hh>
#include <G4SystemOfUnits.hh>
#include <central-event-generator.hh>

using json = nlohmann::json;

thread_local std::random_device CentralEventGenerator::rd{};
thread_local std::mt19937 CentralEventGenerator::random_gen{
    CentralEventGenerator::rd()};

CentralEventGenerator::CentralEventGenerator(
    const G4double dx, const G4double dy, const G4double dz,
    const G4double radius_, const std::vector<double> &momentum_bins_,
    const std::vector<uint64_t> &momentum_hg)
    : beam_dx{dx}, beam_dy{dy}, beam_dz{dz}, radius{radius_},
      momentum_bins{momentum_bins_}, entries_per_axis{momentum_bins.size() - 1},
      momentum_dist{momentum_hg.begin(), momentum_hg.end()},
      pos_unif{-0.5, 0.5} {
  if (entries_per_axis * entries_per_axis * entries_per_axis !=
      momentum_hg.size()) {
    throw std::runtime_error{"The momentum histogram provided is not a cube, "
                             "or the bins don't line up properly."};
  }
  // We only launch photons with this generator
  SetParticleDefinition(G4Gamma::Definition());
}

std::unique_ptr<CentralEventGenerator>
CentralEventGenerator::from(const std::string &config_fn) {
  auto dat = json::parse(std::ifstream{config_fn});
  auto momentum_hg = dat["momentum_hg_kev"].get<std::vector<uint64_t>>();
  return std::make_unique<CentralEventGenerator>(
      dat["beam_dx_mm"].get<double>() * mm,
      dat["beam_dy_mm"].get<double>() * mm,
      dat["beam_dz_mm"].get<double>() * mm, dat["radius_mm"].get<double>() * mm,
      dat["momentum_bins_kev"].get<std::vector<double>>(), momentum_hg);
}

void CentralEventGenerator::GeneratePrimaryVertex(G4Event *evt) {
  // Generate a 1D index from the flattened momentum distribution,
  // and convert it back to a 3D index (momentum is 3D)
  auto mom_idx = momentum_dist(random_gen);
  auto i = mom_idx / (entries_per_axis * entries_per_axis);
  auto j = (mom_idx / entries_per_axis) % entries_per_axis;
  auto k = mom_idx % entries_per_axis;

  // Generate the momentum of the current particle,
  // drawing from the loaded histogram
  std::uniform_real_distribution<double> unif{0.0, 1.0};
  auto gen_mom = [&unif, this](auto idx) {
    return this->momentum_bins[idx] +
           unif(random_gen) * (momentum_bins[idx + 1] - momentum_bins[idx]);
  };
  auto px = gen_mom(i) * keV;
  auto py = gen_mom(j) * keV;
  auto pz = gen_mom(k) * keV;
  auto energy = std::sqrt(px * px + py * py + pz * pz);
  // Normalize the momentum to its direction
  auto pxhat = px / energy, pyhat = py / energy, pzhat = pz / energy;

  // Nominal position: somewhere on the sphere. The origin of the ray along the
  // momentum connecting from the shell to the origin.
  G4ThreeVector nom{-radius * px / energy, -radius * py / energy,
                    -radius * pz / energy};
  // Add a random offset ~|dx, dy, dz| onto the nominal position.
  // This is so that not every single ray passes through the origin.
  // The magnitude is set by the extent of your simulated volume.
  auto dx_scale = pos_unif(random_gen), dy_scale = pos_unif(random_gen),
       dz_scale = pos_unif(random_gen);
  G4ThreeVector pos{nom[0] + dx_scale * beam_dx, nom[1] + dy_scale * beam_dy,
                    nom[2] + dz_scale * beam_dz};

  // Set the physical parameters, and generate the particle
  SetParticlePosition(pos);
  // Pixellating the (px, py, pz) bins can lead to violation of the max energy--
  // the energy histogram uses cubic voxels, but the actual bounding surface is
  // a sphere. Cap the energy at the upper edge of the distribution.
  SetParticleEnergy(std::min(energy, momentum_bins.back() * keV));
  SetParticleMomentumDirection({pxhat, pyhat, pzhat});
  G4ParticleGun::GeneratePrimaryVertex(evt);
}