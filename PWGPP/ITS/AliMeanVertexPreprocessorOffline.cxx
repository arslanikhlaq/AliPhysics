/**************************************************************************
 * Copyright(c) 1998-2011, ALICE Experiment at CERN, All rights reserved. *
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

/*

*/   
// Mean Vertex preprocessor:
// 2) takes data after  pass0 , 
// processes it, and stores either to OCDB .
//
// Davide Caffarri

#include "AliMeanVertexPreprocessorOffline.h"

#include "AliCDBStorage.h"
#include "AliCDBMetaData.h"
#include "AliCDBManager.h"
#include "AliCDBEntry.h"
#include "AliLog.h"

#include <TTimeStamp.h>
#include <TFile.h>
#include <TObjString.h>
#include <TNamed.h>
#include "TClass.h"
#include <TCanvas.h>

#include "AliESDVertex.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TProfile.h"


ClassImp(AliMeanVertexPreprocessorOffline)

//_______________________________________________________  
  
  const Char_t *AliMeanVertexPreprocessorOffline::fgkStatusCodeName[AliMeanVertexPreprocessorOffline::kNStatusCodes] = {
  "ok",
  "open file error or missing histos",
  "too low statistics",
  "problems storing OCDB",
  "write MeanVertex computed online",
  "write SPD vtx offline",
  "lumi region or cov matrix computation problems, default values set"
};


//____________________________________________________
AliMeanVertexPreprocessorOffline::AliMeanVertexPreprocessorOffline():
  TNamed("AliMeanVertexPreprocessorOffline","AliMeanVertexPreprocessorOffline"),
  fStatus(kOk)
{
  //constructor
}
//____________________________________________________

AliMeanVertexPreprocessorOffline::~AliMeanVertexPreprocessorOffline()
{
  //destructor

}
//____________________________________________________
void AliMeanVertexPreprocessorOffline::ProcessOutput(const char *filename, AliCDBStorage *db, Int_t runNb){
	
	TFile *file = TFile::Open(filename);
	if (!file || !file->IsOpen()){
		AliError(Form("cannot open outputfile %s", filename));
		fStatus=kInputError;
		return; 
	}

	if (!db){
	  AliError(Form("no OCDB storage found, return"));
	  fStatus=kInputError;
	  return;
	}
    
	
	TList *list = (TList*)file->Get("MeanVertex");				
	
	TH1F *histTRKvtxX = 0x0;
	TH1F *histTRKvtxY = 0x0;
	TH1F *histTRKvtxZ = 0x0;
	
	TH1F *histSPDvtxX = 0x0;
	TH1F *histSPDvtxY = 0x0;
	TH1F *histSPDvtxZ = 0x0;
	
	
	Bool_t useTRKvtx = kTRUE;
	Bool_t useITSSAvtx = kFALSE;
	Bool_t useSPDvtx = kFALSE;
	Bool_t spdAvailable = kTRUE;
	Bool_t writeMeanVertexSPD = kFALSE;
	Bool_t vertexerSPD3Doff=kFALSE;
	
	
    if (!list) {
		
      histTRKvtxX = (TH1F*)file->Get("hTRKVertexX");
      histTRKvtxY = (TH1F*)file->Get("hTRKVertexY");
      histTRKvtxZ = (TH1F*)file->Get("hTRKVertexZ");
      
      histSPDvtxX = (TH1F*)file->Get("hSPDVertexX");
      histSPDvtxY = (TH1F*)file->Get("hSPDVertexY");
      histSPDvtxZ = (TH1F*)file->Get("hSPDVertexZ");
      
      if (!histTRKvtxX || !histTRKvtxY || !histTRKvtxZ){
	
	useTRKvtx = kFALSE;
	useITSSAvtx = kTRUE;
	
	histTRKvtxX = (TH1F*)file->FindObject("hITSSAVertexX");
	histTRKvtxY = (TH1F*)file->FindObject("hITSSAVertexY");
	histTRKvtxZ = (TH1F*)file->FindObject("hITSSAVertexZ");
	
	if (!histTRKvtxX || !histTRKvtxY || !histTRKvtxZ){
	  
	  useITSSAvtx=kFALSE;
	  useSPDvtx=kTRUE;
	  
	  if (!histSPDvtxX || !histSPDvtxY || !histSPDvtxZ){
	    AliError(Form("cannot find any histograms available from file"));
	    fStatus=kInputError;
	    return;
	  }	
	}
      }
    }	
    
    else{
		
      histTRKvtxX = (TH1F*)list->FindObject("hTRKVertexX");
      histTRKvtxY = (TH1F*)list->FindObject("hTRKVertexY");
      histTRKvtxZ = (TH1F*)list->FindObject("hTRKVertexZ");
      
      histSPDvtxX = (TH1F*)list->FindObject("hSPDVertexX");
      histSPDvtxY = (TH1F*)list->FindObject("hSPDVertexY");
      histSPDvtxZ = (TH1F*)list->FindObject("hSPDVertexZ");
      
      if (!histTRKvtxX || !histTRKvtxY || !histTRKvtxZ){
	
	useTRKvtx = kFALSE;
	useITSSAvtx = kTRUE;
	
	histTRKvtxX = (TH1F*)list->FindObject("hITSSAVertexX");
	histTRKvtxY = (TH1F*)list->FindObject("hITSSAVertexY");
	histTRKvtxZ = (TH1F*)list->FindObject("hITSSAVertexZ");
	
	if (!histTRKvtxX || !histTRKvtxY || !histTRKvtxZ){
	  
	  useITSSAvtx=kFALSE;
	  useSPDvtx=kTRUE;
	  
	  if (!histSPDvtxX || !histSPDvtxY || !histSPDvtxZ){
	    AliError(Form("cannot find any histograms available from list"));
	    fStatus=kInputError;
	    return;
	  }	
	}			 			 
	
      }
    }
    
    
    if (useTRKvtx){
      
      Float_t nEntriesX = histTRKvtxX->GetEffectiveEntries(); 					 
      Float_t nEntriesY = histTRKvtxY->GetEffectiveEntries(); 			 
      Float_t nEntriesZ = histTRKvtxZ->GetEffectiveEntries(); 			 
      
      if (nEntriesX < 50. || nEntriesY<50. || nEntriesZ<50.) {
	AliError(Form("TRK vertex histograms have too few entries for fitting"));
	useTRKvtx=kFALSE;
	useSPDvtx = kTRUE;	
      }
    }
    if (useITSSAvtx){
      
      Float_t nEntriesX = histTRKvtxX->GetEffectiveEntries(); 					 
      Float_t nEntriesY = histTRKvtxY->GetEffectiveEntries(); 			 
      Float_t nEntriesZ = histTRKvtxZ->GetEffectiveEntries(); 			 
      
      if (nEntriesX < 50. || nEntriesY<50. || nEntriesZ<50.) {
	AliError(Form("ITSSA vertex histograms have too few entries for fitting"));
	useITSSAvtx=kFALSE;
	useSPDvtx=kTRUE;
      }
    }
    
    Float_t nEntriesX = histSPDvtxX->GetEffectiveEntries(); 					 
    Float_t nEntriesY = histSPDvtxY->GetEffectiveEntries(); 			 
    Float_t nEntriesZ = histSPDvtxZ->GetEffectiveEntries(); 
    
    if (nEntriesX < 50. || nEntriesY<50. || nEntriesZ<50.) {
      spdAvailable = kFALSE;
      if ((useTRKvtx==kFALSE) && (useITSSAvtx==kFALSE)){
	AliError(Form("Also SPD vertex histograms have too few entries for fitting, return"));
	fStatus=kLowStatistics;
			return;		 
      }
    }
	
    if((nEntriesX == 0.)&&(nEntriesY==0.) && (nEntriesZ>0.)){
      vertexerSPD3Doff = kTRUE;
      AliWarning("Vertexer SPD 3D off");
    }
    
    Double_t xMeanVtx=0., yMeanVtx=0., zMeanVtx=0.;
    Double_t xSigmaVtx=0., ySigmaVtx=0., zSigmaVtx=0.;
    
    
    TF1 *fitVtxX, *fitVtxY, *fitVtxZ;
    
    if (useTRKvtx || useITSSAvtx){
      histTRKvtxX ->Fit("gaus", "M");
      fitVtxX = histTRKvtxX -> GetFunction("gaus");
      xMeanVtx = fitVtxX -> GetParameter(1);
      if (TMath::Abs(xMeanVtx) > 2.) {
	xMeanVtx = 0.;
	writeMeanVertexSPD=kTRUE;
	fStatus=kWriteMeanVertexSPD;
      }	
      
      histTRKvtxY ->Fit("gaus", "M");
      fitVtxY = histTRKvtxY -> GetFunction("gaus");
      yMeanVtx = fitVtxY -> GetParameter(1);
      if (TMath::Abs(yMeanVtx) > 2.) {
	yMeanVtx = 0.;
	writeMeanVertexSPD=kTRUE;
	fStatus=kWriteMeanVertexSPD;
      }	
      
      histTRKvtxZ ->Fit("gaus", "M", "", -12, 12);
      fitVtxZ = histTRKvtxZ -> GetFunction("gaus");
      zMeanVtx = fitVtxZ -> GetParameter(1);
      zSigmaVtx = fitVtxZ -> GetParameter(2);
      if ((TMath::Abs(zMeanVtx) > 20.) || (zSigmaVtx>12.)) {
	zMeanVtx = histTRKvtxZ->GetMean();
	zSigmaVtx = histTRKvtxZ->GetRMS();
	writeMeanVertexSPD=kTRUE;
	fStatus=kWriteMeanVertexSPD;
      }	
      
    }
    
    
    //check fits: compare histo mean with fit mean value 
    Double_t xHistoMean, yHistoMean, zHistoMean;
    Double_t xHistoRMS, yHistoRMS, zHistoRMS;
    
    if (useTRKvtx || useITSSAvtx){
      xHistoMean = histTRKvtxX -> GetMean();	
      xHistoRMS = histTRKvtxX ->GetRMS();
      
      if ((TMath::Abs(xHistoMean-xMeanVtx) > 0.5)){
	AliWarning(Form("Possible problems with the fit mean very different from histo mean... using SPD vertex"));
	useTRKvtx = kFALSE;
	useITSSAvtx = kFALSE;
	useSPDvtx = kTRUE;
	fStatus=kUseOfflineSPDvtx;
      }
      
      yHistoMean = histTRKvtxY ->GetMean();	
      yHistoRMS = histTRKvtxY ->GetRMS();
      
      if ((TMath::Abs(yHistoMean-yMeanVtx) > 0.5)){
	AliWarning(Form("Possible problems with the fit mean very different from histo mean... using SPD vertex"));
	useTRKvtx = kFALSE;
	useITSSAvtx = kFALSE;
	useSPDvtx = kTRUE;
	fStatus=kUseOfflineSPDvtx;
      }
      
      zHistoMean = histTRKvtxZ -> GetMean();	
      zHistoRMS = histTRKvtxZ ->GetRMS();
      
      if ((TMath::Abs(zHistoMean-zMeanVtx) > 1.)){
	AliWarning(Form("Possible problems with the fit mean very different from histo mean... using SPD vertex"));
	useTRKvtx = kFALSE;
	useITSSAvtx = kFALSE;
	useSPDvtx = kTRUE;
	fStatus=kUseOfflineSPDvtx;
      }
    }
    
    
    if ((useSPDvtx) && (spdAvailable) && (!vertexerSPD3Doff)){
      
      histSPDvtxX ->Fit("gaus", "M");
      fitVtxX = histSPDvtxX -> GetFunction("gaus");
      xMeanVtx = fitVtxX -> GetParameter(1);
      if (TMath::Abs(xMeanVtx) > 2.) {
	xMeanVtx = 0.;
	writeMeanVertexSPD=kTRUE;
      }
      
      histSPDvtxY ->Fit("gaus", "M");
      fitVtxY = histSPDvtxY -> GetFunction("gaus");
      yMeanVtx = fitVtxY -> GetParameter(1);
      if (TMath::Abs(yMeanVtx) > 2.) {
	yMeanVtx = 0.;
	writeMeanVertexSPD=kTRUE;
      }	
      
      histSPDvtxZ ->Fit("gaus", "M", "", -12, 12);
      fitVtxZ = histSPDvtxZ -> GetFunction("gaus");
      zMeanVtx = fitVtxZ -> GetParameter(1);
      zSigmaVtx = fitVtxZ -> GetParameter(2);
      if ((TMath::Abs(zMeanVtx) > 20.) || (zSigmaVtx>12.)) {
	zMeanVtx = histSPDvtxZ ->GetMean();
	zSigmaVtx = histSPDvtxZ->GetRMS();
	writeMeanVertexSPD = kTRUE;
      }	
      
    }
    else if ((useSPDvtx) && (!spdAvailable)){
      AliError(Form("Difference between trkVtx and online one, SPD histos not enough entry or SPD 3D vertex off. Writing Mean Vertex SPD"));
      writeMeanVertexSPD = kTRUE;	
    }
    
    
    //check with online position
    
    Double_t posOnline[3], sigmaOnline[3];
    
    if (useTRKvtx || useITSSAvtx || writeMeanVertexSPD){
      AliCDBManager *manCheck = AliCDBManager::Instance();
      manCheck->SetDefaultStorage("raw://");
      manCheck->SetRun(runNb);
		
      AliCDBEntry *entr = manCheck->Get("GRP/Calib/MeanVertexSPD");
      if(entr) {
	AliESDVertex *vtxOnline = (AliESDVertex*)entr->GetObject();
	
	posOnline[0] = vtxOnline->GetX();
	posOnline[1] = vtxOnline->GetY();
	posOnline[2] = vtxOnline->GetZ();
	
	sigmaOnline[0] = vtxOnline->GetXRes();
	sigmaOnline[1] = vtxOnline->GetYRes();
	sigmaOnline[2] = vtxOnline->GetZRes();
	
	//vtxOnline->GetSigmaXYZ(sigmaOnline);
	
	if ((TMath::Abs(posOnline[0]-xMeanVtx) > 0.1) || (TMath::Abs(posOnline[1]-yMeanVtx) > 0.1) || (TMath::Abs(posOnline[2]-zMeanVtx) > 1.)){
	  AliWarning(Form("vertex offline far from the online one"));
	}
      }
    }
    
    
    
    if (writeMeanVertexSPD){
      
      AliWarning(Form("Writing Mean Vertex SPD, Mean Vertex not available"));
      
      Double_t sigma[3]={0.0150, 0.0150, zSigmaVtx};
      
      AliESDVertex  *vertex =  new AliESDVertex(posOnline, sigma, "vertex");
      
      AliCDBId id("GRP/Calib/MeanVertex", runNb, runNb);
      
      AliCDBMetaData metaData;
      metaData.SetBeamPeriod(0); //check!!!!
      metaData.SetResponsible("Davide Caffarri");
      metaData.SetComment("Mean Vertex object used in reconstruction");
      
      if (!db->Put(vertex, id, &metaData)) {
	AliError(Form("Error while putting object in storage %s", db->GetURI().Data()));
      }
      
      delete vertex;
      return;	
    }
    
    
    Float_t meanMult = 40.;
    Float_t p2 = 1.4;
    Float_t resolVtx = 0.05;
    
    Double_t xSigmaMult, ySigmaMult, corrXZ, corrYZ, lumiRegSquaredX, lumiRegSquaredY;
    Double_t covarXZ=0., covarYZ=0.;
    
    Bool_t highMultEnvironment = kFALSE;
    Bool_t highMultppEnvironment = kFALSE;
    Bool_t lowMultppEnvironment = kFALSE;
    
    TF1 *sigmaFitX, *sigmaFitY, *corrFit;
    
    //TH1F *histTRKdefMultX=0;
    //TH1F *histTRKdefMultY=0;
    TH1F *histTRKHighMultX=0;
    TH1F *histTRKHighMultY=0;
    TH2F *histTRKVertexXZ=0;
    TH2F *histTRKVertexYZ=0;
    
    TH2F *histTRKvsMultX=0x0;
    TH2F *histTRKvsMultY=0x0;
    
    if (useTRKvtx){
      if (list){
	      //histTRKdefMultX = (TH1F*)list->FindObject("hTRKVertexXdefMult");
	      //histTRKdefMultY = (TH1F*)list->FindObject("hTRKVertexYdefMult");
	histTRKHighMultX = (TH1F*)list->FindObject("hTRKVertexXHighMult");
	histTRKHighMultY = (TH1F*)list->FindObject("hTRKVertexYHighMult");
	
	histTRKvsMultX = (TH2F*)list->FindObject("hTRKVertexXvsMult");
	histTRKvsMultY = (TH2F*)list->FindObject("hTRKVertexYvsMult");
	
	histTRKVertexXZ = (TH2F*)list->FindObject("hTRKVertexXZ");
	histTRKVertexYZ = (TH2F*)list->FindObject("hTRKVertexYZ");
      }
      
      else {
	      //histTRKdefMultX = (TH1F*)file->Get("hTRKVertexXdefMult");
	      //histTRKdefMultY = (TH1F*)file->Get("hTRKVertexYdefMult");
	histTRKHighMultX = (TH1F*)file->Get("hTRKVertexXHighMult");
	histTRKHighMultY = (TH1F*)file->Get("hTRKVertexYHighMult");
	
	histTRKvsMultX = (TH2F*)file->FindObject("hTRKVertexXvsMult");
	histTRKvsMultY = (TH2F*)file->FindObject("hTRKVertexYvsMult");
	    
	histTRKVertexXZ = (TH2F*)file->Get("hTRKVertexXZ");
	histTRKVertexYZ = (TH2F*)file->Get("hTRKVertexYZ");
      }
      
    }	
    
    if (useITSSAvtx){
      if (list){
	      //histTRKdefMultX = (TH1F*)list->FindObject("hITSSAVertexXdefMult");
	      //histTRKdefMultY = (TH1F*)list->FindObject("hITSSAVertexYdefMult");
	histTRKHighMultX = (TH1F*)list->FindObject("hITSSAVertexXHighMult");
	histTRKHighMultY = (TH1F*)list->FindObject("hITSSAVertexYHighMult");
	
	histTRKvsMultX = (TH2F*)file->FindObject("hITSSAVertexXvsMult");
	histTRKvsMultY = (TH2F*)file->FindObject("hITSSAVertexYvsMult");
	
	histTRKVertexXZ = (TH2F*)list->FindObject("hITSSAVertexXZ");
	histTRKVertexYZ = (TH2F*)list->FindObject("hITSSAVertexYZ");
      }
      
      else {
	      //histTRKdefMultX = (TH1F*)file->Get("hITSSAVertexXdefMult");
	      //histTRKdefMultY = (TH1F*)file->Get("hITSSAVertexYdefMult");
	histTRKHighMultX = (TH1F*)file->Get("hITSSAVertexXHighMult");
	histTRKHighMultY = (TH1F*)file->Get("hITSSAVertexYHighMult");
	
	histTRKvsMultX = (TH2F*)file->FindObject("hITSSAVertexXvsMult");
	histTRKvsMultY = (TH2F*)file->FindObject("hITSSAVertexYvsMult");
	
	histTRKVertexXZ = (TH2F*)file->Get("hITSSAVertexXZ");
	histTRKVertexYZ = (TH2F*)file->Get("hITSSAVertexYZ");
      }
    }
    
    
    TH1D *projXvsMult;
    TH1D *projYvsMult;
    
    Float_t nEntriesMultX=0, nEntriesMultY=0.;

    if ((histTRKHighMultX) && (histTRKHighMultY)){
    
      projXvsMult = (TH1D*)histTRKvsMultX->ProjectionX("projXHighMultPbPb", 150, 300);
      projYvsMult = (TH1D*)histTRKvsMultY->ProjectionX("projYHighMultPbPb", 150, 300);
      
      nEntriesMultX = projXvsMult->GetEffectiveEntries();
      nEntriesMultY = projYvsMult->GetEffectiveEntries();
      
      if ((nEntriesMultX >100) && (nEntriesMultY>100)) {
	AliWarning(Form("Setting High Mulitplicity environment"));	
	highMultEnvironment = kTRUE;
      }
      else {
	
	projXvsMult = (TH1D*)histTRKvsMultX->ProjectionX("projXHighMultPbPb", 10, 30);
	projYvsMult = (TH1D*)histTRKvsMultY->ProjectionX("projYHighMultPbPb", 10, 30);
	
	nEntriesMultX = projXvsMult->GetEffectiveEntries();
	nEntriesMultY = projYvsMult->GetEffectiveEntries();
	
	if ((nEntriesMultX >100) && (nEntriesMultY>100)) {
	  AliWarning(Form("Setting high pp Mulitplicity environment or p-A high multiplicity"));
	  highMultppEnvironment=kTRUE;			
	}
	else {
	  
	  projXvsMult = (TH1D*)histTRKvsMultX->ProjectionX("projXHighMultPbPb", 3, 5);
	  projYvsMult = (TH1D*)histTRKvsMultY->ProjectionX("projYHighMultPbPb", 3, 5);
	  
	  nEntriesMultX = projXvsMult->GetEffectiveEntries();
	  nEntriesMultY = projYvsMult->GetEffectiveEntries();
	  
	  if ((nEntriesMultX >100) && (nEntriesMultY>100)) {
	    AliWarning(Form("Setting low pp Mulitplicity environment"));
	    lowMultppEnvironment=kTRUE;			
	  }
	
	}
	
      }
    
	
      if (lowMultppEnvironment==kTRUE) {
	
	if ((projXvsMult->GetEntries() < 40.) || (projYvsMult->GetEntries() < 40.)){
	  AliWarning(Form("histos for lumi reg calculation not found, default value set"));
	  xSigmaVtx=0.0120;
	  ySigmaVtx=0.0120;
	  fStatus=kLumiRegCovMatrixProblem;
	} else {
	  
	  projXvsMult -> Fit("gaus", "M", "", -0.4, 0.4);
	  sigmaFitX = projXvsMult -> GetFunction("gaus");
	  xSigmaMult = sigmaFitX->GetParameter(2);
	  
	  lumiRegSquaredX = (xSigmaMult*xSigmaMult - ((resolVtx*resolVtx)/TMath::Power(meanMult, p2)));
	  if (lumiRegSquaredX < 0 || lumiRegSquaredX < 1E-5) {
	    AliWarning(Form("Difficult luminous region determination X, keep convoluted sigma"));
	    xSigmaVtx = xSigmaMult;
	    fStatus=kLumiRegCovMatrixProblem;
	  }
	  
	  else {
	    if (lumiRegSquaredX > 0 && lumiRegSquaredX < 0.0005){
	      xSigmaVtx = TMath::Sqrt(lumiRegSquaredX);
	      xSigmaVtx = xSigmaVtx*1.1;
	    }
	    
	    else{ 
	      AliWarning(Form("Not possible to define a luminous region X. Default values set"));
	      xSigmaVtx = 0.0120;
	      fStatus=kLumiRegCovMatrixProblem;
	    }
	  }
	  
	  projYvsMult -> Fit("gaus", "M", "", -0.2, 0.6);
	  sigmaFitY = projYvsMult -> GetFunction("gaus");
	  ySigmaMult = sigmaFitY->GetParameter(2);
	  
	  lumiRegSquaredY= (ySigmaMult*ySigmaMult - ((resolVtx*resolVtx)/TMath::Power(meanMult, p2)));
	  
	  if (lumiRegSquaredY < 0 || lumiRegSquaredY < 1E-5) {
	    AliWarning(Form("Difficult luminous region determination Y, keep convoluted sigma"));
	    ySigmaVtx = ySigmaMult;
	    fStatus=kLumiRegCovMatrixProblem;
	  }
	  
	  else{ 
	    if (lumiRegSquaredY > 0 && lumiRegSquaredY < 0.0005){
	      ySigmaVtx = TMath::Sqrt(lumiRegSquaredY);
	      ySigmaVtx = ySigmaVtx*1.1;
	    }
	    else{ 
	      AliWarning(Form("Not possible to define a luminous region Y. Default values set"));
	      ySigmaVtx = 0.0120;
	      fStatus=kLumiRegCovMatrixProblem;
	    }
	  }
	}
	
	TProfile *htrkXZ = histTRKVertexXZ ->ProfileY();
	htrkXZ -> Fit("pol1", "M", "", -10., 10.);
	corrFit = htrkXZ->GetFunction("pol1");
	corrXZ = corrFit->GetParameter(1);
	
	if (TMath::Abs(corrXZ) > 0.01) {
	  AliWarning(Form("Problems in the correlation fitting, not update the covariance matrix"));
	  corrXZ =0.;
	  fStatus=kLumiRegCovMatrixProblem;
	}
	else{
	  covarXZ = corrXZ * zSigmaVtx*zSigmaVtx;
	  
	}
	
	TProfile *htrkYZ = histTRKVertexYZ ->ProfileY();
	htrkYZ -> Fit("pol1", "M", "", -10., 10.);
	corrFit = htrkYZ->GetFunction("pol1");
	corrYZ = corrFit->GetParameter(1);
	
	if (TMath::Abs(corrYZ) > 0.01) {
	  AliWarning(Form("Problems in the correlation fitting, not update the covariance matrix"));
	  corrYZ =0.;
	  fStatus=kLumiRegCovMatrixProblem;
	}
	else{
	  covarYZ = corrYZ*zSigmaVtx*zSigmaVtx;
	}
	
      }
    }
    
     if (highMultEnvironment==kTRUE || highMultppEnvironment==kTRUE){
    
    projXvsMult -> Fit("gaus", "M", "", -0.4, 0.4);
    sigmaFitX = projXvsMult -> GetFunction("gaus");
    xSigmaMult = sigmaFitX->GetParameter(2);
	  
      if ((xSigmaMult <0) || (xSigmaMult>0.03)){
	AliWarning(Form("Problems with luminosiy region determination, update of the postion only"));
	xSigmaMult = 0.;
	xSigmaVtx = 0.0120;
	fStatus=kLumiRegCovMatrixProblem;
      }
      else{
	xSigmaVtx = xSigmaMult;
	xSigmaVtx = xSigmaVtx*1.1;
      }
	  
      projYvsMult -> Fit("gaus", "M", "", -0.2, 0.5);
      TCanvas *c = new TCanvas("nwC", "nwC");
      c->cd();
      projYvsMult->Draw();
      sigmaFitY = projYvsMult -> GetFunction("gaus");
      ySigmaMult = sigmaFitY->GetParameter(2);
	  
      if ((ySigmaMult <0) || (ySigmaMult>0.03)){
	AliWarning(Form("Problems with luminosiy region determination, update of the postion only"));
	ySigmaMult = 0.;
	ySigmaVtx = 0.0120;
	fStatus=kLumiRegCovMatrixProblem;
      }
      else{
	ySigmaVtx = ySigmaMult;
	ySigmaVtx = ySigmaVtx*1.1;
      }
    	  
      TProfile *htrkXZ = histTRKVertexXZ ->ProfileY();
      htrkXZ -> Fit("pol1", "M", "", -10., 10.);
      corrFit = htrkXZ->GetFunction("pol1");
      corrXZ = corrFit->GetParameter(1);
	  
      if (TMath::Abs(corrXZ) > 0.01) {
	AliWarning(Form("Problems in the correlation fitting, not update the covariance matrix"));
	corrXZ =0.;
	fStatus=kLumiRegCovMatrixProblem;
      }
      else{
	covarXZ = corrXZ * zSigmaVtx*zSigmaVtx;
	    
      }
	  
      TProfile *htrkYZ = histTRKVertexYZ ->ProfileY();
      htrkYZ -> Fit("pol1", "M", "", -10., 10.);
      corrFit = htrkYZ->GetFunction("pol1");
      corrYZ = corrFit->GetParameter(1);
	  
      if (TMath::Abs(corrYZ) > 0.01) {
	AliWarning(Form("Problems in the correlation fitting, not update the covariance matrix"));
	corrYZ =0.;
	fStatus=kLumiRegCovMatrixProblem;
      }
      else{
	covarYZ = corrYZ*zSigmaVtx*zSigmaVtx;
      }
	  
     }
	
     Double_t position[3], covMatrix[6];
     Double_t chi2=1.; 
     Int_t nContr=1;	
     
     position[0] = xMeanVtx;
     position[1] = yMeanVtx;
     position[2] = zMeanVtx;
     
     covMatrix[0] = xSigmaVtx*xSigmaVtx;
     covMatrix[1] = 0.; //xy
     covMatrix[2] = ySigmaVtx*ySigmaVtx;
     covMatrix[3] = covarXZ;
     covMatrix[4] = covarYZ;
     covMatrix[5] = zSigmaVtx*zSigmaVtx;
     
     
     //Printf ("%f, %f, %f, %f", xSigmaVtx, ySigmaVtx, covarXZ, covarYZ);
     
     AliESDVertex  *vertex =  new AliESDVertex(position, covMatrix, chi2, nContr, "vertex");
     
     AliCDBId id("GRP/Calib/MeanVertex", runNb, runNb);
     
     AliCDBMetaData metaData;
     metaData.SetBeamPeriod(0); //check!!!!
     metaData.SetResponsible("Davide Caffarri");
     metaData.SetComment("Mean Vertex object used in reconstruction");
     
     if (!db->Put(vertex, id, &metaData)) {
       AliError(Form("Error while putting object in storage %s", db->GetURI().Data()));
       fStatus=kStoreError;
     }
     
     delete vertex;
     
     Int_t status=GetStatus();
     if (status == 0) {
       AliInfo(Form("MeanVertex calibration successful: %s (status=%d)", fgkStatusCodeName[fStatus], status));
     }
     else if (status > 0) {
       AliInfo(Form("MeanVertex calibration failed: %s (status=%d)", fgkStatusCodeName[fStatus], status));
     }
     else if (status < 0) {
       AliInfo(Form("MeanVertex calibration but not fatal error: %s (status=%d)", fgkStatusCodeName[fStatus], status));
     }
     
     
}

//__________________________________________________________________________
Int_t AliMeanVertexPreprocessorOffline::GetStatus(){
  /*
   * get status
   */

  switch (fStatus) {

    /* OK, return zero */
  case kOk:
    return 0;
    break;
    
    /* non-fatal error, return negative status */
  case kLowStatistics:
  case kWriteMeanVertexSPD:
  case kUseOfflineSPDvtx:
  case kLumiRegCovMatrixProblem:
    return -fStatus;
    break;
    
    /* fatal error, return positive status */
  case kInputError: 
  case kStoreError:
    return fStatus;
    break;
    
    /* anything else, return negative large number */
  default:
    return -999;
    break;
  }
  
  /* should never arrive here, anyway return negative large number */
  return -999;
}

