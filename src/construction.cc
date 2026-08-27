#include <G4Material.hh>
#include <random>

#include <configs.hh>
#include <construction.hh>
#include <detectors.hh>
#include <materials.hh>

#include <CLHEP/Vector/EulerAngles.h>
#include <G4Box.hh>
#include <G4Exception.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4PVPlacement.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4RotationMatrix.hh>
#include <G4SDManager.hh>
#include <G4VisAttributes.hh>
#include <stdexcept>

static constexpr bool checkOverlaps = true;
static G4OpticalSurface *siOpticalSurface();
void attachEsrOpticalSurface(G4LogicalVolume *lv);
void attachLambertianOpticalSurface(G4LogicalVolume *lv);

DetectorConstruction::DetectorConstruction(std::string meta_fn)
    : G4VUserDetectorConstruction(),

      // We pass in the metadata file name which contains a
      // JSON array.
      // For some reason, json::parse parses the array to
      // a nested array(?) so we only want the first element.
      meta_fn{meta_fn}, siLogVols{}, crLogVols{}, perfectLogVols{} {
  Materials::makeMaterials();
}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  auto *worldPlacement = makeWorld();

  // Import 3D models via CADMesh
  importSolids();

  return worldPlacement;
}

G4PVPlacement *DetectorConstruction::makeWorld() {
  G4VisAttributes va;

  auto gcf = GlobalConfigs::instance();
  G4Material *vac = G4Material::GetMaterial("vacuum");
  // const G4double worldSize = 100 * km;
  auto *worldBox = new G4Box("World", gcf.configOption<double>("world-x") * mm,
                             gcf.configOption<double>("world-y") * mm,
                             gcf.configOption<double>("world-z") * mm);

  worldLogVol = new G4LogicalVolume(worldBox, vac, "World");
  va.SetColor(1, 1, 1, 0.05);
  va.SetVisibility(true);
  worldLogVol->SetVisAttributes(va);

  return new G4PVPlacement(nullptr, G4ThreeVector(), worldLogVol, "World",
                           nullptr, false, 0, checkOverlaps);
}

void DetectorConstruction::importSolids() {
  auto meta = json::parse(std::ifstream{meta_fn});

  for (auto &[key_, mdat] : meta.items()) {
    G4VSolid *solid{nullptr};

    // Specialize if we need to import a primitive type
    if (mdat.contains("primitive_type")) {
      solid = importPrimitive(key_, mdat);
    } else if (mdat.contains("file")) {
      auto fn = mdat["file"].get<std::string>();
      if (fn.find(".stl") == std::string::npos) {
        throw std::runtime_error("Only supports .stl files");
      }
      auto mesh = CADMesh::TessellatedMesh::FromSTL(fn);
      // scale from whatever unit in file to mm (Geant native units)
      mesh->SetScale(mdat["scale"].get<double>());

      for (auto s : mesh->GetSolids()) {
        if (s->GetName() == key_) {
          solid = s;
          break;
        }
      }
    }
    if (solid == nullptr) {
      throw std::runtime_error{
          "G4VSolid could not be isolated from imported mesh or primitive."};
    }

    configureSolid(key_, solid, mdat);
  }
}

/*
Import a primitive solid from some given metadata.
Currently only supports:
  - G4Box
*/
G4VSolid *DetectorConstruction::importPrimitive(const std::string &name,
                                                const json &meta) {
  auto type = meta["primitive_type"].get<std::string>();
  if (type == "box") {
    return new G4Box(name + "-generated-box", meta["halfx"].get<double>(),
                     meta["halfy"].get<double>(), meta["halfz"].get<double>());
  }

  // Failed all of the if statements
  throw std::runtime_error{type + " is not a supported primitive to import"};
}

void DetectorConstruction::configureSolid(const std::string &name,
                                          G4VSolid *solid, const json &mdat) {
  // for generating random colors
  static std::default_random_engine en{std::random_device{}()};
  static std::uniform_real_distribution<double> dist(0, 1);

  std::vector<double> ea = mdat["euler_rotation"];
  auto rotMat = new G4RotationMatrix;
  // Set phi, theta, psi all at once
  rotMat->set(ea[2], ea[1], ea[0]);

  auto mat = mdat["material"].get<std::string>();
  auto *material = G4Material::GetMaterial(mat);
  if (material == nullptr) {
    auto *nm = G4NistManager::Instance();
    material = nm->FindOrBuildMaterial(mat);
    if (material == nullptr) {
      material = buildMaterialFromJson(mat, mdat[mat]);
    }
    if (material == nullptr) {
      throw std::runtime_error{"Cannot find material " + mat};
    }
  }
  auto *lv = new G4LogicalVolume(solid, material, name);

  // Give the shape a random color to differentiate it
  G4VisAttributes va;
  if (!mdat.contains("color")) {
    va.SetColor(dist(en), dist(en), dist(en), 0.8);
  } else {
    auto col = mdat["color"].get<std::vector<double>>();
    va.SetColor(col[0], col[1], col[2], col[3]);
  }
  lv->SetVisAttributes(va);

  std::vector<double> v = mdat["translation"];
  G4ThreeVector translate(v[0], v[1], v[2]);
  (void)new G4PVPlacement(rotMat, translate, lv, name, worldLogVol, false,
                          0, checkOverlaps);

  // Some things need the physical volumes placed before being configured
  configureVolume(lv, mdat);
}

void DetectorConstruction::configureVolume(G4LogicalVolume *lv,
                                           const json &met) {

  auto type = met["type"].get<std::string>();
  if (type == "optical_detector") {
    auto surf = siOpticalSurface();
    (void)new G4LogicalSkinSurface("si_det_skin", lv, surf);
    siLogVols.push_back(lv);
  } else if (type == "specular_reflector") {
    attachEsrOpticalSurface(lv);
  } else if (type == "lambertian_reflector") {
    attachLambertianOpticalSurface(lv);
  } else if (type == "scintillator") {
    crLogVols.push_back(lv);
  } else if (type == "roughener") {
    configureRoughener(lv, met["other_volume"].get<std::string>());
  } else if (type == "perfect_detector") {
    perfectLogVols.push_back(lv);
  } else if (type != "passive") {
    throw std::runtime_error{"Unknown geometry type: " + type};
  }
  /*
   In terms of optics, nothing else needs to happen here.
   Geant will assume that the Fresnel equations hold at optical interfaces
      without an explicit skin or boundary defined.

   In the future if we want to define more complicated geometries,
      the metadata might need to contain information on what pieces
      are in contact with other pieces so that we can define
      a G4LogicalBorderSurface between them, if we need to have that.

   For most situations--simple roughening, or polishing,
      and readout from a polished face--we can get away
      with the Fresnel equation behavior.
  */
}

void DetectorConstruction::configureRoughener(G4LogicalVolume *lv,
                                              const std::string &other) {
  // Attach a "roughening" surface between all crystals

  // Get the physical volume placements from the "store"
  auto name = lv->GetName();
  auto pvs = G4PhysicalVolumeStore::GetInstance();
  auto thisPlacement = pvs->GetVolume(name);
  auto otherPlacement = pvs->GetVolume(other);

  // First, define the optical surface to use a rough LUT model
  auto surface = new G4OpticalSurface(name + "-optical-surf");
  surface->SetModel(DAVIS);
  surface->SetFinish(Rough_LUT);
  surface->SetType(dielectric_LUTDAVIS);

  // Need to define interaction type into and out of scintillator
  (void)new G4LogicalBorderSurface(name + "-into-crystal", thisPlacement,
                                   otherPlacement, surface);
  (void)new G4LogicalBorderSurface(name + "-outof-crystal", otherPlacement,
                                   thisPlacement, surface);
}

void DetectorConstruction::ConstructSDandField() {
  auto *sd = new SiSensitiveDetector("si_det");
  G4SDManager::GetSDMpointer()->AddNewDetector(sd);
  for (auto lv : siLogVols) {
    lv->SetSensitiveDetector(sd);
  }

  auto *cd = new CrystalSensitiveDetector("crdet");
  G4SDManager::GetSDMpointer()->AddNewDetector(cd);
  for (auto lv : crLogVols) {
    lv->SetSensitiveDetector(cd);
  }

  auto *pd = new PerfectSensitiveDetector("im literally perfect");
  G4SDManager::GetSDMpointer()->AddNewDetector(pd);
  for (auto lv : perfectLogVols) {
    lv->SetSensitiveDetector(pd);
  }
}

static G4OpticalSurface *siOpticalSurface() {
  static G4ThreadLocal G4OpticalSurface *ss = nullptr;
  if (ss)
    return ss;
  ss = new G4OpticalSurface("si_surf");
  ss->SetMaterialPropertiesTable(G4NistManager::Instance()
                                     ->FindOrBuildMaterial("G4_Si")
                                     ->GetMaterialPropertiesTable());
  ss->SetModel(unified);
  ss->SetFinish(polished);
  ss->SetType(dielectric_dielectric);
  return ss;
}

void attachLambertianOpticalSurface(G4LogicalVolume *lv) {
  G4ThreadLocal static G4OpticalSurface *surf = nullptr;
  if (surf == nullptr) {
    surf = new G4OpticalSurface("lambertian-optical-surface");
    surf->SetModel(unified);
    surf->SetType(dielectric_dielectric);
    surf->SetFinish(groundfrontpainted);
    surf->SetSigmaAlpha(0.);
    surf->SetMaterialPropertiesTable(
        G4NistManager::Instance()
            ->FindOrBuildMaterial(Materials::kNIST_TEFLON)
            ->GetMaterialPropertiesTable());
  }

  (void)new G4LogicalSkinSurface("lambertian-skin-surface", lv, surf);
}

void attachEsrOpticalSurface(G4LogicalVolume *lv) {
  G4ThreadLocal static G4OpticalSurface *surf = nullptr;
  if (surf == nullptr) {
    surf = new G4OpticalSurface("specular-optical-surface");
    surf->SetModel(unified);
    surf->SetType(dielectric_metal);
    surf->SetFinish(ground);
    auto sigmaAlpha = GlobalConfigs::instance().configOption<double>(
        "specular-sigma-alpha-deg");
    surf->SetSigmaAlpha(sigmaAlpha * deg);

    // Values here have been tuned to match ESR experiments
    // with LYSO crystals
    const std::unordered_map<const char *, const std::vector<G4double>> props =
        {{"TRANSMITTANCE", {0, 0}},
         {"EFFICIENCY", {0, 0}},
         // With small sigma_alpha (as is case of ESR),
         // this gives close to experiment;
         // see doi 10.1109/TNS.2008.2001408
         {"SPECULARLOBECONSTANT", {0.90, 0.90}},
         {"SPECULARSPIKECONSTANT", {0.1, 0.1}},
         {"BACKSCATTERCONSTANT", {0, 0}},
         {"REFLECTIVITY", {0.99, 0.99}}};

    // Apply across whole optical photon range
    auto *pt = new G4MaterialPropertiesTable();
    std::vector<G4double> energies = {1e-3 * eV, 6 * eV};
    for (const auto &[name, vals] : props) {
      pt->AddProperty(name, energies, vals);
    }
    // UV-enhanced ESR
    const auto reflectivities = std::vector<double>{0.99, 0.99};

    surf->SetMaterialPropertiesTable(pt);
  }

  // Attach the surface to the logical volume
  (void)new G4LogicalSkinSurface("specular-skin-surface", lv, surf);
}

/**
Given a material definition in JSON format, construct a G4Material.

The format is like this:

    "material": "material name",
    "material name": {
        "density": <density as double, g/cm3>,
        "components": {
            "elt1": <mass frac as double>,
            "elt2": <mass frac as double>,
            "elt3": <mass frac as double>,
            ...
        }
    }
*/
G4Material *buildMaterialFromJson(std::string const &name, json const &meta) {
  auto *mat = new G4Material(name, meta["density"].get<double>() * g / cm3,
                             meta["components"].size());
  for (auto const &[element, fraction] : meta["components"].items()) {
    auto frac = fraction.get<double>();
    auto elt = G4NistManager::Instance()->FindOrBuildElement(element);
    mat->AddElementByMassFraction(elt, frac);
  }
  return mat;
}