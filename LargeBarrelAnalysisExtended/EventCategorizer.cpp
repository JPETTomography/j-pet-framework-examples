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
			new TH1F("two_hit_event_posZ_diff",
								"Abs Z Position Difference Between Two Hits in Event",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("first_hit_x_pos",
								"First hit X position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("second_hit_x_pos",
								"Second hit X position",
								100, -60.0, 60.0)
		);
				getStatistics().createHistogram(
			new TH1F("first_hit_y_pos",
								"Second hit Y position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("second_hit_y_pos",
								"Second hit Y position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("first_hit_z_pos",
								"Second hit Y position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("second_hit_z_pos",
								"Second hit Y position",
								100, -60.0, 60.0)
		);
		getStatistics().createHistogram(
			new TH1F("distance_timediff",
								"Hit distance/time_diff",
								200, 0.0, 35.0)
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
			new TH2F("hit_time_diff_vs_theta_diff",
								"Two Hit time difference vs. abs theta difference",
								100, 0.0, 6000.0,
								360, -0.5, 359.5)
		);
		getStatistics().createHistogram(
			new TH2F("hit_vel_vs_theta_diff",
								"Two Hit velocity vs. abs theta difference",
								200, 0.0, 35.0,
								360, -0.5, 359.5)
		);
	}
}

void EventCategorizer::exec(){

	//Analysis of Events consisting of two hits that come from Layer 1 or 2
	//Layer 3 is ignored, since it is not callibrated
	if(auto event = dynamic_cast<const JPetEvent*const>(getEvent())){
		if(event->getHits().size()==2){

			JPetHit firstHit = event->getHits().at(0);
			JPetHit secondHit = event->getHits().at(1);

			if(firstHit.getBarrelSlot().getLayer().getID()!=3
					&& secondHit.getBarrelSlot().getLayer().getID()!=3){

				float thetaDiff = fabs(firstHit.getBarrelSlot().getTheta()
														-secondHit.getBarrelSlot().getTheta());
				float timeDiff = fabs(firstHit.getTime()-secondHit.getTime());
				float zDiff = fabs(firstHit.getPosZ()-secondHit.getPosZ());
				float distance = sqrt(pow(firstHit.getPosX()-secondHit.getPosX(),2)
															+pow(firstHit.getPosY()-secondHit.getPosY(),2)
															+pow(firstHit.getPosZ()-secondHit.getPosZ(),2));

				if (fSaveControlHistos){
					getStatistics().getHisto1D("two_hit_event_theta_diff").Fill(thetaDiff);
					getStatistics().getHisto1D("two_hit_event_posZ_diff").Fill(zDiff);
					getStatistics().getHisto1D("first_hit_x_pos").Fill(firstHit.getPosX());
					getStatistics().getHisto1D("second_hit_x_pos").Fill(secondHit.getPosX());
					getStatistics().getHisto1D("first_hit_y_pos").Fill(firstHit.getPosY());
					getStatistics().getHisto1D("second_hit_y_pos").Fill(secondHit.getPosY());
					getStatistics().getHisto1D("first_hit_z_pos").Fill(firstHit.getPosZ());
					getStatistics().getHisto1D("second_hit_z_pos").Fill(secondHit.getPosZ());
					getStatistics().getHisto1D("distance_timediff").Fill(1000.0*distance/timeDiff);
					getStatistics().getHisto2D("hit_distanece_vs_time_diff").Fill(distance,timeDiff);
					getStatistics().getHisto2D("hit_distanece_vs_theta_diff").Fill(distance,thetaDiff);
					getStatistics().getHisto2D("hit_time_diff_vs_theta_diff").Fill(timeDiff,thetaDiff);
					getStatistics().getHisto2D("hit_vel_vs_theta_diff").Fill(1000.0*distance/timeDiff,thetaDiff);
				}
			}
		}
	}
}

void EventCategorizer::terminate(){

	INFO("Two hit Events done. Writing conrtrol histograms.");

}

void EventCategorizer::setWriter(JPetWriter* writer) { fWriter = writer; }

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
	assert(fWriter);
	for (const auto & event : events) {
		fWriter->write(event);
	}
}
