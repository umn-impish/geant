#include <functional>
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
      surf->SetFinish(ground);
      surf->SetSigmaAlpha(0.);
      auto* pt = new G4MaterialPropertiesTable();

      // TODO: maybe make this customizable?
      // this is equivalent to the Lambertian scattering probability in this situation
      const double DEFECT_PROB = 0.05;
      const std::unordered_map<
        const char*,
        const std::vector<G4double>
      > props = {
          // want everything to reflect off
          {"REFLECTIVITY", {1, 1}},
          {"TRANSMITTANCE", {0, 0}},
          {"EFFICIENCY", {0, 0}},
          {"SPECULARSPIKECONSTANT", {1 - DEFECT_PROB, 1 - DEFECT_PROB}},
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

  std::function<void(G4LogicalVolume*)>
  selectSurfaceAttachmentFunction(const std::string& choice)
  {
    using AttachFunc = std::function<void(G4LogicalVolume*)>;
    const std::unordered_map<std::string, AttachFunc>
    ATTACH_SURFACE_OPTIONS = {
        {"teflon", attachLambertianOpticalSurface},
        {"esr", attachSpecularOpticalSurface}
    };
    return ATTACH_SURFACE_OPTIONS.at(choice);
  }
}

DetectorConstruction::DetectorConstruction() :
  G4VUserDetectorConstruction(),
  worldPlacement(nullptr),
  worldLogVol(nullptr),
  scintBox(nullptr),
  scintPlacement(nullptr),
  scintLogVol(nullptr),
  opticalDetectorLogVols(),
  scintSensDet(),
  opticalSensDet()
{
  Materials::makeMaterials();
  verifyReflectorStatus();
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
  const G4double worldSize = 10 * cm;
  G4Box* worldBox =    
    new G4Box(
      "world", 0.5*worldSize, 0.5*worldSize, 0.5*worldSize);
      
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

void DetectorConstruction::makeScintillator()
{
  G4VisAttributes va;
  const auto& gc = GlobalConfigs::instance();
  const auto& choice = gc.configOption<std::string>(
      GlobalConfigs::kSCINTILLATOR_MATERIAL);

  const auto converted = Materials::selectScintillator(choice);
  auto* scintMat = G4Material::GetMaterial(converted);

  const auto dx = gc.configOption<double>(GlobalConfigs::kSCINTILLATOR_DX) * mm;
  const auto dy = gc.configOption<double>(GlobalConfigs::kSCINTILLATOR_DY) * mm;
  const auto dz = gc.configOption<double>(GlobalConfigs::kSCINTILLATOR_DZ) * mm;
  scintBox = new G4Box("scint-box", dx/2, dy/2, dz/2);

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
  using GC = GlobalConfigs;
  const auto& gc = GC::instance();
  if (!gc.configOption<bool>(GC::kBUILD_SCINTILLATOR_CLADDING))
    return;

  const auto airGap = gc.configOption<double>(
      GC::kSCINTILLATOR_CLADDING_AIR_GAP_THICKNESS) * micrometer;
  
  const double halfThick = (gc.configOption<double>(GC::kREFLECTOR_THICKNESS) * millimeter) / 2;
  const auto extra = airGap + halfThick;
  const G4double hx = scintBox->GetXHalfLength() + extra,
                 hy = scintBox->GetYHalfLength() + extra,
                 hz = scintBox->GetZHalfLength() + extra;
  auto* preCut = new G4Box("", hx, hy, hz);

  auto* sliceOut = new G4Box(
      "", hx - halfThick, hy - halfThick, hz - halfThick);
  G4ThreeVector translate(0, 0, 0);
  auto* slicedWithCap = new G4SubtractionSolid(
      "", 
      preCut, sliceOut, nullptr, translate);
  // pad the slice so we don't have like a 1-atom thick remainder
  const G4double slicePad = 20*mm;
  auto* chopCap = new G4Box(
      "", hx + slicePad, hy + slicePad, extra);
  translate = G4ThreeVector(0, 0, hz);
  auto* sliced = new G4SubtractionSolid(
      "sliced-reflector",
      slicedWithCap, chopCap, nullptr, translate);

  std::string choice = gc.configOption<std::string>(GC::kSCINTILLATOR_CLADDING_TYPE);
  G4Material* refMat = Materials::selectReflectorMaterial(choice);
  auto* reflectorLogVol = new G4LogicalVolume(sliced, refMat, "reflector-log");
  G4VisAttributes va;
  va.SetColor(0, 1, 0, 0.1);
  va.SetVisibility(true);
  reflectorLogVol->SetVisAttributes(va);
  auto f = selectSurfaceAttachmentFunction(choice);
  f(reflectorLogVol);
   
  (void) new G4PVPlacement(
    nullptr, G4ThreeVector(), reflectorLogVol,
    "reflector-phys", worldLogVol, false, 0, checkOverlaps);
}

void DetectorConstruction::attachScintillatorSensitiveDetector()
{
  if (scintLogVol == nullptr) {
    G4Exception(
      "DetectorConstruction",
      "",
      JustWarning,
      "Scintillator logical volume is null");
    return;
  }
  auto* sd = new CrystalSensitiveDetector("crystal-sens-det");
  scintSensDet.Put(sd);

  G4SDManager::GetSDMpointer()->AddNewDetector(sd);
  scintLogVol->SetSensitiveDetector(sd);
}

void DetectorConstruction::attachOpticalSensitiveDetector()
{
  if (opticalDetectorLogVols.size() == 0) {
    G4Exception(
      "DetectorConstruction",
      "",
      JustWarning,
      "Optical detector logical volumes are empty");
    return;
  }
  auto* sd = new SiSensitiveDetector("optical-sens-det");
  opticalSensDet.Put(sd);

  G4SDManager::GetSDMpointer()->AddNewDetector(sd);
  for (auto* lv : opticalDetectorLogVols)
    lv->SetSensitiveDetector(sd);
}

void DetectorConstruction::finishScintillatorSides()
{
  auto hx = scintBox->GetXHalfLength(),
       hy = scintBox->GetYHalfLength(),
       hz = scintBox->GetZHalfLength();
  const auto airGap = GlobalConfigs::
    instance()
    .configOption<double>(
      GlobalConfigs::kSCINTILLATOR_CLADDING_AIR_GAP_THICKNESS) * micrometer;
  const auto thickness = airGap / 2;
  auto* xyBox = new G4Box(
    "xy-finisher", hx, hy, thickness);
  auto* yzBox = new G4Box(
    "yz-finisher", thickness, hy, hz);
  auto* xzBox = new G4Box(
    "xz-finisher", hx, thickness, hz);

  using GC = GlobalConfigs;
  const auto& gc = GC::instance();
  
  using tv = G4ThreeVector;
  const std::unordered_multimap<
    std::string, std::pair<G4Box*, tv>>
  applyTheseFinishes = {
    {GC::kSCINTILLATOR_XZ_FACES_FINISH, {xzBox, tv(0, hy + thickness, 0)}},
    {GC::kSCINTILLATOR_XZ_FACES_FINISH, {xzBox, tv(0, -hy - thickness, 0)}},
    {GC::kSCINTILLATOR_YZ_FACES_FINISH, {yzBox, tv(hx + thickness, 0, 0)}},
    {GC::kSCINTILLATOR_YZ_FACES_FINISH, {yzBox, tv(-hx - thickness, 0, 0)}},
    {GC::kSCINTILLATOR_MINUS_Z_FACE_FINISH, {xyBox, tv(0, 0, -hz - thickness)}},
    // {"Polished_LUT", {xyBox, tv(0, 0, hz + thickness)}},
  };

  const std::unordered_map<std::string, G4OpticalSurfaceFinish>

  #define SURFACE_LOOKER_UPPER(srf) {#srf, srf}
  surfaceLookup = {
    SURFACE_LOOKER_UPPER(Polished_LUT),
    SURFACE_LOOKER_UPPER(Rough_LUT),
    SURFACE_LOOKER_UPPER(PolishedTeflon_LUT),
    SURFACE_LOOKER_UPPER(RoughTeflon_LUT),
    SURFACE_LOOKER_UPPER(PolishedESR_LUT),
    SURFACE_LOOKER_UPPER(RoughESR_LUT),
    SURFACE_LOOKER_UPPER(PolishedESRGrease_LUT),
    SURFACE_LOOKER_UPPER(RoughESRGrease_LUT),
  };

  auto* vac = G4Material::GetMaterial(Materials::kVACUUM);
  G4VisAttributes va;
  va.SetColor(1, 0, 0, 0.8);
  va.SetVisibility(false);

  std::size_t finishNum = 0;
  for (const auto& [finishKey, dataPair] : applyTheseFinishes) {
    std::string finishStr;
    try { finishStr = gc.configOption<std::string>(finishKey); }
    catch (const std::out_of_range& e) { finishStr = finishKey; }

    G4OpticalSurfaceFinish finish;
    try { finish = surfaceLookup.at(finishStr); }
    catch (const std::out_of_range& e) {
      G4cerr << finishStr << " is not a valid finish type." << G4endl;
      throw;
    }
    
    auto [box, translate] = dataPair;
    auto partialName = box->GetName() + '-' + std::to_string(finishNum++);

    auto* lv = new G4LogicalVolume(box, vac, partialName + "-lv");
    lv->SetVisAttributes(va);

    auto* pv = new G4PVPlacement(
        nullptr, translate, lv,
        partialName + "-pv", worldLogVol, false, 0, checkOverlaps);

    auto* surf = new G4OpticalSurface(partialName + "-optical-surf");
    surf->SetModel(DAVIS);
    surf->SetFinish(finish);
    surf->SetType(dielectric_LUTDAVIS);
    // NB: do both orders because it matters
    (void) new G4LogicalBorderSurface(partialName + "-border-surf-out-in", pv, scintPlacement, surf);
    (void) new G4LogicalBorderSurface(partialName + "-border-surf-in-out", scintPlacement, pv, surf);
  }
}

void DetectorConstruction::makeLightguide()
{
  using GC = GlobalConfigs;
  const auto& gc = GC::instance();

  auto thick = gc.configOption<double>(GC::kLIGHT_GUIDE_THICKNESS);
  auto hy = scintBox->GetYHalfLength(), hx = scintBox->GetXHalfLength();
  lgBox = new G4Box("light-guide-box", hx, hy, thick/2);

  auto* pdms = G4Material::GetMaterial(Materials::kPDMS);
  auto* lgLv = new G4LogicalVolume(lgBox, pdms, "light-guide-lv");

  G4ThreeVector translate(0, 0, scintBox->GetZHalfLength() + thick/2);
  G4VisAttributes va;
  va.SetColor(1, 1, 0, 0.5);
  va.SetVisibility(true);
  lgLv->SetVisAttributes(va);
  (void) new G4PVPlacement(
      nullptr, translate, lgLv, "light-guide-place",
      worldLogVol, false, 0, checkOverlaps
  );
}

void DetectorConstruction::makeOpticalDetector()
{
  using GC = GlobalConfigs;
  const auto& gc = GC::instance();
  
  auto numRows = gc.configOption<int>(GC::kNUM_SIPM_ROWS);
  auto numPerRow = gc.configOption<int>(GC::kSIPMS_PER_ROW);
  auto side = gc.configOption<double>(GC::kSIPM_SIDE_LENGTH) * mm;
  auto gap = gc.configOption<double>(GC::kSIPM_SPACING) * mm;

  double SIPM_THICK = 1 * mm;

  double zCenter = scintBox->GetZHalfLength() + lgBox->GetZHalfLength()*2 + SIPM_THICK/2;
  double delta = side + gap;
  double xOrigin = -(numPerRow - 1) * delta / 2;
  double yOrigin = -(numRows - 1) * delta / 2;

  auto* si = G4Material::GetMaterial(Materials::kNIST_SI);
  G4OpticalSurface* surf = opticalDetectorSurface();
  G4VisAttributes va{G4Color(0, 0, 1, 0.3)};
  for (int i = 0; i < numRows; ++i) {
    for (int j = 0; j < numPerRow; ++j) {
      G4ThreeVector center(xOrigin + j*delta, yOrigin + i*delta, zCenter);

      std::string baseName = "sipm" + std::to_string(i) + std::to_string(j);
      auto* box = new G4Box(
        baseName + "-box",
        side/2, side/2, SIPM_THICK/2);
      auto* lv = new G4LogicalVolume(
        box, si,
        baseName + "-lv");
      lv->SetVisAttributes(va);
      (void) new G4PVPlacement(
        nullptr, center, lv, baseName + "-place",
        worldLogVol, false, 0, checkOverlaps
      );

      // for later. . .
      opticalDetectorLogVols.push_back(lv);
      (void) new G4LogicalSkinSurface(
        baseName + "-skin", lv, surf);
    }
  }
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

void verifyReflectorStatus()
{
  using GC = GlobalConfigs;
  const auto& gc = GC::instance();

  auto useReflector = gc.configOption<bool>(
      GC::kBUILD_SCINTILLATOR_CLADDING);
  if (!useReflector) return;

  const std::vector<std::string> facesToCheck = {
    GC::kSCINTILLATOR_XZ_FACES_FINISH,
    GC::kSCINTILLATOR_YZ_FACES_FINISH,
    GC::kSCINTILLATOR_MINUS_Z_FACE_FINISH
  };

  for (const std::string& face : facesToCheck) {
    auto finish = gc.configOption<std::string>(face);
    bool uhOh = (
      finish.find("Teflon") != std::string::npos || 
      finish.find("ESR") != std::string::npos
    );
    if (uhOh) {
      throw std::runtime_error(
        "Do not use a Teflon or ESR LUT finish on the scintillator "
        "when you are also building the scintillator cladding.");
    }
  }
}

}
