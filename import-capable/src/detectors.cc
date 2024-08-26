#include <G4HCofThisEvent.hh>
#include <G4OpticalPhoton.hh>
#include <G4SDManager.hh>
#include <G4Step.hh>
#include <G4THitsCollection.hh>
#include <G4TouchableHistory.hh>

#include <hits.hh>
#include <detectors.hh>

SiSensitiveDetector::
SiSensitiveDetector(const G4String& detectorName) :
    G4VSensitiveDetector(detectorName),
    thisCollectionName(detectorName + "_si_sens_det"),
    hitsCollectionId(-1)
{
    collectionName.insert(thisCollectionName);
}

SiSensitiveDetector::~SiSensitiveDetector()
{ }

void SiSensitiveDetector::
Initialize(G4HCofThisEvent* hcote)
{
    // FIXME: copied from the scintillator class. maybe we should just subclass? ugh.
    // polymorphism in the hitscollection
    hitsCollection = new G4THitsCollection<VirtualHit>(
        SensitiveDetectorName, thisCollectionName);
    hitsCollectionId = G4SDManager::GetSDMpointer()->GetCollectionID(thisCollectionName);

    hcote->AddHitsCollection(hitsCollectionId, hitsCollection);
}

G4bool SiSensitiveDetector::
ProcessHits(G4Step*, G4TouchableHistory*)
{
    /* G4cout << "*** in optical" << G4endl; */
    // with 100% efficiency hits don't really register...
    /* if (step->GetTrack()->GetParticleDefinition() == G4OpticalPhoton::Definition()) */
        /* processOptical(step); */

    return false;
}

void SiSensitiveDetector::
processOptical(const G4Step* step)
{
    auto edep = step->GetTotalEnergyDeposit();
    if (edep == 0)
        return;

    auto pos = step->GetPostStepPoint()->GetPosition();
    // we want GlobalTime because it's the time since the last *event* started!
    // LocalTime is the time since the track was created (we don't want this)
    auto t = step->GetPostStepPoint()->GetGlobalTime();
    auto* hit = new SiHit(pos, t, edep);
    hitsCollection->insert(hit);
}

CrystalSensitiveDetector::
CrystalSensitiveDetector(const G4String& detectorName) :
    G4VSensitiveDetector(detectorName),
    thisCollectionName(detectorName + "_scint"),
    hitsCollectionId(-1)
{
    collectionName.insert(thisCollectionName);
}

CrystalSensitiveDetector::
~CrystalSensitiveDetector()
{ }

void CrystalSensitiveDetector::
Initialize(G4HCofThisEvent* hcote)
{
    // polymorphism in the hitscollection
    hitsCollection = new G4THitsCollection<VirtualHit>(
        SensitiveDetectorName, thisCollectionName);
    hitsCollectionId = G4SDManager::GetSDMpointer()->GetCollectionID(thisCollectionName);
    hcote->AddHitsCollection(hitsCollectionId, hitsCollection);
}

G4bool CrystalSensitiveDetector::
ProcessHits(G4Step* step, G4TouchableHistory* /* unused */)
{
    auto def = step->GetTrack()->GetParticleDefinition();
    // we don't want optical photons to count as hits
    // especially if scintillation is on
    if (def == G4OpticalPhoton::Definition()) return false;

    G4double depositedEnergy = step->GetTotalEnergyDeposit();
    if (depositedEnergy == 0) return false;

    G4ThreeVector pos = step->GetPostStepPoint()->GetPosition();

    auto* hit = new CrystalHit(depositedEnergy, pos);

    hitsCollection->insert(hit);
    return true;
}