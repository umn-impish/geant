#include <hits.hh>

#include <G4Allocator.hh>
#include <G4Circle.hh>
#include <G4Point3D.hh>
#include <G4UnitsTable.hh>
#include <G4VVisManager.hh>
#include <G4VisAttributes.hh>

G4ThreadLocal G4Allocator<SiHit> *sialloc = nullptr;
G4ThreadLocal G4Allocator<CrystalHit> *cralloc = nullptr;

VirtualHit::VirtualHit(const G4ThreeVector &position,
                       const G4ThreeVector &momentum, G4double arrivalTime)
    : arrivalTime{arrivalTime}, position{position}, momentum{momentum} {}

VirtualHit::~VirtualHit() {}

const G4ThreeVector &VirtualHit::peekPosition() const { return position; }

G4double VirtualHit::peekArrivalTime() const { return arrivalTime; }

const G4ThreeVector &VirtualHit::peekMomentum() const { return momentum; }

void VirtualHit::Draw() {
  auto *visMan = G4VVisManager::GetConcreteInstance();
  if (visMan == nullptr)
    return;

  G4Circle c(peekPosition());
  c.SetDiameter(G4VMarker::screen, 5);

  c.SetFillStyle(G4Circle::filled);
  G4VisAttributes a(G4Color(1, 0, 0));
  c.SetVisAttributes(a);
  visMan->Draw(c);
}

SiHit::SiHit(const G4ThreeVector &position, const G4ThreeVector &momentum,
             G4double arrivalTime, G4double depositedEnergy)
    : VirtualHit(position, momentum, arrivalTime),
      depositedEnergy(depositedEnergy) {}

void SiHit::Print() {
  auto f = G4cout.flags();
  G4cout << "=== Begin SiHit ===" << G4endl
         << "Energy deposited: " << std::setw(7)
         << G4BestUnit(depositedEnergy, "Energy") << G4endl
         << "Position:         " << std::setw(7)
         << G4BestUnit(position, "Length") << G4endl
         << "Arrival time:     " << std::setw(7)
         << G4BestUnit(arrivalTime, "Time") << G4endl
         << "=== End SiCrystalHit ===" << G4endl;
  G4cout.flags(f);
  G4cout.flush();
}

void *SiHit::operator new(size_t) {
  if (sialloc == nullptr)
    sialloc = new G4Allocator<SiHit>;
  return (void *)sialloc->MallocSingle();
}

void SiHit::operator delete(void *h) { sialloc->FreeSingle((SiHit *)h); }

G4double SiHit::peekDepositedEnergy() const { return depositedEnergy; }

VirtualHit::HitType SiHit::hitType() const { return HitType::Si; }

CrystalHit::CrystalHit(G4double depositedEnergy, const G4ThreeVector &position,
                       const G4ThreeVector &momentum)
    : // XXX ignore arrival time
      VirtualHit(position, momentum, 0), depositedEnergy(depositedEnergy) {}

VirtualHit::HitType CrystalHit::hitType() const { return HitType::Crystal; }

G4double CrystalHit::peekDepositedEnergy() const { return depositedEnergy; }

void *CrystalHit::operator new(size_t) {
  if (cralloc == nullptr)
    cralloc = new G4Allocator<CrystalHit>;
  return (void *)cralloc->MallocSingle();
}

void CrystalHit::operator delete(void *h) {
  cralloc->FreeSingle((CrystalHit *)h);
}