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
  
	for( unsigned i=0; i<hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::calculateTOT( hits[i] );
		if( TOTofHit >= fMinAnnihilationTOT && TOTofHit <= fMaxAnnihilationTOT && fabs( hits[i].getPosZ() ) < fMaxZPos )
		{
			imagingEvent.addHit(hits[i]);
		}
	}
	if( EventCategorizerTools::checkFor2Gamma(imagingEvent, getStatistics(), fSaveControlHistos, fBackToBackAngleWindow, fMaxTimeDiff, fMaxDistOfDecayPlaneFromCenter) )
	{
		imagingEvent.setEventType(JPetEventType::k2Gamma);
	}
	if( EventCategorizerTools::checkFor3Gamma(imagingEvent, getStatistics(), fSaveControlHistos, fDecayInto3MinAngle, fMaxTimeDiff, fMaxDistOfDecayPlaneFromCenter) )
	{
		imagingEvent.setEventType(JPetEventType::k3Gamma);
	}
	
	return imagingEvent;
}
