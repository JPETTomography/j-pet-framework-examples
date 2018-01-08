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
 *  @file EventCategorizerCosmic.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "EventCategorizerCosmic.h"
#include "../LargeBarrelAnalysis/EventCategorizerTools.h"

using namespace jpet_options_tools;

using namespace std;

EventCategorizerCosmic::EventCategorizerCosmic(const char* name): JPetUserTask(name) {}

bool EventCategorizerCosmic::init()
{

  INFO("Event categorization started.");
  
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  
  if ( isOptionSet(fParams.getOptions(), fMinCosmicTOTParamKey) )
    kMinCosmicTOT = getOptionAsFloat(fParams.getOptions(), fMinCosmicTOTParamKey);
  
  
  if (fSaveControlHistos) 
  {
	getStatistics().createHistogram(
		      new TH1F("Cosmic_TOT",
			      "TOT of Cosmic Hits",
			      1000, -0.5, 99.5)
	);
		getStatistics().getHisto1D("Cosmic_TOT").SetXTitle("TOT [ns]");
		getStatistics().getHisto1D("Cosmic_TOT").SetYTitle("Counts");
	
	getStatistics().createHistogram(
			new TH1F("Cosmic_Hits_in_event",
			      "Number of Cosmic Hits in Event", 
			      50, -0.5, 49.5)
	);
		getStatistics().getHisto1D("Cosmic_Hits_in_event").SetXTitle("Number of Cosmic Hits in Event");
		getStatistics().getHisto1D("Cosmic_Hits_in_event").SetYTitle("Counts");
  }
  return true;
}

bool EventCategorizerCosmic::exec()
{
  vector<JPetEvent> events;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) 
  {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {

	const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
	unsigned CosmicHits = 0;
	if (event.getHits().size() >= 1) 
	{
		vector<JPetHit> hits = event.getHits();	 	
		CosmicHits = CosmicAnalysis( hits );	
		if( CosmicHits )
		{
			JPetEvent newEvent;
			for (auto i = hits.begin(); i != hits.end(); ++i) 
			{
				auto& hit = *i;
				newEvent.addHit(hit);
			}
			newEvent.setEventType(JPetEventType::kUnknown);
			for( unsigned i=0; i<CosmicHits; i++ )
			{
				newEvent.addEventType( JPetEventType::kUnknown );
			}
			events.push_back( newEvent );
		}
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

bool EventCategorizerCosmic::terminate()
{
  return true;
}

void EventCategorizerCosmic::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}


int EventCategorizerCosmic::CosmicAnalysis( vector<JPetHit> Hits )
{
	int NmbOfCosmics = 0;
	for( unsigned i=0; i<Hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::CalcTOT( Hits[i] );
		if( TOTofHit >= kMinCosmicTOT/1000 )
		{
			NmbOfCosmics++;
			if( fSaveControlHistos )
			{
				getStatistics().getHisto1D("Cosmic_TOT").Fill( TOTofHit );
			}
		}
	}
	if( fSaveControlHistos )
	{
		getStatistics().getHisto1D("Cosmic_Hits_in_event").Fill( NmbOfCosmics );
	}
	return NmbOfCosmics;
}