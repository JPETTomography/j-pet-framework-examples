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
#include "EventCategorizerImaging.h"
#include "EventCategorizerTools.h"

using namespace std;

int EventCategorizerImaging::Imaging( vector<JPetHit> Hits, double MinAnnihilationTOT, double MaxAnnihilationTOT, JPetStatistics& Stats, bool SaveControlHistos )
{
	int NmbOfAnnihilations = 0;
	vector<JPetHit> AnnihilationHits;
	for( unsigned i=0; i<Hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::CalcTOT( Hits[i] );
		if( TOTofHit >= MinAnnihilationTOT && TOTofHit <= MaxAnnihilationTOT && fabs( Hits[i].getPosZ() ) < 23 )
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
				if( SaveControlHistos )
				{
					Stats.getHisto1D("Imaging_2_Hit_Angles_2D").Fill( AngleDiffinXY );
					Stats.getHisto1D("Imaging_2_Hit_Time_Diff").Fill( TimeDiff );
				}
				if( TimeDiff < 5 && AngleDiffinXY > 95 )
				{
					TVector3 RecoPos = EventCategorizerTools::RecoPosition( AnnihilationHits[i], AnnihilationHits[j] );
					if( NmbOfAnnihilations )
						NmbOfAnnihilations *= 2;
					else
						NmbOfAnnihilations += 2;
					if ( SaveControlHistos )
					{
						Stats.getHisto2D("Imaging_2_Hit_pos").Fill( RecoPos(1), RecoPos(0) );
						Stats.getHisto1D("Imaging_2_Hit_posZ").Fill( RecoPos(2) );
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
					if ( SaveControlHistos )
					{
						Stats.getHisto2D("Imaging_3_Hit_Angles").Fill( angles[0] + angles[1], angles[1] - angles[0] );
						Stats.getHisto1D("Imaging_3_Hit_Distance_from_0").Fill( DistanceFromZero );
					}
					if( angles[0] + angles[1] > 181 && DistanceFromZero < 10 )
					{
						double TimeDiff3Hit = fabs( 2*EventCategorizerTools::NormalizeTime( AnnihilationHits[i] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[j] ) - EventCategorizerTools::NormalizeTime( AnnihilationHits[k] ) )/1000;
						if ( SaveControlHistos )
						{
							Stats.getHisto1D("Imaging_3_Hit_Time_Diff").Fill( TimeDiff3Hit );
						}
						if( TimeDiff3Hit < 1 )
						{
							TVector3 RecoPos3Hit = EventCategorizerTools::RecoPosition3Hit( AnnihilationHits[i], AnnihilationHits[j], AnnihilationHits[k] );
							if( NmbOfAnnihilations )
								NmbOfAnnihilations *= 3;
							else
								NmbOfAnnihilations += 3;
							if ( SaveControlHistos )
							{
								Stats.getHisto2D("Imaging_3_Hit_pos").Fill( RecoPos3Hit(1), RecoPos3Hit(0) );
								Stats.getHisto1D("Imaging_3_Hit_posZ").Fill( RecoPos3Hit(2) );
							}
						}
					}
					
				}
			}
		}
	}
	
	return NmbOfAnnihilations;
}