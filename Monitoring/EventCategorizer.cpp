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

	fOutputEvents = new JPetTimeWindow("JPetEvent");
	
	fBarrelMap = new JPetGeomMapping(getParamBank());
	
	if (fSaveControlHistos){
	  
	  		getStatistics().createHistogram(
					new TH2F("XY_plane_scint",
										"XY plane for scintillators",
										400, -100, 100,
										400, -100, 100)
				);
			getStatistics().getHisto2D("XY_plane_scint").SetXTitle("Y axis");
			getStatistics().getHisto2D("XY_plane_scint").SetYTitle("X axis");
	  
	  		getStatistics().createHistogram(
					new TH2F("XY_plane",
										"XY",
										199, -49.75, 49.75,
										199, -49.75, 49.75)
				);
			getStatistics().getHisto2D("XY_plane").SetXTitle("Y axis");
			getStatistics().getHisto2D("XY_plane").SetYTitle("X axis");
			
			getStatistics().createHistogram(
					new TH1F("Z_plane",
										"Z",
										199, -49.75, 49.75)
				);
			getStatistics().getHisto1D("Z_plane").SetXTitle("Z axis");
			getStatistics().getHisto1D("Z_plane").SetYTitle("Counts");
			
			getStatistics().createHistogram(
					new TH2F("TimeDifferenceVsID",
										"TimeDiffVsID",
										999, -49.95, 49.95,
										200, 0, 200)
				);
			getStatistics().getHisto2D("TimeDifferenceVsID").SetXTitle("Time difference between side A and side B on 1 threshold [ns]");
			getStatistics().getHisto2D("TimeDifferenceVsID").SetYTitle("Scintillator ID");
					
			getStatistics().createHistogram(
					new TH2F("TOFVsID",
										"TOFVsID",
										2999, -149.95, 149.95,
										200, 0, 200)
				);
			getStatistics().getHisto2D("TOFVsID").SetXTitle("Time of flight between back to back hits on 1 threshold [ns]");
			getStatistics().getHisto2D("TOFVsID").SetYTitle("Scintillator ID");
			
			getStatistics().createHistogram(
							new TH2F("TOTvsID",
								 "TOT",
								 200, 0, 200,
								 1500, 0, 150)
							);
			getStatistics().getHisto2D("TOTvsID").SetXTitle("Scintillator ID");
			getStatistics().getHisto2D("TOTvsID").SetYTitle("TOT [ns]");

	  
		getStatistics().createHistogram(
			new TH2F("HitDistanceVsTDiff",
								"Two Hit distance vs. abs time difference",
								100, 0.0, 150.0,
								100, 0.0, 10.0)
		);
		getStatistics().getHisto2D("HitDistanceVsTDiff").SetYTitle("Time difference [ns]");
		getStatistics().getHisto2D("HitDistanceVsTDiff").SetXTitle("Distance between hits [cm]");	
		
		getStatistics().createHistogram(
			new TH2F("HitDistanceVsTDiff_btb",
								"Opposite Hits distance vs. abs time difference for opposite hits",
								100, 0.0, 150.0,
								100, 0.0, 10.0)
		);
		getStatistics().getHisto2D("HitDistanceVsTDiff_btb").SetYTitle("Time difference [ns]");
		getStatistics().getHisto2D("HitDistanceVsTDiff_btb").SetXTitle("Distance between hits [cm]");		

		getStatistics().createHistogram(
			new TH2F("3_hit_angles",
								"3 Hit angles difference",
								360, -0.5, 359.5,
								360, -0.5, 359.5)
		);
		getStatistics().getHisto2D("3_hit_angles").SetXTitle("Smallest angle + Second smallest angle [deg]");
		getStatistics().getHisto2D("3_hit_angles").SetYTitle("Second smallest angle - Smallest angle [deg]");

		getStatistics().createHistogram(
			new TH1F("HitCounts_vs_ID",
								"Counts of Hits per scintillator",
								200, 0, 200)
		);
		getStatistics().getHisto1D("HitCounts_vs_ID").SetXTitle("Scintillator ID");
		getStatistics().getHisto1D("HitCounts_vs_ID").SetYTitle("Hit count");
	}
}

void EventCategorizer::exec(){

	//Analysis of Events consisting of two hits that come from Layer 1 or 2
	//Layer 3 is ignored, since it is not callibrated
  
	if(auto timeWindow = dynamic_cast<const JPetTimeWindow*const>(getEvent())){
	  uint n = timeWindow->getNumberOfEvents();
	  for(uint i=0;i<n;++i){

	    const auto & event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
			  vector<JPetHit> hits = event.getHits();    
		/*if(event.getHits().size() == 1){
			JPetHit firstHit = hits.at(0);
			getStatistics().getHisto1D("TOT").Fill( CalcTOT( firstHit ) );
			if( JPetHitUtils::getTimeDiffAtThr( firstHit, 1 ) / 1000. < 100 )
			{
				    getStatistics().getHisto2D("TimeDifferenceVsID").Fill( JPetHitUtils::getTimeDiffAtThr( firstHit , 1) / 1000., firstHit.getScintillator().getID() );
			}
		}*/
		if( hits.size() > 0){

		  for(unsigned k=0;k<hits.size();k++){
		    	JPetHit firstHit = hits.at(k);
			getStatistics().getHisto1D("HitCounts_vs_ID").Fill( firstHit.getScintillator().getID() );
			getStatistics().getHisto2D("TOTvsID").Fill( firstHit.getScintillator().getID(), CalcTOT( firstHit ) );
			if( JPetHitUtils::getTimeDiffAtThr( firstHit, 1 ) / 1000. < 100 )
			{
				    getStatistics().getHisto2D("TimeDifferenceVsID").Fill( JPetHitUtils::getTimeDiffAtThr( firstHit , 1) / 1000., firstHit.getScintillator().getID() );
			}
		    for(unsigned j=k+1;j<hits.size();j++){
			    JPetHit secondHit = hits.at(j);

				if( JPetHitUtils::getTimeDiffAtThr( firstHit, 1 ) / 1000. < 100 && JPetHitUtils::getTimeDiffAtThr( secondHit, 1 ) / 1000. < 100 
				  && firstHit.getBarrelSlot().getLayer().getID() == secondHit.getBarrelSlot().getLayer().getID() )
				{
					double tof = fabs( JPetHitUtils::getTimeAtThr( firstHit, 1) - JPetHitUtils::getTimeAtThr( secondHit, 1) );
					tof /= 1000.; // [ns]



					int delta_ID2 = (fabs(firstHit.getScintillator().getID() - secondHit.getScintillator().getID()) <= fBarrelMap->getSlotsCount( firstHit.getBarrelSlot().getLayer() ) / 2) ? fabs(firstHit.getScintillator().getID() - secondHit.getScintillator().getID() ) : ( fBarrelMap->getSlotsCount( firstHit.getBarrelSlot().getLayer() ) - (firstHit.getScintillator().getID() - secondHit.getScintillator().getID() ) );

					int half_layer = 24 + (firstHit.getBarrelSlot().getLayer().getID()==3 ? 24 : 0);
					if( tof < 150.0 && delta_ID2 == half_layer)
					{
							 getStatistics().getHisto2D("TOFVsID").Fill( (JPetHitUtils::getTimeAtThr(firstHit, 1) -
								JPetHitUtils::getTimeAtThr(secondHit, 1))/1000, firstHit.getScintillator().getID() );
							 getStatistics().getHisto2D("TOFVsID").Fill( (JPetHitUtils::getTimeAtThr(secondHit, 1) -
								JPetHitUtils::getTimeAtThr(firstHit, 1))/1000, secondHit.getScintillator().getID() );
							 fillPlanes( firstHit, secondHit );
					}
				}
        //  if(firstHit.getBarrelSlot().getLayer().getID()!=3
          //  && secondHit.getBarrelSlot().getLayer().getID()!=3){

            float thetaDiff = fabs(firstHit.getBarrelSlot().getTheta()
              -secondHit.getBarrelSlot().getTheta());
            float timeDiff = fabs(firstHit.getTime()-secondHit.getTime())/1000;
            float distance = sqrt(pow(firstHit.getPosX()-secondHit.getPosX(),2)
              +pow(firstHit.getPosY()-secondHit.getPosY(),2)
              +pow(firstHit.getPosZ()-secondHit.getPosZ(),2));

            if (fSaveControlHistos){
              getStatistics().getHisto2D("HitDistanceVsTDiff")
                .Fill(distance,timeDiff);
              if(thetaDiff>=180.0 && thetaDiff<181.0){
                getStatistics().getHisto2D("HitDistanceVsTDiff_btb")
                  .Fill(distance,timeDiff);
              }
            }
		    }
		  }
		}

		if(event.getHits().size() == 3){
          JPetHit firstHit = event.getHits().at(0);
          JPetHit secondHit = event.getHits().at(1);
          JPetHit thirdHit = event.getHits().at(2);
	  std::vector<double> angles;
	  angles.push_back(firstHit.getBarrelSlot().getTheta());
	  angles.push_back(secondHit.getBarrelSlot().getTheta());
	  angles.push_back(thirdHit.getBarrelSlot().getTheta());
	  std::sort( angles.begin(), angles.begin() +3 );
	  float theta_1_2 = angles[1] - angles[0];
	  float theta_2_3 = angles[2] - angles[1];
	  float theta_3_1 = 360 - theta_1_2 - theta_2_3;
	  angles.clear();
	  angles.push_back(theta_1_2);
	  angles.push_back(theta_2_3);
	  angles.push_back(theta_3_1);	  
	  std::sort( angles.begin(), angles.begin() +3 );
	  getStatistics().getHisto2D("3_hit_angles").Fill( angles[0]+angles[1], angles[1] - angles[0]);
         /* float theta_1_2 = fabs(firstHit.getBarrelSlot().getTheta()
            -secondHit.getBarrelSlot().getTheta());
          float theta_2_3 = fabs(secondHit.getBarrelSlot().getTheta()
            -thirdHit.getBarrelSlot().getTheta());
	  float theta_3_1 = 360 - theta_1_2 - theta_2_3; */ //fabs(thirdHit.getBarrelSlot().getTheta()
           // -firstHit.getBarrelSlot().getTheta());
/*if( theta_1_2 + theta_2_3 + theta_3_1 > 361 )
  std::cout << theta_1_2 + theta_2_3 + theta_3_1  << " " << theta_1_2 << " " << theta_2_3 << " " << theta_3_1 << std::endl;
			if( theta_3_1 >= theta_2_3 && theta_3_1 >= theta_1_2 )
			{
				if( theta_2_3 >= theta_1_2 )
					getStatistics().getHisto2D("3_hit_angles").Fill(theta_1_2, theta_2_3);
				else
					getStatistics().getHisto2D("3_hit_angles").Fill(theta_2_3, theta_1_2);
			}
			else if( theta_2_3 >= theta_3_1 && theta_2_3 >= theta_1_2 )
			{
				if( theta_3_1 >= theta_1_2 )
					getStatistics().getHisto2D("3_hit_angles").Fill(theta_1_2, theta_3_1);
				else
					getStatistics().getHisto2D("3_hit_angles").Fill(theta_3_1, theta_1_2);
			}
			else if( theta_1_2 >= theta_3_1 && theta_1_2 >= theta_2_3 )
			{
				if( theta_3_1 >= theta_2_3 )
					getStatistics().getHisto2D("3_hit_angles").Fill(theta_2_3, theta_3_1);
				else
					getStatistics().getHisto2D("3_hit_angles").Fill(theta_3_1, theta_2_3);			  
			}*/
		}
	}
	}
}

void EventCategorizer::fillPlanes( const JPetHit & Hit1, const JPetHit & Hit2)
{
	double tof = fabs( JPetHitUtils::getTimeAtThr(Hit1, 1) - JPetHitUtils::getTimeAtThr(Hit2, 1) )/1000;
	double VecLength = sqrt( pow(Hit1.getPosX()-Hit2.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY()-Hit2.getPosY(),2)
			+pow(Hit1.getPosZ()-Hit2.getPosZ(),2) );
	getStatistics().getHisto2D("XY_plane_scint").Fill( Hit1.getPosY(), Hit1.getPosX() );
	getStatistics().getHisto2D("XY_plane_scint").Fill( Hit2.getPosY(), Hit2.getPosX() );
	double middleX = (Hit1.getPosX()+Hit2.getPosX() )/2;
	double middleY = (Hit1.getPosY()+Hit2.getPosY() )/2;
	double middleZ = (Hit1.getPosZ()+Hit2.getPosZ() )/2;
	double Fraction = 2*tof*29.979246/VecLength;
	if( JPetHitUtils::getTimeAtThr(Hit1, 1) >= JPetHitUtils::getTimeAtThr(Hit2, 1) )
	{
	getStatistics().getHisto2D("XY_plane").Fill( middleY + Fraction*( Hit1.getPosY()-middleY ), middleX + Fraction*( Hit1.getPosX()-middleX ) );
	getStatistics().getHisto1D("Z_plane").Fill( middleZ + Fraction*( Hit1.getPosZ()-middleZ ) );
	}
	else
	 {
	getStatistics().getHisto2D("XY_plane").Fill( middleY + Fraction*( Hit2.getPosY()-middleY ), middleX + Fraction*( Hit2.getPosX()-middleX ) );
	getStatistics().getHisto1D("Z_plane").Fill( middleZ + Fraction*( Hit2.getPosZ()-middleZ ) );
	}
}

double EventCategorizer::CalcTOT( JPetHit Hit )
{
	double TOT = 0;
	  std::map<int, double> lead_timesA, lead_timesB, trail_timesA, trail_timesB;
	  lead_timesA = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	  lead_timesB = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	  trail_timesA = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
	  trail_timesB = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);	  
	  for(int i=1;i<=Hit.getSignalA().getRecoSignal().getRawSignal().getNumberOfPoints(JPetSigCh::Leading);i++){
                    auto leadSearch = lead_timesA.find(i);
                    auto trailSearch = trail_timesA.find(i);
                    if (leadSearch != lead_timesA.end()
                        && trailSearch != trail_timesA.end())
                            TOT+=(trailSearch->second - leadSearch->second);
                }
          for(int i=1;i<=Hit.getSignalB().getRecoSignal().getRawSignal().getNumberOfPoints(JPetSigCh::Leading);i++){
                    auto leadSearch = lead_timesB.find(i);
                    auto trailSearch = trail_timesB.find(i);
                    if (leadSearch != lead_timesB.end()
                        && trailSearch != trail_timesB.end())
                            TOT+=(trailSearch->second - leadSearch->second);
                }
	return TOT/1000.;
}

void EventCategorizer::terminate(){

	INFO("More than one hit Events done. Writing conrtrol histograms.");

}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
	for (const auto & event : events) {
	  fOutputEvents->add<JPetEvent>(event);
	}
}
