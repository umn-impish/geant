#pragma once

#include <G4VUserDetectorConstruction.hh>
#include <G4PVPlacement.hh>
#include <G4VPhysicalVolume.hh>

#include <nlohmann/json.hpp>
#define USE_CADMESH_TETGEN
#include <CADMesh.hh>
using json = nlohmann::json;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction() =delete;

    DetectorConstruction(std::string);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    std::string meta_fn;
    std::vector<G4LogicalVolume*> siLogVols;
    std::vector<G4LogicalVolume*> crLogVols;
    G4LogicalVolume* worldLogVol;
    G4PVPlacement* makeWorld();
    void importSolids();
    void configureVolume(G4LogicalVolume*, const json&);
    // void importStl(const json&);
    // void importObj(const json&);

    void importMesh(
        const std::string& name,
        std::shared_ptr<CADMesh::TessellatedMesh> mesh,
        const json& mdat
    );
};
