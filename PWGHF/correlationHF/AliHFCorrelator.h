#ifndef AliHFCorrelator_H
#define AliHFCorrelator_H

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

//
//             Base class for Heavy Flavour Correlations Analysis
//             Single Event and Mixed Event Analysis are implemented
//-----------------------------------------------------------------------
//          
//
//						   Author S.Bjelogrlic
//                         Utrecht University 
//                      sandro.bjelogrlic@cern.ch
//
//-----------------------------------------------------------------------

/* $Id$ */

#include "AliHFAssociatedTrackCuts.h"
#include "AliEventPoolManager.h"
#include "AliVParticle.h"
#include "AliReducedParticle.h"
#include "AliVertexingHFUtils.h"


class AliHFCorrelator : public TNamed
{
	
 public:
	
	AliHFCorrelator();
	AliHFCorrelator(const Char_t* name, AliHFAssociatedTrackCuts *cuts, Bool_t ppOrPbPb);
	virtual ~AliHFCorrelator();
	
	// enum for setting which associated particle type to work with
	enum{
	  kUndefined=0,
	  kHadron=1,
	  kKaon,
	  kKZero,
	  kElectron
	};

	//setters
	void SetDeltaPhiInterval (Double_t min, Double_t max){
		fPhiMin = min; fPhiMax = max;
		if(TMath::Abs(fPhiMin-fPhiMax) != 2*TMath::Pi()) AliInfo("AliHFCorrelator::Warning: the delta phi interval is not set to 2 Pi");
	}
	void SetEventMixing(Bool_t mixON){fmixing=mixON;}
	void SetTriggerParticleProperties(Double_t ptTrig, Double_t phiTrig, Double_t etaTrig)
	{fPtTrigger = ptTrig; fPhiTrigger = phiTrig; fEtaTrigger = etaTrig;}
	void SetTriggerParticleDaughterCharge(Short_t charge) {fDCharge=charge;}
	
	
	void SetAssociatedParticleType(Int_t type){fselect = type;}
	void SetAODEvent(AliAODEvent* inputevent){fAODEvent = inputevent;}
	void SetMCArray(TClonesArray* mcArray){fmcArray = mcArray;}
	void SetUseMC(Bool_t useMC){fmontecarlo = useMC;}
	void SetApplyDisplacementCut(Int_t applycut){fUseImpactParameter = applycut;}
	void SetPIDmode(Int_t mode){fPIDmode = mode;}
	
	void SetD0Properties(AliAODRecoDecayHF2Prong* d, Int_t D0hyp)
	{fD0cand = d; fhypD0 = D0hyp;}
	
	void SetUseReco(Bool_t useReco) {fUseReco = useReco;}
	
	
	
        Bool_t DefineEventPool(); // Definition of the Event pool parameters
	Bool_t Initialize(); // function that initlize everything for the analysis	
	Bool_t ProcessEventPool(); // processes the event pool
	Bool_t ProcessAssociatedTracks(Int_t EventLoopIndex, const TObjArray* associatedTracks=NULL); //
	Bool_t Correlate(Int_t loopindex); // function that computes the correlations between the trigger particle and the track n. loopindex
	Bool_t PoolUpdate(const TObjArray* associatedTracks=NULL);// updates the event pool
	Double_t SetCorrectPhiRange(Double_t phi); // sets all the angles in the correct range
	void SetPidAssociated() {fhadcuts->SetPidAssociated();}

	//getters
	AliEventPool* GetPool() {return fPool;}
	TObjArray * GetTrackArray(){return fAssociatedTracks;}
	AliHFAssociatedTrackCuts* GetSelectionCuts() {return fhadcuts;}
	AliReducedParticle* GetAssociatedParticle() {return fReducedPart;}
	
	Int_t GetNofTracks(){return fNofTracks;}
	Int_t GetNofEventsInPool(){return fPoolContent;}

	Double_t GetDeltaPhi(){return fDeltaPhi;} // Delta Phi, needs to be called after the method correlate 
	Double_t GetDeltaEta(){return fDeltaEta;} // Delta Eta
	
	Double_t GetAssociatedKZeroInvariantmass(){return fk0InvMass;}
	
	
	
	// methods to reduce the tracks to correlate with track selection cuts applied here
	TObjArray*  AcceptAndReduceTracks(AliAODEvent* inputEvent); // selecting hadrons and kaons
	TObjArray*  AcceptAndReduceKZero(AliAODEvent* inputEvent); // selecting kzeros
	
	
 private:

	AliHFCorrelator(const AliHFCorrelator& vtxr);
	AliHFCorrelator& operator=(const AliHFCorrelator& vtxr );

	AliEventPoolManager* fPoolMgr;         //! event pool manager
	AliEventPool * fPool; //! Pool for event mixing
	AliHFAssociatedTrackCuts* fhadcuts;//! hadron cuts
	AliAODEvent * fAODEvent;//! AOD Event
	TObjArray* fAssociatedTracks; // Array of associated tracks
	TClonesArray* fmcArray; //mcarray
	AliReducedParticle * fReducedPart; // reduced AOD particle;
	AliAODRecoDecayHF2Prong* fD0cand; //D0 candidate
	Int_t fhypD0; //hypothesis necessary for
	Int_t fDCharge; // charge of a daughter of the D meson
	
	Bool_t fmixing;// switch for event mixing
	Bool_t fmontecarlo; // switch for MonteCarlo
	Bool_t fsystem; // select pp (kFALSE) or PbPb (kTRUE)
	Bool_t fUseReco; // switch to use reconstruction (kTRUE) or MC truth (kFALSE)
	
	Int_t fselect; // 1 for hadrons, 2 for kaons, 3 for KZeros
	Int_t fUseImpactParameter; // switch to use the impact parameter cut
	Int_t fPIDmode; // set the PID mode for Kaon identification
	
	Int_t fNofTracks; // number of tracks in track array
	Int_t fPoolContent; //  n of events in pool
	
	Double_t fPhiMin; // min for phi
	Double_t fPhiMax; // max for phi
	
	Double_t fPtTrigger; // pt of the trigger D meson
	Double_t fPhiTrigger; // phi of the trigger D meson
	Double_t fEtaTrigger; // Eta of the trigger D meson

	
	Double_t fDeltaPhi; // delta phi between D meson and associated track
	Double_t fDeltaEta; // delta eta between D meson and associated track
	
	Double_t fk0InvMass; // KZero invariant mass
	
	
	ClassDef(AliHFCorrelator,3); // class for HF correlations	
};





#endif
