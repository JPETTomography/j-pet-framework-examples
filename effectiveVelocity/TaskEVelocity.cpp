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
 *  @file TaskEVelocity.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetHitUtils/JPetHitUtils.h>
#include "TaskEVelocity.h"
using namespace std;

void TaskEVelocity::setSlotsAndCuts(){

slots.push_back(4);
slots.push_back(20);
slots.push_back(35);

TOTcuts.push_back(0);
TOTcuts.push_back(43);
TOTcuts.push_back(44);
TOTcuts.push_back(45);
TOTcuts.push_back(46);
TOTcuts.push_back(47);
TOTcuts.push_back(48);
TOTcuts.push_back(49);
TOTcuts.push_back(50);
TOTcuts.push_back(51);
TOTcuts.push_back(52);
TOTcuts.push_back(53);
TOTcuts.push_back(54);

}

TaskEVelocity::TaskEVelocity(const char * name, const char * description):JPetTask(name, description){}
TaskEVelocity::~TaskEVelocity(){}
void TaskEVelocity::init(const JPetTaskInterface::Options& opts){
	setSlotsAndCuts();
	std::cout << "Slots size: " <<slots.size() <<std::endl;
	std::cout <<"Cuts size: " << TOTcuts.size() << std::endl;
	fBarrelMap.buildMappings(getParamBank());
	for(auto & layer : getParamBank().getLayers()){
		for (int thr=1;thr<=4;thr++){
		/*
			// create histograms of Delta ID
			char * histo_name = Form("Delta_ID_for_coincidences_layer_%d_thr_%d", fBarrelMap.getLayerNumber(*layer.second), thr);
			char * histo_title = Form("%s;#Delta ID", histo_name); 
			int n_slots_in_half_layer = fBarrelMap.getNumberOfSlots(*layer.second) / 2;
			getStatistics().createHistogram( new TH1F(histo_name, histo_title,
				n_slots_in_half_layer, 0.5, n_slots_in_half_layer+0.5)
			);
			
			// create histograms of TOF vs Delta ID
			histo_name = Form("TOF_vs_Delta_ID_layer_%d_thr_%d", fBarrelMap.getLayerNumber(*layer.second), thr);
			histo_title = Form("%s;#Delta ID;TOF [ns]", histo_name); 
			getStatistics().createHistogram( new TH2F(histo_name, histo_title,
				n_slots_in_half_layer, 0.5, n_slots_in_half_layer+0.5,
				100, 0., 15.)
			);
			
			// create histograms for TOT vs TOT
			for(char side : {'A', 'B'} ){
				histo_name = Form("TOT_vs_TOT_layer_%d_thr_%d_side_%c", fBarrelMap.getLayerNumber(*layer.second), thr, side);
				histo_title = Form("%s;TOT [ns];TOT [ns]", histo_name); 
				getStatistics().createHistogram( new TH2F(histo_name, histo_title, 120, 0., 120., 120, 0., 120.));
			}
		*/
			// create Histogram of TOT for selected slot side A i
			char* histo_name,*histo_title;
			for(unsigned int i = 0; i < slots.size(); i++){
/*				histo_name = Form("TOT_layer_%d_slot_%d_thr%d_side_A_noCoin", fBarrelMap.getLayerNumber(*layer.second), slots[i], thr);
				histo_title = Form("%s;TOT [ns]", histo_name);
				getStatistics().createHistogram( new TH1F(histo_name, histo_title, 700,0.0, 70.0));
			
				histo_name = Form("TOT_layer_%d_slot_%d_thr%d_side_A_coin", fBarrelMap.getLayerNumber(*layer.second), slots[i], thr);
				histo_title = Form("%s;TOT [ns]", histo_name);
				getStatistics().createHistogram( new TH1F(histo_name, histo_title, 700,0.0, 70.0));
				for(unsigned int j = 0; j < TOTcuts.size(); j++){
				histo_name = Form("#DeltaT_layer_%d_slot_%d_thr%d_cutOn%f", fBarrelMap.getLayerNumber(*layer.second), slots[i], thr, TOTcuts[j]);
				histo_title = Form("%s;#DeltaT [ns]", histo_name);
				getStatistics().createHistogram( new TH1F(histo_name, histo_title, 500,-20.0, 0.0));
				}
*/				
				histo_name = Form("#DeltaT_vs_TOT_layer_%d_slot_%d_thr%d_side_A_noCoin", fBarrelMap.getLayerNumber(*layer.second), slots[i], thr);
				histo_title = Form("%s;TOT [ns];#DeltaT [ns]", histo_name);
				getStatistics().createHistogram( new TH2F(histo_name, histo_title, 600, 0.0, 60.0, 500, -20, 0));

				histo_name = Form("#DeltaT_vs_1/TOT_layer_%d_slot_%d_thr%d_side_A_noCoin", fBarrelMap.getLayerNumber(*layer.second), slots[i], thr);
				histo_title = Form("%s;1/TOT [ns];#DeltaT [ns]", histo_name);
				getStatistics().createHistogram( new TH2F(histo_name, histo_title, 1000, 0.0, 0.2, 500, -20, 0));
			}
		}
	}
}
void TaskEVelocity::exec(){
	//getting the data from event in propriate format
	if(auto currHit=dynamic_cast<const JPetHit*const>(getEvent())){
		if (fHits.empty()) {
			fHits.push_back(*currHit);
		} else {
			if (fHits[0].getTimeWindowIndex() == currHit->getSignalB().getTimeWindowIndex()) {
				fHits.push_back(*currHit);
			} else {
				fillCoincidenceHistos(fHits);
				fHits.clear();
				fHits.push_back(*currHit);
			}
		}
	}
}
// this method considers all possible 2-strip coincidences
// among the hits from a single time window
void TaskEVelocity::fillCoincidenceHistos(const vector<JPetHit>& hits){
	for (auto i = hits.begin(); i != hits.end(); ++i) {
		for (auto j = i; ++j != hits.end(); /**/) {
			auto& hit1 = *i;
			auto& hit2 = *j;
			// if there are two hits from the same layer but different scintillators
			if (
				(hit1.getBarrelSlot().getLayer() == hit2.getBarrelSlot().getLayer()) &&
				(hit1.getScintillator() != hit2.getScintillator())
			) {
				// study the coincidences independently for each threshold
				for(int thr=1;thr<=4;thr++){
					if( isGoodTimeDiff(hit1, thr) && isGoodTimeDiff(hit2, thr) ){
						double tof = fabs( JPetHitUtils::getTimeAtThr(hit1, thr) -
							JPetHitUtils::getTimeAtThr(hit2, thr)
						);
						tof /= 1000.; // [ns]
						// check coincidence window
						//if( tof < 50.0 ){
						if( true ){
							// study the coincidence and fill histograms
							int delta_ID = fBarrelMap.calcDeltaID(hit1, hit2);
							//fillDeltaIDhisto(delta_ID, thr, hit1.getBarrelSlot().getLayer());
							//fillTOFvsDeltaIDhisto(delta_ID, thr, hit1, hit2);
							// fill TOT vs TOT histos
							//fillTOTvsTOThisto(delta_ID, thr, hit1, hit2);
							for(unsigned int i = 0; i < slots.size(); i++){
/*
							if( slots[i] == hit1.getBarrelSlot().getID()  ){
								fillTOTnoCoin(delta_ID, thr, hit1, 'A');
							}
							if( slots[i] == hit1.getBarrelSlot().getID()  ){
								fillTOTwithCoin(delta_ID, thr, hit1, 'A');
							}
							if( slots[i] == hit2.getBarrelSlot().getID()  ){
								fillTOTnoCoin(delta_ID, thr, hit2, 'A');
							}
							if( slots[i] == hit2.getBarrelSlot().getID()  ){
								fillTOTwithCoin(delta_ID, thr, hit2, 'A');
							}
							if( slots[i] == hit1.getBarrelSlot().getID()  ){
								fillDeltaT(delta_ID, thr, hit1);
							}
							if( slots[i] == hit2.getBarrelSlot().getID()  ){
								fillDeltaT(delta_ID, thr, hit2);
							}
*/
							if( slots[i] == hit1.getBarrelSlot().getID() ){
								fillDeltaTvsTOT(thr, hit1);
							}
							if( slots[i] == hit2.getBarrelSlot().getID() ){
								fillDeltaTvsTOT(thr, hit2);
							}
							}
						}
					}
				}
			}
		}
	}
}
void TaskEVelocity::terminate(){}
const char * TaskEVelocity::formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold, const char * prefix = ""){
	int slot_number = fBarrelMap.getSlotNumber(slot);
	int layer_number = fBarrelMap.getLayerNumber(slot.getLayer()); 
	return Form("%slayer_%d_slot_%d_thr_%d",
		    prefix,
	     layer_number,
	     slot_number,
	     threshold
	);
}

void TaskEVelocity::fillDeltaIDhisto(int delta_ID, int threshold, const JPetLayer & layer){
	int layer_number = fBarrelMap.getLayerNumber(layer);
	const char * histo_name = Form("Delta_ID_for_coincidences_layer_%d_thr_%d", layer_number, threshold);
	getStatistics().getHisto1D(histo_name).Fill(delta_ID);
}

void TaskEVelocity::fillTOFvsDeltaIDhisto(int delta_ID, int thr, const JPetHit & hit1, const JPetHit & hit2){
	int layer_number = fBarrelMap.getLayerNumber(hit1.getBarrelSlot().getLayer());
	const char * histo_name = Form("TOF_vs_Delta_ID_layer_%d_thr_%d",
				       fBarrelMap.getLayerNumber(hit1.getBarrelSlot().getLayer()),
				       thr);
	
	double tof = fabs( JPetHitUtils::getTimeAtThr(hit1, thr) -
	JPetHitUtils::getTimeAtThr(hit2, thr)
	);
	
	tof /= 1000.; // to have the TOF in ns instead of ps
	
	getStatistics().getHisto2D(histo_name).Fill(delta_ID, tof);
}


bool TaskEVelocity::isGoodTimeDiff(const JPetHit & hit, int thr){
	double mean_timediff = getAuxilliaryData().getValue("timeDiffAB mean values",
							    formatUniqueSlotDescription(hit.getBarrelSlot(),
								    thr, "timeDiffAB_")
	);
	double this_hit_timediff = JPetHitUtils::getTimeDiffAtThr(hit, thr) / 1000.; // [ns]
	return( fabs( this_hit_timediff - mean_timediff ) < 2.4 );
}

void TaskEVelocity::fillDeltaTvsTOT( int thr, const JPetHit& hit)
{
	double tot = hit.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr) / 1000.0;
	double timediff = JPetHitUtils::getTimeDiffAtThr(hit, thr) / 1000.0;

	char* histo_name = Form("#DeltaT_vs_TOT_layer_%d_slot_%d_thr%d_side_A_noCoin", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), hit.getBarrelSlot().getID(), thr);
	getStatistics().getHisto2D(histo_name).Fill(tot, timediff);

	
	histo_name = Form("#DeltaT_vs_1/TOT_layer_%d_slot_%d_thr%d_side_A_noCoin", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), hit.getBarrelSlot().getID(), thr);
	getStatistics().getHisto2D(histo_name).Fill(1.0/tot, timediff);

}

void TaskEVelocity::fillTOTvsTOThisto(int delta_ID, int thr, const JPetHit & hit1, const JPetHit & hit2){
	int n_slots_in_half_layer = fBarrelMap.getNumberOfSlots(hit1.getBarrelSlot().getLayer()) / 2;
	if( delta_ID != n_slots_in_half_layer )return; // skip non-opposite coincidences
	double totA1 = hit1.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr);
	double totB1 = hit1.getSignalB().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr);
	double totA2 = hit2.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr);
	double totB2 = hit2.getSignalB().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr);
	char * histo_name;
	
	// fill side A
	histo_name = Form("TOT_vs_TOT_layer_%d_thr_%d_side_A",
			  fBarrelMap.getLayerNumber(hit1.getBarrelSlot().getLayer()), thr);  
	getStatistics().getHisto2D(histo_name).Fill(totA1/1000., totA2/1000.);
	
	// fill side B
	histo_name = Form("TOT_vs_TOT_layer_%d_thr_%d_side_B",
			  fBarrelMap.getLayerNumber(hit1.getBarrelSlot().getLayer()), thr);  
	getStatistics().getHisto2D(histo_name).Fill(totB1/1000., totB2/1000.);
	
}
void TaskEVelocity::fillTOTnoCoin(int delta_ID, int thr,const JPetHit& hit,char side)
{
	int n_slots_in_half_layer = fBarrelMap.getNumberOfSlots(hit.getBarrelSlot().getLayer()) / 2;
//	if( delta_ID != n_slots_in_half_layer )return; // skip non-opposite coincidences
	double tot = hit.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr);
	char* histo_name;
	switch(side){
		case 'A':
			histo_name = Form("TOT_layer_%d_slot_%d_thr%d_side_A_noCoin", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), hit.getBarrelSlot().getID(), thr);
			getStatistics().getHisto1D(histo_name).Fill(tot/1000.);
			break;
		case 'B':
			histo_name = Form("TOT_layer_%d_slot_%d_thr%d_side_B", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), 0, thr);
			getStatistics().getHisto1D(histo_name).Fill(tot/1000.);
			break;
	}
	
}

void TaskEVelocity::fillTOTwithCoin(int delta_ID, int thr,const JPetHit& hit,char side)
{
        int n_slots_in_half_layer = fBarrelMap.getNumberOfSlots(hit.getBarrelSlot().getLayer()) / 2;
	if( delta_ID != n_slots_in_half_layer )return; // skip non-opposite coincidences
        double tot = hit.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr);
        char* histo_name;
        switch(side){
                case 'A':
                        histo_name = Form("TOT_layer_%d_slot_%d_thr%d_side_A_coin", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), hit.getBarrelSlot().getID(), thr);
                        getStatistics().getHisto1D(histo_name).Fill(tot/1000.);
                        break;
                case 'B':
                        histo_name = Form("TOT_layer_%d_slot_%d_thr%d_side_B", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), hit.getBarrelSlot().getID(), thr);
                        getStatistics().getHisto1D(histo_name).Fill(tot/1000.);
                        break;
        }

}

void TaskEVelocity::fillDeltaT(int delta_ID, int thr,const JPetHit& hit)
{
//        int n_slots_in_half_layer = fBarrelMap.getNumberOfSlots(hit.getBarrelSlot().getLayer()) / 2;
//        if( delta_ID != n_slots_in_half_layer )return; // skip non-opposite coincidences
	double tot = hit.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdNumber().at(thr)/1000.0;
	for(unsigned int j = 0; j < TOTcuts.size(); j++){
		if( !( tot > TOTcuts[j] ) ) continue;
		double timediff = JPetHitUtils::getTimeDiffAtThr(hit, thr) / 1000.;
	        char* histo_name;
        	histo_name = Form("#DeltaT_layer_%d_slot_%d_thr%d_cutOn%f", fBarrelMap.getLayerNumber(hit.getBarrelSlot().getLayer()), hit.getBarrelSlot().getID(), thr, TOTcuts[j]);
	        getStatistics().getHisto1D(histo_name).Fill( timediff );
        }

}
void TaskEVelocity::setWriter(JPetWriter* writer){fWriter =writer;}
