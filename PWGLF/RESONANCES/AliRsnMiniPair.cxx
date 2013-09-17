#include "AliRsnMiniParticle.h"
#include "AliRsnMiniPair.h"

ClassImp(AliRsnMiniPair)

//__________________________________________________________________________________________________
void AliRsnMiniPair::Fill
(AliRsnMiniParticle *p1, AliRsnMiniParticle *p2, Double_t m1, Double_t m2, Double_t refMass)
{
//
// Fill this object with data coming
// from arguments
//

   p1->Set4Vector(fP1[0], m1, kFALSE);
   p2->Set4Vector(fP2[0], m2, kFALSE);
   p1->Set4Vector(fP1[1], m1, kTRUE );
   p2->Set4Vector(fP2[1], m2, kTRUE );

   fMother = -1;
   if (p1->Mother() == p2->Mother()) {
      fMother = p1->Mother();
      fMotherPDG = p1->MotherPDG();
   }

   Int_t i;
   for (i = 0; i < 2; i++) {
      fSum[i] = fP1[i] + fP2[i];
      fRef[i].SetXYZM(fSum[i].X(), fSum[i].Y(), fSum[i].Z(), refMass);
   }
}

//__________________________________________________________________________________________________
Double_t AliRsnMiniPair::CosThetaStar(Bool_t useMC)
{
//
// Return cosine of angle of one daughter to the resonance momentum in its rest frame
//

   TLorentzVector &mother    = fSum[ID(useMC)];
   TLorentzVector &daughter0 = fP1[ID(useMC)];
//    TLorentzVector &daughter1 = fP2[ID(useMC)];
   TVector3 momentumM(mother.Vect());
   TVector3 normal(mother.Y() / momentumM.Mag(), -mother.X() / momentumM.Mag(), 0.0);

   // Computes first the invariant mass of the mother
//    Double_t mass0      = daughter0.M();
//    Double_t mass1      = daughter1.M();
//    Double_t p0         = daughter0.Vect().Mag();
//    Double_t p1         = daughter1.Vect().Mag();
//    Double_t E0         = TMath::Sqrt(mass0 * mass0 + p0 * p0);
//    Double_t E1         = TMath::Sqrt(mass1 * mass1 + p1 * p1);
//    Double_t MotherMass = TMath::Sqrt((E0 + E1) * (E0 + E1) - (p0 * p0 + 2.0 * daughter0.Vect().Dot(daughter1.Vect()) + p1 * p1));
//    MotherMass = mother.M();

   // Computes components of beta
   Double_t betaX = -mother.X() / mother.E();
   Double_t betaY = -mother.Y() / mother.E();
   Double_t betaZ = -mother.Z() / mother.E();

   // Computes Lorentz transformation of the momentum of the first daughter
   // into the rest frame of the mother and theta*
   daughter0.Boost(betaX, betaY, betaZ);
   TVector3 momentumD = daughter0.Vect();

   Double_t cosThetaStar = normal.Dot(momentumD) / momentumD.Mag();

   return cosThetaStar;
}

//__________________________________________________________________________________________________
void AliRsnMiniPair::InvertP(Bool_t first)
{
//
// Inverts one 4-momentum and recompute sum
//

   Int_t i;
   for (i = 0; i < 2; i++) {
      if (first) fP1[i].Vect() *= -1.0;
      else       fP2[i].Vect() *= -1.0;
      fSum[i] = fP1[i] + fP2[i];
      fRef[i].SetXYZM(fSum[i].X(), fSum[i].Y(), fSum[i].Z(), fRef[i].M());
   }
}

//__________________________________________________________________________________________________
void AliRsnMiniPair::FillRef(Double_t mass)
{
//
// Fill ref 4-vectors using the passed mass and the values in 'sum'
//

   Int_t i;
   for (i = 0; i < 2; i++) {
      fRef[i].SetXYZM(fSum[i].X(), fSum[i].Y(), fSum[i].Z(), mass);
   }
}

//__________________________________________________________________________________________________
Double_t AliRsnMiniPair::InvMassRes() const
{
//
// Return invariant mass resolution
//

   if (fSum[1].M() <= 0.0) return 1E20;

   return (fSum[0].M() - fSum[1].M()) / fSum[1].M();
}

//__________________________________________________________________________________________________
Double_t AliRsnMiniPair::InvMassDiff() const
{
//
// Return invariant mass resolution
//

   if (fSum[1].M() <= 0.0) return 1E20;

   return (fSum[0].M() - fSum[1].M());
}

//__________________________________________________________________________________________________
Double_t AliRsnMiniPair::PtRatio(Bool_t mc) const
{
//
// Return ratio of transverse momenta of daughters
//

   Double_t num = TMath::Abs(fP1[ID(mc)].Perp() - fP2[ID(mc)].Perp());
   Double_t den = TMath::Abs(fP1[ID(mc)].Perp() + fP2[ID(mc)].Perp());

   if (den <= 0.0) return 1E20;

   return num / den;
}

//__________________________________________________________________________________________________
Double_t AliRsnMiniPair::DipAngle(Bool_t mc) const
{
//
// Opening angle in a Z-T space
//

   const TLorentzVector &p1 = fP1[ID(mc)];
   const TLorentzVector &p2 = fP2[ID(mc)];

   return ((p1.Perp() * p2.Perp() + p1.Z() * p2.Z()) / p1.Mag() / p2.Mag());
}

//__________________________________________________________________________________________________
Double_t AliRsnMiniPair::DaughterPt(Int_t daughterId, Bool_t mc)
{
  //returns pt of the <id> daughter 
  // if MC returns generated momenta
  if (daughterId==0)
    return fP1[ID(mc)].Pt();
  else 
    return fP2[ID(mc)].Pt();
}

//__________________________________________________________________________________________________
void AliRsnMiniPair::DaughterPxPyPz(Int_t daughterId, Bool_t mc, Double_t *pxpypz)
{
  //returns px,py,pz of the <id> daughter by saving them into pxpypz
  // if MC returns generated momenta
  if (!pxpypz)    return;
 
  if (daughterId==0){
    pxpypz[0]=fP1[ID(mc)].Px();
    pxpypz[1]=fP1[ID(mc)].Py();
    pxpypz[2]=fP1[ID(mc)].Pz();
  } else {
    pxpypz[0]=fP2[ID(mc)].Px();
    pxpypz[1]=fP2[ID(mc)].Py();
    pxpypz[2]=fP2[ID(mc)].Pz();
  }
  return;
}
