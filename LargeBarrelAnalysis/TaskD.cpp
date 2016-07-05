/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file TaskD.cpp
 */

#include "./TaskD.h"
#include "JPetWriter/JPetWriter.h"
#include <iostream>

//ClassImp(TaskD);

TaskD::TaskD(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskD::init(const JPetTaskInterface::Options& opts)
{
  fBarrelMap.buildMappings(getParamBank());

  // create histograms for time differences at each slot and each threshold
  for(auto & scin : getParamBank().getScintillators()){
    for (int thr=1;thr<=4;thr++){
      const char * histo_name = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_");
      getStatistics().createHistogram( new TH1F(histo_name, histo_name, 2000, -20., 20.) );
    }
  }

  // create histograms for time diffrerence vs slot ID
  for(auto & layer : getParamBank().getLayers()){
    for (int thr=1;thr<=4;thr++){
      const char * histo_name = Form("TimeDiffVsID_layer_%d_thr_%d", fBarrelMap.getLayerNumber(*layer.second), thr);
      const char * histo_titile = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name); 
      
      int n_slots_in_layer = fBarrelMap.getNumberOfSlots(*layer.second);
      
      getStatistics().createHistogram( new TH2F(histo_name, histo_titile, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
						120, -20., 20.) );
    }
  }
}


void TaskD::exec()
{
  // A dummy analysis example:
  JPetHit hit = (JPetHit&) (*getEvent());
  fillHistosForHit(hit);
  fWriter->write(hit);
}


void TaskD::terminate()
{
  // save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
  // so that they are available to the consecutive modules
  getAuxilliaryData().createMap("timeDiffAB mean values");
  
  for(auto & slot : getParamBank().getBarrelSlots()){
    for (int thr=1;thr<=4;thr++){
      const char * histo_name = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_");
      double mean = getStatistics().getHisto1D(histo_name).GetMean();
      getAuxilliaryData().setValue("timeDiffAB mean values", histo_name, mean);
    }
  }
  
}

void TaskD::fillHistosForHit(const JPetHit & hit){
  
  std::map<int,double> lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  std::map<int,double> lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);

  for(auto & thr_time_pair : lead_times_A){
    int thr = thr_time_pair.first;
    if( lead_times_B.count(thr) > 0 ){ // if there was leading time at the same threshold at opposite side
      double timeDiffAB = lead_times_A[thr] - lead_times_B[thr];
      timeDiffAB /= 1000.; // we want the plots in ns instead of ps
      // fill the appropriate histogram
      const char * histo_name = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_");
      getStatistics().getHisto1D(histo_name).Fill( timeDiffAB );
      // fill the timeDiffAB vs slot ID histogram
      int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
      int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
      getStatistics().getHisto2D(Form("TimeDiffVsID_layer_%d_thr_%d", layer_number, thr)).Fill( slot_number,
												timeDiffAB);
    }
  }

}

const char * TaskD::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){

  int slot_number = fBarrelMap.getSlotNumber(slot);
  int layer_number = fBarrelMap.getLayerNumber(slot.getLayer()); 
  
  return Form("%slayer_%d_slot_%d_thr_%d",
	      prefix,
	      layer_number,
	      slot_number,
	      threshold
	      );
}
