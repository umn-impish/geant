#pragma once

#include <G4Cache.hh>
#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>

class G4Box;
class G4VPhysicalVolume;
class G4LogicalVolume;

namespace ImpressForGrips
{

class CrystalSensitiveDetector;
class SiSensitiveDetector;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;
  private:
    void makeWorld();

    void makeScintillator();
    void finishScintillatorSides();
    void attachScintillatorSensitiveDetector();

    void makeReflector();
    void makeLightguide();

    void makeOpticalDetector();
    void attachOpticalSensitiveDetector();

    G4VPhysicalVolume* worldPlacement;
    G4LogicalVolume* worldLogVol;
    G4Box* scintBox;
    G4VPhysicalVolume* scintPlacement;
    G4LogicalVolume* scintLogVol;
    G4LogicalVolume* opticalDetectorLogVol;

    constexpr static G4double SI_SIDE = 2 * cm;
    constexpr static G4double SI_THICK = 0.5 * mm;

    // 4x 80 micron wrapping
    constexpr static G4double REFLECTOR_THICK = 320 * micrometer;

    G4Cache<CrystalSensitiveDetector*> scintSensDet;
    G4Cache<SiSensitiveDetector*> opticalSensDet;
};
}
