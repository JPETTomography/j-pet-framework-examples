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
 *  @file TimeCalibration.cpp
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cctype>
#include <JPetWriter/JPetWriter.h>
#include "TimeCalibration.h"
#include "TF1.h"
#include "TString.h"
#include <TDirectory.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

TimeCalibration::TimeCalibration(const char * name, const char * description):JPetTask(name, description){}

void TimeCalibration::init(const JPetTaskInterface::Options& opts){

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	fBarrelMap.buildMappings(getParamBank());

// create histograms for time differences at each slot and each threshold

	auto scints = getParamBank().getScintillators();

	for (const auto & scin: scints) {

		for (int thr=1;thr<=4;thr++){//over threshold

//histos for leading edge
			const char * histo_name_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().createHistogram( new TH1F(histo_name_l, histo_name_l, 100, -20., 20.) );
//histograms for leading edge refference detector time difference
                        const char * histo_name_Ref_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_leading_");
			getStatistics().createHistogram( new TH1F(histo_name_Ref_l, histo_name_Ref_l, 100, -2000., 2000.) );
			
//histos for trailing edge			
			const char * histo_name_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().createHistogram( new TH1F(histo_name_t, histo_name_t, 100, -20., 20.) );
//histograms for leading edge refference detector time difference
                        const char * histo_name_Ref_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_trailing_");
			getStatistics().createHistogram( new TH1F(histo_name_Ref_t, histo_name_Ref_t, 100, -2000., 2000.) );
		}
	}


// create histograms for time diffrerence vs slot ID

	auto layers = getParamBank().getLayers();

	for(const auto& layer: layers) {

		for (int thr=1;thr<=4;thr++){

//histos for leading edge
			const char * histo_name_l = Form("TimeDiffVsID_layer%d_thr%d_leading", fBarrelMap.getLayerNumber(*layer.second), thr);
			const char * histo_titile_l = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name_l);

//histos for trailing edge
			const char * histo_name_t = Form("TimeDiffVsID_layer%d_thr%d_trailing", fBarrelMap.getLayerNumber(*layer.second), thr);
			const char * histo_titile_t = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name_t);


			int n_slots_in_layer = fBarrelMap.getNumberOfSlots(*layer.second);

			getStatistics().createHistogram( new TH2F(histo_name_l, histo_titile_l, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
								  120, -20., 20.) );

			getStatistics().createHistogram( new TH2F(histo_name_t, histo_titile_t, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
								  120, -20., 20.) );
		
		}
	}


}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TimeCalibration::exec(){
  double RefTimeLead[4]={-1.e43,-1.e43,-1.e43,-1.e43};
  double RefTimeTrail[4]={-1.e43,-1.e43,-1.e43,-1.e43};

  const int kPMidRef = 385;


	//getting the data from event in propriate format
	if(auto hit =dynamic_cast<const JPetHit*const>(getEvent())){
     	  //
	  //taking refference detector hits times (scin=193)
	  //
	  int PMid = hit->getSignalB().getRecoSignal().getRawSignal().getPM().getID();
	
	  if(PMid==kPMidRef){
	    auto lead_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	    auto trail_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);                                                   
	    for(auto & thr_time_pair : lead_times_B){
	      int thr = thr_time_pair.first;
	      RefTimeLead[thr] = thr_time_pair.second;

	    }
	    for(auto & thr_time_pair : trail_times_B){
	      int thr = thr_time_pair.first;
	      RefTimeTrail[thr] = thr_time_pair.second;
	      
	    }
	    fRefTimesL.push_back(RefTimeLead[1]);
	    fRefTimesT.push_back(RefTimeTrail[1]);
	  }
	  
	  if (fhitsCalib.empty()) {
	       fhitsCalib.push_back(*hit);  
	  } else {
	    if (fhitsCalib[0].getTimeWindowIndex() == hit->getTimeWindowIndex()) {
		fhitsCalib.push_back(*hit);
	    } else {
	      for (auto i = fhitsCalib.begin(); i != fhitsCalib.end(); i++) {
		fillHistosForHit(*i,fRefTimesL,fRefTimesT);
              }	
	        fhitsCalib.clear();
		fRefTimesL.clear();
		fRefTimesT.clear();
	        fhitsCalib.push_back(*hit);

//If the first hit in the next time window is the refference detector hit save its times
		if(PMid==kPMidRef){
		  auto lead_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
		  auto trail_times_B = hit->getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
		  for(auto & thr_time_pair : lead_times_B){
		    int thr = thr_time_pair.first;
		    RefTimeLead[thr] = thr_time_pair.second;
		  }
		  for(auto & thr_time_pair : trail_times_B){
		    int thr = thr_time_pair.first;
		    RefTimeTrail[thr] = thr_time_pair.second;
		  }
		  fRefTimesL.push_back(RefTimeLead[1]);
		  fRefTimesT.push_back(RefTimeTrail[1]);
		}
	    }
	  }



	  
      	  if(PMid==kPMidRef){
	      fhitsCalib.push_back(*hit);
	  fWriter->write(*hit);
	}	
	

	}  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TimeCalibration::terminate(){


std::vector<int> vectorOfNumbers;
std::string str(gDirectory->GetFile()->GetName());//getting directory of analysing file


}


//////////////////////////////////

void TimeCalibration::fillHistosForHit(const JPetHit & hit, const std::vector<double>  & fRefTimesL, const std::vector<double> & fRefTimesT){

	auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

	auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
        auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
	

//leading edge

	for(auto & thr_time_pair : lead_times_A){

		int thr = thr_time_pair.first;

		if( lead_times_B.count(thr) > 0 ){ // if there was leading time at the same threshold at opposite side

			double timeDiffAB_l = lead_times_A[thr] - lead_times_B[thr];
			timeDiffAB_l/= 1000.; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().getHisto1D(histo_name_l).Fill( timeDiffAB_l);

			// fill the timeDiffAB vs slot ID histogram

			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_leading", layer_number, thr)).Fill( slot_number,
															timeDiffAB_l);
//Assuming that the refference signal is always the first one we take the first hit after the hit in the refference detector
//Here we take advantage of the fact that the hits are somehow ordered in time, which is true for now if this is not true we
//have to insert this part of the code (and the one on the beginning of this function outside this function and create a vecor
//of hits and then for each refference hit look for the next hit in time
			for(int i = 0; i < fRefTimesL.size();i++) {
			double timeDiffHit_L = (lead_times_A[thr] + lead_times_B[thr])/2. -fRefTimesL[i];
			timeDiffHit_L = timeDiffHit_L/1000000.;//ps -> micros
			const char * histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
			getStatistics().getHisto1D(histo_name_Ref_l).Fill(timeDiffHit_L);
			
			}
		}
	}


//trailing
	for(auto & thr_time_pair : trail_times_A){
		int thr = thr_time_pair.first;

		if( trail_times_B.count(thr) > 0 ){ // if there was trailing time at the same threshold at opposite side

			double timeDiffAB_t = trail_times_A[thr] - trail_times_B[thr];
			timeDiffAB_t/= 1000.; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().getHisto1D(histo_name_t).Fill( timeDiffAB_t);

			// fill the timeDiffAB vs slot ID histogram
			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_trailing", layer_number, thr)).Fill( slot_number,
															 timeDiffAB_t);
//Assuming that the refference signal is always the first one we take the first hit after the hit in the refference detector                                                                              
//Here we take advantage of the fact that the hits are somehow ordered in time, which is true for now if this is not true we
//have to insert this part of the code (and the one on the beginning of this function outside this function and create a vecor
//of hits and then for each refference hit look for the next hit in time. 
			for(int i = 0; i < fRefTimesT.size();i++) {
                          double timeDiffHit_T = (trail_times_A[thr] + trail_times_B[thr])/2. -fRefTimesT[i];
			  timeDiffHit_T = timeDiffHit_T/1000000.; //ps->micros
			  const char * histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
			  getStatistics().getHisto1D(histo_name_Ref_t).Fill(timeDiffHit_T);
			  
			  }	
	        }
	}
}


const char * TimeCalibration::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){

	int slot_number = fBarrelMap.getSlotNumber(slot);
	int layer_number = fBarrelMap.getLayerNumber(slot.getLayer()); 

	return Form("%slayer_%d_slot_%d_thr_%d",prefix,layer_number,slot_number,threshold);

}
void TimeCalibration::setWriter(JPetWriter* writer){fWriter =writer;}

