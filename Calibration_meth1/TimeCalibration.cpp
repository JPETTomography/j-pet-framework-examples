/**ifstream
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
	//
	std::ofstream output;
	output.open(OutputFile,std::ios::app); //file will be overwritten           
	 if(output.tellp()==0){
	  output << "# Time calibration constants" << std::endl;
	  output << "# For side A we apply only the correction from refference detector, for side B the correction is equal to the sum of the A-B" << std::endl;
          output << "# correction and offset with respect to the refference detector. For side A we report the sigmas and chi2/ndf for fit to the time difference spectra with refference detector" << std::endl;
	  output << "# while the same quality variables for fits to the A-B time difference are given for B side section" << std::endl;
	  output << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | offset_value_leading | offset_uncertainty_leading | offset_value_trailing | offset_uncertainty_trailing | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing" << std::endl;
	  output << "# Calibration started on "<< __DATE__ << " at " << __TIME__ << std::endl;
	  }
	  else{
	    output << "# Calibration started on "<< __DATE__ << " at " << __TIME__ << std::endl;
	  output.close();
	  }
	//
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
//
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

//getting Layer and Slots numbers from analysing files name
std::vector<int> vectorOfNumbers;
std::string str(gDirectory->GetFile()->GetName());//getting directory of analysing file

int number=0;

 for (unsigned int i=str.size()-1;i>0; i--){

	if (isdigit(str[i])){

          std::stringstream ss;
          ss<<str[i];
          ss>>number; //convert string into int and store it in "asInt"

	  //std::cout << "number" << number << std::endl;
         vectorOfNumbers.push_back(number);

	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
	// so that they are available to the consecutive modules
	getAuxilliaryData().createMap("timeDiffAB mean values");
//
//create output txt file with calibration parameters 
//
	std::ofstream results_fit;
        results_fit.open(OutputFile, std::ios::app); //file will be overwritten

	for(auto & slot : getParamBank().getBarrelSlots()){

		for (int thr=1;thr<=4;thr++){
//scintillators
			const char * histo_name_l = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_leading_");
			double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_l, mean_l);

			TH1F* histoToSave_leading = &(getStatistics().getHisto1D(histo_name_l));

			const char * histo_name_t = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffAB_trailing_");
			double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_t, mean_t);

			TH1F* histoToSave_trailing = &(getStatistics().getHisto1D(histo_name_t));
//reference detector
			const char * histo_name_Ref_l = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffRef_leading_");
			double mean_Ref_l = getStatistics().getHisto1D(histo_name_Ref_l).GetMean();
			getAuxilliaryData().setValue("timeDiffRef mean values", histo_name_Ref_l, mean_Ref_l);

			TH1F* histoToSave_Ref_leading = &(getStatistics().getHisto1D(histo_name_Ref_l));

			const char * histo_name_Ref_t = formatUniqueSlotDescription(*(slot.second), thr, "timeDiffRef_trailing_");
			double mean_Ref_t = getStatistics().getHisto1D(histo_name_Ref_t).GetMean();
			getAuxilliaryData().setValue("timeDiffref mean values", histo_name_Ref_t, mean_Ref_t);

			TH1F* histoToSave_Ref_trailing = &(getStatistics().getHisto1D(histo_name_Ref_t));
//non zero histos 
// slot.first - ID
// slot.second - wskaznik na JPetBarrelSlot
//save fit parameters only for layerX and SlotY (taken from analysing files name)
//fit just for proper slot
			if(histoToSave_leading->GetEntries() != 0 && histoToSave_trailing->GetEntries() != 0
                          &&  (slot.second)->getLayer().getID()==vectorOfNumbers[2]
			   && (slot.first==(10*vectorOfNumbers[1]+vectorOfNumbers[0]))){

//std::cout << "layer=" << vectorOfNumbers[2] << std::endl;
//std::cout << "slot=" << 10*vectorOfNumbers[1]+vectorOfNumbers[0] << std::endl;

//fit scintilators
			int highestBin_l = histoToSave_leading->GetBinCenter(histoToSave_leading->GetMaximumBin());
			histoToSave_leading->Fit("gaus","","", highestBin_l-5, highestBin_l+5);
			histoToSave_leading->Draw();
		
			int highestBin_t = histoToSave_trailing->GetBinCenter(histoToSave_trailing->GetMaximumBin());
			histoToSave_trailing->Fit("gaus","","", highestBin_t-5, highestBin_t+5);
			histoToSave_trailing->Draw();

		
			TF1 *fit_l = histoToSave_leading->GetFunction("gaus");
			TF1 *fit_t = histoToSave_trailing->GetFunction("gaus");

			double position_peak_l = fit_l->GetParameter(1);
   			double position_peak_error_l=fit_l->GetParError(1);
			double sigma_peak_l =fit_l->GetParameter(2);
			double chi2_ndf_l = fit_l->GetChisquare()/fit_l->GetNDF();

			double position_peak_t = fit_t->GetParameter(1);
   			double position_peak_error_t=fit_t->GetParError(1);
			double sigma_peak_t =fit_t->GetParameter(2);
			double chi2_ndf_t = fit_t->GetChisquare()/fit_t->GetNDF();
//fit reference detector

			int highestBin_Ref_l = histoToSave_Ref_leading->GetBinCenter(histoToSave_Ref_leading->GetMaximumBin());
			histoToSave_Ref_leading->Fit("gaus","","", highestBin_Ref_l-5, highestBin_Ref_l+5); //range for fit
			TF1 *fit_Ref_l = histoToSave_Ref_leading->GetFunction("gaus");
			fit_Ref_l->SetRange(highestBin_Ref_l-100, highestBin_Ref_l+100); //range to draw gaus function
			histoToSave_Ref_leading->Draw();


			int highestBin_Ref_t = histoToSave_Ref_trailing->GetBinCenter(histoToSave_Ref_trailing->GetMaximumBin());
			histoToSave_Ref_trailing->Fit("gaus","","", highestBin_Ref_t-5, highestBin_Ref_t+5); //range for fit
 			TF1 *fit_Ref_t = histoToSave_Ref_trailing->GetFunction("gaus");
			fit_Ref_t->SetRange(highestBin_Ref_t-100, highestBin_Ref_t+100); //range to draw gaus function
			histoToSave_Ref_trailing->Draw();


			double position_peak_Ref_l = fit_Ref_l->GetParameter(1);
   			double position_peak_error_Ref_l=fit_Ref_l->GetParError(1);
			double sigma_peak_Ref_l =fit_Ref_l->GetParameter(2);
			double chi2_ndf_Ref_l = fit_Ref_l->GetChisquare()/fit_Ref_l->GetNDF();

			double position_peak_Ref_t = fit_Ref_t->GetParameter(1);
   			double position_peak_error_Ref_t=fit_Ref_t->GetParError(1);
			double sigma_peak_Ref_t =fit_Ref_t->GetParameter(2);
			double chi2_ndf_Ref_t = fit_Ref_t->GetChisquare()/fit_Ref_t->GetNDF();

// writing to apropriate format (txt file)
//side A
//AB offset=0, offset just from reference detector
			results_fit << (slot.second)->getLayer().getID() << "\t" <<  slot.first << "\t" << "A" << "\t" << thr << "\t" << position_peak_Ref_l << "\t" << position_peak_error_Ref_l  << "\t" << position_peak_Ref_t << "\t" << position_peak_error_Ref_t  << "\t" << sigma_peak_Ref_l << "\t" <<sigma_peak_Ref_t << "\t"  <<chi2_ndf_Ref_l << "\t" <<chi2_ndf_Ref_t <<std::endl;

//side B
//AB offset!=0, offset from reference detector: total offset AB offset + Ref Det offset
                        results_fit << (slot.second)->getLayer().getID() << "\t" <<  slot.first << "\t" << "B" << "\t" << thr << "\t" << position_peak_Ref_l+position_peak_l << "\t" << sqrt(pow(position_peak_error_Ref_l,2) + pow(position_peak_error_l,2))  << "\t" << position_peak_Ref_t+ position_peak_t << "\t" << sqrt(pow(position_peak_error_Ref_t,2)+pow(position_peak_error_t,2)) << "\t" << sigma_peak_l << "\t" << sigma_peak_t <<"\t" << chi2_ndf_l << "\t" <<chi2_ndf_t <<std::endl;

			}
			
		}
	}
	results_fit.close();
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

			double timeDiffAB_l = lead_times_B[thr] - lead_times_A[thr];
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
//taken minimum time difference between Ref and Scint
			double timeDiffLmin=10000000000000.;
			for(unsigned int i = 0; i < fRefTimesL.size();i++) { 
			double timeDiffHit_L = (lead_times_A[thr] + lead_times_B[thr])/2. -fRefTimesL[i];
			 timeDiffHit_L = timeDiffHit_L/1000.;//ps -> ns
			 if(fabs(timeDiffHit_L) < timeDiffLmin){
			  timeDiffLmin = timeDiffHit_L;
			 }
			}
				const char * histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
				getStatistics().getHisto1D(histo_name_Ref_l).Fill(timeDiffLmin);
		}
	}



//trailing
	for(auto & thr_time_pair : trail_times_A){
		int thr = thr_time_pair.first;

		if( trail_times_B.count(thr) > 0 ){ // if there was trailing time at the same threshold at opposite side

			double timeDiffAB_t = trail_times_B[thr] - trail_times_A[thr];
			timeDiffAB_t/= 1000.; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().getHisto1D(histo_name_t).Fill( timeDiffAB_t);

			// fill the timeDiffAB vs slot ID histogram
			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_trailing", layer_number, thr)).Fill( slot_number, timeDiffAB_t);
															
//Assuming that the refference signal is always the first one we take the first hit after the hit in the refference detector                                                                              
//Here we take advantage of the fact that the hits are somehow ordered in time, which is true for now if this is not true we
//have to insert this part of the code (and the one on the beginning of this function outside this function and create a vecor
//of hits and then for each refference hit look for the next hit in time. 
//taken minimal time difference between Ref and Scint
			double timeDiffTmin=10000000000000.;
			for(unsigned int i = 0; i < fRefTimesT.size();i++) { 
                            double timeDiffHit_T = (trail_times_A[thr] + trail_times_B[thr])/2. -fRefTimesT[i];
			    timeDiffHit_T = timeDiffHit_T/1000.; //ps->ns
			  if(fabs(timeDiffHit_T) < timeDiffTmin){
			    timeDiffTmin = timeDiffHit_T;
			  }
			}
			  const char * histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
			  getStatistics().getHisto1D(histo_name_Ref_t).Fill(timeDiffTmin);	
	        }
	}
}


const char * TimeCalibration::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){

	int slot_number = fBarrelMap.getSlotNumber(slot);
	int layer_number = fBarrelMap.getLayerNumber(slot.getLayer()); 

	return Form("%slayer_%d_slot_%d_thr_%d",prefix,layer_number,slot_number,threshold);

}
void TimeCalibration::setWriter(JPetWriter* writer){fWriter =writer;}
