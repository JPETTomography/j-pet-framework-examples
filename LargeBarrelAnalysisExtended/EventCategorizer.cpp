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
 *  @file EventCategorizer.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "EventCategorizer.h"

using namespace std;

EventCategorizer::EventCategorizer(const char * name, const char * description):JPetTask(name, description){}

void EventCategorizer::init(const JPetTaskInterface::Options&){

	INFO("Event categorization started.");
	INFO("Looking at two hit Events on Layer 1&2 only - creating only control histograms");

	if (fSaveControlHistos){
		getStatistics().createHistogram(
			new TH1F("two_hit_event_theta_diff",
								"Abs Theta Difference Between Two Hits in Event",
								360, -0.5, 359.5)
		);
		getStatistics().createHistogram(
			new TH1F("two_hit_event_theta_diff_cut",
								"Abs Theta Difference Between Opposite Hits in Event",
								360, -0.5, 359.5)
		);
		getStatistics().createHistogram(
			new TH1F("hits_x_pos",
								"Hits X position",
								100, -60.0, 60.0)
		);
    getStatistics().createHistogram(
			new TH1F("hits_y_pos",
								"Hits Y position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("hits_z_pos",
								"Hits Z position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("hits_x_pos_cut",
								"Opposite Hits X position",
								100, -60.0, 60.0)
		);
    getStatistics().createHistogram(
			new TH1F("hits_y_pos_cut",
								"Opposite Hits Y position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("hits_z_pos_cut",
								"Opposite Hits Z position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH2F("hit_distanece_vs_time_diff",
								"Two Hit distance vs. abs time difference",
								100, 0.0, 150.0,
								100, 0.0, 6000.0)
		);
		getStatistics().createHistogram(
			new TH2F("hit_distanece_vs_theta_diff",
								"Two Hit distance vs. abs theta difference",
								100, 0.0, 150.0,
								360, -0.5, 359.5)
		);
		getStatistics().createHistogram(
			new TH2F("hit_distanece_vs_time_diff_cut",
								"Opposite Hits distance vs. abs time difference",
								100, 0.0, 150.0,
								100, 0.0, 6000.0)
		);
		getStatistics().createHistogram(
			new TH2F("hit_distanece_vs_theta_diff_cut",
								"Opposite Hit distance vs. abs theta difference",
								100, 0.0, 150.0,
								360, -0.5, 359.5)
		);

		getStatistics().createHistogram(
			new TH2F("3_hit_angles",
								"3 Hit angles difference 1-2, 2-3",
								360, -0.5, 359.5,
								360, -0.5, 359.5)
		);
	}
}

void EventCategorizer::exec(){

	//Analysis of Events consisting of two hits that come from Layer 1 or 2
	//Layer 3 is ignored, since it is not callibrated
	if(auto event = dynamic_cast<const JPetEvent*const>(getEvent())){
		if(event->getHits().size() > 1){

		  vector<JPetHit> hits = event->getHits();

		  for(int i=0;i<hits.size();i++){
		    for(int j=i+1;j<hits.size();j++){
                JPetHit firstHit = hits.at(i);
			    JPetHit secondHit = hits.at(j);

                if(firstHit.getBarrelSlot().getLayer().getID()!=3
                && secondHit.getBarrelSlot().getLayer().getID()!=3){

                float thetaDiff = fabs(firstHit.getBarrelSlot().getTheta()-secondHit.getBarrelSlot().getTheta());
                float timeDiff = fabs(firstHit.getTime()-secondHit.getTime());

                float distance = sqrt(pow(firstHit.getPosX()-secondHit.getPosX(),2)
                                     +pow(firstHit.getPosY()-secondHit.getPosY(),2)
                                     +pow(firstHit.getPosZ()-secondHit.getPosZ(),2));

                if (fSaveControlHistos){
                  getStatistics().getHisto1D("two_hit_event_theta_diff").Fill(thetaDiff);
                  getStatistics().getHisto1D("hits_x_pos").Fill(firstHit.getPosX());
                  getStatistics().getHisto1D("hits_y_pos").Fill(firstHit.getPosY());
                  getStatistics().getHisto1D("hits_z_pos").Fill(firstHit.getPosZ());
                  getStatistics().getHisto1D("hits_x_pos").Fill(secondHit.getPosX());
                  getStatistics().getHisto1D("hits_y_pos").Fill(secondHit.getPosY());
                  getStatistics().getHisto1D("hits_z_pos").Fill(secondHit.getPosZ());
                  getStatistics().getHisto2D("hit_distanece_vs_time_diff").Fill(distance,timeDiff);
                  getStatistics().getHisto2D("hit_distanece_vs_theta_diff").Fill(distance,thetaDiff);

                  if(thetaDiff>=180.0 && thetaDiff<181.0){
                    getStatistics().getHisto1D("two_hit_event_theta_diff_cut").Fill(thetaDiff);
                    getStatistics().getHisto1D("hits_x_pos_cut").Fill(firstHit.getPosX());
                    getStatistics().getHisto1D("hits_y_pos_cut").Fill(firstHit.getPosY());
                    getStatistics().getHisto1D("hits_z_pos_cut").Fill(firstHit.getPosZ());
                    getStatistics().getHisto1D("hits_x_pos_cut").Fill(secondHit.getPosX());
                    getStatistics().getHisto1D("hits_y_pos_cut").Fill(secondHit.getPosY());
                    getStatistics().getHisto1D("hits_z_pos_cut").Fill(secondHit.getPosZ());
                    getStatistics().getHisto2D("hit_distanece_vs_time_diff_cut").Fill(distance,timeDiff);
                    getStatistics().getHisto2D("hit_distanece_vs_theta_diff_cut").Fill(distance,thetaDiff);
                  }
                }
}
		    }
		  }
		}

		if(event->getHits().size() == 3){
          JPetHit firstHit = event->getHits().at(0);
          JPetHit secondHit = event->getHits().at(1);
          JPetHit thirdHit = event->getHits().at(2);

          float theta_1_2 = fabs(firstHit.getBarrelSlot().getTheta()-secondHit.getBarrelSlot().getTheta());
          float theta_2_3 = fabs(secondHit.getBarrelSlot().getTheta()-thirdHit.getBarrelSlot().getTheta());

          getStatistics().getHisto2D("3_hit_angles").Fill(theta_1_2,theta_2_3);
		}
	}
}

void EventCategorizer::terminate(){

	INFO("More than one hit Events done. Writing conrtrol histograms.");

}

void EventCategorizer::setWriter(JPetWriter* writer) { fWriter = writer; }

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
	assert(fWriter);
	for (const auto & event : events) {
		fWriter->write(event);
	}
}
