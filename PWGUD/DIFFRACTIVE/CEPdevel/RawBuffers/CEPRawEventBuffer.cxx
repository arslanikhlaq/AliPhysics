/// ////////////////////////////////////////////////////////////////////////////
///
/// CEP raw event buffer
///
/// structure to hold event information
///
//______________________________________________________________________________
#include "CEPRawEventBuffer.h"
#include "CEPTrackBuffer.h"
#include "AliCEPBase.h"
#include "AliESDtrack.h"

ClassImp(CEPRawEventBuffer)

//______________________________________________________________________________
CEPRawEventBuffer::CEPRawEventBuffer()
    : TObject()
    , fEventNumber(CEPTrackBuffer::kdumval)
    , fnTracks(0)  
    , fnCaloTracks(0)  
    , fCEPRawTracks(new TObjArray())
    , fCEPRawCaloClusterTracks(new TObjArray())
    , fADCellBuffer(0x0)
    , fV0Buffer(0x0)
    , fEMCalBuffer(0x0)
    , fPHOSBuffer(0x0)
    , fFMDBuffer(0x0)
{

}

//______________________________________________________________________________
CEPRawEventBuffer::~CEPRawEventBuffer()
{
    // delete fCEPRawTracks and all the tracks it contains
    if (fCEPRawTracks) {
        fCEPRawTracks->SetOwner(kTRUE);
        fCEPRawTracks->Clear();
        delete fCEPRawTracks;
        fCEPRawTracks = 0x0;
    }
    // delete fCEPRawCaloClusterTracks and all the tracks it contains
    if (fCEPRawCaloClusterTracks) {
        fCEPRawCaloClusterTracks->SetOwner(kTRUE);
        fCEPRawCaloClusterTracks->Clear();
        delete fCEPRawCaloClusterTracks;
        fCEPRawCaloClusterTracks = 0x0;
    }    
    if (fADCellBuffer) { delete fADCellBuffer;  fADCellBuffer = 0x0; }
    if (fV0Buffer)     { delete fADCellBuffer;  fADCellBuffer = 0x0; }
    if (fEMCalBuffer)  { delete fEMCalBuffer;   fEMCalBuffer  = 0x0; }
    if (fPHOSBuffer)   { delete fPHOSBuffer;    fPHOSBuffer   = 0x0; }
    if (fFMDBuffer)    { delete fFMDBuffer;     fFMDBuffer    = 0x0; }
}
//______________________________________________________________________________
void CEPRawEventBuffer::Reset()
{
    // reset all private variables
    fEventNumber     = AliCEPBase::kdumval;

    // clear the track list
    fCEPRawTracks->SetOwner(kTRUE);
    fCEPRawTracks->Clear();
    // clear the calo cluster list
    fCEPRawCaloClusterTracks->SetOwner(kTRUE);
    fCEPRawCaloClusterTracks->Clear();

    fADCellBuffer->Reset();
    fV0Buffer->Reset();
    fEMCalBuffer->Reset();
    fPHOSBuffer->Reset();
    fFMDBuffer->Reset();
}

//______________________________________________________________________________
void CEPRawEventBuffer::AddTrack(CEPRawTrackBuffer* trk)
{
    // add track to next element
    fCEPRawTracks->Add(trk);
    fnTracks++;
}

//______________________________________________________________________________
void CEPRawEventBuffer::AddTrack(CEPRawCaloClusterTrack* caloTrk)
{
    // AddTrack overwritten for filling calo cluster Array
    fCEPRawCaloClusterTracks->Add(caloTrk);
    fnCaloTracks++;
}

//______________________________________________________________________________
CEPRawTrackBuffer* CEPRawEventBuffer::GetTrack(UInt_t ind)
{
    // initialize the result track
    CEPRawTrackBuffer *trk = NULL;

    if (fCEPRawTracks->GetEntries() > ind) 
    {
        trk = (CEPRawTrackBuffer*) fCEPRawTracks->At(ind);
    }

    return trk;
}

//______________________________________________________________________________
CEPRawCaloClusterTrack* CEPRawEventBuffer::GetCaloClusterTrack(UInt_t ind)
{
    // initialize the result track
    CEPRawCaloClusterTrack *caloTrk = NULL;

    if (fCEPRawCaloClusterTracks->GetEntries() > ind) 
    {
        caloTrk = (CEPRawCaloClusterTrack*) fCEPRawCaloClusterTracks->At(ind);
    }

    return caloTrk;
}

//______________________________________________________________________________
Bool_t CEPRawEventBuffer::RemoveTrack(UInt_t ind)
{
    // initialize the result track
    Bool_t done = kFALSE;
    CEPRawTrackBuffer *trk = NULL;

    if (fCEPRawTracks->GetEntries() > ind) {
        trk = (CEPRawTrackBuffer*) fCEPRawTracks->RemoveAt(ind);
        fCEPRawTracks->Compress();

        // update track counters
        fnTracks--;
        
        done = kTRUE;
    }
    return done;
}

//______________________________________________________________________________
Bool_t CEPRawEventBuffer::RemoveCaloCluster(UInt_t ind)
{
    // initialize the result track
    Bool_t done = kFALSE;
    CEPRawCaloClusterTrack *caloTrk = NULL;

    if (fCEPRawCaloClusterTracks->GetEntries() > ind) {
        caloTrk = (CEPRawCaloClusterTrack*) fCEPRawCaloClusterTracks->RemoveAt(ind);
        fCEPRawCaloClusterTracks->Compress();

        // update track counters
        fnCaloTracks--;
        
        done = kTRUE;
    }
    return done;
}

//______________________________________________________________________________
void CEPRawEventBuffer::SetEventVariables(AliESDEvent* ESDobj)
{
    // create sub-detector buffers
    fADCellBuffer = new CEPRawADCellBuffer();
    fV0Buffer     = new CEPRawV0Buffer();
    fEMCalBuffer  = new CEPRawCaloBuffer();
    fPHOSBuffer   = new CEPRawCaloBuffer();
    fFMDBuffer    = new CEPRawFMDBuffer();
    // fill the buffers
    fADCellBuffer->SetADVariables(ESDobj->GetADData()); 
    fV0Buffer->SetV0Variables(ESDobj->GetVZEROData()); 
    fEMCalBuffer->SetCaloVariables(ESDobj->GetEMCALCells()); 
    fPHOSBuffer->SetCaloVariables(ESDobj->GetPHOSCells()); 
    fFMDBuffer->SetFMDVariables(ESDobj->GetFMDData()); 
    
    // fill in the raw track information
    UInt_t nTracks = ESDobj->GetNumberOfTracks();
    for (UInt_t i(0); i<nTracks; i++)
    {
        // Initialize track
        AliESDtrack* aliTrk = NULL;
        CEPRawTrackBuffer* trk = new CEPRawTrackBuffer();
        // get a track from the ESD object
        aliTrk = (AliESDtrack*)ESDobj->GetTrack(i);
        // fill raw track buffer
        trk->SetTrackVariables(aliTrk);
        // add track to the CEPRawEventBuffer
        AddTrack(trk);
    }
    // fill in raw calo cluster information
    UInt_t nCaloTracks = ESDobj->GetNumberOfCaloClusters();
    for (UInt_t i(0); i<nCaloTracks; i++)
    {
        // Initialize calo cluster
        AliESDCaloCluster* aliCluster = NULL;
        CEPRawCaloClusterTrack* caloTrk = new CEPRawCaloClusterTrack();
        // get calo cluster from the ESD object
        aliCluster = (AliESDCaloCluster*)ESDobj->GetCaloCluster(i);
        // fill raw calo-cluster buffer
        caloTrk->SetCaloClusterVariables(aliCluster);
        // add track to the 
        AddTrack(caloTrk);
    }
}
  
//______________________________________________________________________________
