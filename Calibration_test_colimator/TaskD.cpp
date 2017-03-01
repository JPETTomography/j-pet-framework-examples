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


#include <cstdlib>
#include <string>
#include <sstream>
#include <cctype>
#include <JPetWriter/JPetWriter.h>
#include "TaskD.h"
#include <TFile.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>     // std::cout
#include <fstream> 

using namespace std;


TaskD::TaskD(const char * name, const char * description):JPetTask(name, description){}

void TaskD::init(const JPetTaskInterface::Options&){

	fBarrelMap.buildMappings(getParamBank());

// create histograms for time differences at each slot and each threshold

	auto scints = getParamBank().getScintillators(); 

//open Input_file with callibration
std::ifstream Input_file("/home/mskurzok/j-pet-framework-examples/Calibration_test/TimeConstants.txt");

//check whether file can be opened
if( Input_file.good() )
{
    cout<<":-)"<<endl;
} else
{
    cout<<":-("<<endl;
} 


  int layer_nr;
  int slot_nr; 
  int thr_nr; 
  char side;

  double offset_L;
  double offset_T;

  double delta_offset_L;
  double delta_offset_T;

  double sigma_offset_L;
  double sigma_offset_T;
 
  double chi2_offset_L;
  double chi2_offset_T;


while(Input_file>>layer_nr>>slot_nr>>side >>thr_nr>>offset_L>> delta_offset_L>> offset_T>> delta_offset_T>> sigma_offset_L>> sigma_offset_T>> chi2_offset_L>> chi2_offset_T){


 	if(side=='A'){

		if(layer_nr==1){

		corrA_L[layer_nr][slot_nr][thr_nr]=offset_L;
		corrA_T[layer_nr][slot_nr][thr_nr]=offset_T;
		}

		if(layer_nr==2){

		corrA_L[layer_nr][slot_nr-48][thr_nr]=offset_L;
		corrA_T[layer_nr][slot_nr-48][thr_nr]=offset_T;
		
		}


	}

	if(side=='B'){

		if(layer_nr==1){	

		corrB_L[layer_nr][slot_nr][thr_nr]=offset_L;
		corrB_T[layer_nr][slot_nr][thr_nr]=offset_T;
		}

		if(layer_nr==2){

		corrB_L[layer_nr][slot_nr-48][thr_nr]=offset_L;
		corrB_T[layer_nr][slot_nr-48][thr_nr]=offset_T;
		
		}

	}

}

Input_file.close();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

for (const auto & scin: scints) {

		for (int thr=1;thr<=4;thr++){//over threshold

//histos for leading edge
			const char * histo_name_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().createHistogram( new TH1F(histo_name_l, histo_name_l, 400, -20., 20.) );
//histograms for leading edge refference detector time difference
                        const char * histo_name_Ref_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_leading_");
			getStatistics().createHistogram( new TH1F(histo_name_Ref_l, histo_name_Ref_l, 600,-30.,30.) );
			
//histos for trailing edge			
			const char * histo_name_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().createHistogram( new TH1F(histo_name_t, histo_name_t, 400, -20., 20.) );
//histograms for leading edge refference detector time difference
                        const char * histo_name_Ref_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_trailing_");
			getStatistics().createHistogram( new TH1F(histo_name_Ref_t, histo_name_Ref_t, 600,-30.,30.) );
		}
	}


// create histograms for time diffrerence vs slot ID

	auto layers = getParamBank().getLayers();

	for(const auto& layer: layers) {

		for (int thr=1;thr<=4;thr++){

//histos for leading edge
//time difference
			const char * histo_name_l = Form("TimeDiffVsID_layer%d_thr%d_leading", fBarrelMap.getLayerNumber(*layer.second), thr);
			const char * histo_titile_l = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name_l);
			

//histos for trailing edge
//time difference
			const char * histo_name_t = Form("TimeDiffVsID_layer%d_thr%d_trailing", fBarrelMap.getLayerNumber(*layer.second), thr);
			const char * histo_titile_t = Form("%s;Slot ID; TimeDiffAB [ns]", histo_name_t);


			int n_slots_in_layer = fBarrelMap.getNumberOfSlots(*layer.second);
//time difference
			getStatistics().createHistogram( new TH2F(histo_name_l, histo_titile_l, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
								  120, -20., 20.) );

			getStatistics().createHistogram( new TH2F(histo_name_t, histo_titile_t, n_slots_in_layer, 0.5, n_slots_in_layer+0.5,
								  120, -20., 20.) );


		
		}
	}


}

void TaskD::exec(){
	//getting the data from event in propriate format
	if(auto hit =dynamic_cast<const JPetHit*const>(getEvent())){
		fillHistosForHit(*hit);
		fWriter->write(*hit);
	}
}


void TaskD::terminate(){
	// save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
	// so that they are available to the consecutive modules
	getAuxilliaryData().createMap("timeDiffAB mean values");

	for(auto & slot : getParamBank().getBarrelSlots()){

		for (int thr=1;thr<=4;thr++){

			const char * histo_name_l = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_leading_");
			double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_l, mean_l);

			TH1F* histoToSave_leading = &(getStatistics().getHisto1D(histo_name_l));

			const char * histo_name_t = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_trailing_");
			double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_t, mean_t);

			TH1F* histoToSave_trailing = &(getStatistics().getHisto1D(histo_name_t));
		}
	}
	
}

void TaskD::fillHistosForHit(const JPetHit & hit){

//get time for leading edge
	auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
//get time for trailing edge
	auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

	auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
        auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);



	for(auto & thr_time_pair : lead_times_A){

		int thr = thr_time_pair.first;
		if( lead_times_B.count(thr) > 0 ){ // if there was leading time at the same threshold at opposite side

			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer());
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot());
		
//time correction for A and B
			double timeDiffAB_l = (lead_times_B[thr]/1000.-corrB_L[layer_number][slot_number][thr]) - (lead_times_A[thr]/1000.-corrA_L[layer_number][slot_number][thr]);


			// fill the appropriate histogram
			const char * histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().getHisto1D(histo_name_l).Fill(timeDiffAB_l);


			// fill the timeDiffAB vs slot ID histogram
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_leading", layer_number, thr)).Fill(slot_number, timeDiffAB_l);



		}
}



	for(auto & thr_time_pair : trail_times_A){
		int thr = thr_time_pair.first;

		if( trail_times_B.count(thr) > 0 ){ // if there was trailing time at the same threshold at opposite side

			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );

			double timeDiffAB_t = (trail_times_B[thr]/1000. -corrB_T[layer_number][slot_number][thr])- (trail_times_A[thr]/1000.-corrA_T[layer_number][slot_number][thr]);


			// fill the appropriate histogram
			const char * histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().getHisto1D(histo_name_t).Fill( timeDiffAB_t);

			// fill the timeDiffAB vs slot ID histogram
			
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_trailing", layer_number, thr)).Fill( slot_number, timeDiffAB_t);

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
void TaskD::setWriter(JPetWriter* writer){fWriter =writer;}
