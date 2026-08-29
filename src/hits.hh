#pragma once

#include <G4ThreeVector.hh>
#include <G4VHit.hh>

class VirtualHit : public G4VHit {
public:
  VirtualHit(const G4ThreeVector &position, const G4ThreeVector &momentum,
             G4double arrivalTime);
  ~VirtualHit() = 0;
  enum class HitType { Si, Crystal };

  virtual HitType hitType() const = 0;
  virtual const G4ThreeVector &peekPosition() const;
  virtual const G4ThreeVector &peekMomentum() const;
  virtual G4double peekArrivalTime() const;

  virtual void Draw() override;

protected:
  G4double arrivalTime;
  G4ThreeVector position;
  G4ThreeVector momentum;
};

class SiHit : public VirtualHit {
public:
  SiHit(const G4ThreeVector &position, const G4ThreeVector &momentum,
        G4double arrivalTime, G4double depositedEnergy);
  ~SiHit() = default;
  const SiHit &operator=(const SiHit &rhs);
  bool operator==(const SiHit &rhs);

  G4double peekDepositedEnergy() const;

  void Print() override;
  HitType hitType() const override;

  void *operator new(size_t);
  void operator delete(void *);

private:
  G4double depositedEnergy;
};

class CrystalHit : public VirtualHit {
public:
  CrystalHit(G4double depositedEnergy, const G4ThreeVector &position,
             const G4ThreeVector &momentum);
  ~CrystalHit() = default;
  const CrystalHit &operator=(const CrystalHit &rhs);

  HitType hitType() const override;
  G4double peekDepositedEnergy() const;

  void *operator new(size_t);
  void operator delete(void *);

private:
  G4double depositedEnergy;
};
