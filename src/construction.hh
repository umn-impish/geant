#pragma once

#include <G4PVPlacement.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VSolid.hh>
#include <G4VUserDetectorConstruction.hh>

#include <nlohmann/json.hpp>
#define USE_CADMESH_TETGEN
#include <CADMesh.hh>
using json = nlohmann::json;

G4Material *buildMaterialFromJson(std::string const &, json const &);

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction() = delete;

  DetectorConstruction(std::string);
  virtual ~DetectorConstruction();

  virtual G4VPhysicalVolume *Construct() override;
  void ConstructSDandField() override;

private:
  std::string meta_fn;
  std::vector<G4LogicalVolume *> siLogVols;
  std::vector<G4LogicalVolume *> crLogVols;
  std::vector<G4LogicalVolume *> perfectLogVols;
  G4LogicalVolume *worldLogVol;
  G4PVPlacement *makeWorld();
  void importSolids();
  G4VSolid *importPrimitive(const std::string &, const json &);
  void configureVolume(G4LogicalVolume *, const json &);
  void configureRoughener(G4LogicalVolume *, const std::string &);

  void configureSolid(const std::string &name, G4VSolid *solid,
                      const json &mdat);
};
