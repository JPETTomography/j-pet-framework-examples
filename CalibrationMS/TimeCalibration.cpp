/*
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
#include <time.h>

using namespace std;

TimeCalibration::TimeCalibration(const char * name, const char * description):JPetTask(name, description){}

void TimeCalibration::init(const JPetTaskInterface::Options& opts){
  time_t time_calib;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	fBarrelMap.buildMappings(getParamBank());

        if (opts.count(fCalibRunKey)) {
	  CalibRun= atof(opts.at(fCalibRunKey).c_str());
        }

// create histograms for time differences at each slot and each threshold

//	auto scints = getParamBank().getScintillators();
	time(&time_calib);
	//
	std::ofstream output;
	output.open(OutputFile,std::ios::app); //file will be overwritten           
	 if(output.tellp()==0){
	  output << "# Time calibration constants" << std::endl;
	  output << "# For side A we apply only the correction from refference detector, for side B the correction is equal to the sum of the A-B" << std::endl;
          output << "# correction and offset with respect to the refference detector. For side A we report the sigmas and chi2/ndf for fit to the time difference spectra with refference detector" << std::endl;
	  output << "# while the same quality variables for fits to the A-B time difference are given for B side section" << std::endl;
	  output << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | offset_value_leading | offset_uncertainty_leading | offset_value_trailing | offset_uncertainty_trailing | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing" << std::endl;
	  output << "# Calibration started on "<< ctime(&time_calib);
	  }
	  else{
	       output << "# Calibration started on "<< ctime(&time_calib);
	  output.close();
	  }
	//
	 for (auto & scin : getParamBank().getScintillators()) {

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
  const double RefTimeLead[4]={-1.e43,-1.e43,-1.e43,-1.e43};
  const double RefTimeTrail[4]={-1.e43,-1.e43,-1.e43,-1.e43};

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
double frac_err=0.3;  //maximal fractional uncertainty accepted by calibration
int min_ev = 10;     //minimal number of events for a distribution to be fitted
 for (unsigned int i=str.size()-1;i>0; i--){

	if (std::stoi(str[i])){

          std::stringstream ss;
          ss<<str[i];
          ss>>number; //convert string into int and store it in "asInt"

	  //	  cout << "number" << number << endl;
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
        results_fit.open(OutputFile, std::ios::app); 

	for(auto & scin : getParamBank().getScintillators()){

		for (int thr=1;thr<=4;thr++){
//scintillators
		  const char * histo_name_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_leading_");
			double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_l, mean_l);
			TH1F* histoToSave_leading = &(getStatistics().getHisto1D(histo_name_l));

			const char * histo_name_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffAB_trailing_");
			double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();
			getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_t, mean_t);

			TH1F* histoToSave_trailing = &(getStatistics().getHisto1D(histo_name_t));
//reference detector
			const char * histo_name_Ref_l = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_leading_");
			double mean_Ref_l = getStatistics().getHisto1D(histo_name_Ref_l).GetMean();
			getAuxilliaryData().setValue("timeDiffRef mean values", histo_name_Ref_l, mean_Ref_l);

			TH1F* histoToSave_Ref_leading = &(getStatistics().getHisto1D(histo_name_Ref_l));

			const char * histo_name_Ref_t = formatUniqueSlotDescription(scin.second->getBarrelSlot(), thr, "timeDiffRef_trailing_");
			double mean_Ref_t = getStatistics().getHisto1D(histo_name_Ref_t).GetMean();
			getAuxilliaryData().setValue("timeDiffref mean values", histo_name_Ref_t, mean_Ref_t);

			TH1F* histoToSave_Ref_trailing = &(getStatistics().getHisto1D(histo_name_Ref_t));
//non zero histos 
// slot.first - ID
// slot.second - wskaznik na JPetBarrelSlot
//save fit parameters only for layerX and SlotY (taken from analysing files name)
//fit just for proper slot
		        int layer_barrel = (scin.second)->getBarrelSlot().getLayer().getID();
			auto scintillator_barrel = scin.second->getBarrelSlot().getID();
			int LayerId = vectorOfNumbers[2];
			int ScintId = 10*vectorOfNumbers[1]+vectorOfNumbers[0];
			//
//For run I we calibrate L3 with data taken for L1, for the rest of runs we had L3 irradiated
//so we can do direct calibration
//
			int ScintL3_1 = 0.;
			int ScintL3_2 = 0.;
			//
			if(CalibRun<2 && LayerId==1) {

			  ScintL3_1 = 2*ScintId -2;
			  ScintL3_2 = 2*ScintId -1;

			    if(ScintL3_1 < 1 || ScintL3_2 < 1){

			      ScintL3_1 = 48;
			      ScintL3_2 = 1;
			    }
			}  
			//
			if((histoToSave_leading->GetEntries() != 0 && histoToSave_trailing->GetEntries() != 0)
			&& ((layer_barrel==LayerId && scintillator_barrel==ScintId) || (layer_barrel==3 && (scintillator_barrel==ScintL3_1 || scintillator_barrel==ScintL3_2)) )){

			  if(histoToSave_Ref_leading->GetEntries()<=min_ev){
			    results_fit << "#WARNING: Statistics used to determine the leading edge calibration constant with respect to the refference detector was less than "<<min_ev<<" events!"<<endl;
			  }
			  if(histoToSave_Ref_trailing->GetEntries()<=min_ev){
                            results_fit << "#WARNING: Statistics used to determine the trailing edge calibration constant with respect to the refference detector was less than "<<min_ev<<" events!"<<endl;
                          }
			  if(histoToSave_leading->GetEntries()<=min_ev){
			    results_fit << "#WARNING: Statistics used to determine the leading edge A-B calibration constant was less than "<<min_ev<<" events!"<<endl;
			  }
			  if(histoToSave_trailing->GetEntries()<=min_ev){
			    results_fit << "#WARNING: Statistics used to determine the trailing edge A-B calibration constant was less than "<<min_ev<<" events!"<<endl;
			  }
//fit scintilators
			cout <<"#############" << endl;
			cout <<"CALIB_INFO: Fitting histogams for layer= " << layer_barrel << ", slot= "<< scintillator_barrel <<", threshold= "<<thr<<endl; 
			cout <<"#############" << endl;

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

			if((position_peak_error_l/position_peak_l)>=frac_err) {
			results_fit <<"#WFIT: Large uncertainty on the calibration constant!"<<endl;
			}

			if((position_peak_error_t/position_peak_t)>=frac_err) {
			results_fit <<"#WFIT: Large uncertainty on the calibration constant!"<<endl;
			}

//fit reference detector

			int highestBin_Ref_l = histoToSave_Ref_leading->GetBinCenter(histoToSave_Ref_leading->GetMaximumBin());
			histoToSave_Ref_leading->Fit("gaus","","", highestBin_Ref_l-5, highestBin_Ref_l+5); //range for fit
			TF1 *fit_Ref_l = histoToSave_Ref_leading->GetFunction("gaus");
			fit_Ref_l->SetRange(highestBin_Ref_l-20, highestBin_Ref_l+20); //range to draw gaus function
			histoToSave_Ref_leading->Draw();


			int highestBin_Ref_t = histoToSave_Ref_trailing->GetBinCenter(histoToSave_Ref_trailing->GetMaximumBin());
			histoToSave_Ref_trailing->Fit("gaus","","", highestBin_Ref_t-5, highestBin_Ref_t+5); //range for fit
 			TF1 *fit_Ref_t = histoToSave_Ref_trailing->GetFunction("gaus");
			fit_Ref_t->SetRange(highestBin_Ref_t-20, highestBin_Ref_t+20); //range to draw gaus function
			histoToSave_Ref_trailing->Draw();


			double position_peak_Ref_l = fit_Ref_l->GetParameter(1);
   			double position_peak_error_Ref_l=fit_Ref_l->GetParError(1);
			double sigma_peak_Ref_l =fit_Ref_l->GetParameter(2);
			double chi2_ndf_Ref_l = fit_Ref_l->GetChisquare()/fit_Ref_l->GetNDF();

			double position_peak_Ref_t = fit_Ref_t->GetParameter(1);
   			double position_peak_error_Ref_t=fit_Ref_t->GetParError(1);
			double sigma_peak_Ref_t =fit_Ref_t->GetParameter(2);
			double chi2_ndf_Ref_t = fit_Ref_t->GetChisquare()/fit_Ref_t->GetNDF();

			if((position_peak_error_Ref_l/position_peak_Ref_l)>=frac_err) {
			results_fit <<"#WFIT: Large uncertainty on the calibration constant!"<<endl;
			}

			if((position_peak_error_Ref_t/position_peak_Ref_t)>=frac_err) {
			results_fit <<"#WFIT: Large uncertainty on the calibration constant!"<<endl;
			}
			
			//
// writing to apropriate format (txt file)
//We assume that all the corrections will be ADDED to the times of channels
//side A
//offset = C2 (ref. det) - C1/2 (AB calib)
			float CAl = (position_peak_Ref_l-Cl[layer_barrel]) + position_peak_l/2.;
			float SigCAl = sqrt(pow(position_peak_error_Ref_l/2.,2) + pow(position_peak_error_l,2) + pow(SigCl[layer_barrel],2));
			float CAt = position_peak_t/2. + position_peak_Ref_t;
			float SigCAt = sqrt(pow(position_peak_error_Ref_t/2.,2) + pow(position_peak_error_t,2) + pow(SigCl[layer_barrel],2));
			//
			float CBl = (position_peak_Ref_l-Cl[layer_barrel]) - position_peak_l/2.;
			float SigCBl = SigCAl;
			float CBt = position_peak_Ref_t - position_peak_t/2.;
                        float SigCBt = SigCAt;
			scintillator_barrel = scintillator_barrel - (layer_barrel-1)*48;    //to match the convension during the calibration loading 
			
			results_fit << layer_barrel << "\t" << scintillator_barrel<< "\t" << "A" << "\t" << thr << "\t" << CAl << "\t" << SigCAl << "\t" << CAt << "\t" << SigCAt << "\t" << sigma_peak_Ref_l
                        << "\t" <<sigma_peak_Ref_t << "\t"  <<chi2_ndf_Ref_l << "\t" <<chi2_ndf_Ref_t <<endl;

//side B
//offset = C2 (ref. det) -C1/2 (AB calib) 
                        results_fit << layer_barrel << "\t" << scintillator_barrel << "\t" << "B" << "\t" << thr << "\t" <<CBl << "\t" << SigCBl << "\t" << CBt << "\t" << SigCBt << "\t" << sigma_peak_l
                        << "\t" << sigma_peak_t <<"\t" << chi2_ndf_l << "\t" <<chi2_ndf_t <<endl;
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

			double timeDiffAB_l = lead_times_A[thr] - lead_times_B[thr];
			timeDiffAB_l*= 0.001; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
			getStatistics().getHisto1D(histo_name_l).Fill( timeDiffAB_l);

			// fill the timeDiffAB vs slot ID histogram

			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_leading", layer_number, thr)).Fill( slot_number,
															timeDiffAB_l);
//
//take minimum time difference between Ref and Scint
			double timeDiffLmin=10000000000000.;
			for(unsigned int i = 0; i < fRefTimesL.size();i++) { 
			double timeDiffHit_L = (lead_times_A[thr] + lead_times_B[thr])/2. -fRefTimesL[i];
			 timeDiffHit_L = 0.001*timeDiffHit_L;//ps -> ns
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

			double timeDiffAB_t = trail_times_A[thr] - trail_times_B[thr];
			timeDiffAB_t*= 0.001; // we want the plots in ns instead of ps

			// fill the appropriate histogram
			const char * histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
			getStatistics().getHisto1D(histo_name_t).Fill( timeDiffAB_t);

			// fill the timeDiffAB vs slot ID histogram
			int layer_number = fBarrelMap.getLayerNumber( hit.getBarrelSlot().getLayer() );
			int slot_number = fBarrelMap.getSlotNumber( hit.getBarrelSlot() );
			getStatistics().getHisto2D(Form("TimeDiffVsID_layer%d_thr%d_trailing", layer_number, thr)).Fill( slot_number, timeDiffAB_t);
															
// 
//taken minimal time difference between Ref and Scint
			double timeDiffTmin=10000000000000.;
			for(unsigned int i = 0; i < fRefTimesT.size();i++) { 
                            double timeDiffHit_T = (trail_times_A[thr] + trail_times_B[thr])/2. -fRefTimesT[i];
			    timeDiffHit_T = 0.001*timeDiffHit_T; //ps->ns
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

