#include <G4Gamma.hh>
#include <G4GeneralParticleSource.hh>
#include <G4GeneralParticleSourceData.hh>
#include <G4ParticleDefinition.hh>
#include <actions.hh>
#include <analysis.hh>
#include <configs.hh>
#include <detectors.hh>

#include <G4EventManager.hh>
#include <G4ProcessManager.hh>
#include <G4SystemOfUnits.hh>

ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const {
  SetUserAction(new RunAction);
}

void ActionInitialization::Build() const {
  SetUserAction(new RunAction);
  SetUserAction(new EventAction);
  SetUserAction(new SteppingAction);
  SetUserAction(new PrimaryGeneratorAction);
}

CentralEventGenerator::CentralEventGenerator(
    const G4double dx, const G4double dy, const G4double dz,
    const G4double radius_, const std::vector<double> &momentum_bins_,
    const std::vector<uint64_t> &momentum_hg)
    : beam_dx{dx}, beam_dy{dy}, beam_dz{dz}, radius{radius_},
      momentum_bins{momentum_bins_}, entries_per_axis{momentum_bins.size() - 1},
      random_gen{std::random_device()()},
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
           unif(this->random_gen) *
               (momentum_bins[idx + 1] - momentum_bins[idx]) / 2;
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
  // Add a random amount |dx, dy, dz| onto the nominal position.
  // This is so that not every single ray passes through the origin.
  // The amount is set by the extent of your simulated volume.
  G4ThreeVector pos{nom[0] + pos_unif(random_gen) * beam_dx,
                    nom[1] + pos_unif(random_gen) * beam_dy,
                    nom[2] + pos_unif(random_gen) * beam_dz};

  // Set the physical parameters, and generate the particle
  SetParticlePosition(pos);
  // Pixellating the (px, py, pz) bins can lead to violation of the max energy--
  // the energy histogram uses cubic voxels, but the actual bounding surface is
  // a sphere. Cap the energy at the upper edge of the distribution.
  SetParticleEnergy(std::min(energy, momentum_bins.back() * keV));
  SetParticleMomentumDirection({pxhat, pyhat, pzhat});
  G4ParticleGun::GeneratePrimaryVertex(evt);
}

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction() {
  auto gf = GlobalConfigs::instance();
  auto option = gf.configOption<std::string>("generator-type");
  if (option == "central-generator") {
    auto fn = gf.configOption<std::string>("central-generator-config");
    gen = CentralEventGenerator::from(fn);
  } else if (option == "gps") {
    gen = std::make_unique<G4GeneralParticleSource>();
  } else {
    throw std::runtime_error{"Unknown primary generator '" + option + "'"};
  }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *evt) {
  std::visit(
      [evt](auto &g) {
        g->GeneratePrimaryVertex(evt);
        double e = g->GetParticleEnergy();
        Analysis::instance().addIncidentEnergy(e);
      },
      gen);
}

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run *) {
  Analysis::instance().initFiles(IsMaster());
}

void RunAction::EndOfRunAction(const G4Run *) {
  Analysis::instance().saveFiles(IsMaster());
}

EventAction::EventAction()
    : G4UserEventAction(), scintillatedPhotonsPerEvent{0} {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  scintillatedPhotonsPerEvent.Put(0);
}

void EventAction::EndOfEventAction(const G4Event *evt) {
  if (!evt)
    return;
  auto &anInst = Analysis::instance();
  anInst.saveEvent(evt);
  anInst.saveScintillated(scintillatedPhotonsPerEvent.Get());
}

void EventAction::addScintillatedPhotons(size_t add) {
  scintillatedPhotonsPerEvent.Put(scintillatedPhotonsPerEvent.Get() + add);
}

// Things needed for the SteppingAction
namespace {
static const std::map<size_t, const char *> optProcLookup = {
    {0, "Undefined,"},
    {1, "Transmission,"},
    {2, "FresnelRefraction,"},
    {3, "FresnelReflection,"},
    {4, "TotalInternalReflection,"},
    {5, "LambertianReflection,"},
    {6, "LobeReflection,"},
    {7, "SpikeReflection,"},
    {8, "BackScattering,"},
    {9, "Absorption,"},
    {10, "Detection,"},
    {11, "NotAtBoundary,"},
    {12, "SameMaterial,"},
    {13, "StepTooSmall,"},
    {14, "NoRINDEX,"},
    {15, "PolishedLumirrorAirReflection,"},
    {16, "PolishedLumirrorGlueReflection,"},
    {17, "PolishedAirReflection,"},
    {18, "PolishedTeflonAirReflection,"},
    {19, "PolishedTiOAirReflection,"},
    {20, "PolishedTyvekAirReflection,"},
    {21, "PolishedVM2000AirReflection,"},
    {22, "PolishedVM2000GlueReflection,"},
    {23, "EtchedLumirrorAirReflection,"},
    {24, "EtchedLumirrorGlueReflection,"},
    {25, "EtchedAirReflection,"},
    {26, "EtchedTeflonAirReflection,"},
    {27, "EtchedTiOAirReflection,"},
    {28, "EtchedTyvekAirReflection,"},
    {29, "EtchedVM2000AirReflection,"},
    {30, "EtchedVM2000GlueReflection,"},
    {31, "GroundLumirrorAirReflection,"},
    {32, "GroundLumirrorGlueReflection,"},
    {33, "GroundAirReflection,"},
    {34, "GroundTeflonAirReflection,"},
    {35, "GroundTiOAirReflection,"},
    {36, "GroundTyvekAirReflection,"},
    {37, "GroundVM2000AirReflection,"},
    {38, "GroundVM2000GlueReflection,"},
    {39, "Dichroic,"},
    {40, "CoatedDielectricReflection,"},
    {41, "CoatedDielectricRefraction,"},
    {42, "CoatedDielectricFrustratedTransmission"},
};

const G4OpBoundaryProcess *findOpticalBoundary(const G4Step *step) {
  const auto *pv =
      step->GetTrack()->GetDefinition()->GetProcessManager()->GetProcessList();

  for (size_t i = 0; i < pv->size(); ++i) {
    if ((*pv)[i]->GetProcessName() == "OpBoundary") {
      return static_cast<const G4OpBoundaryProcess *>((*pv)[i]);
    }
  }

  G4Exception("findOpticalBoundary", "fob1", RunMustBeAborted,
              "issue finding optical boundary");

  return nullptr;
}
} // namespace

SteppingAction::SteppingAction() : G4UserSteppingAction() {}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step *step) {
  trackScintillation(step);
  auto *track = step->GetTrack();
  if (track->GetDefinition() == G4OpticalPhoton::Definition()) {
    processOptical(step);
  }
}

void SteppingAction::trackScintillation(const G4Step *step) {
  const std::vector<const G4Track *> *secs;
  if ((secs = step->GetSecondaryInCurrentStep()) && secs->size() > 0) {
    size_t numOpticals =
        std::count_if(secs->begin(), secs->end(), [](const G4Track *t) {
          return t->GetParticleDefinition()->GetParticleName() ==
                 "opticalphoton";
        });
    // store the number of optical photons generated
    // to be saved at end of event
    auto *ea = dynamic_cast<EventAction *>(
        G4EventManager::GetEventManager()->GetUserEventAction());
    if (ea)
      ea->addScintillatedPhotons(numOpticals);
  }
}

void SteppingAction::processOptical(const G4Step *step) {
  auto *track = step->GetTrack();
  const double KILL_LENGTH = 500 * m;
  if (track->GetTrackLength() > KILL_LENGTH) {
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  static const G4ThreadLocal G4OpBoundaryProcess *boundary =
      findOpticalBoundary(step);
  if (boundary == nullptr)
    return;

  const auto *prePt = step->GetPreStepPoint();
  const auto *postPt = step->GetPostStepPoint();

  const auto *preVol = prePt ? prePt->GetPhysicalVolume() : nullptr;
  const auto *postVol = postPt ? postPt->GetPhysicalVolume() : nullptr;

  const G4String preName = preVol ? preVol->GetName() : "";
  const G4String postName = postVol ? postVol->GetName() : "";

  if (postPt->GetStepStatus() == fGeomBoundary) {
    auto stat = boundary->GetStatus();
    switch (stat) {
    case Detection:
      // detect it here because it dies before it can actually register inside
      // the detector
      processDetected(preVol, postVol, step);
      break;
    case Absorption:
    case TotalInternalReflection:
    case StepTooSmall:
    case Transmission:
    case FresnelRefraction:
    case FresnelReflection:
    case NoRINDEX:
    default:
      break;
    }
  }
}

void SteppingAction::processDetected(const G4VPhysicalVolume *preVol,
                                     const G4VPhysicalVolume *postVol,
                                     const G4Step *step) {
  std::array<const G4VPhysicalVolume *, 2> volz = {preVol, postVol};
  for (const auto *v : volz) {
    const auto *sdLogVol = v->GetLogicalVolume();
    auto *siSd =
        dynamic_cast<SiSensitiveDetector *>(sdLogVol->GetSensitiveDetector());
    if (siSd)
      siSd->processOptical(step);
  }
}
