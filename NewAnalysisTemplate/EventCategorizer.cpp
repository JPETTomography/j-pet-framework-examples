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
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "EventCategorizerCosmic.h"
#include "EventCategorizerImaging.h"
#include "EventCategorizerPhysics.h"

using namespace jpet_options_tools;

using namespace std;

EventCategorizer::EventCategorizer(const char* name): JPetUserTask(name) {}

bool EventCategorizer::init()
{

  INFO("Event categorization started.");
  
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if ( isOptionSet(fParams.getOptions(), fCosmicAnalysisParamKey) )
    kCosmicAnalysis = getOptionAsBool(fParams.getOptions(), fCosmicAnalysisParamKey);  
  if ( isOptionSet(fParams.getOptions(), fMinCosmicTOTParamKey) )
    kMinCosmicTOT = getOptionAsFloat(fParams.getOptions(), fMinCosmicTOTParamKey);
  
  if ( isOptionSet(fParams.getOptions(), fImagingParamKey) )
    kImaging = getOptionAsBool(fParams.getOptions(), fImagingParamKey);  
  if ( isOptionSet(fParams.getOptions(), fMinAnnihilationParamKey) )
    kMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), fMinAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), fMaxAnnihilationParamKey) )
    kMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), fMaxAnnihilationParamKey);
  
  if ( isOptionSet(fParams.getOptions(), fPhysicsAnalysisParamKey) )
    kPhysicsAnalysis = getOptionAsBool(fParams.getOptions(), fPhysicsAnalysisParamKey);  
  if ( isOptionSet(fParams.getOptions(), fMinDeexcitationParamKey) )
    kMinDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), fMinDeexcitationParamKey);
  if ( isOptionSet(fParams.getOptions(), fMaxDeexcitationParamKey) )
    kMaxDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), fMaxDeexcitationParamKey);
  
  if (fSaveControlHistos) 
  {
	if( kCosmicAnalysis )
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
	if( kImaging )
	{
		getStatistics().createHistogram(
			new TH2F("Imaging_2_Hit_pos",
				      "Pos reco",
				      218, -54.5, 54.5,
				      218, -54.5, 54.5)
		);
			getStatistics().getHisto2D("Imaging_2_Hit_pos").SetXTitle("Y [cm]");
			getStatistics().getHisto2D("Imaging_2_Hit_pos").SetYTitle("X [cm]");
		
		getStatistics().createHistogram(
			new TH1F("Imaging_2_Hit_posZ",
				      "Z pos reco",
				      220, -54.5, 54.5)
		);
			getStatistics().getHisto1D("Imaging_2_Hit_posZ").SetXTitle("Z [cm]");
			getStatistics().getHisto1D("Imaging_2_Hit_posZ").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Imaging_2_Hit_Angles_2D",
					"Annihilation Hits angles",
					360, -0.5, 359.5)
		);
			getStatistics().getHisto1D("Imaging_2_Hit_Angles_2D").SetXTitle("Angle between annihilation hits [deg]");
			getStatistics().getHisto1D("Imaging_2_Hit_Angles_2D").SetYTitle("Counts");	
			
		getStatistics().createHistogram(
			new TH1F("Imaging_2_Hit_Time_Diff",
					"Annihilation Hits Time Difference",
					2000, -0.5, 195.5)
		);
			getStatistics().getHisto1D("Imaging_2_Hit_Time_Diff").SetXTitle("Time difference between annihilation hits [ns]");
			getStatistics().getHisto1D("Imaging_2_Hit_Time_Diff").SetYTitle("Counts");	
	  
		getStatistics().createHistogram(
			new TH2F("Imaging_3_Hit_pos",
				      "Pos reco",
				      218, -54.5, 54.5,
				      218, -54.5, 54.5)
		);
			getStatistics().getHisto2D("Imaging_3_Hit_pos").SetXTitle("Y [cm]");
			getStatistics().getHisto2D("Imaging_3_Hit_pos").SetYTitle("X [cm]");
		
		getStatistics().createHistogram(
			new TH1F("Imaging_3_Hit_posZ",
				      "Z pos reco",
				      220, -54.5, 54.5)
		);
			getStatistics().getHisto1D("Imaging_3_Hit_posZ").SetXTitle("Z [cm]");
			getStatistics().getHisto1D("Imaging_3_Hit_posZ").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH2F("Imaging_3_Hit_Angles",
					"Annihilation Hits angles",
					360, -0.5, 359.5,
					360, -0.5, 359.5)
		);
			getStatistics().getHisto2D("Imaging_3_Hit_Angles").SetXTitle("Sum of the two smallest angles [deg]");
			getStatistics().getHisto2D("Imaging_3_Hit_Angles").SetYTitle("Difference of the two smallest angles [deg]");	
			
		getStatistics().createHistogram(
			new TH1F("Imaging_3_Hit_Distance_from_0",
					"3 Annihilation Hit plane distance",
					500, -0.5, 49.5)
		);
			getStatistics().getHisto1D("Imaging_3_Hit_Distance_from_0").SetXTitle("Distance of hit surface from zero [cm]");
			getStatistics().getHisto1D("Imaging_3_Hit_Distance_from_0").SetYTitle("Counts");	
			
		getStatistics().createHistogram(
			new TH1F("Imaging_3_Hit_Time_Diff",
					"Annihilation Hits Time Difference",
					2000, -0.5, 195.5)
		);
			getStatistics().getHisto1D("Imaging_3_Hit_Time_Diff").SetXTitle("Time difference between annihilation hits [ns]");
			getStatistics().getHisto1D("Imaging_3_Hit_Time_Diff").SetYTitle("Counts");	
	}
	if( kPhysicsAnalysis )
	{
		getStatistics().createHistogram(
			new TH1F("TOT",
				      "TOT of Hits",
				      1000, -0.5, 99.5)
		);
			getStatistics().getHisto1D("TOT").SetXTitle("TOT [ns]");
			getStatistics().getHisto1D("TOT").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Quick_Positronium_Lifetime",
					"Annihilation-Deexcitation Hits Time Difference",
					4000, -195.5, 195.5)
		);
			getStatistics().getHisto1D("Quick_Positronium_Lifetime").SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
			getStatistics().getHisto1D("Quick_Positronium_Lifetime").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into2G_AnniAngle",
					"Annihilation Hits angle",
					180, -0.5, 179.5)
		);
			getStatistics().getHisto1D("Decay_Into2G_AnniAngle").SetXTitle("Angle between annihilation hits [deg]");
			getStatistics().getHisto1D("Decay_Into2G_AnniAngle").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into2G_TimeDiff",
					"Annihilation Hits Time Difference",
					2000, -0.5, 195.5)
		);
			getStatistics().getHisto1D("Decay_Into2G_TimeDiff").SetXTitle("Time difference between annihilation hits [ns]");
			getStatistics().getHisto1D("Decay_Into2G_TimeDiff").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into2G_DistFromZero",
					"Difference of Hit Plane from Zero",
					600, -0.5, 59.5)
		);
			getStatistics().getHisto1D("Decay_Into2G_DistFromZero").SetXTitle("Time difference between annihilation hits [cm]");
			getStatistics().getHisto1D("Decay_Into2G_DistFromZero").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH2F("Decay_Into2G_pos",
				      "Pos reco",
				      218, -54.5, 54.5,
				      218, -54.5, 54.5)
		);
			getStatistics().getHisto2D("Decay_Into2G_pos").SetXTitle("Y [cm]");
			getStatistics().getHisto2D("Decay_Into2G_pos").SetYTitle("X [cm]");
		
		getStatistics().createHistogram(
			new TH1F("Decay_Into2G_posZ",
				      "Z pos reco",
				      220, -54.5, 54.5)
		);
			getStatistics().getHisto1D("Decay_Into2G_posZ").SetXTitle("Z [cm]");
			getStatistics().getHisto1D("Decay_Into2G_posZ").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into2G_Lifetime",
					"Annihilation-Deexcitation Hits Time Difference",
					4000, -195.5, 195.5)
		);
			getStatistics().getHisto1D("Decay_Into2G_Lifetime").SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
			getStatistics().getHisto1D("Decay_Into2G_Lifetime").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH2F("Decay_Into3G_AnniAngle",
					"Annihilation Hits angles",
					360, -0.5, 359.5,
					360, -0.5, 359.5)
		);
			getStatistics().getHisto2D("Decay_Into3G_AnniAngle").SetXTitle("Sum of the two smallest angles [deg]");
			getStatistics().getHisto2D("Decay_Into3G_AnniAngle").SetYTitle("Difference of the two smallest angles [deg]");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into3G_TimeDiff",
					"Annihilation Hits Time Difference",
					2000, -0.5, 195.5)
		);
			getStatistics().getHisto1D("Decay_Into3G_TimeDiff").SetXTitle("Time difference between annihilation hits [ns]");
			getStatistics().getHisto1D("Decay_Into3G_TimeDiff").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into3G_DistFromZero",
					"Difference of Hit Plane from Zero",
					600, -0.5, 59.5)
		);
			getStatistics().getHisto1D("Decay_Into3G_DistFromZero").SetXTitle("Time difference between annihilation hits [cm]");
			getStatistics().getHisto1D("Decay_Into3G_DistFromZero").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH2F("Decay_Into3G_pos",
				      "Pos reco",
				      218, -54.5, 54.5,
				      218, -54.5, 54.5)
		);
			getStatistics().getHisto2D("Decay_Into3G_pos").SetXTitle("Y [cm]");
			getStatistics().getHisto2D("Decay_Into3G_pos").SetYTitle("X [cm]");
		
		getStatistics().createHistogram(
			new TH1F("Decay_Into3G_posZ",
				      "Z pos reco",
				      220, -54.5, 54.5)
		);
			getStatistics().getHisto1D("Decay_Into3G_posZ").SetXTitle("Z [cm]");
			getStatistics().getHisto1D("Decay_Into3G_posZ").SetYTitle("Counts");
			
		getStatistics().createHistogram(
			new TH1F("Decay_Into3G_Lifetime",
					"Annihilation-Deexcitation Hits Time Difference",
					4000, -195.5, 195.5)
		);
			getStatistics().getHisto1D("Decay_Into3G_Lifetime").SetXTitle("Time difference between annihilation and deexcitation hits [ns]");
			getStatistics().getHisto1D("Decay_Into3G_Lifetime").SetYTitle("Counts");
			
		getStatistics().createHistogram(
				new TH1F("Annihilation_Hits_in_event",
				      "Number of Annihilation Hits in Event", 
				      50, -0.5, 49.5)
		);
			getStatistics().getHisto1D("Annihilation_Hits_in_event").SetXTitle("Number of Annihilation Hits in Event");
			getStatistics().getHisto1D("Annihilation_Hits_in_event").SetYTitle("Counts");
			
		getStatistics().createHistogram(
				new TH1F("Deexcitation_Hits_in_event",
				      "Number of Deexcitation Hits in Event", 
				      50, -0.5, 49.5)
		);
			getStatistics().getHisto1D("Deexcitation_Hits_in_event").SetXTitle("Number of Deexcitation Hits in Event");
			getStatistics().getHisto1D("Deexcitation_Hits_in_event").SetYTitle("Counts");
	}
  }
  return true;
}

bool EventCategorizer::exec()
{
  vector<JPetEvent> events;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) 
  {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {

	const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
	unsigned CosmicHits = 0, ImagingHits = 0;
	vector<unsigned> PhysicHits;
	PhysicHits.push_back(0);
	PhysicHits.push_back(0);
	if (event.getHits().size() >= 1) 
	{
		vector<JPetHit> hits = event.getHits();	 
		if( kCosmicAnalysis )
		{		
			CosmicHits = EventCosmic.CosmicAnalysis( hits, kMinCosmicTOT/1000, getStatistics(), fSaveControlHistos );	
		}
		if( kImaging && event.getHits().size() > 1 )
		{
			ImagingHits = EventImaging.Imaging( hits, kMinAnnihilationTOT/1000, kMaxAnnihilationTOT/1000, getStatistics(), fSaveControlHistos );
		}
		if( kPhysicsAnalysis && event.getHits().size() > 1 )
		{
			PhysicHits = EventPhysics.PhysicsAnalysis( hits, kMinAnnihilationTOT/1000, kMaxAnnihilationTOT/1000, kMinDeexcitationTOT/1000, kMaxDeexcitationTOT/1000, getStatistics(), fSaveControlHistos );
		}
		if( CosmicHits + ImagingHits + PhysicHits[0] + PhysicHits[1] )
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
			if( ImagingHits )
			{
				do
				{
					if( ImagingHits % 2 == 0 )
					{
						newEvent.addEventType( JPetEventType::k2Gamma );
						ImagingHits /= 2;
					}
					if( ImagingHits % 3 == 0 )
					{
						newEvent.addEventType( JPetEventType::k3Gamma );
						ImagingHits /= 3;
					}
				  
				}while( ImagingHits > 1 );
			}
			if( PhysicHits[1] )
			{
				do
				{
					if( PhysicHits[1] % 2 == 0 )
					{
						if( ! newEvent.isTypeOf( JPetEventType::k2Gamma ) )
							newEvent.addEventType( JPetEventType::k2Gamma );
						PhysicHits[1] /= 2;
					}
					if( PhysicHits[1] % 3 == 0 )
					{
						if( ! newEvent.isTypeOf( JPetEventType::k3Gamma ) )
							newEvent.addEventType( JPetEventType::k3Gamma );
						PhysicHits[1] /= 3;
					}
					if( PhysicHits[1] % 5 == 0 )
					{
						newEvent.addEventType( JPetEventType::kPrompt );
						PhysicHits[1] /= 5;
					}
				  
				}while( PhysicHits[1] > 1 );
			}
			if( PhysicHits[0] )
			{
				for( unsigned i=0; i<PhysicHits[0]; i++ )
				{
					newEvent.addEventType( JPetEventType::kScattered );
				}
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

bool EventCategorizer::terminate()
{
  //INFO("More than one hit Events done. Writing control histograms.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}
