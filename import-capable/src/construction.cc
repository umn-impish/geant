#include <random>

#include <configs.hh>
#include <construction.hh>
#include <detectors.hh>
#include <materials.hh>

#include <G4Box.hh>
#include <G4Exception.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4PVPlacement.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4RotationMatrix.hh>
#include <G4SDManager.hh>
#include <G4VisAttributes.hh>
#include <CLHEP/Vector/EulerAngles.h>

static constexpr bool checkOverlaps = true;
static G4OpticalSurface* siOpticalSurface();
void attachEsrOpticalSurface(G4LogicalVolume* lv);
void attachLambertianOpticalSurface(G4LogicalVolume* lv);

DetectorConstruction::DetectorConstruction(std::string meta_fn) :
  G4VUserDetectorConstruction(),

    // We pass in the metadata file name which contains a
    // JSON array.
    // For some reason, json::parse parses the array to 
    // a nested array(?) so we only want the first element.
    meta_fn{meta_fn},
    siLogVols{},
    crLogVols{}
{
    Materials::makeMaterials();
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
    const G4double worldSize = 200*cm;
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
    auto meta = json::parse(std::ifstream{meta_fn});

    const std::string stl = "stl", obj = "obj";
    for (auto& [key_, mdat] : meta.items()) {
        auto fn = mdat["file"].get<std::string>();
        if (fn.find(".stl") != std::string::npos) {
            auto mesh = CADMesh::TessellatedMesh::FromSTL(fn);
            importMesh(key_, mesh, mdat);
        }
        else if (fn.find(".obj") != std::string::npos) {
            // Assume the .obj file describes the 
            // entire detector geometry (all components)
            auto mesh = CADMesh::TessellatedMesh::FromOBJ(
                mdat["file"].get<std::string>());
            auto solids = mesh->GetSolids();

            for (auto s : solids) {
                auto name = s->GetName();
                std::cout << "PART NAME IS " << name << std::endl;
                auto cur_meta = mdat[name];
                importMesh(key_ + name, mesh, cur_meta);
            }
        }
        else {
            throw std::runtime_error{"Unrecognized file format: " + fn};
        }
    }
}


void DetectorConstruction::importMesh(
    const std::string& name,
    std::shared_ptr<CADMesh::TessellatedMesh> mesh,
    const json& mdat
) {
    // for generating random colors
    static std::default_random_engine en{std::random_device{}()};
    static std::uniform_real_distribution<double> dist(0, 1);

    // scale from whatever unit in file to mm (Geant native units)
    mesh->SetScale(mdat["scale"].get<double>());

    std::vector<double> v = mdat["translation"];
    G4ThreeVector translate(v[0], v[1], v[2]);
    mesh->SetOffset(translate);

    // auto solid = mesh->GetSolid();
    std::vector<G4VSolid*> solids = mesh->GetSolids();

    for (auto solid : solids) {
        if (solids.size() > 1 && solid->GetName() != name) {
            continue;
        }

        std::vector<double> ea = mdat["euler_rotation"];
        auto rotMat = new G4RotationMatrix;
        // Set phi, theta, psi all at once
        rotMat->set(ea[2], ea[1], ea[0]);

        auto mat = mdat["material"].get<std::string>();
        auto* material = G4Material::GetMaterial(mat);
        if (material == nullptr) {
            auto* nm = G4NistManager::Instance();
            material = nm->FindOrBuildMaterial(mat);
            if (material == nullptr) {
                throw std::runtime_error{"Cannot find material " + mat};
            }
        }
        auto* lv = new G4LogicalVolume(
            solid, material, name
        );

        // Give the shape a random color to differentiate it
        G4VisAttributes va;
        if (!mdat.contains("color")) {
            va.SetColor(
                dist(en),
                dist(en),
                dist(en),
                0.4
            );
        } else {
            auto col = mdat["color"].get<std::vector<double> >();
            va.SetColor(col[0], col[1], col[2], col[3]);
        }
        lv->SetVisAttributes(va);

        (void) new G4PVPlacement(
            rotMat, G4ThreeVector(), lv,
            name, worldLogVol,
            false, 0, checkOverlaps
        );

        // Some things need the physical volumes placed before being configured
        configureVolume(lv, mdat);
    }
}

void DetectorConstruction::configureVolume(G4LogicalVolume* lv, const json &met) {

    auto type = met["type"].get<std::string>();
    if (type == "optical_detector") {
        auto surf = siOpticalSurface();
        (void) new G4LogicalSkinSurface("si_det_skin", lv, surf);
        siLogVols.push_back(lv);
    }
    else if (type == "specular_reflector") {
        attachEsrOpticalSurface(lv);
    }
    else if (type == "lambertian_reflector") {
        attachLambertianOpticalSurface(lv);
    }
    else if (type == "scintillator") {
        crLogVols.push_back(lv);
    }
    else if (type == "roughener") {
        configureRoughener(lv, met["other_volume"].get<std::string>());
    }
    else if (type != "passive") {
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

void DetectorConstruction::configureRoughener(G4LogicalVolume* lv, const std::string& other) {
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
    (void) new G4LogicalBorderSurface(
        name + "-into-crystal", thisPlacement, otherPlacement, surface);
    (void) new G4LogicalBorderSurface(
        name + "-outof-crystal", otherPlacement, thisPlacement, surface);
}

void DetectorConstruction::ConstructSDandField() {
    auto* sd = new SiSensitiveDetector("si_det");
    G4SDManager::GetSDMpointer()->AddNewDetector(sd);
    for (auto lv : siLogVols) {
        lv->SetSensitiveDetector(sd);
    }

    auto* cd = new CrystalSensitiveDetector("crdet");
    G4SDManager::GetSDMpointer()->AddNewDetector(cd);
    for (auto lv : crLogVols) {
        lv->SetSensitiveDetector(cd);
    }
}


static G4OpticalSurface* siOpticalSurface() {
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

void attachLambertianOpticalSurface(G4LogicalVolume* lv) {
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

void attachEsrOpticalSurface(G4LogicalVolume* lv) {
    G4ThreadLocal static G4OpticalSurface* surf = nullptr;
    if (surf == nullptr) {
        surf = new G4OpticalSurface("specular-optical-surface");
        surf->SetModel(unified);
        surf->SetType(dielectric_metal);
        surf->SetFinish(ground);
        auto sigmaAlpha = GlobalConfigs::instance().configOption<double>("specular_sigma_alpha_deg");
        surf->SetSigmaAlpha(sigmaAlpha * deg);
        auto* pt = new G4MaterialPropertiesTable();

        // Values here have been tuned to match ESR experiments
        // with LYSO crystals
        const std::unordered_map<
            const char*,
            const std::vector<G4double>
        > props = {
            {"TRANSMITTANCE", {0, 0}},
            {"EFFICIENCY", {0, 0}},
            {"SPECULARSPIKECONSTANT", {0.05, 0.05}},
            {"SPECULARLOBECONSTANT", {0.95, 0.95}},
            {"BACKSCATTERCONSTANT", {0, 0}},
            {"REFLECTIVITY", {0.99, 0.99}}
        };

        // Apply across whole optical photon range
        std::vector<G4double> energies = {1e-3*eV, 6*eV};
        for (const auto& [name, vals] : props) {
            pt->AddProperty(name, energies, vals);
        }
        // UV-enhanced ESR
        const auto reflectivities = std::vector<double>{
            0.99, 0.99
        };

        surf->SetMaterialPropertiesTable(pt);
    }

    // Attach the surface to the logical volume
    (void) new G4LogicalSkinSurface("specular-skin-surface", lv, surf);
}
