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
    
  if ( isOptionSet(fParams.getOptions(), fMinAnnihilationParamKey) )
    kMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), fMinAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), fMaxAnnihilationParamKey) )
    kMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), fMaxAnnihilationParamKey);
  
  
  if (fSaveControlHistos) 
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
	unsigned ImagingHits = 0;
	if (event.getHits().size() > 1) 
	{
		vector<JPetHit> hits = event.getHits();	 
		ImagingHits = Imaging( hits );
		if( ImagingHits )
		{
			JPetEvent newEvent;
			for (auto i = hits.begin(); i != hits.end(); ++i) 
			{
				auto& hit = *i;
				newEvent.addHit(hit);
			}
			newEvent.setEventType(JPetEventType::kUnknown);
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

bool EventCategorizerImaging::terminate()
{
  //INFO("More than one hit Events done. Writing control histograms.");
  return true;
}

void EventCategorizerImaging::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}


int EventCategorizerImaging::Imaging( vector<JPetHit> Hits )
{
	int NmbOfAnnihilations = 0;
	vector<JPetHit> AnnihilationHits;
	for( unsigned i=0; i<Hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::CalcTOT( Hits[i] );
		if( TOTofHit >= kMinAnnihilationTOT/1000 && TOTofHit <= kMaxAnnihilationTOT/1000 && fabs( Hits[i].getPosZ() ) < 23 )
		{
			AnnihilationHits.push_back( Hits[i] );
		}
	}
	if( AnnihilationHits.size() < 2 )
		return 0;
	for( unsigned i=0; i<AnnihilationHits.size() - 1; i++ )
	{
		for( unsigned j=i+1; j<AnnihilationHits.size(); j++ )
		{
			int scatterTest = EventCategorizerTools::CheckIfScattered( AnnihilationHits[i], AnnihilationHits[j], 0.25 );
			if( !scatterTest && AnnihilationHits[i].getScintillator().getID() != AnnihilationHits[j].getScintillator().getID() )
			{
				float AngleDiffinXY = EventCategorizerTools::CalcAngle2D( AnnihilationHits[i], AnnihilationHits[j] );
				double TimeDiff = fabs( AnnihilationHits[j].getTime() - AnnihilationHits[i].getTime() )/1000.;
				if( fSaveControlHistos )
				{
					getStatistics().getHisto1D("Imaging_2_Hit_Angles_2D").Fill( AngleDiffinXY );
					getStatistics().getHisto1D("Imaging_2_Hit_Time_Diff").Fill( TimeDiff );
				}
				if( TimeDiff < 5 && AngleDiffinXY > 95 )
				{
					TVector3 RecoPos = EventCategorizerTools::RecoPosition( AnnihilationHits[i], AnnihilationHits[j] );
					if( NmbOfAnnihilations )
						NmbOfAnnihilations *= 2;
					else
						NmbOfAnnihilations += 2;
					if( fSaveControlHistos )
					{
						getStatistics().getHisto2D("Imaging_2_Hit_pos").Fill( RecoPos(1), RecoPos(0) );
						getStatistics().getHisto1D("Imaging_2_Hit_posZ").Fill( RecoPos(2) );
					}
				}
			}
		}
	}
	if( AnnihilationHits.size() < 3 )
		return NmbOfAnnihilations;
	for( unsigned i=0; i<AnnihilationHits.size() - 2; i++ )
	{
		for( unsigned j=i+1; j<AnnihilationHits.size() - 1; j++ )
		{
			for( unsigned k=j+1; k<AnnihilationHits.size(); k++ )
			{
				int scatterTest = EventCategorizerTools::CheckIfScattered( AnnihilationHits[i], AnnihilationHits[j], 0.25 ) + EventCategorizerTools::CheckIfScattered( AnnihilationHits[i], AnnihilationHits[k], 0.25 ) + EventCategorizerTools::CheckIfScattered( AnnihilationHits[j], AnnihilationHits[k], 0.25 );
				if( !scatterTest && AnnihilationHits[i].getScintillator().getID() != AnnihilationHits[j].getScintillator().getID() && AnnihilationHits[i].getScintillator().getID() != AnnihilationHits[k].getScintillator().getID() && AnnihilationHits[j].getScintillator().getID() != AnnihilationHits[k].getScintillator().getID() )
				{
					vector<double> angles = EventCategorizerTools::CalcAnglesFrom3Hit( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
					double DistanceFromZero = EventCategorizerTools::CalcDistanceOfSurfaceAndZero( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
					if( fSaveControlHistos )
					{
						getStatistics().getHisto2D("Imaging_3_Hit_Angles").Fill( angles[0] + angles[1], angles[1] - angles[0] );
						getStatistics().getHisto1D("Imaging_3_Hit_Distance_from_0").Fill( DistanceFromZero );
					}
					if( angles[0] + angles[1] > 185 && DistanceFromZero < 10 )
					{
						double TimeDiff3Hit = fabs( 2*EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[j] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[k] ) )/1000;
						if ( fSaveControlHistos )
						{
							getStatistics().getHisto1D("Imaging_3_Hit_Time_Diff").Fill( TimeDiff3Hit );
						}
						if( TimeDiff3Hit < 1 )
						{
							TVector3 RecoPos3Hit = EventCategorizerTools::RecoPosition3Hit( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
							if( NmbOfAnnihilations )
								NmbOfAnnihilations *= 3;
							else
								NmbOfAnnihilations += 3;
							if ( fSaveControlHistos )
							{
								getStatistics().getHisto2D("Imaging_3_Hit_pos").Fill( RecoPos3Hit(1), RecoPos3Hit(0) );
								getStatistics().getHisto1D("Imaging_3_Hit_posZ").Fill( RecoPos3Hit(2) );
							}
						}
					}
					
				}
			}
		}
	}
	
	return NmbOfAnnihilations;
}