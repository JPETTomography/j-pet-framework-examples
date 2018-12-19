/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This program draw deltaT histograms for each strip. To each histogram a gaussian function is fited.
 * Program saved to results.txt following data: slot_ID position threshold Mean_of_Gaussian_fit Error_of_Gaussian_fit Chi_Square NDF
 *
 *  @file DeltaTFinder.cpp
 */

#include <iostream>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "DeltaTFinder.h"


using namespace std;
using namespace jpet_options_tools;

DeltaTFinder::DeltaTFinder(const char* name): JPetUserTask(name){}

bool DeltaTFinder::init()
{

  INFO("DeltaT extraction started.");

  fOutputEvents = new JPetTimeWindow("JPetHit");

  fBarrelMap = new JPetGeomMapping( getParamBank() );

  if (fSaveControlHistos) {
    // create histograms for time differences at each slot and each threshold
    for (auto & scin : getParamBank().getScintillators()) {
      for (int thr = 1; thr <= 4; thr++) {
        const char* histo_name = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_");
        getStatistics().createHistogram( new TH1F(histo_name, histo_name, 400, -20., 20.) );
      }
    }
    // create histograms for time diffrerence vs slot ID
    for (auto & layer : getParamBank().getLayers()) {
      for (int thr = 1; thr <= 4; thr++) {
        const char* histo_name = Form("TimeDiffVsID_layer_%d_thr_%d", (int)fBarrelMap->getLayerNumber(*layer.second), thr);
        const char* histo_titile = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name);
        int n_slots_in_layer = fBarrelMap->getSlotsCount(*layer.second);
        getStatistics().createHistogram( new TH2F(histo_name, histo_titile, n_slots_in_layer, 0.5, n_slots_in_layer + 0.5,
                                         120, -20., 20.) );
      }
    }
  }

  std::string file_path = "";

  if (isOptionSet(fParams.getOptions(), fInput_file_key )) {
    file_path = getOptionAsString(fParams.getOptions(), fInput_file_key );
  }

  std::string sPos = "";

  if ( isOptionSet(fParams.getOptions(), fNumberOfPositionsKey ) ) {
    sPos = getOptionAsString(fParams.getOptions(), fNumberOfPositionsKey);
  }

  int positions = std::stoi(sPos);

  for (int i = 1; i <= positions; i++) {
    std::string pos = fPosition;
    pos += boost::lexical_cast<std::string>(i);
    pos += "_std::string";
    if (isOptionSet(fParams.getOptions(), pos)) {
      auto res = retrievePositionAndFileName(getOptionAsString(fParams.getOptions(), pos));
      file_path = JPetCommonTools::extractFileNameFromFullPath( file_path );      
      file_path = JPetCommonTools::stripFileNameSuffix( file_path );      
      if ( file_path == res.second )
        fPos = res.first;
    }
  }

  if (isOptionSet(fParams.getOptions(),  fOutputPath_key ))
    fOutputPath = getOptionAsString(fParams.getOptions(),  fOutputPath_key );

  if (isOptionSet(fParams.getOptions(), fVelocityCalibFile_key ) )
    fOutputVelocityCalibName = getOptionAsString(fParams.getOptions(),  fVelocityCalibFile_key );

  return true;
}

const char* DeltaTFinder::formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix = "")
{
  int slot_number = fBarrelMap->getSlotNumber(slot);
  int layer_number = fBarrelMap->getLayerNumber(slot.getLayer());
  return Form("%slayer_%d_slot_%d_thr_%d",
              prefix,
              layer_number,
              slot_number,
              threshold
             );
}

bool DeltaTFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint nhits = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < nhits; ++i) {
      fillHistosForHit( dynamic_cast<const JPetHit&>(timeWindow->operator[](i)) );
    }
  } else {
    return false;
  }
  return true;

}

bool DeltaTFinder::terminate()
{
  std::ofstream outStream;
  outStream.open( (fOutputPath + fOutputVelocityCalibName).c_str() , std::ios_base::app);
  std::map<int, char> thresholdConversionMap;
  thresholdConversionMap[1] = 'a';
  thresholdConversionMap[2] = 'b';
  thresholdConversionMap[3] = 'c';
  thresholdConversionMap[4] = 'd';

  TString results_folder_name = (fOutputPath + "Results/position_" + boost::lexical_cast<std::string>(fPos)).c_str();
  int res = system("mkdir -p " + results_folder_name);
  if (res == -1) {
    ERROR("Error while creating the folder:" +results_folder_name);
    return false;
  }

  for (auto & slot : getParamBank().getBarrelSlots()) {
    for (int thr = 1; thr <= 4; thr++) {
      const char* histo_name = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_");
      TH1F* histoToSave = getStatistics().getHisto1D(histo_name);
      int highestBin = histoToSave->GetBinCenter( histoToSave->GetMaximumBin() );
      histoToSave->Fit("gaus", "", "", highestBin - fRangeAroundMaximumBin, highestBin + fRangeAroundMaximumBin);
      TCanvas* c = new TCanvas();
      histoToSave->Draw();
      std::string sHistoName = (std::string)results_folder_name;
      sHistoName += "/" + std::string(histo_name) + "_position_" + boost::lexical_cast<std::string>(fPos) + ".png";
      c->SaveAs( (sHistoName).c_str() );
      if ( histoToSave->GetEntries() != 0 ) {
        TF1* fit = histoToSave->GetFunction("gaus");
        outStream << slot.first << "\t" << fPos << "\t" << thresholdConversionMap[thr] << "\t" << fit->GetParameter(1)
                  << "\t" << fit->GetParError(1) << "\t" << fit->GetChisquare() << "\t" << fit->GetNDF() << std::endl;
      }

    }
  }
  outStream.close();
  delete fBarrelMap;

  INFO("DeltaT extraction ended.");

  return true;
}

void DeltaTFinder::fillHistosForHit(const JPetHit& hit)
{
  
  auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  for (auto & thr_time_pair : lead_times_A) {
    int thr = thr_time_pair.first;
    if ( lead_times_B.count(thr) > 0 ) { // if there was leading time at the same threshold at opposite side
      double timeDiffAB = lead_times_A[thr] - lead_times_B[thr];
      timeDiffAB /= 1000.; // we want the plots in ns instead of ps
      // fill the appropriate histogram
      const char* histo_name = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_");
      getStatistics().getHisto1D(histo_name)->Fill( timeDiffAB );

      // fill the timeDiffAB vs slot ID histogram
      int layer_number = fBarrelMap->getLayerNumber( hit.getBarrelSlot().getLayer() );
      int slot_number = fBarrelMap->getSlotNumber( hit.getBarrelSlot() );
      getStatistics().getHisto2D(Form("TimeDiffVsID_layer_%d_thr_%d", layer_number, thr))->Fill( slot_number,
          timeDiffAB);
    }
  }
}


std::vector<std::string> DeltaTFinder::split(const std::string inString)
{
  std::istringstream iss(inString);
  using StrIt = std::istream_iterator<std::string>;
  std::vector<std::string> container {StrIt{iss}, StrIt{}};
  return container;
}

std::pair<int, std::string> DeltaTFinder::retrievePositionAndFileName(const std::string inString)
{
  auto res = split(inString);
  if (res.size() != 2) {
    ERROR( "Problem with reading options file line: " + inString );
    return std::make_pair(-1, "");
  } else {
    auto pos = std::stoi(res[0]);
    return std::make_pair(pos, res[1]);
  }

}
