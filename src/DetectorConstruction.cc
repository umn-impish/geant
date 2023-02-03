#include <unordered_map>

#include "DetectorConstruction.hh"
#include "CrystalSensitiveDetector.hh"
#include "SiSensitiveDetector.hh"
#include <GlobalConfigs.hh>

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4Orb.hh"
#include "G4OpticalSurface.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include <materials/Materials.hh>

namespace ImpressForGrips
{
static G4OpticalSurface* opticalDetectorSurface();

// anonymous namespace so other files can't access these
namespace {
  G4bool checkOverlaps = true;
  void attachLambertianOpticalSurface(G4LogicalVolume* lv)
  {
    G4ThreadLocal static G4OpticalSurface* surf = nullptr;
    if (surf == nullptr) {
      surf = new G4OpticalSurface("lambertian-optical-surface");
      surf->SetModel(unified);
      surf->SetType(dielectric_dielectric);
      surf->SetFinish(groundfrontpainted);
      surf->SetSigmaAlpha(0.);
      surf->SetMaterialPropertiesTable(
          G4NistManager::Instance()->
          FindOrBuildMaterial(Materials::kNIST_TEFLON)->
          GetMaterialPropertiesTable());
    }

    (void) new G4LogicalSkinSurface("lambertian-skin-surface", lv, surf);
  }

  void attachSpecularOpticalSurface(G4LogicalVolume* lv)
  {
    G4ThreadLocal static G4OpticalSurface* surf = nullptr;
    if (surf == nullptr) {
      surf = new G4OpticalSurface("specular-optical-surface");
      surf->SetModel(unified);
      surf->SetType(dielectric_metal);
      surf->SetFinish(polished);
      surf->SetSigmaAlpha(0.);
      auto* pt = new G4MaterialPropertiesTable();

      const std::unordered_map<
        const char*,
        const std::vector<G4double>
      > props = {
          // want everything to reflect off
          {"REFLECTIVITY", {1, 1}},
          {"TRANSMITTANCE", {0, 0}},
          {"EFFICIENCY", {0, 0}},
          // dielectric_metal only uses spike reflection
          {"SPECULARSPIKECONSTANT", {1, 1}},
          {"SPECULARLOBECONSTANT", {0, 0}},
          {"BACKSCATTERCONSTANT", {0, 0}},
      };

      const std::vector<G4double> energies = {1e-3*eV, 6*eV};
      for (const auto& [name, vals] : props) {
        pt->AddProperty(name, energies, vals);
      }

      surf->SetMaterialPropertiesTable(pt);
    }

    (void) new G4LogicalSkinSurface("specular-skin-surface", lv, surf);
  }
}

DetectorConstruction::DetectorConstruction() :
  G4VUserDetectorConstruction(),
  worldPlacement(nullptr),
  worldLogVol(nullptr)
{
  Materials::makeMaterials();
}

DetectorConstruction::~DetectorConstruction()
{ }

G4VPhysicalVolume* DetectorConstruction::Construct()
{  
  makeWorld();
  makeScintillator();
  finishScintillatorSides();
  makeReflector();
  makeLightguide();
  makeOpticalDetector();
  
  return worldPlacement;
}

void DetectorConstruction::ConstructSDandField()
{
  attachScintillatorSensitiveDetector();
  attachOpticalSensitiveDetector();
}

void DetectorConstruction::makeWorld()
{
  G4VisAttributes va;

  G4Material* vac = G4Material::GetMaterial(Materials::kVACUUM);
  const G4double worldSize = 10*cm;
  G4Box* worldBox =    
    new G4Box("World",
       0.5*worldSize, 0.5*worldSize, 0.5*worldSize);
      
  worldLogVol = new G4LogicalVolume(
      worldBox, vac, "world-lv");
  va.SetColor(1, 1, 1, 0.05);
  va.SetVisibility(true);
  worldLogVol->SetVisAttributes(va);

  worldPlacement = 
    new G4PVPlacement(
      nullptr, G4ThreeVector(), worldLogVol,
      "world-placement", nullptr, false, 0, checkOverlaps);
}

void DetectorConstruction::makeOpticalDetector()
{
  G4VisAttributes va;
  auto* si = G4NistManager::Instance()->FindOrBuildMaterial(Materials::kNIST_SI);
  auto* siBox = new G4Box(
      "optical-det-box", SI_SIDE/2, SI_SIDE/2, SI_THICK/2);

  opticalDetectorLogVol = new G4LogicalVolume(
      siBox, si, "optical-det-log");
  va.SetColor(0, 0, 1, 0.5);
  va.SetVisibility(true);
  opticalDetectorLogVol->SetVisAttributes(va);

  G4ThreeVector translate(0, 0, CRYST_SIZE/2 + SI_THICK/2);
  (void) new G4PVPlacement(
    nullptr, translate, opticalDetectorLogVol,
    "optical-det-phys", worldLogVol, false, 0, checkOverlaps);

    auto* surf = opticalDetectorSurface();
    (void) new G4LogicalSkinSurface(
      "optical-det-skin", opticalDetectorLogVol, surf);
}

void DetectorConstruction::makeScintillator()
{
  G4VisAttributes va;
  const auto& gc = GlobalConfigs::instance();
  const auto& choice = gc.configOption<std::string>(
      GlobalConfigs::kSCINTILLATOR_MATERIAL);

  const auto converted = Materials::selectScintillator(choice);
  G4cout << "selected material is " << converted << G4endl;
  auto* scintMat = G4Material::GetMaterial(converted);

  const auto x = gc.configOption<double>(GlobalConfigs::kSCINTILLATOR_WIDTH);
  const auto y = gc.configOption<double>(GlobalConfigs::kSCINTILLATOR_LENGTH);
  const auto z = gc.configOption<double>(GlobalConfigs::kSCINTILLATOR_DEPTH);
  auto* scintBox = new G4Box("scint-box", x/2, y/2, z/2);

  scintLogVol = new G4LogicalVolume(scintBox, scintMat, "scint-log");
  va.SetColor(0.35, 0.5, 0.92, 0.8);
  va.SetVisibility(true);
  scintLogVol->SetVisAttributes(va);

  scintPlacement = new G4PVPlacement(
    nullptr, G4ThreeVector(), scintLogVol,
    "scint-placement", worldLogVol, false, 0, checkOverlaps);
}

void DetectorConstruction::makeReflector()
{
  // TODO: add
}

void DetectorConstruction::makeTeflon()
{
  G4VisAttributes va;

  const G4double precutHalfThick = (AIR_GAP + TEF_THICK + CRYST_SIZE)/2;

  auto* ptfe = G4NistManager::Instance()->FindOrBuildMaterial(Materials::kNIST_TEFLON);
  auto* preCutPtfeBox = new G4Box(
    "precut_ptfe", precutHalfThick, precutHalfThick, precutHalfThick);

  auto addAirGapHalfXyz = 0.5 * (CRYST_SIZE + AIR_GAP);
  auto* crystPlusAirGap = new G4Box(
    "", addAirGapHalfXyz, addAirGapHalfXyz, addAirGapHalfXyz);
  G4ThreeVector translateSubtract(0, 0, 0);
  auto* slicedPtfe = new G4SubtractionSolid(
    "sliced_ptfe", preCutPtfeBox, crystPlusAirGap, nullptr, translateSubtract);

  translateSubtract = G4ThreeVector(0, 0, 1.8 * cm);
  auto* cutCap = new G4Box("", precutHalfThick + 5*mm, precutHalfThick + 5*mm, precutHalfThick);
  auto* slicedPtfeOpen = new G4SubtractionSolid(
    "sliced_ptfe_open", slicedPtfe, cutCap, nullptr, translateSubtract);

  auto* ptfeLogVol = new G4LogicalVolume(
    slicedPtfeOpen, ptfe, "ptfe_log");
  va.SetColor(0, 1, 0, 0.1);
  va.SetVisibility(true);
  ptfeLogVol->SetVisAttributes(va);
  attachLambertianOpticalSurface(ptfeLogVol);

  (void) new G4PVPlacement(
    nullptr, G4ThreeVector(), ptfeLogVol,
    "ptfe_phys", worldLogVol, false, 0, checkOverlaps);
}

void DetectorConstruction::makeEsr()
{
  // TODO: add
}


void DetectorConstruction::attachScintillatorSensitiveDetector()
{
  auto* sd = new CrystalSensitiveDetector("crystal-sens-det");
  scintSensDet.Put(sd);

  G4SDManager::GetSDMpointer()->AddNewDetector(sd);
  scintLogVol->SetSensitiveDetector(sd);
}

void DetectorConstruction::attachOpticalSensitiveDetector()
{
  auto* sd = new SiSensitiveDetector("optical-sens-det");
  opticalSensDet.Put(sd);

  G4SDManager::GetSDMpointer()->AddNewDetector(sd);
  opticalDetectorLogVol->SetSensitiveDetector(sd);
}

void DetectorConstruction::finishScintillatorSides()
{
  auto* surf = new G4OpticalSurface("");
  // See G4 documentation on UNIFIED model
  surf->SetType(dielectric_LUTDAVIS);
  surf->SetModel(DAVIS);
  surf->SetFinish(Rough_LUT);
  (void) new G4LogicalBorderSurface(
    "scint-logical-border-surf", 
    scintPlacement, worldPlacement, surf);
}

void DetectorConstruction::makeLightguide()
{
  // TODO: add
}

static G4OpticalSurface* opticalDetectorSurface()
{
    static G4ThreadLocal G4OpticalSurface* ss = nullptr;
    if (ss) return ss;
    ss = new G4OpticalSurface("optical-det-surf");
    ss->SetMaterialPropertiesTable(
        G4NistManager::Instance()
        ->FindOrBuildMaterial(Materials::kNIST_SI)
        ->GetMaterialPropertiesTable());
    ss->SetModel(unified);
    ss->SetFinish(polished);
    ss->SetType(dielectric_dielectric);
    return ss;
}

}
