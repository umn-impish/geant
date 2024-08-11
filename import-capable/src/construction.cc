#include <random>

#include <construction.hh>
#include <detectors.hh>
#include <materials.hh>

#include <G4Box.hh>
#include <G4Exception.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4PVPlacement.hh>
#include <G4RotationMatrix.hh>
#include <G4SDManager.hh>
#include <G4VisAttributes.hh>
#include <CLHEP/Vector/EulerAngles.h>

#define USE_CADMESH_TETGEN
#include <CADMesh.hh>

static constexpr bool checkOverlaps = true;
static G4OpticalSurface* siOpticalSurface();
void attachSpecularOpticalSurface(G4LogicalVolume* lv);

DetectorConstruction::DetectorConstruction(std::vector<ConstructionMeta> meta_) :
  G4VUserDetectorConstruction(),
  meta{meta_},
  siLogVols{}
{
    Materials::makeMaterials();

    std::cout << "size is" << meta.size() << std::endl;

    for (const auto& met : meta) {
        std::cout << met.file << std::endl;
        std::cout.flush();
    }
}

DetectorConstruction::~DetectorConstruction()
{ }

G4VPhysicalVolume*
DetectorConstruction::Construct() {  
    auto* worldPlacement = makeWorld();

    // Import 3D models via CADMesh
    importSolids();

    return worldPlacement;
}

G4PVPlacement*
DetectorConstruction::makeWorld() {
    G4VisAttributes va;

    G4Material* vac = G4Material::GetMaterial("vacuum");
    const G4double worldSize = 10*cm;
    auto* worldBox =    
    new G4Box("World",
        0.5*worldSize, 0.5*worldSize, 0.5*worldSize);
      
    worldLogVol = new G4LogicalVolume(worldBox, vac, "World");
    va.SetColor(1, 1, 1, 0.05);
    va.SetVisibility(true);
    worldLogVol->SetVisAttributes(va);

    return
    new G4PVPlacement(
        nullptr, G4ThreeVector(), worldLogVol,
        "World", nullptr, false, 0, checkOverlaps);
}

void DetectorConstruction::importSolids() {
    auto nm = G4NistManager::Instance();

    for (const auto& mdat : meta) {
        singleImport(mdat);
    }
}


void DetectorConstruction::singleImport(const ConstructionMeta& mdat) {
    static std::default_random_engine en{std::random_device{}()};
    static std::uniform_real_distribution<double> dist(0, 1);

    auto mesh = CADMesh::TessellatedMesh::FromSTL(mdat.file);
    // scale from whatever unit in file to mm (Geant native units)
    mesh->SetScale(mdat.scale);

    const auto& v = mdat.translation;
    G4ThreeVector translate(v[0], v[1], v[2]);
    mesh->SetOffset(translate);

    auto solid = mesh->GetSolid();

    const auto& ea = mdat.euler_rotation;
    auto rotMat = new G4RotationMatrix;
    rotMat->setPhi(ea[0]);
    rotMat->setTheta(ea[1]);
    rotMat->setPsi(ea[2]);

    auto* material = G4Material::GetMaterial(mdat.material);
    if (material == nullptr) {
        auto* nm = G4NistManager::Instance();
        material = nm->FindOrBuildMaterial(mdat.material);
        if (material == nullptr) {
            throw std::runtime_error{"Cannot find material " + mdat.material};
        }
    }
    auto* lv = new G4LogicalVolume(
        solid, material, mdat.file + "_logical"
    );

    // Give the shape a random color to differentiate it
    G4VisAttributes va;
    va.SetColor(
        dist(en),
        dist(en),
        dist(en),
        0.4
    );

    lv->SetVisAttributes(va);

    configureVolume(lv, mdat);

    (void) new G4PVPlacement(
        rotMat, G4ThreeVector(), lv,
        mdat.file + "_placement", worldLogVol,
        false, 0, checkOverlaps
    );
}

void DetectorConstruction::configureVolume(G4LogicalVolume* lv, const ConstructionMeta &met) {
    if (met.type == "optical_detector") {
        auto surf = siOpticalSurface();
        (void) new G4LogicalSkinSurface("si_det_skin", lv, surf);
        siLogVols.push_back(lv);
    }
    if (met.type == "specular_reflector") {
        attachSpecularOpticalSurface(lv);
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

void DetectorConstruction::ConstructSDandField() {
    auto* sd = new SiSensitiveDetector("si_det");
    G4SDManager::GetSDMpointer()->AddNewDetector(sd);
    for (auto lv : siLogVols) {
        lv->SetSensitiveDetector(sd);
    }
}


static G4OpticalSurface* siOpticalSurface()
{
    static G4ThreadLocal G4OpticalSurface* ss = nullptr;
    if (ss) return ss;
    ss = new G4OpticalSurface("si_surf");
    ss->SetMaterialPropertiesTable(
        G4NistManager::Instance()
        ->FindOrBuildMaterial("G4_Si")
        ->GetMaterialPropertiesTable());
    ss->SetModel(unified);
    ss->SetFinish(polished);
    ss->SetType(dielectric_dielectric);
    return ss;
}

void attachSpecularOpticalSurface(G4LogicalVolume* lv) {
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
        const double DEFECT_PROB = 0.02;
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

        // Apply across whole optical photon range
        const std::vector<G4double> energies = {1e-3*eV, 6*eV};
        for (const auto& [name, vals] : props) {
            pt->AddProperty(name, energies, vals);
        }

        surf->SetMaterialPropertiesTable(pt);
    }

    // Attach the surface to the logical volume
    (void) new G4LogicalSkinSurface("specular-skin-surface", lv, surf);
}