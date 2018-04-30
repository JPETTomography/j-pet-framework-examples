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
 *  @file EventCategorizerPhysics.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "EventCategorizerPhysics.h"
#include "../LargeBarrelAnalysis/EventCategorizerTools.h"

using namespace jpet_options_tools;

using namespace std;

EventCategorizerPhysics::EventCategorizerPhysics(const char* name): JPetUserTask(name) {}

bool EventCategorizerPhysics::init()
{

  INFO("Event categorization started.");
  
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  
  if ( isOptionSet(fParams.getOptions(), kMinAnnihilationParamKey) )
	fMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMinAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), kMaxAnnihilationParamKey) )
	fMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), kMaxAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), kMinDeexcitationParamKey) )
	fMinDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), kMinDeexcitationParamKey);
  if ( isOptionSet(fParams.getOptions(), kMaxDeexcitationParamKey) )
	fMaxDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), kMaxDeexcitationParamKey);
  
  if (fSaveControlHistos) 
  {
	getStatistics().createHistogram(
	  new TH1F("TOT",
	    "TOT of Hits",
	    1000, -0.5, 99.5)
	);
	getStatistics().getHisto1D("TOT")->SetXTitle("TOT [ns]");
	getStatistics().getHisto1D("TOT")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Quick_Positronium_Lifetime",
	    "Annihilation-Deexcitation Hits Time Difference",
	    4000, -195.5, 195.5)
	);
	getStatistics().getHisto1D("Quick_Positronium_Lifetime")->SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
	getStatistics().getHisto1D("Quick_Positronium_Lifetime")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into2G_AnniAngle",
	    "Annihilation Hits angle",
	    180, -0.5, 179.5)
	);
	getStatistics().getHisto1D("Decay_Into2G_AnniAngle")->SetXTitle("Angle between annihilation hits [deg]");
	getStatistics().getHisto1D("Decay_Into2G_AnniAngle")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into2G_TimeDiff",
	    "Annihilation Hits Time Difference",
	    2000, -0.5, 195.5)
	);
	getStatistics().getHisto1D("Decay_Into2G_TimeDiff")->SetXTitle("Time difference between annihilation hits [ns]");
	getStatistics().getHisto1D("Decay_Into2G_TimeDiff")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into2G_DistFromZero",
	    "Difference of Hit Plane from Zero",
	    600, -0.5, 59.5)
	);
	getStatistics().getHisto1D("Decay_Into2G_DistFromZero")->SetXTitle("Time difference between annihilation hits [cm]");
	getStatistics().getHisto1D("Decay_Into2G_DistFromZero")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH2F("Decay_Into2G_pos",
	    "Pos reco",
	    218, -54.5, 54.5,
	    218, -54.5, 54.5)
	);
	getStatistics().getHisto2D("Decay_Into2G_pos")->SetXTitle("Y [cm]");
	getStatistics().getHisto2D("Decay_Into2G_pos")->SetYTitle("X [cm]");
	
	getStatistics().createHistogram(
	  new TH1F("Decay_Into2G_posZ",
	    "Z pos reco",
	    220, -54.5, 54.5)
	);
	getStatistics().getHisto1D("Decay_Into2G_posZ")->SetXTitle("Z [cm]");
	getStatistics().getHisto1D("Decay_Into2G_posZ")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into2G_Lifetime",
	    "Annihilation-Deexcitation Hits Time Difference",
	    4000, -195.5, 195.5)
	);
	getStatistics().getHisto1D("Decay_Into2G_Lifetime")->SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
	getStatistics().getHisto1D("Decay_Into2G_Lifetime")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH2F("Decay_Into3G_AnniAngle",
	    "Annihilation Hits angles",
	    360, -0.5, 359.5,
	    360, -0.5, 359.5)
	);
	getStatistics().getHisto2D("Decay_Into3G_AnniAngle")->SetXTitle("Sum of the two smallest angles [deg]");
	getStatistics().getHisto2D("Decay_Into3G_AnniAngle")->SetYTitle("Difference of the two smallest angles [deg]");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into3G_TimeDiff",
	    "Annihilation Hits Time Difference",
	    2000, -0.5, 195.5)
	);
	getStatistics().getHisto1D("Decay_Into3G_TimeDiff")->SetXTitle("Time difference between annihilation hits [ns]");
	getStatistics().getHisto1D("Decay_Into3G_TimeDiff")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into3G_DistFromZero",
	    "Difference of Hit Plane from Zero",
	    600, -0.5, 59.5)
	);
	getStatistics().getHisto1D("Decay_Into3G_DistFromZero")->SetXTitle("Time difference between annihilation hits [cm]");
	getStatistics().getHisto1D("Decay_Into3G_DistFromZero")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH2F("Decay_Into3G_pos",
	    "Pos reco",
	    218, -54.5, 54.5,
	    218, -54.5, 54.5)
	);
	getStatistics().getHisto2D("Decay_Into3G_pos")->SetXTitle("Y [cm]");
	getStatistics().getHisto2D("Decay_Into3G_pos")->SetYTitle("X [cm]");
	
	getStatistics().createHistogram(
	  new TH1F("Decay_Into3G_posZ",
	    "Z pos reco",
	    220, -54.5, 54.5)
	);
	getStatistics().getHisto1D("Decay_Into3G_posZ")->SetXTitle("Z [cm]");
	getStatistics().getHisto1D("Decay_Into3G_posZ")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Decay_Into3G_Lifetime",
	    "Annihilation-Deexcitation Hits Time Difference",
	    4000, -195.5, 195.5)
	);
	getStatistics().getHisto1D("Decay_Into3G_Lifetime")->SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
	getStatistics().getHisto1D("Decay_Into3G_Lifetime")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Annihilation_Hits_in_event",
	    "Number of Annihilation Hits in Event", 
	    50, -0.5, 49.5)
	);
	getStatistics().getHisto1D("Annihilation_Hits_in_event")->SetXTitle("Number of Annihilation Hits in Event");
	getStatistics().getHisto1D("Annihilation_Hits_in_event")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Deexcitation_Hits_in_event",
	    "Number of Deexcitation Hits in Event", 
	    50, -0.5, 49.5)
	);
	getStatistics().getHisto1D("Deexcitation_Hits_in_event")->SetXTitle("Number of Deexcitation Hits in Event");
	getStatistics().getHisto1D("Deexcitation_Hits_in_event")->SetYTitle("Counts");
  }
  return true;
}

bool EventCategorizerPhysics::exec()
{
  vector<JPetEvent> events;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) 
  {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {

	const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
	if (event.getHits().size() >= 1) 
	{
		vector<JPetHit> hits = event.getHits();	 
		JPetEvent physicEvent = physicsAnalysis( hits );
		if( physicEvent.getHits().size() )
			events.push_back( physicEvent );
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

bool EventCategorizerPhysics::terminate()
{
  return true;
}

void EventCategorizerPhysics::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}

JPetEvent EventCategorizerPhysics::physicsAnalysis( vector<JPetHit> hits )
{
	JPetEvent physicEvent;
  
	vector<JPetHit> annihilationHits;
	vector<JPetHit> deexcitationHits;
	
	float zPositionCut = 23.;
	float minimalAngleForDecayInto2 = 165.;
	float maximalTimeDiffForDecayInto2 = 5.;
	float maximalDistFromZeroOfDecayPlaneForDecayInto2 = 5.;
	float minimalAngleForDecayInto3 = 185.;
	float maximalTimeDiffForDecayInto3 = 1.;
	float maximalDistFromZeroOfDecayPlaneForDecayInto3 = 10.;
	
	for( unsigned i=0; i<hits.size(); i++ )
	{
		if( fabs( hits[i].getPosZ() ) < zPositionCut )
		{
			double TOTofHit = EventCategorizerTools::calcTOT( hits[i] );
			if( fSaveControlHistos )
			{
				getStatistics().getHisto1D("TOT")->Fill( TOTofHit );
			}
			if( TOTofHit >= fMinAnnihilationTOT/1000 && TOTofHit <= fMaxAnnihilationTOT/1000 )
			{
				annihilationHits.push_back( hits[i] );
			}
			if( TOTofHit >= fMinDeexcitationTOT/1000 && TOTofHit <= fMaxDeexcitationTOT/1000 )
			{
				deexcitationHits.push_back( hits[i] );
			}
		}
	}
	if( fSaveControlHistos )
	{
		getStatistics().getHisto1D("Annihilation_Hits_in_event")->Fill( annihilationHits.size() );
		getStatistics().getHisto1D("Deexcitation_Hits_in_event")->Fill( deexcitationHits.size() );
	}
	if( annihilationHits.size() * deexcitationHits.size() )
	{
		for( unsigned i=0; i<annihilationHits.size(); i++ )
		{
			if( fSaveControlHistos )
			{
				getStatistics().getHisto1D("Quick_Positronium_Lifetime")->Fill(  EventCategorizerTools::normalizeTime( annihilationHits[i] ) - EventCategorizerTools::normalizeTime( deexcitationHits[0] ) );
			}
			break;
		}
		if( annihilationHits.size() == 2 && deexcitationHits.size() == 1 )
		{
			float angleDiff = EventCategorizerTools::calcAngle( annihilationHits[0], annihilationHits[1] );
			float timeDiff = fabs( EventCategorizerTools::normalizeTime(annihilationHits[1]) - EventCategorizerTools::normalizeTime( annihilationHits[0] ) );
			float distFrom0 = EventCategorizerTools::calcDistanceOfSurfaceAndZero( deexcitationHits[0], annihilationHits[0], annihilationHits[1] );
			if( fSaveControlHistos )
			{
				getStatistics().getHisto1D("Decay_Into2G_AnniAngle")->Fill( angleDiff );
				getStatistics().getHisto1D("Decay_Into2G_TimeDiff")->Fill( timeDiff );
				getStatistics().getHisto1D("Decay_Into2G_DistFromZero")->Fill( distFrom0 );
			}
			if( annihilationHits[0].getScintillator().getID() != annihilationHits[1].getScintillator().getID() && deexcitationHits[0].getScintillator().getID() != annihilationHits[1].getScintillator().getID() && annihilationHits[0].getScintillator().getID() != deexcitationHits[0].getScintillator().getID() )
			{
				if( timeDiff < maximalTimeDiffForDecayInto2 && angleDiff > minimalAngleForDecayInto2 && distFrom0 < maximalDistFromZeroOfDecayPlaneForDecayInto2 )
				{
					TVector3 recoPos = EventCategorizerTools::recoPosition( annihilationHits[0], annihilationHits[1] );
					if( fSaveControlHistos )
					{
						getStatistics().getHisto2D("Decay_Into2G_pos")->Fill( recoPos(1), recoPos(0) );
						getStatistics().getHisto1D("Decay_Into2G_posZ")->Fill( recoPos(2) );
					}
					if( recoPos.Mag() < 5 )
					{
						physicEvent.addHit(annihilationHits[0]);
						physicEvent.addHit(annihilationHits[1]);
						physicEvent.addHit(deexcitationHits[0]);
						physicEvent.setEventType(JPetEventType::k2Gamma);
						physicEvent.addEventType(JPetEventType::kPrompt);
						
						if( fSaveControlHistos )
						{
							getStatistics().getHisto1D("Decay_Into2G_Lifetime")->Fill( ( EventCategorizerTools::normalizeTime( annihilationHits[1] ) + EventCategorizerTools::normalizeTime( annihilationHits[0] ) )/2 - EventCategorizerTools::normalizeTime( deexcitationHits[0] ) );
						}
					}
				}
			}
		}
		if( annihilationHits.size() >= 3 && deexcitationHits.size() == 1 )
		{
			TVector3 bestPositionFromZero(100,100,100);
			float bestLifetime = 0.;
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
						if( deexcitationHits[0].getScintillator().getID() != annihilationHits[k].getScintillator().getID() && deexcitationHits[0].getScintillator().getID() != annihilationHits[i].getScintillator().getID() && annihilationHits[j].getScintillator().getID() != deexcitationHits[0].getScintillator().getID() && annihilationHits[i].getScintillator().getID() != annihilationHits[j].getScintillator().getID() && annihilationHits[i].getScintillator().getID() != annihilationHits[k].getScintillator().getID() && annihilationHits[j].getScintillator().getID() != annihilationHits[k].getScintillator().getID() )
						{
							vector<double> angles = EventCategorizerTools::calcAnglesFrom3Hit( annihilationHits[i], annihilationHits[j], annihilationHits[k] );
							double distanceFromZero = EventCategorizerTools::calcDistanceOfSurfaceAndZero( annihilationHits[i], annihilationHits[j], annihilationHits[k] );
							if( fSaveControlHistos )
							{
								getStatistics().getHisto2D("Decay_Into3G_angles")->Fill( angles[0] + angles[1], angles[1] - angles[0] );
								getStatistics().getHisto1D("Decay_Into3G_distance_from_0")->Fill( distanceFromZero );
							}
							if( angles[0] + angles[1] > minimalAngleForDecayInto3 && distanceFromZero < maximalDistFromZeroOfDecayPlaneForDecayInto3 )
							{
								double timeDiff3Hit = fabs( 2*EventCategorizerTools::normalizeTime( annihilationHits[i] ) - EventCategorizerTools::normalizeTime( annihilationHits[j] ) - EventCategorizerTools::normalizeTime( annihilationHits[k] ) )/1000;
								if( fSaveControlHistos )
								{
									getStatistics().getHisto1D("Decay_Into3G_Time_Diff")->Fill( timeDiff3Hit );
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
										bestLifetime = ( EventCategorizerTools::normalizeTime( annihilationHits[i] ) + EventCategorizerTools::normalizeTime( annihilationHits[j] ) + EventCategorizerTools::normalizeTime( annihilationHits[k] ) )/3 - EventCategorizerTools::normalizeTime( deexcitationHits[0] );
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
				physicEvent.addHit(annihilationHits[bestIndices[0]]);
				physicEvent.addHit(annihilationHits[bestIndices[1]]);
				physicEvent.addHit(annihilationHits[bestIndices[2]]);
				physicEvent.addHit(deexcitationHits[0]);
				if( physicEvent.getEventType() == JPetEventType::kUnknown )
				{
					physicEvent.setEventType(JPetEventType::k3Gamma);
					physicEvent.addEventType(JPetEventType::kPrompt);
				}
				else
				{
					physicEvent.addEventType(JPetEventType::k3Gamma);
				}
				
				if( fSaveControlHistos )
				{
					getStatistics().getHisto2D("Decay_Into3G_pos")->Fill( bestPositionFromZero(1), bestPositionFromZero(0) );
					getStatistics().getHisto1D("Decay_Into3G_posZ")->Fill( bestPositionFromZero(2) );
					getStatistics().getHisto1D("Decay_Into3G_Lifetime")->Fill( bestLifetime );
				}			  
			}
		}
	}
	return physicEvent;
}