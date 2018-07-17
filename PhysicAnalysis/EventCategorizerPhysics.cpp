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
	
  if ( isOptionSet(fParams.getOptions(), kMaxZPosParamKey) )
	fMaxZPos = getOptionAsFloat(fParams.getOptions(), kMaxZPosParamKey);
  if ( isOptionSet(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey) )
	fMaxDistOfDecayPlaneFromCenter = getOptionAsFloat(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey);
  if ( isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey) )
	fMaxTimeDiff = getOptionAsFloat(fParams.getOptions(), kMaxTimeDiffParamKey);
  if ( isOptionSet(fParams.getOptions(), kBackToBackAngleWindowParamKey) )
	fBackToBackAngleWindow = getOptionAsFloat(fParams.getOptions(), kBackToBackAngleWindowParamKey);
  if ( isOptionSet(fParams.getOptions(), kDecayInto3MinAngleParamKey) )
	fDecayInto3MinAngle = getOptionAsFloat(fParams.getOptions(), kDecayInto3MinAngleParamKey);
  
  if (fSaveControlHistos) 
  {
	getStatistics().createHistogram(
	  new TH1F("TOT", "TOT of Hits",
	    1000, -0.5, 99.5));
	getStatistics().getHisto1D("TOT")->SetXTitle("TOT [ns]");
	getStatistics().getHisto1D("TOT")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Positronium_Lifetime", "Annihilation-Deexcitation Hits Time Difference",
	    4000, -195.5, 195.5));
	getStatistics().getHisto1D("Positronium_Lifetime")->SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
	getStatistics().getHisto1D("Positronium_Lifetime")->SetYTitle("Counts");
    
	getStatistics().createHistogram(
	  new TH2F("DecayInto2_XY", "Pos reco",
	    218, -54.5, 54.5,
	    218, -54.5, 54.5));
	getStatistics().getHisto2D("DecayInto2_XY")->SetXTitle("Y [cm]");
	getStatistics().getHisto2D("DecayInto2_XY")->SetYTitle("X [cm]");
	
	getStatistics().createHistogram(
	  new TH1F("DecayInto2_Z", "Z pos reco",
	    220, -54.5, 54.5));
	getStatistics().getHisto1D("DecayInto2_Z")->SetXTitle("Z [cm]");
	getStatistics().getHisto1D("DecayInto2_Z")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("DecayInto2_Angles", "Annihilation Hits angles",
	    360, -0.5, 359.5));
	getStatistics().getHisto1D("DecayInto2_Angles")->SetXTitle("Angle between annihilation hits [deg]");
	getStatistics().getHisto1D("DecayInto2_Angles")->SetYTitle("Counts");	
		
	getStatistics().createHistogram(
	  new TH1F("DecayInto2_TimeDiff", "Annihilation Hits Time Difference",
	    2000, -0.5, 195.5));
	getStatistics().getHisto1D("DecayInto2_TimeDiff")->SetXTitle("Time difference between annihilation hits [ns]");
	getStatistics().getHisto1D("DecayInto2_TimeDiff")->SetYTitle("Counts");	
	
	getStatistics().createHistogram(
	  new TH1F("DecayInto2_Distance", "Annihilation Hit plane distance",
	    500, -0.5, 49.5));
	getStatistics().getHisto1D("DecayInto2_Distance")->SetXTitle("Distance of hit surface from zero [cm]");
	getStatistics().getHisto1D("DecayInto2_Distance")->SetYTitle("Counts");	
		
	getStatistics().createHistogram(
	  new TH2F("DecayInto3_Angles", "Annihilation Hits angles",
	    360, -0.5, 359.5,
	    360, -0.5, 359.5));
	getStatistics().getHisto2D("DecayInto3_Angles")->SetXTitle("Sum of the two smallest angles [deg]");
	getStatistics().getHisto2D("DecayInto3_Angles")->SetYTitle("Difference of the two smallest angles [deg]");	
		
	getStatistics().createHistogram(
	  new TH1F("DecayInto3_Distance", "3 Annihilation Hit plane distance",
	    500, -0.5, 49.5));
	getStatistics().getHisto1D("DecayInto3_Distance")->SetXTitle("Distance of hit surface from zero [cm]");
	getStatistics().getHisto1D("DecayInto3_Distance")->SetYTitle("Counts");	
		
	getStatistics().createHistogram(
	  new TH1F("DecayInto3_TimeDiff", "Annihilation Hits Time Difference",
	    2000, -0.5, 195.5));
	getStatistics().getHisto1D("DecayInto3_TimeDiff")->SetXTitle("Time difference between annihilation hits [ns]");
	getStatistics().getHisto1D("DecayInto3_TimeDiff")->SetYTitle("Counts");	
	
	getStatistics().createHistogram(
	  new TH1F("Annihilation_Hits_in_event", "Number of Annihilation Hits in Event", 
	    50, -0.5, 49.5));
	getStatistics().getHisto1D("Annihilation_Hits_in_event")->SetXTitle("Number of Annihilation Hits in Event");
	getStatistics().getHisto1D("Annihilation_Hits_in_event")->SetYTitle("Counts");
		
	getStatistics().createHistogram(
	  new TH1F("Deexcitation_Hits_in_event", "Number of Deexcitation Hits in Event", 
	    50, -0.5, 49.5));
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
  
	JPetEvent annihilationHits;
	JPetEvent deexcitationHits;
	
	for( unsigned i=0; i<hits.size(); i++ )
	{
		if( fabs( hits[i].getPosZ() ) < fMaxZPos )
		{
			double TOTofHit = EventCategorizerTools::calculateTOT( hits[i] );
			if( fSaveControlHistos )
			{
				getStatistics().getHisto1D("TOT")->Fill( TOTofHit );
			}
			if( TOTofHit >= fMinAnnihilationTOT && TOTofHit <= fMaxAnnihilationTOT )
			{
				physicEvent.addHit( hits[i] );
				annihilationHits.addHit( hits[i] );
			}
			if( TOTofHit >= fMinDeexcitationTOT && TOTofHit <= fMaxDeexcitationTOT )
			{
				physicEvent.addHit( hits[i] );
				deexcitationHits.addHit( hits[i] );
			}
		}
	}
	
	if( fSaveControlHistos )
	{
		getStatistics().getHisto1D("Annihilation_Hits_in_event")->Fill( annihilationHits.getHits().size() );
		getStatistics().getHisto1D("Deexcitation_Hits_in_event")->Fill( deexcitationHits.getHits().size() );
	}
	if( deexcitationHits.getHits().size() > 0 )
	{
		physicEvent.setEventType(JPetEventType::kPrompt);
		if( annihilationHits.getHits().size() > 0 )
		{
			getStatistics().getHisto1D("Positronium_Lifetime")->Fill( annihilationHits.getHits().at(0).getTime()/1000. - deexcitationHits.getHits().at(0).getTime()/1000. );
		}
	}
	if( EventCategorizerTools::checkFor2Gamma(annihilationHits, getStatistics(), fSaveControlHistos, fBackToBackAngleWindow, fMaxTimeDiff, fMaxDistOfDecayPlaneFromCenter) )
	{
		physicEvent.setEventType(JPetEventType::k2Gamma);
	}
	if( EventCategorizerTools::checkFor3Gamma(annihilationHits, getStatistics(), fSaveControlHistos, fDecayInto3MinAngle, fMaxTimeDiff, fMaxDistOfDecayPlaneFromCenter) )
	{
		physicEvent.setEventType(JPetEventType::k3Gamma);
	}
	
	return physicEvent;
}
