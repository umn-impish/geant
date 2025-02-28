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
    const G4double worldSize = 50*cm;
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
            importMesh(key_ + fn, mesh, mdat);
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
            solid, material, name + "_logical"
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

        configureVolume(lv, mdat);

        (void) new G4PVPlacement(
            rotMat, G4ThreeVector(), lv,
            name + "_placement", worldLogVol,
            false, 0, checkOverlaps
        );
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
        surf->SetSigmaAlpha(0.);
        auto* pt = new G4MaterialPropertiesTable();

        const std::unordered_map<
            const char*,
            const std::vector<G4double>
        > props = {
            {"TRANSMITTANCE", {0, 0}},
            {"EFFICIENCY", {0, 0}},
            {"SPECULARSPIKECONSTANT", {0, 0}},
            {"SPECULARLOBECONSTANT", {0, 0}},
            {"BACKSCATTERCONSTANT", {0, 0}},
        };

        // Apply across whole optical photon range
        std::vector<G4double> energies = {1e-3*eV, 6*eV};
        for (const auto& [name, vals] : props) {
            pt->AddProperty(name, energies, vals);
        }

        // The reflectivity of ESR is more complicated
        const auto refl_energies = std::vector<double>{
            0.1*eV, 8*eV
            /* 1.04*eV, 1.04*eV, 1.05*eV, 1.06*eV, 1.07*eV, 1.07*eV, 1.08*eV, 1.08*eV, 1.09*eV, 1.10*eV, 
            1.11*eV, 1.11*eV, 1.12*eV, 1.13*eV, 1.13*eV, 1.14*eV, 1.15*eV, 1.16*eV, 1.16*eV, 1.17*eV, 
            1.18*eV, 1.19*eV, 1.20*eV, 1.20*eV, 1.21*eV, 1.21*eV, 1.22*eV, 1.22*eV, 1.23*eV, 1.23*eV, 
            1.24*eV, 1.24*eV, 1.25*eV, 1.25*eV, 1.25*eV, 1.26*eV, 1.27*eV, 1.27*eV, 1.28*eV, 1.29*eV, 
            1.29*eV, 1.30*eV, 1.31*eV, 1.31*eV, 1.32*eV, 1.33*eV, 1.34*eV, 1.36*eV, 1.38*eV, 1.40*eV, 
            1.41*eV, 1.42*eV, 1.43*eV, 1.45*eV, 1.45*eV, 1.47*eV, 1.48*eV, 1.50*eV, 1.51*eV, 1.52*eV, 
            1.53*eV, 1.55*eV, 1.57*eV, 1.58*eV, 1.60*eV, 1.61*eV, 1.62*eV, 1.64*eV, 1.65*eV, 1.66*eV, 
            1.67*eV, 1.68*eV, 1.69*eV, 1.70*eV, 1.71*eV, 1.72*eV, 1.75*eV, 1.76*eV, 1.78*eV, 1.80*eV, 
            1.82*eV, 1.84*eV, 1.85*eV, 1.88*eV, 1.90*eV, 1.92*eV, 1.93*eV, 1.96*eV, 1.98*eV, 2.00*eV, 
            2.03*eV, 2.06*eV, 2.09*eV, 2.11*eV, 2.13*eV, 2.17*eV, 2.19*eV, 2.23*eV, 2.26*eV, 2.29*eV, 
            2.33*eV, 2.37*eV, 2.41*eV, 2.44*eV, 2.48*eV, 2.52*eV, 2.55*eV, 2.59*eV, 2.64*eV, 2.69*eV, 
            2.73*eV, 2.77*eV, 2.80*eV, 2.84*eV, 2.85*eV, 2.89*eV, 2.93*eV, 2.96*eV, 3.01*eV, 3.05*eV, 
            3.09*eV, 3.13*eV, 3.16*eV, 3.17*eV, 3.19*eV, 3.20*eV, 3.23*eV, 3.25*eV, 3.27*eV, 3.28*eV, 
            3.29*eV, 3.30*eV, 3.31*eV, 3.33*eV, 3.34*eV, 3.36*eV, 3.40*eV, 3.45*eV, 3.50*eV, 3.53*eV */
        };
        const auto reflectivities = std::vector<double>{
            0.92, 0.92
            /* 0.17, 0.17, 0.17, 0.18, 0.17, 0.17, 0.17, 0.17, 0.17, 0.18, 
            0.18, 0.18, 0.17, 0.16, 0.16, 0.17, 0.18, 0.18, 0.18, 0.18, 
            0.18, 0.18, 0.18, 0.19, 0.20, 0.21, 0.23, 0.25, 0.29, 0.34, 
            0.40, 0.47, 0.52, 0.58, 0.62, 0.67, 0.74, 0.81, 0.87, 0.91, 
            0.94, 0.96, 0.97, 0.98, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 
            0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 
            0.99, 0.99, 0.99, 0.98, 0.97, 0.96, 0.96, 0.95, 0.95, 0.95, 
            0.95, 0.95, 0.95, 0.96, 0.97, 0.98, 0.98, 0.98, 0.98, 0.98, 
            0.99, 0.99, 0.98, 0.98, 0.98, 0.99, 0.98, 0.98, 0.98, 0.98, 
            0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 
            0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.97, 
            0.97, 0.97, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.97, 0.97, 
            0.97, 0.96, 0.94, 0.92, 0.89, 0.86, 0.74, 0.60, 0.41, 0.35, 
            0.28, 0.23, 0.18, 0.15, 0.14, 0.13, 0.13, 0.13, 0.12, 0.12 */
        };

        pt->AddProperty("REFLECTIVITY", refl_energies, reflectivities);

        surf->SetMaterialPropertiesTable(pt);
    }

    // Attach the surface to the logical volume
    (void) new G4LogicalSkinSurface("specular-skin-surface", lv, surf);
}
