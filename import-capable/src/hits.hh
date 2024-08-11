#pragma once

#include <G4ThreeVector.hh>
#include <G4VHit.hh>

class VirtualHit : public G4VHit
{
public:
    VirtualHit(const G4ThreeVector& position, G4double arrivalTime);
    ~VirtualHit() =0;
    enum class HitType { Si };

    virtual HitType hitType() const =0;
    virtual const G4ThreeVector& peekPosition() const;
    virtual G4double peekArrivalTime() const;

    virtual void Draw() override;

protected:
    G4double arrivalTime;
    G4ThreeVector position;
};

class SiHit : public VirtualHit {
public:
    SiHit(
        const G4ThreeVector& position,
        G4double arrivalTime, G4double depositedEnergy);
    ~SiHit();
    const SiHit& operator=(const SiHit& rhs);
    bool operator==(const SiHit& rhs);

    G4double peekDepositedEnergy() const;

    void* operator new(size_t sz);
    void operator delete(void* toDelete);

    void Print() override;
    HitType hitType() const override;

private:
    G4double depositedEnergy;
};
