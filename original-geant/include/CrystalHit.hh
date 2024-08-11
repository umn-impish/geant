#pragma once

#include "G4VHit.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"

#include "VirtualHit.hh"

namespace ImpressForGrips
{
class CrystalHit : public VirtualHit
{
    public:
        CrystalHit(G4double depositedEnergy, const G4ThreeVector& position);
        ~CrystalHit();
        const CrystalHit& operator=(const CrystalHit& rhs);
        bool operator==(const CrystalHit& rhs);

        void* operator new(size_t sz);
        void operator delete(void* toDelete);

        void Print() override;
        HitType hitType() const override
        { return HitType::ScintCryst; }

        G4double peekDepositedEnergy() const
        { return depositedEnergy; }

        static const char*
        peekAssociatedChannelId() { return "-1"; }

    private:
        G4double depositedEnergy;
};
}
