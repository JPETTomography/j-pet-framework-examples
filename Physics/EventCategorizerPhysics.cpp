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
#include "EventCategorizerPhysics.h"
#include "EventCategorizerTools.h"

using namespace std;

vector<unsigned> EventCategorizerPhysics::PhysicsAnalysis( std::vector<JPetHit> Hits, double MinAnnihilationTOT, double MaxAnnihilationTOT, double MinDeexcitationTOT, double MaxDeexcitationTOT, JPetStatistics& Stats, bool SaveControlHistos )
{
	vector<unsigned> NmbOfPhysicalPhenomena;
	NmbOfPhysicalPhenomena.push_back(0);
	NmbOfPhysicalPhenomena.push_back(0);
	vector<JPetHit> AnnihilationHits;
	vector<JPetHit> DeexcitationHits;
	for( unsigned i=0; i<Hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::CalcTOT( Hits[i] );
		if( SaveControlHistos )
		{
			Stats.getHisto1D("TOT").Fill( TOTofHit );
		}
		if( TOTofHit >= MinAnnihilationTOT && TOTofHit <= MaxAnnihilationTOT && fabs( Hits[i].getPosZ() ) < 23 )
		{
			AnnihilationHits.push_back( Hits[i] );
		}
		if( TOTofHit >= MinDeexcitationTOT && TOTofHit <= MaxDeexcitationTOT && fabs( Hits[i].getPosZ() ) < 23 )
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
	if( SaveControlHistos )
	{
		Stats.getHisto1D("Annihilation_Hits_in_event").Fill( AnnihilationHits.size() );
		Stats.getHisto1D("Deexcitation_Hits_in_event").Fill( DeexcitationHits.size() );
	}
	if( AnnihilationHits.size() * DeexcitationHits.size() )
	{
		for( unsigned i=0; i<AnnihilationHits.size(); i++ )
		{
			int ScatterTest = EventCategorizerTools::CheckIfScattered( DeexcitationHits[0], AnnihilationHits[i],  0.5 );
			if( !ScatterTest )
			{
				if( SaveControlHistos )
				{
					Stats.getHisto1D("Quick_Positronium_Lifetime").Fill(  EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) - EventCategorizerTools::NormalizeTime( DeexcitationHits[0] ) );
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
			if( SaveControlHistos )
			{
				Stats.getHisto1D("Decay_Into2G_AnniAngle").Fill( AngleDiff );
				Stats.getHisto1D("Decay_Into2G_TimeDiff").Fill( TimeDiff );
				Stats.getHisto1D("Decay_Into2G_DistFromZero").Fill( DistFrom0 );
			}
			if( TimeDiff < 5 && AngleDiff > 165 && !scatterTest && DistFrom0 < 10 && AnnihilationHits[0].getScintillator().getID() != AnnihilationHits[1].getScintillator().getID() && DeexcitationHits[0].getScintillator().getID() != AnnihilationHits[1].getScintillator().getID() && AnnihilationHits[0].getScintillator().getID() != DeexcitationHits[0].getScintillator().getID() )
			{
				TVector3 RecoPos = EventCategorizerTools::RecoPosition( AnnihilationHits[0], AnnihilationHits[1] );
				if ( SaveControlHistos )
				{
					Stats.getHisto2D("Decay_Into2G_pos").Fill( RecoPos(1), RecoPos(0) );
					Stats.getHisto1D("Decay_Into2G_posZ").Fill( RecoPos(2) );
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
					Stats.getHisto1D("Decay_Into2G_Lifetime").Fill( ( EventCategorizerTools::NormalizeTime( AnnihilationHits[1] ) + EventCategorizerTools::NormalizeTime( AnnihilationHits[0] ) )/2 - EventCategorizerTools::NormalizeTime( DeexcitationHits[0] ) );
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
							if ( SaveControlHistos )
							{
								Stats.getHisto2D("Decay_Into3G_angles").Fill( angles[0] + angles[1], angles[1] - angles[0] );
								Stats.getHisto1D("Decay_Into3G_distance_from_0").Fill( DistanceFromZero );
							}
							if( angles[0] + angles[1] > 181 && DistanceFromZero < 10 )
							{
								double TimeDiff3Hit = fabs( 2*EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[j] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[k] ) )/1000;
								if ( SaveControlHistos )
								{
									Stats.getHisto1D("Decay_Into3G_Time_Diff").Fill( TimeDiff3Hit );
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
				if( SaveControlHistos )
				{
					Stats.getHisto2D("Decay_Into3G_pos").Fill( BestPositionFromZero(1), BestPositionFromZero(0) );
					Stats.getHisto1D("Decay_Into3G_posZ").Fill( BestPositionFromZero(2) );
					Stats.getHisto1D("Decay_Into3G_Lifetime").Fill( BestLifetime );
				}			  
			}
		}
	}
	return NmbOfPhysicalPhenomena;
}