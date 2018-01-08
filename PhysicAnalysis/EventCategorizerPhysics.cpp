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
  
  if ( isOptionSet(fParams.getOptions(), fMinAnnihilationParamKey) )
    kMinAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), fMinAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), fMaxAnnihilationParamKey) )
    kMaxAnnihilationTOT = getOptionAsFloat(fParams.getOptions(), fMaxAnnihilationParamKey);
  if ( isOptionSet(fParams.getOptions(), fMinDeexcitationParamKey) )
    kMinDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), fMinDeexcitationParamKey);
  if ( isOptionSet(fParams.getOptions(), fMaxDeexcitationParamKey) )
    kMaxDeexcitationTOT = getOptionAsFloat(fParams.getOptions(), fMaxDeexcitationParamKey);
  
  if (fSaveControlHistos) 
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
	vector<unsigned> PhysicHits;
	PhysicHits.push_back(0);
	PhysicHits.push_back(0);
	if (event.getHits().size() >= 1) 
	{
		vector<JPetHit> hits = event.getHits();	 
		PhysicHits = PhysicsAnalysis( hits );
		if( PhysicHits[0] + PhysicHits[1] )
		{
			JPetEvent newEvent;
			for (auto i = hits.begin(); i != hits.end(); ++i) 
			{
				auto& hit = *i;
				newEvent.addHit(hit);
			}
			newEvent.setEventType(JPetEventType::kUnknown);
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

bool EventCategorizerPhysics::terminate()
{
  //INFO("More than one hit Events done. Writing control histograms.");
  return true;
}

void EventCategorizerPhysics::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}

vector<unsigned> EventCategorizerPhysics::PhysicsAnalysis( std::vector<JPetHit> Hits )
{
	vector<unsigned> NmbOfPhysicalPhenomena;
	NmbOfPhysicalPhenomena.push_back(0);
	NmbOfPhysicalPhenomena.push_back(0);
	vector<JPetHit> AnnihilationHits;
	vector<JPetHit> DeexcitationHits;
	for( unsigned i=0; i<Hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::CalcTOT( Hits[i] );
		if( fSaveControlHistos )
		{
			getStatistics().getHisto1D("TOT").Fill( TOTofHit );
		}
		if( TOTofHit >= kMinAnnihilationTOT/1000 && TOTofHit <= kMaxAnnihilationTOT/1000 && fabs( Hits[i].getPosZ() ) < 23 )
		{
			AnnihilationHits.push_back( Hits[i] );
		}
		if( TOTofHit >= kMinDeexcitationTOT/1000 && TOTofHit <= kMaxDeexcitationTOT/1000 && fabs( Hits[i].getPosZ() ) < 23 )
		{
			DeexcitationHits.push_back( Hits[i] );
		}
		if( i != Hits.size() - 1 )
		{
			for( unsigned j=i+1; j<Hits.size(); j++ )
			{
				int ScatterTest = EventCategorizerTools::CheckIfScattered( Hits[i], Hits[j],  0.5 );
				if( ScatterTest )
				{
					NmbOfPhysicalPhenomena[0] += 1;
				}
			}
		}
	}
	if( fSaveControlHistos )
	{
		getStatistics().getHisto1D("Annihilation_Hits_in_event").Fill( AnnihilationHits.size() );
		getStatistics().getHisto1D("Deexcitation_Hits_in_event").Fill( DeexcitationHits.size() );
	}
	if( AnnihilationHits.size() * DeexcitationHits.size() )
	{
		for( unsigned i=0; i<AnnihilationHits.size(); i++ )
		{
			int ScatterTest = EventCategorizerTools::CheckIfScattered( DeexcitationHits[0], AnnihilationHits[i],  0.5 );
			if( !ScatterTest )
			{
				if( fSaveControlHistos )
				{
					getStatistics().getHisto1D("Quick_Positronium_Lifetime").Fill(  EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) - EventCategorizerTools::NormalizeTime( DeexcitationHits[0] ) );
				}
				break;
			}
		}
		if( AnnihilationHits.size() == 2 && DeexcitationHits.size() == 1 )
		{
			int scatterTest = EventCategorizerTools::CheckIfScattered( AnnihilationHits[0], AnnihilationHits[1], 0.25 ) + EventCategorizerTools::CheckIfScattered( AnnihilationHits[0], DeexcitationHits[0], 0.25 ) + EventCategorizerTools::CheckIfScattered( DeexcitationHits[0], AnnihilationHits[1], 0.25 );
			float AngleDiff = EventCategorizerTools::CalcAngle( AnnihilationHits[0], AnnihilationHits[1] );
			float TimeDiff = fabs( EventCategorizerTools::NormalizeTime(AnnihilationHits[1]) - EventCategorizerTools::NormalizeTime( AnnihilationHits[0] ) );
			float DistFrom0 = EventCategorizerTools::CalcDistanceOfSurfaceAndZero( DeexcitationHits[0], AnnihilationHits[0], AnnihilationHits[1] );
			if( fSaveControlHistos )
			{
				getStatistics().getHisto1D("Decay_Into2G_AnniAngle").Fill( AngleDiff );
				getStatistics().getHisto1D("Decay_Into2G_TimeDiff").Fill( TimeDiff );
				getStatistics().getHisto1D("Decay_Into2G_DistFromZero").Fill( DistFrom0 );
			}
			if( TimeDiff < 5 && AngleDiff > 165 && !scatterTest && DistFrom0 < 10 && AnnihilationHits[0].getScintillator().getID() != AnnihilationHits[1].getScintillator().getID() && DeexcitationHits[0].getScintillator().getID() != AnnihilationHits[1].getScintillator().getID() && AnnihilationHits[0].getScintillator().getID() != DeexcitationHits[0].getScintillator().getID() )
			{
				TVector3 RecoPos = EventCategorizerTools::RecoPosition( AnnihilationHits[0], AnnihilationHits[1] );
				if( fSaveControlHistos )
				{
					getStatistics().getHisto2D("Decay_Into2G_pos").Fill( RecoPos(1), RecoPos(0) );
					getStatistics().getHisto1D("Decay_Into2G_posZ").Fill( RecoPos(2) );
				}
				if( RecoPos.Mag() < 5 )
				{
					if( !NmbOfPhysicalPhenomena[1] )
					{
						NmbOfPhysicalPhenomena[1] += 2;
						NmbOfPhysicalPhenomena[1] *= 5;
					}
					else
					{
						NmbOfPhysicalPhenomena[1] *= 2;
						NmbOfPhysicalPhenomena[1] *= 5;
					}
					if( fSaveControlHistos )
					{
						getStatistics().getHisto1D("Decay_Into2G_Lifetime").Fill( ( EventCategorizerTools::NormalizeTime( AnnihilationHits[1] ) + EventCategorizerTools::NormalizeTime( AnnihilationHits[0] ) )/2 - EventCategorizerTools::NormalizeTime( DeexcitationHits[0] ) );
					}
				}
			}
		}
		if( AnnihilationHits.size() >= 3 && DeexcitationHits.size() == 1 )
		{
			TVector3 BestPositionFromZero(100,100,100);
			float BestLifetime = 0.;
			bool IsThereDecayInto3 = false;
			for( unsigned i=0; i<AnnihilationHits.size() - 2; i++ )
			{
				for( unsigned j=i+1; j<AnnihilationHits.size() - 1; j++ )
				{
					for( unsigned k=j+1; k<AnnihilationHits.size(); k++ )
					{
						int scatterTest = EventCategorizerTools::CheckIfScattered( AnnihilationHits[i], AnnihilationHits[j], 0.25 ) + EventCategorizerTools::CheckIfScattered( AnnihilationHits[i], AnnihilationHits[k], 0.25 ) + EventCategorizerTools::CheckIfScattered( AnnihilationHits[j], AnnihilationHits[k], 0.25 );
						if( !scatterTest && DeexcitationHits[0].getScintillator().getID() != AnnihilationHits[k].getScintillator().getID() && DeexcitationHits[0].getScintillator().getID() != AnnihilationHits[i].getScintillator().getID() && AnnihilationHits[j].getScintillator().getID() != DeexcitationHits[0].getScintillator().getID() && AnnihilationHits[i].getScintillator().getID() != AnnihilationHits[j].getScintillator().getID() && AnnihilationHits[i].getScintillator().getID() != AnnihilationHits[k].getScintillator().getID() && AnnihilationHits[j].getScintillator().getID() != AnnihilationHits[k].getScintillator().getID() )
						{
							vector<double> angles = EventCategorizerTools::CalcAnglesFrom3Hit( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
							double DistanceFromZero = EventCategorizerTools::CalcDistanceOfSurfaceAndZero( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
							if( fSaveControlHistos )
							{
								getStatistics().getHisto2D("Decay_Into3G_angles").Fill( angles[0] + angles[1], angles[1] - angles[0] );
								getStatistics().getHisto1D("Decay_Into3G_distance_from_0").Fill( DistanceFromZero );
							}
							if( angles[0] + angles[1] > 181 && DistanceFromZero < 10 )
							{
								double TimeDiff3Hit = fabs( 2*EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[j] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[k] ) )/1000;
								if( fSaveControlHistos )
								{
									getStatistics().getHisto1D("Decay_Into3G_Time_Diff").Fill( TimeDiff3Hit );
								}
								if( TimeDiff3Hit < 1 )
								{
									TVector3 RecoPos3Hit = EventCategorizerTools::RecoPosition3Hit( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
									if( RecoPos3Hit.Mag() < BestPositionFromZero.Mag() )
									{
										BestPositionFromZero = RecoPos3Hit;
										BestLifetime = ( EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) + EventCategorizerTools::NormalizeTime( AnnihilationHits[j] ) + EventCategorizerTools::NormalizeTime( AnnihilationHits[k] ) )/3 - EventCategorizerTools::NormalizeTime( DeexcitationHits[0] );
										if( ! IsThereDecayInto3 )
											IsThereDecayInto3 = true;
									}
									
								}
							}
						}
					}
				}
			}
			if( IsThereDecayInto3 )
			{
				if( !NmbOfPhysicalPhenomena[1] )
				{
					NmbOfPhysicalPhenomena[1] += 3;
					NmbOfPhysicalPhenomena[1] *= 5;
				}
				else
				{
					NmbOfPhysicalPhenomena[1] *= 3;
					NmbOfPhysicalPhenomena[1] *= 5;
				}
				if( fSaveControlHistos )
				{
					getStatistics().getHisto2D("Decay_Into3G_pos").Fill( BestPositionFromZero(1), BestPositionFromZero(0) );
					getStatistics().getHisto1D("Decay_Into3G_posZ").Fill( BestPositionFromZero(2) );
					getStatistics().getHisto1D("Decay_Into3G_Lifetime").Fill( BestLifetime );
				}			  
			}
		}
	}
	return NmbOfPhysicalPhenomena;
}