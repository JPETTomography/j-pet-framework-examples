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
#include "EventCategorizerCosmic.h"
#include "EventCategorizerTools.h"

using namespace std;

int EventCategorizerCosmic::CosmicAnalysis( vector<JPetHit> Hits, double MinTOT, JPetStatistics& Stats, bool SaveControlHistos )
{
	int NmbOfCosmics = 0;
	for( unsigned i=0; i<Hits.size(); i++ )
	{
		double TOTofHit = EventCategorizerTools::CalcTOT( Hits[i] );
		if( TOTofHit >= MinTOT )
		{
			NmbOfCosmics++;
			if( SaveControlHistos )
			{
				Stats.getHisto1D("Cosmic_TOT").Fill( TOTofHit );
			}
		}
	}
	if( SaveControlHistos )
	{
		Stats.getHisto1D("Cosmic_Hits_in_event").Fill( NmbOfCosmics );
	}
	return NmbOfCosmics;
}