#include <TTree.h>
#include <TFile.h>

// #include "framework-includes/JPetGeantEventInformation.h"
// #include "framework-includes/JPetGeantDecayTree.h"
// #include "framework-includes/JPetGeantEventPack.h"
// #include "framework-includes/JPetGeantScinHits.h"

#include "JPetGeantEventInformation/JPetGeantEventInformation.h"
#include "JPetGeantDecayTree/JPetGeantDecayTree.h"
#include "JPetGeantEventPack/JPetGeantEventPack.h"
#include "JPetGeantScinHits/JPetGeantScinHits.h"

int convert(const std::string inputFile, const std::string outputName = "test"){
  
  Int_t bufsize = 32000;
  Int_t splitlevel = 2; 

  // output file
  TString outName = outputName+".mcGeant.root";
  TFile* out_file = new TFile(outName, "RECREATE");
  
  JPetGeantEventPack* event_pack = new JPetGeantEventPack();
  TTree* T = new TTree("T", "GATE output", splitlevel);
  T->Branch("eventPack", &event_pack, bufsize, splitlevel);

  Long64_t entry = 0;

  std::ifstream gateFile;
  gateFile.open(inputFile.c_str());

  if(!gateFile.good())
  {
    std::cout <<"Cannot find input file\n";
    return 1;
  }

  double x1 = 0, y1 = 0, z1 = 0, t1, E_dep1 = 0, emissionX1 = 0, emissionY1 = 0, emissionZ1 = 0;
  int ID1 = 0, coincidenceType = 0;
  double x2 = 0, y2 = 0, z2 = 0, t2, E_dep2 = 0, emissionX2 = 0, emissionY2 = 0, emissionZ2 = 0;
  int ID2 = 0;

  while (gateFile >> x1 >> y1 >> z1 >> t1 >> x2 >> y2 >> z2 >> t2 >> ID1 >> ID2 >> E_dep1 >> E_dep2 >> coincidenceType >> emissionX1 >> emissionY1 >> emissionZ1 >> emissionX2 >> emissionY2 >> emissionZ2 )
  {
    /************************************************************************/
    /* Inside event loop                                                    */
    /************************************************************************/
    if( entry % 1000 )
      std::cout << "Read " << entry << " lines\n";

    event_pack->SetEventNumber(entry++);
    JPetGeantEventInformation *info = event_pack->GetEventInformation();
    info->SetThreeGammaGen(false);
    info->SetTwoGammaGen(true);

    // true annihilation point
    info->SetVtxPosition(emissionX1, emissionY1, emissionZ1);
 
      info->SetMomentumGamma(1, 0,0,0); 

      JPetGeantScinHits *geantHitFirst = event_pack->ConstructNextHit();

      Int_t scintillator_id = ID1+200;  // make sure that your IDs are the same as in config file
      Double_t deposited_energy = E_dep1; // in keV!
      Double_t hit_time = t1/1E9*1E3;         // file has time in ns, converting to ps!

      geantHitFirst->Fill(event_pack->GetEventNumber(), scintillator_id, 1,
                     22, // photon particle code
                     1, deposited_energy, hit_time);
      if(coincidenceType == 1 )
        geantHitFirst->SetGenGammaMultiplicity(2); // 2 for true, 12 for detector scatter, 102 for phantom scatter
      else if(coincidenceType == 2)
        geantHitFirst->SetGenGammaMultiplicity(102); // 2 for true, 12 for detector scatter, 102 for phantom scatter
      else if(coincidenceType == 3)
        geantHitFirst->SetGenGammaMultiplicity(12); // 2 for true, 12 for detector scatter, 102 for phantom scatter
      else if(coincidenceType == 4)
        geantHitFirst->SetGenGammaMultiplicity(102); // 2 for true, 12 for detector scatter, 102 for phantom scatter

      geantHitFirst->SetHitPosition(x1, y1, z1); 

      info->SetMomentumGamma(2, emissionX2, emissionY2, emissionZ2); // this is second emission point, will be different in case of random coincidences than VtxPosition

      JPetGeantScinHits *geantHitSecond = event_pack->ConstructNextHit();
      scintillator_id = ID2+200;
      deposited_energy = E_dep2; // in keV!
      hit_time = t2/1E3;         // file has time in ns, converting to ps!

      geantHitSecond->Fill(event_pack->GetEventNumber(), scintillator_id, 2,
                     22, // photon particle code
                     1, deposited_energy, hit_time);
      if(coincidenceType == 1 )
        geantHitSecond -> SetGenGammaMultiplicity(2); // 2 for true, 12 for detector scatter, 102 for phantom scatter
      else if(coincidenceType == 2)
        geantHitSecond -> SetGenGammaMultiplicity(102); // 2 for true, 12 for detector scatter, 102 for phantom scatter
      else if(coincidenceType == 3)
        geantHitSecond -> SetGenGammaMultiplicity(12); // 2 for true, 12 for detector scatter, 102 for phantom scatter
      else if(coincidenceType == 4)
        geantHitSecond -> SetGenGammaMultiplicity(102); // 2 for true, 12 for detector scatter, 102 for phantom scatter

      geantHitSecond->SetHitPosition(x2, y2, z2);

    T->Fill();
    event_pack->Clear();
    
  }
  /************************************************************************/
  /* end of event loop                                                    */
  /************************************************************************/

  T->Write();
  out_file->Close();
  gateFile.close();

 return 0; 
}
