#pragma once

#include <G4VUserDetectorConstruction.hh>
#include <G4PVPlacement.hh>
#include <G4VPhysicalVolume.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct ConstructionMeta {
    std::string file;
    std::string material;
    std::string type;
    double scale;
    std::vector<double> translation;
    std::vector<double> euler_rotation;
};

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction() =delete;

    DetectorConstruction(std::vector<ConstructionMeta> meta);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    std::vector<ConstructionMeta> meta;
    std::vector<G4LogicalVolume*> siLogVols;
    G4LogicalVolume* worldLogVol;
    G4PVPlacement* makeWorld();
    void importSolids();
    void configureVolume(G4LogicalVolume*, const ConstructionMeta&);
    void singleImport(const ConstructionMeta&);
};
