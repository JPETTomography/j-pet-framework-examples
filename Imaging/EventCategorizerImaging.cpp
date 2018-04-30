/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizerImaging.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "EventCategorizerImaging.h"
#include "../LargeBarrelAnalysis/EventCategorizerTools.h"

using namespace jpet_options_tools;

using namespace std;

EventCategorizerImaging::EventCategorizerImaging(const char* name): JPetUserTask(name) {}

bool EventCategorizerImaging::init()
{

  INFO("Event categorization started.");
  
  fOutputEvents = new JPetTimeWindow("JPetEvent");
    
  if ( isOptionSet(fParams.getOptions(), kMinAnnihilationParamKey) )
	fMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMinAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), kMaxAnnihilationParamKey) )
	fMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMaxAnnihilationParamKey);
  
  
  if (fSaveControlHistos) 
  {
	getStatistics().createHistogram(
	  new TH2F("Imaging_2_Hit_pos",
	    "Pos reco",
	    218, -54.5, 54.5,
	    218, -54.5, 54.5)
	);
	getStatistics().getHisto2D("Imaging_2_Hit_pos")->SetXTitle("Y [cm]");
	getStatistics().getHisto2D("Imaging_2_Hit_pos")->SetYTitle("X [cm]");
	
	getStatistics().createHistogram(
	  new TH1F("Imaging_2_Hit_posZ",
	    "Z pos reco",
	    220, -54.5, 54.5)
	);
	getStatistics().getHisto1D("Imaging_2_Hit_posZ")->SetXTitle("Z [cm]");
	getStatistics().getHisto1D("Imaging_2_Hit_posZ")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Imaging_2_Hit_Angles_2D",
	    "Annihilation Hits angles",
	    360, -0.5, 359.5)
	);
	getStatistics().getHisto1D("Imaging_2_Hit_Angles_2D")->SetXTitle("Angle between annihilation hits [deg]");
	getStatistics().getHisto1D("Imaging_2_Hit_Angles_2D")->SetYTitle("Counts");	
		
	getStatistics().createHistogram(
	  new TH1F("Imaging_2_Hit_Time_Diff",
	    "Annihilation Hits Time Difference",
	    2000, -0.5, 195.5)
	);
	getStatistics().getHisto1D("Imaging_2_Hit_Time_Diff")->SetXTitle("Time difference between annihilation hits [ns]");
	getStatistics().getHisto1D("Imaging_2_Hit_Time_Diff")->SetYTitle("Counts");	
  
	getStatistics().createHistogram(
	new TH2F("Imaging_3_Hit_pos",
	    "Pos reco",
	    218, -54.5, 54.5,
	    218, -54.5, 54.5)
	);
	getStatistics().getHisto2D("Imaging_3_Hit_pos")->SetXTitle("Y [cm]");
	getStatistics().getHisto2D("Imaging_3_Hit_pos")->SetYTitle("X [cm]");
	
	getStatistics().createHistogram(
	  new TH1F("Imaging_3_Hit_posZ",
	    "Z pos reco",
	    220, -54.5, 54.5)
	);
	getStatistics().getHisto1D("Imaging_3_Hit_posZ")->SetXTitle("Z [cm]");
	getStatistics().getHisto1D("Imaging_3_Hit_posZ")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH2F("Imaging_3_Hit_Angles",
	    "Annihilation Hits angles",
	    360, -0.5, 359.5,
	    360, -0.5, 359.5)
	);
	getStatistics().getHisto2D("Imaging_3_Hit_Angles")->SetXTitle("Sum of the two smallest angles [deg]");
	getStatistics().getHisto2D("Imaging_3_Hit_Angles")->SetYTitle("Difference of the two smallest angles [deg]");	
		
	getStatistics().createHistogram(
	  new TH1F("Imaging_3_Hit_Distance_from_0",
	    "3 Annihilation Hit plane distance",
	    500, -0.5, 49.5)
	);
	getStatistics().getHisto1D("Imaging_3_Hit_Distance_from_0")->SetXTitle("Distance of hit surface from zero [cm]");
	getStatistics().getHisto1D("Imaging_3_Hit_Distance_from_0")->SetYTitle("Counts");	
		
	getStatistics().createHistogram(
	  new TH1F("Imaging_3_Hit_Time_Diff",
	    "Annihilation Hits Time Difference",
	    2000, -0.5, 195.5)
	);
	getStatistics().getHisto1D("Imaging_3_Hit_Time_Diff")->SetXTitle("Time difference between annihilation hits [ns]");
	getStatistics().getHisto1D("Imaging_3_Hit_Time_Diff")->SetYTitle("Counts");	
  }
  return true;
}

bool EventCategorizerImaging::exec()
{
  vector<JPetEvent> events;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) 
  {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {

	const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
	if (event.getHits().size() > 1) 
	{
		vector<JPetHit> hits = event.getHits();	 
		JPetEvent imagingEvent = imageReconstruction( hits );
		if( imagingEvent.getHits().size() )
			events.push_back( imagingEvent );
	}
	
      
    }
  } 
  else 
  {
	  return false;
  }
  if( events.size() )
	saveEvents(events);
  events.clear();
  return true;
}

bool EventCategorizerImaging::terminate()
{
  return true;
}

void EventCategorizerImaging::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}


JPetEvent EventCategorizerImaging::imageReconstruction( vector<JPetHit> hits )
{
	JPetEvent imagingEvent;
	float zPositionCut = 23.;
	float minimalAngleForDecayInto2 = 95.;
	float maximalTimeDiffForDecayInto2 = 5.;
	float minimalAngleForDecayInto3 = 185.;
	float maximalTimeDiffForDecayInto3 = 1.;
	float maximalDistFromZeroOfDecayPlane = 10.;
  
	vector<JPetHit> annihilationHits;
	for( unsigned i=0; i<hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::calcTOT( hits[i] );
		if( TOTofHit >= fMinAnnihilationTOT/1000 && TOTofHit <= fMaxAnnihilationTOT/1000 && fabs( hits[i].getPosZ() ) < zPositionCut )
		{
			annihilationHits.push_back( hits[i] );
		}
	}
	if( annihilationHits.size() < 2 )
		return imagingEvent;
	for( unsigned i=0; i<annihilationHits.size() - 1; i++ )
	{
		for( unsigned j=i+1; j<annihilationHits.size(); j++ )
		{
			if( annihilationHits[i].getScintillator().getID() != annihilationHits[j].getScintillator().getID() )
			{
				float angleDiffinXY = EventCategorizerTools::calcAngle2D( annihilationHits[i], annihilationHits[j] );
				double timeDiff = fabs( annihilationHits[j].getTime() - annihilationHits[i].getTime() )/1000.;
				if( fSaveControlHistos )
				{
					getStatistics().getHisto1D("Imaging_2_Hit_Angles_2D")->Fill( angleDiffinXY );
					getStatistics().getHisto1D("Imaging_2_Hit_Time_Diff")->Fill( timeDiff );
				}
				if( timeDiff < maximalTimeDiffForDecayInto2 && angleDiffinXY > minimalAngleForDecayInto2 )
				{
					imagingEvent.addHit(annihilationHits[i]);
					imagingEvent.addHit(annihilationHits[j]);
					imagingEvent.setEventType(JPetEventType::k2Gamma);
					
					TVector3 recoPos = EventCategorizerTools::recoPosition( annihilationHits[i], annihilationHits[j] );
					if( fSaveControlHistos )
					{
						getStatistics().getHisto2D("Imaging_2_Hit_pos")->Fill( recoPos(1), recoPos(0) );
						getStatistics().getHisto1D("Imaging_2_Hit_posZ")->Fill( recoPos(2) );
					}
				}
			}
		}
	}
	if( annihilationHits.size() < 3 )
		return imagingEvent;
	
	
	TVector3 bestPositionFromZero(100,100,100);
	bool isThereDecayInto3 = false;
	vector<unsigned> bestIndices;
	bestIndices.push_back(0);
	bestIndices.push_back(0);
	bestIndices.push_back(0);
	for( unsigned i=0; i<annihilationHits.size() - 2; i++ )
	{
		for( unsigned j=i+1; j<annihilationHits.size() - 1; j++ )
		{
			for( unsigned k=j+1; k<annihilationHits.size(); k++ )
			{
				if( annihilationHits[i].getScintillator().getID() != annihilationHits[j].getScintillator().getID() && annihilationHits[i].getScintillator().getID() != annihilationHits[k].getScintillator().getID() && annihilationHits[j].getScintillator().getID() != annihilationHits[k].getScintillator().getID() )
				{
					vector<double> angles = EventCategorizerTools::calcAnglesFrom3Hit( annihilationHits[i], annihilationHits[j], annihilationHits[k] );
					double distanceFromZero = EventCategorizerTools::calcDistanceOfSurfaceAndZero( annihilationHits[i], annihilationHits[j], annihilationHits[k] );
					if( fSaveControlHistos )
					{
						getStatistics().getHisto2D("Imaging_3_Hit_Angles")->Fill( angles[0] + angles[1], angles[1] - angles[0] );
						getStatistics().getHisto1D("Imaging_3_Hit_Distance_from_0")->Fill( distanceFromZero );
					}
					if( angles[0] + angles[1] > minimalAngleForDecayInto3 && distanceFromZero < maximalDistFromZeroOfDecayPlane )
					{
						double timeDiff3Hit = fabs( 2*EventCategorizerTools::normalizeTime( annihilationHits[i] ) - EventCategorizerTools::normalizeTime( annihilationHits[j] ) - EventCategorizerTools::normalizeTime( annihilationHits[k] ) )/1000;
						if ( fSaveControlHistos )
						{
							getStatistics().getHisto1D("Imaging_3_Hit_Time_Diff")->Fill( timeDiff3Hit );
						}
						if( timeDiff3Hit < maximalTimeDiffForDecayInto3 )
						{
							TVector3 recoPos3Hit = EventCategorizerTools::recoPosition3Hit( annihilationHits[i], annihilationHits[j], annihilationHits[k] );
							if( recoPos3Hit.Mag() < bestPositionFromZero.Mag() )
							{
								bestIndices[0] = i;
								bestIndices[1] = j;
								bestIndices[2] = k;
								bestPositionFromZero = recoPos3Hit;
								isThereDecayInto3 = true;
							}
						}
					}
					
				}
			}
		}
	}
	if( isThereDecayInto3 )
	{
		imagingEvent.addHit(annihilationHits[bestIndices[0]]);
		imagingEvent.addHit(annihilationHits[bestIndices[1]]);
		imagingEvent.addHit(annihilationHits[bestIndices[2]]);
		if( imagingEvent.getEventType() == JPetEventType::kUnknown )
			imagingEvent.setEventType(JPetEventType::k3Gamma);
		if( imagingEvent.getEventType() == JPetEventType::k2Gamma )
			imagingEvent.addEventType(JPetEventType::k3Gamma);
		
		if ( fSaveControlHistos )
		{
			getStatistics().getHisto2D("Imaging_3_Hit_pos")->Fill( bestPositionFromZero(1), bestPositionFromZero(0) );
			getStatistics().getHisto1D("Imaging_3_Hit_posZ")->Fill( bestPositionFromZero(2) );
		}			  
	}
	
	return imagingEvent;
}