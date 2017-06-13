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
 *  @file TaskDVelocity.cpp
 */

#include <iostream>
#include <map>
#include <JPetWriter/JPetWriter.h>
#include "TaskDVelocity.h"
#include "TF1.h"


TaskDVelocity::TaskDVelocity(const char * name, const char * description):JPetTask(name, description){}
void TaskDVelocity::init(const JPetTaskInterface::Options& opts){
	fBarrelMap.buildMappings(getParamBank());
	// create histograms for time differences at each slot and each threshold
	for(auto & scin : getParamBank().getScintillators()){
		for (int thr=1;thr<=4;thr++){
			const char * histo_name = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_");
			getStatistics().createHistogram( new TH1F(histo_name, histo_name, 400, -20., 20.) );
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
void TaskDVelocity::exec(){
	//getting the data from event in propriate format
	if(auto hit =dynamic_cast<const JPetHit*const>(getEvent())){
			fillHistosForHit(*hit);
			fWriter->write(*hit);
	}
}


void TaskDVelocity::terminate(){
	// save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
	// so that they are available to the consecutive modules
	getAuxilliaryData().createMap("timeDiffAB mean values");
	std::ofstream kiko;
	kiko.open("results.txt",std::ios_base::app);
	std::map<int, char> thresholdConversionMap;
	thresholdConversionMap[1] = 'a';
	thresholdConversionMap[2] = 'b';
	thresholdConversionMap[3] = 'c';
	thresholdConversionMap[4] = 'd';
	int position = 200;
	
	TString results_folder_name = ("Results/position_"+std::to_string(position)).c_str();
	system("mkdir -p "+ results_folder_name);

	for(auto & slot : getParamBank().getBarrelSlots()){
		for (int thr=1;thr<=4;thr++){
			const char * histo_name = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_");
			double mean = getStatistics().getHisto1D(histo_name).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name, mean);
			TH1F* histoToSave = &(getStatistics().getHisto1D(histo_name) );
			int highestBin = histoToSave->GetBinCenter( histoToSave->GetMaximumBin() );
			histoToSave->Fit("gaus","","", highestBin-2, highestBin+2);
			TCanvas* c = new TCanvas();
			histoToSave->Draw();
			std::string sHistoName = "Results/position_"+std::to_string(position)+"/"+histo_name; sHistoName+="_position_"+std::to_string(position)+".png"; 
			c->SaveAs( (sHistoName).c_str() );
			if( histoToSave->GetEntries() != 0 )
			{
				TF1 *fit = histoToSave->GetFunction("gaus");
				kiko << slot.first << "\t" << position << "\t" << thresholdConversionMap[thr] << "\t" << fit->GetParameter(1)   
				     <<"\t" << fit->GetParError(1) << "\t" << fit->GetChisquare() << "\t" << fit->GetNDF() << std::endl;
			}
			
		}
	}
	kiko.close();
	
}

void TaskDVelocity::fillHistosForHit(const JPetHit & hit){
	auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
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
/*
			if(slot_number == 25 && layer_number == 1 && thr==4)
			{
//				std::cout <<"doing shit for " << hit.getScinID() << std::endl;
				ofstream out;
				out.open("deltaT.txt",std::ios_base::app);
				out << timeDiffAB << std::endl;
				out.close();
			}
*/
		}
	}
}
const char * TaskDVelocity::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){
	int slot_number = fBarrelMap.getSlotNumber(slot);
	int layer_number = fBarrelMap.getLayerNumber(slot.getLayer()); 
	return Form("%slayer_%d_slot_%d_thr_%d",
		prefix,
		layer_number,
		slot_number,
		threshold
	);
}
void TaskDVelocity::setWriter(JPetWriter* writer){fWriter =writer;}
