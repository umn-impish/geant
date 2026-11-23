#include <G4ThreeVector.hh>
#include <cmath>
#include <fstream>

#include <G4Gamma.hh>
#include <G4RotationMatrix.hh>
#include <G4SystemOfUnits.hh>
#include <central-event-generator.hh>
#include <iterator>
#include <msgpack.hpp>
#include <zlib.h>

thread_local std::random_device CentralEventGenerator::rd{};
thread_local std::mt19937 CentralEventGenerator::random_gen{
    CentralEventGenerator::rd()};

CentralEventGenerator::CentralEventGenerator(
    const G4double dx, const G4double dy, const G4double dz,
    const G4double rotation, const std::vector<double> &momentum_bins_,
    const std::vector<uint32_t> &momentum_hg)
    : bounding_dx{dx}, bounding_dy{dy}, bounding_dz{dz},
      polar_rotation{rotation}, momentum_bins{momentum_bins_},
      entries_per_axis{momentum_bins.size() - 1},
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
  std::ifstream cfg_file{config_fn, std::ios::binary};
  std::vector<char> cfg_buf(std::istreambuf_iterator<char>{cfg_file},
                            std::istreambuf_iterator<char>{});
  auto handle = msgpack::unpack(cfg_buf.data(), cfg_buf.size());
  auto des = handle.get();
  std::unordered_map<std::string, msgpack::object> cfg_map;
  des.convert(cfg_map);

  auto momentum_bins = cfg_map["momentum_bins_kev"].as<std::vector<double>>();

  // Extract the compressed histogram
  auto entries_per_axis = momentum_bins.size() - 1;
  // The number of elements in the histogram we expect to process
  auto num_hg_elts = static_cast<uLongf>(entries_per_axis * entries_per_axis *
                                         entries_per_axis);
  std::vector<uint32_t> momentum_hg(num_hg_elts);
  auto num_bytes = sizeof(decltype(momentum_hg)::value_type) * num_hg_elts;
  auto mom_hg_bin = cfg_map["momentum_hg_kev"].via.bin;
  int decomp_res =
      uncompress(reinterpret_cast<Bytef *>(momentum_hg.data()), &num_bytes,
                 reinterpret_cast<const Bytef *>(mom_hg_bin.ptr),
                 static_cast<uLongf>(mom_hg_bin.size));
  if (decomp_res != Z_OK) {
    std::ostringstream oss;
    oss << "Failed to decompress momentum histogram: " << zError(decomp_res)
        << std::endl
        << "Num elts was " << num_hg_elts << " and vector size was "
        << momentum_hg.size() << ";\n"
        << "Compressed size was " << static_cast<uLongf>(mom_hg_bin.size)
        << std::endl
        << "Number of elements written into histogram: " << num_hg_elts
        << std::endl;
    throw std::runtime_error{oss.str()};
  }

  return std::make_unique<CentralEventGenerator>(
      cfg_map["bounding_dx_mm"].as<double>() * mm,
      cfg_map["bounding_dy_mm"].as<double>() * mm,
      cfg_map["bounding_dz_mm"].as<double>() * mm,
      cfg_map["polar_rotation_degrees"].as<double>() * deg, momentum_bins,
      momentum_hg);
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
  G4ThreeVector p{gen_mom(i) * keV, gen_mom(j) * keV, gen_mom(k) * keV};
  auto energy = p.r();
  auto phat = p.unit().rotate(polar_rotation, G4ThreeVector{0, 1, 0});

  // Pick one of the directions proporitonal to the projected area of the box
  // that we see from the current momentum vantage point.
  // Area [ij] is foreshortened by abs(angle of rotation) around axis [k];
  // refer to this area as a[k].
  auto ax = bounding_dy * bounding_dz * std::abs(phat.x());
  auto ay = bounding_dx * bounding_dz * std::abs(phat.y());
  auto az = bounding_dx * bounding_dy * std::abs(phat.z());
  atot = ax + ay + az;

  // Generate a random amount of projected area and "bin" it into face
  // probability bins using if/else statements.
  // We are generating a random point on the bounding box face.
  auto area_thresh = unif(random_gen) * atot;
  G4double x, y, z;
  if (area_thresh < ax) {
    x = 0.5 * (phat.x() > 0 ? -bounding_dx : bounding_dx);
    y = pos_unif(random_gen) * bounding_dy;
    z = pos_unif(random_gen) * bounding_dz;
  } else if (area_thresh < (ax + ay)) {
    x = pos_unif(random_gen) * bounding_dx;
    y = 0.5 * (phat.y() > 0 ? -bounding_dy : bounding_dy);
    z = pos_unif(random_gen) * bounding_dz;

  } else {
    x = pos_unif(random_gen) * bounding_dx;
    y = pos_unif(random_gen) * bounding_dy;
    z = 0.5 * (phat.z() > 0 ? -bounding_dz : bounding_dz);
  }

  // Originate the photon a short distance away from the bounding box
  auto eps = 1 * um;
  G4ThreeVector origin_r(x, y, z);
  origin_r -= eps * phat;

  SetParticlePosition(origin_r);
  // Pixellating the (px, py, pz) bins can lead to violation of the max energy--
  // the energy histogram uses cubic voxels, but the actual bounding surface is
  // a sphere. Cap the energy at the upper edge of the distribution.
  SetParticleEnergy(std::min(energy, momentum_bins.back() * keV));
  SetParticleMomentumDirection(phat);
  G4ParticleGun::GeneratePrimaryVertex(evt);
}

double CentralEventGenerator::projected_area() { return atot; }
