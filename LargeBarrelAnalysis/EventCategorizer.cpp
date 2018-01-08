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

using namespace std;

EventCategorizer::EventCategorizer(const char* name): JPetUserTask(name) {}

bool EventCategorizer::init()
{

  INFO("Event categorization started.");

 // fBarrelMap = new JPetGeomMapping(getParamBank());
  
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if (fSaveControlHistos) 
  {
		getStatistics().createHistogram(
			new TH1F("TOT",
									"TOT",
									1500, 0, 150)
			);
		getStatistics().getHisto1D("TOT").SetXTitle("TOT [ns]");
		getStatistics().getHisto1D("TOT").SetYTitle("Counts");
    
		getStatistics().createHistogram(
			new TH1F("Quick_T_diff",
								"",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("Quick_T_diff").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("Quick_T_diff").SetYTitle("Counts");	
		
		getStatistics().createHistogram(
			new TH1F("Quick_T_diff_norm",
								"",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("Quick_T_diff_norm").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("Quick_T_diff_norm").SetYTitle("Counts");
    
			getStatistics().createHistogram(
				new TH2F("3_hit_Z_vs_ID",
									"",
									101, -50.5, 50.5,
									196, 0, 196)
			);
			getStatistics().getHisto2D("3_hit_Z_vs_ID").SetXTitle("Z [cm]");
			getStatistics().getHisto2D("3_hit_Z_vs_ID").SetYTitle("Id of scintillator");
		
			getStatistics().createHistogram(
				new TH2F("3_hit_angles_2D",
									"3 Hit angles difference",
									360, -0.5, 359.5,
									360, -0.5, 359.5)
			);
			getStatistics().getHisto2D("3_hit_angles_2D").SetXTitle("Sum of two smallest angles [deg]");
			getStatistics().getHisto2D("3_hit_angles_2D").SetYTitle("Difference of these angles [deg]");
			
			getStatistics().createHistogram(
				new TH1F("3_hit_angles_distance_from_0",
									"3 Hit distance",
									500, -0.5, 49.5)
			);
			getStatistics().getHisto1D("3_hit_angles_distance_from_0").SetXTitle("Distance of hit surface from zero [cm]");
			getStatistics().getHisto1D("3_hit_angles_distance_from_0").SetYTitle("Counts");
			
			getStatistics().createHistogram(
				new TH2F("3_hit_angles_3D",
									"3 Hit angles difference",
									360, -0.5, 359.5,
									360, -0.5, 359.5)
			);
			getStatistics().getHisto2D("3_hit_angles_3D").SetXTitle("Sum of two smallest angles [deg]");
			getStatistics().getHisto2D("3_hit_angles_3D").SetYTitle("Difference of these angles [deg]");
		
		getStatistics().createHistogram(
			new TH1F("3_hit_T_diff_3D_2cut",
								"Tdiff",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("3_hit_T_diff_3D_2cut").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("3_hit_T_diff_3D_2cut").SetYTitle("Counts");
		
				getStatistics().createHistogram(
			new TH1F("3_hit_T_diff_3D_2cut_layer",
								"Tdiff",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("3_hit_T_diff_3D_2cut_layer").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("3_hit_T_diff_3D_2cut_layer").SetYTitle("Counts");
		
		getStatistics().createHistogram(
			new TH1F("3_hit_T_diff_3D_2cut_layer_annipos",
								"Tdiff",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("3_hit_T_diff_3D_2cut_layer_annipos").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("3_hit_T_diff_3D_2cut_layer_annipos").SetYTitle("Counts");
		
		getStatistics().createHistogram(
			new TH2F("4_hit_angles_3D",
								"3 Hit angles difference",
								360, -0.5, 359.5,
								360, -0.5, 359.5)
		);
		getStatistics().getHisto2D("4_hit_angles_3D").SetXTitle("Sum of two smallest angles [deg]");
		getStatistics().getHisto2D("4_hit_angles_3D").SetYTitle("Difference of these angles [deg]");
	
		getStatistics().createHistogram(
			new TH1F("4_hit_T_diff",
								"Tdiff",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("4_hit_T_diff").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("4_hit_T_diff").SetYTitle("Counts");
		
		getStatistics().createHistogram(
			new TH1F("4_hit_angles_distance_from_0",
								"4 Hit distance",
								500, -0.5, 49.5)
		);
		getStatistics().getHisto1D("4_hit_angles_distance_from_0").SetXTitle("Distance of hit surface from zero [cm]");
		getStatistics().getHisto1D("4_hit_angles_distance_from_0").SetYTitle("Counts");
		
		getStatistics().createHistogram(
			new TH1F("4_hit_T_diff_anni",
								"Tdiff",
								40000, -200, 200)
		);
		getStatistics().getHisto1D("4_hit_T_diff").SetXTitle("Time difference [ns]");
		getStatistics().getHisto1D("4_hit_T_diff").SetYTitle("Counts");	
		
			getStatistics().createHistogram(
			new TH2F("4_hit_T_diff_anni_vs_angles",
								"",
								40000, -200, 200,
								360, -0.5, 359.5)
			);
			getStatistics().getHisto2D("4_hit_T_diff_anni_vs_angles").SetXTitle("Time difference [ns]");
			getStatistics().getHisto2D("4_hit_T_diff_anni_vs_angles").SetYTitle("Sum of the two smallest angles [deg]");	
			
			getStatistics().createHistogram(
			new TH2F("4_hit_TOT1_vs_angles",
								"",
								1500, 0, 150,
								360, -0.5, 359.5)
			);
			getStatistics().getHisto2D("4_hit_TOT1_vs_angles").SetXTitle("TOT [ns]");
			getStatistics().getHisto2D("4_hit_TOT1_vs_angles").SetYTitle("Sum of two smallest angles [deg]");
			
			getStatistics().createHistogram(
			new TH2F("4_hit_TOT2_vs_angles",
								"",
								1500, 0, 150,
								360, -0.5, 359.5)
			);
			getStatistics().getHisto2D("4_hit_TOT2_vs_angles").SetXTitle("TOT [ns]");
			getStatistics().getHisto2D("4_hit_TOT2_vs_angles").SetYTitle("Sum of two smallest angles [deg]");
			
			getStatistics().createHistogram(
			new TH2F("4_hit_TOT3_vs_angles",
								"",
								1500, 0, 150,
								360, -0.5, 359.5)
			);
			getStatistics().getHisto2D("4_hit_TOT3_vs_angles").SetXTitle("TOT [ns]");
			getStatistics().getHisto2D("4_hit_TOT3_vs_angles").SetYTitle("Sum of two smallest angles [deg]");
			
			getStatistics().createHistogram(
			new TH1F("Scatter_Test",
								"",
								40000, -200, 200)
			);
			getStatistics().getHisto1D("Scatter_Test").SetXTitle("Scatter test [ns]");
			getStatistics().getHisto1D("Scatter_Test").SetYTitle("Counts");	
		
		getStatistics().createHistogram(
			new TH2F("4_hit_pos",
								"Pos reco",
								218, -54.5, 54.5,
								218, -54.5, 54.5)
		);
		getStatistics().getHisto2D("4_hit_pos").SetXTitle("Y [cm]");
		getStatistics().getHisto2D("4_hit_pos").SetYTitle("X [cm]");
		
		getStatistics().createHistogram(
			new TH1F("4_hit_posZ",
								"Tdiff",
								218, -54.5, 54.5)
		);
		getStatistics().getHisto1D("4_hit_posZ").SetXTitle("Z [cm]");
		getStatistics().getHisto1D("4_hit_posZ").SetYTitle("Counts");	
		
		
		getStatistics().createHistogram(
			new TH2F("3_hit_pos",
								"Pos reco",
								218, -54.5, 54.5,
								218, -54.5, 54.5)
		);
		getStatistics().getHisto2D("3_hit_pos").SetXTitle("Y [cm]");
		getStatistics().getHisto2D("3_hit_pos").SetYTitle("X [cm]");
		
		getStatistics().createHistogram(
			new TH1F("3_hit_posZ",
								"Tdiff",
								218, -54.5, 54.5)
		);
		getStatistics().getHisto1D("3_hit_posZ").SetXTitle("Z [cm]");
		getStatistics().getHisto1D("3_hit_posZ").SetYTitle("Counts");
		
		
							      getStatistics().createHistogram(
								      new TH2F("4_hit_pos_Alek",
													      "Pos reco",
													      218, -54.5, 54.5,
													      218, -54.5, 54.5)
							      );
							      getStatistics().getHisto2D("4_hit_pos_Alek").SetXTitle("Y [cm]");
							      getStatistics().getHisto2D("4_hit_pos_Alek").SetYTitle("X [cm]");
							      
							      getStatistics().createHistogram(
								      new TH1F("4_hit_posZ_Alek",
													      "Tdiff",
													      218, -54.5, 54.5)
							      );
							      getStatistics().getHisto1D("4_hit_posZ_Alek").SetXTitle("Z [cm]");
							      getStatistics().getHisto1D("4_hit_posZ_Alek").SetYTitle("Counts");
							      
							      getStatistics().createHistogram(
								      new TH1F("4_hit_pos_abs_Alek_Diff",
													      "Pos reco",
													      220, -55, 55)
							      );
							      getStatistics().getHisto1D("4_hit_pos_abs_Alek_Diff").SetXTitle("Position Diff [cm]");
							      getStatistics().getHisto1D("4_hit_pos_abs_Alek_Diff").SetYTitle("Counts");
							      
							      getStatistics().createHistogram(
								      new TH2F("4_hit_pos_Alek2",
													      "Pos reco",
													      218, -54.5, 54.5,
													      218, -54.5, 54.5)
							      );
							      getStatistics().getHisto2D("4_hit_pos_Alek2").SetXTitle("Y [cm]");
							      getStatistics().getHisto2D("4_hit_pos_Alek2").SetYTitle("X [cm]");
							      
							      getStatistics().createHistogram(
								      new TH1F("4_hit_posZ_Alek2",
													      "Tdiff",
													      218, -54.5, 54.5)
							      );
							      getStatistics().getHisto1D("4_hit_posZ_Alek2").SetXTitle("Z [cm]");
							      getStatistics().getHisto1D("4_hit_posZ_Alek2").SetYTitle("Counts");
							      
							      getStatistics().createHistogram(
								      new TH1F("4_hit_pos_abs_Alek_Diff2",
													      "Pos reco",
													      220, -55, 55)
							      );
							      getStatistics().getHisto1D("4_hit_pos_abs_Alek_Diff2").SetXTitle("Position Diff [cm]");
							      getStatistics().getHisto1D("4_hit_pos_abs_Alek_Diff2").SetYTitle("Counts");
    
  }
  return true;
}

bool EventCategorizer::exec()
{

  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {

      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      if (event.getHits().size() >= 1) 
      {

        vector<JPetHit> hits = event.getHits();
	
	std::vector < JPetHit > AnniHits;
	std::vector < JPetHit > AnniHitsDec3;
	std::vector < JPetHit > DeexHits;
	
	for (auto i = hits.begin(); i != hits.end(); ++i) 
	{
		auto& hit1 = *i;
	  
		//
		// Calculating TOT of Hit in   ns
		//
		
		double TOT = CalcTOT( hit1 );
		getStatistics().getHisto1D("TOT").Fill( TOT );
		//
		// Categorize hit based on TOT
		//
		
		
		getStatistics().getHisto2D("3_hit_Z_vs_ID").Fill( hit1.getPosZ(), hit1.getScintillator().getID() );
		if( TOT > 30 && TOT < 50 && fabs( hit1.getPosZ() ) < 23 ) // Deexcitation cut
		{
			DeexHits.push_back( hit1 );
		}
		else if( TOT > 10 && TOT < 20 && fabs( hit1.getPosZ() ) < 23 ) // Annihilation cut - stronger one
		{
			AnniHits.push_back( hit1 );
		}	
		if( TOT > 1 && TOT < 29 && fabs( hit1.getPosZ() ) < 23  ) // Annihilation cut - weaker one
		{
			AnniHitsDec3.push_back( hit1 );
		}
	}
	
	if( DeexHits.size() == 1 && AnniHits.size() > 0 )
	{
		for( unsigned i=0; i<AnniHits.size(); i++ )
		{
			  if( AnniHits[i].getTime() > DeexHits[0].getTime() - 100000 )//ps
			  {
				  int test = CheckIfScattered( DeexHits[0], AnniHits[i],  0.5, getStatistics() );
				  if( ! test)
				  {
					  getStatistics().getHisto1D("Quick_T_diff").Fill(  AnniHits[i].getTime()/1000 - DeexHits[0].getTime()/1000 );
					  getStatistics().getHisto1D("Quick_T_diff_norm").Fill(  NormalizeTime( AnniHits[i] ) - NormalizeTime( DeexHits[0] ) );
					  break;
				  }
			  }
		}
	}
	
	
	if( DeexHits.size() == 1 && AnniHits.size() == 2 )
	{
	  
		  //
		  // Calculating angles between scintillators that were hit, and sorting them as above (angles like above but in 2D)
		  //
		  std::vector<double> angles2;
		  angles2.push_back(DeexHits[0].getBarrelSlot().getTheta());
		  angles2.push_back(AnniHits[0].getBarrelSlot().getTheta());
		  angles2.push_back(AnniHits[1].getBarrelSlot().getTheta());
		  std::sort( angles2.begin(), angles2.begin() +3 );
		  float theta_1_2 = angles2[1] - angles2[0];
		  float theta_2_3 = angles2[2] - angles2[1];
		  float theta_3_1 = 360 - theta_1_2 - theta_2_3; 
		  angles2.clear();
		  angles2.push_back(theta_1_2);
		  angles2.push_back(theta_2_3);
		  angles2.push_back(theta_3_1);	 
		  std::sort( angles2.begin(), angles2.begin() +3 );
		  
		  
		  //
		  // Plotting Z-coefficient of hits to check wheter Z is not outside the scintilator ( Length of scintillator - 50 cm )
		  //
		  
		  getStatistics().getHisto2D("3_hit_Z_vs_ID").Fill( DeexHits[0].getPosZ(), DeexHits[0].getScintillator().getID() );	
		 // getStatistics().getHisto2D("3_hit_Z_vs_ID").Fill( AnniHits[0].getPosZ(), AnniHits[0].getScintillator().getID() );
		  //getStatistics().getHisto2D("3_hit_Z_vs_ID").Fill( AnniHits[1].getPosZ(), AnniHits[1].getScintillator().getID() );
		  
		  //
		  // Plotting dependence of 'Difference between two smallest angles' (angles2[1] is always greater than angles2[0]) as a function of 'Sum of two smallest angles' in 2D
		  //
		  
		  getStatistics().getHisto2D("3_hit_angles_2D").Fill( angles2[1] + angles2[0], angles2[1] - angles2[0] );
		  
		  
		  //
		  // Cutting 'Difference between two smallest angles' and plotting the same histogram as above to check how projection on X-axis is changing
		  //
		  
		  //
		  // Plotting Distance of surface created by hits (hit surface - surface coming through all 3 hit points, where hit point is 3D point (x,y,z - coefficients))
		  //
		  
		  getStatistics().getHisto1D("3_hit_angles_distance_from_0").Fill( CalcDistanceOfSurfaceAndZero( DeexHits[0], AnniHits[0], AnniHits[1] ) );
		  
		  
		  //
		  // Checking if sum of two smallest angles in 2D is close to 180 degrees
		  //
		  
		  //
		  // The same procedure as in the beginning, but for events that fulfilled the 2 annihilation, 1 deexcitation criteria
		  //
		  
		  std::vector<double> angles3;
		  double angle1 = CalcAngle( DeexHits[0], AnniHits[0]);
		  double angle2 = CalcAngle( DeexHits[0], AnniHits[1]);
		  double angle3 = CalcAngle( AnniHits[0], AnniHits[1]);
		  angles3.push_back( angle1 );
		  angles3.push_back( angle2 );
		  angles3.push_back( angle3 );
		  std::sort( angles3.begin(), angles3.begin() +3 );
		 
		  //
		  // Plotting dependence of 'Difference between two smallest angles' (angles2[1] is always greater than angles2[0]) as a function of 'Sum of two smallest angles' in 3D
		  //
		  
		  getStatistics().getHisto2D("3_hit_angles_3D").Fill( angles3[1] + angles3[0], angles3[1] - angles3[0] );
		 
		  
		  if( CalcDistanceOfSurfaceAndZero( DeexHits[0], AnniHits[0], AnniHits[1] ) < 3 && fabs( DeexHits[0].getPosZ() ) < 23 && fabs( AnniHits[0].getPosZ() ) < 23 && fabs( AnniHits[1].getPosZ() ) < 23 )
		  {
				//
				// Checking the back-to-back condition for annihilatino hits. So the angle is between them in 3D is close to 180 degrees
				// Also additional cut on time difference between annihialtion hits and difference between two smallest angles, so to get more clear events
				//
				
				if( fabs( CalcAngle( AnniHits[0], AnniHits[1]) - 180 ) < 3 && fabs( (AnniHits[0].getTime() - AnniHits[1].getTime())/1000 ) < 1 && angles2[1] - angles2[0] <= 130 && DeexHits[0].getScintillator().getID() != AnniHits[0].getScintillator().getID() && DeexHits[0].getScintillator().getID() != AnniHits[1].getScintillator().getID() && AnniHits[1].getScintillator().getID() != AnniHits[0].getScintillator().getID() )
				{
					  //std::cout << AnniHits[0].getScintillator().getID() << " " << AnniHits[1].getScintillator().getID() << " " << fBarrelMap->calcDeltaID( AnniHits[0].getBarrelSlot(), AnniHits[1].getBarrelSlot() ) << " " << fBarrelMap->getSlotNumber( AnniHits[0].getBarrelSlot() ) << " " << fBarrelMap->getSlotNumber( AnniHits[1].getBarrelSlot() ) << " " << fBarrelMap->getSlotsCount( AnniHits[0].getBarrelSlot().getLayer() ) << " " << AnniHits[0].getBarrelSlot().getLayer().getID() << " " << AnniHits[1].getBarrelSlot().getLayer().getID() << " " << std::abs( (int)fBarrelMap->getSlotNumber( AnniHits[0].getBarrelSlot() ) - (int)fBarrelMap->getSlotNumber( AnniHits[1].getBarrelSlot() ) ) << std::endl;
					  
					  //
					  // Plotting the estimator of positronium lifetime after these two cuts
					  //
					  
					  getStatistics().getHisto1D("3_hit_T_diff_3D_2cut").Fill(  (AnniHits[0].getTime()/1000 + AnniHits[1].getTime()/1000)/2 - DeexHits[0].getTime()/1000 );
					  
					  //
					  // Calculating the different layer hit factor - as if hits were in different layers, they needed to fly longer distance.
					  //
					   
					  TVector3 RecoPosit = RecoPos2Hit( AnniHits[0], AnniHits[1] );
					  
					  getStatistics().getHisto2D("3_hit_pos").Fill( RecoPosit(1), RecoPosit(0) );
					  getStatistics().getHisto1D("3_hit_posZ").Fill( RecoPosit(2) );
					  TVector3 Zeroo(0,0,0);
					  //
					  // Plotting the estimator of positronium lifetime after these two cuts, and with additional different later hit factor
					  //					
					  
					  getStatistics().getHisto1D("3_hit_T_diff_3D_2cut_layer").Fill(  (AnniHits[0].getTime()/1000 - CalcDistAdd( AnniHits[0] ) - CalcDistAdd( AnniHits[1] ) + AnniHits[1].getTime()/1000)/2 - DeexHits[0].getTime()/1000 - CalcDistAdd( DeexHits[0] ) );
					  getStatistics().getHisto1D("3_hit_T_diff_3D_2cut_layer_annipos").Fill( ( NormalizeTimeToPoint(AnniHits[0], RecoPosit ) + NormalizeTimeToPoint(AnniHits[1], RecoPosit ) )/2 - NormalizeTimeToPoint(DeexHits[0], Zeroo ) );
					  
				}
		  }
	  } // Similiar procedure as above started for o-Ps decay
	  else if( DeexHits.size() == 1 && AnniHitsDec3.size() >= 3 )
	  {
	    
		  std::vector<double> angles4;
		  
		  for( unsigned i=0; i<AnniHitsDec3.size()-2; i++ )
		  {
			    for( unsigned j=i+1; j<AnniHitsDec3.size()-1; j++ )
			    {
				      for( unsigned k=j+1; k<AnniHitsDec3.size(); k++ )
				      {
					
					if( AnniHitsDec3[i].getScintillator().getID() != AnniHitsDec3[j].getScintillator().getID() && AnniHitsDec3[i].getScintillator().getID() != AnniHitsDec3[k].getScintillator().getID() && AnniHitsDec3[k].getScintillator().getID() != AnniHitsDec3[j].getScintillator().getID() && DeexHits[0].getScintillator().getID() != AnniHitsDec3[i].getScintillator().getID() && DeexHits[0].getScintillator().getID() != AnniHitsDec3[j].getScintillator().getID() && DeexHits[0].getScintillator().getID() != AnniHitsDec3[k].getScintillator().getID() )
					{		
					  
						angles4.clear();
						double angle11 = CalcAngle( AnniHitsDec3[i], AnniHitsDec3[j]);
						double angle22 = CalcAngle( AnniHitsDec3[j], AnniHitsDec3[k]);
						double angle33 = CalcAngle( AnniHitsDec3[k], AnniHitsDec3[i]);
						angles4.push_back( angle11 );
						angles4.push_back( angle22 );
						angles4.push_back( angle33 );
						std::sort( angles4.begin(), angles4.begin() +3 );
						getStatistics().getHisto2D("4_hit_angles_3D").Fill( angles4[1] + angles4[0], angles4[1] - angles4[0] );
						getStatistics().getHisto1D("4_hit_angles_distance_from_0").Fill( CalcDistanceOfSurfaceAndZero( AnniHitsDec3[i], AnniHitsDec3[j], AnniHitsDec3[k] ) );
						if( angles4[1] + angles4[0] > 190 && CalcDistanceOfSurfaceAndZero( AnniHitsDec3[i], AnniHitsDec3[j], AnniHitsDec3[k] ) < 5 )
						{
							double TOT1 = CalcTOT( AnniHitsDec3[i] );
							double TOT2 = CalcTOT( AnniHitsDec3[j] );
							double TOT3 = CalcTOT( AnniHitsDec3[k] );
							if( TOT1 < TOT2 && TOT1 < TOT3 )
							{
								if( TOT2 < TOT3 )
								{
									getStatistics().getHisto2D("4_hit_TOT1_vs_angles").Fill( TOT1 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT2_vs_angles").Fill( TOT2 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT3_vs_angles").Fill( TOT3 ,angles4[1] + angles4[0] );
								}
								else
								{
									getStatistics().getHisto2D("4_hit_TOT1_vs_angles").Fill( TOT1 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT2_vs_angles").Fill( TOT3 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT3_vs_angles").Fill( TOT2 ,angles4[1] + angles4[0] );
								}
							}
							else if( TOT2 < TOT1 && TOT2 < TOT3 )
							{
								if( TOT1 < TOT3 )
								{
									getStatistics().getHisto2D("4_hit_TOT1_vs_angles").Fill( TOT2 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT2_vs_angles").Fill( TOT1 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT3_vs_angles").Fill( TOT3 ,angles4[1] + angles4[0] );
								}
								else
								{
									getStatistics().getHisto2D("4_hit_TOT1_vs_angles").Fill( TOT2 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT2_vs_angles").Fill( TOT3 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT3_vs_angles").Fill( TOT1 ,angles4[1] + angles4[0] );
								}
							}
							else
							{
								if( TOT1 < TOT2 )
								{
									getStatistics().getHisto2D("4_hit_TOT1_vs_angles").Fill( TOT3 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT2_vs_angles").Fill( TOT1 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT3_vs_angles").Fill( TOT2 ,angles4[1] + angles4[0] );
								}
								else
								{
									getStatistics().getHisto2D("4_hit_TOT1_vs_angles").Fill( TOT3 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT2_vs_angles").Fill( TOT2 ,angles4[1] + angles4[0] );
									getStatistics().getHisto2D("4_hit_TOT3_vs_angles").Fill( TOT1 ,angles4[1] + angles4[0] );
								}
							}
						  
							getStatistics().getHisto1D("4_hit_T_diff_anni").Fill(  fabs( 2*NormalizeTime( AnniHitsDec3[i] ) - NormalizeTime( AnniHitsDec3[j] ) - NormalizeTime( AnniHitsDec3[k] ) )/2 );
							
							  int test = CheckIfScattered( AnniHitsDec3[i], AnniHitsDec3[j], 0.2, getStatistics() ) + CheckIfScattered( AnniHitsDec3[i], AnniHitsDec3[k], 0.2, getStatistics() ) + CheckIfScattered( AnniHitsDec3[j], AnniHitsDec3[k], 0.2, getStatistics() ) + CheckIfScattered( DeexHits[0], AnniHitsDec3[i], 0.2, getStatistics() ) + CheckIfScattered( DeexHits[0], AnniHitsDec3[j], 0.2, getStatistics() ) + CheckIfScattered( DeexHits[0], AnniHitsDec3[k], 0.2, getStatistics() );
							  std::cout << test << std::endl;
							if( fabs( 2*NormalizeTime( AnniHitsDec3[i] ) - NormalizeTime( AnniHitsDec3[j] ) - NormalizeTime( AnniHitsDec3[k] ) )/2 < 1.5 /* && !test*/ )
							{
							  std::cout << NormalizeTime( AnniHitsDec3[j] ) - NormalizeTime( AnniHitsDec3[i] ) << " " << NormalizeTime( AnniHitsDec3[k] ) - NormalizeTime( AnniHitsDec3[i] ) << std::endl;
							  getStatistics().getHisto1D("4_hit_T_diff").Fill(  ( NormalizeTime( AnniHitsDec3[i] ) + NormalizeTime( AnniHitsDec3[j] ) + NormalizeTime( AnniHitsDec3[k] ) )/3  - NormalizeTime( DeexHits[0] ) );
							  getStatistics().getHisto2D("4_hit_T_diff_anni_vs_angles").Fill(  ( NormalizeTime( AnniHitsDec3[i] ) + NormalizeTime( AnniHitsDec3[j] ) + NormalizeTime( AnniHitsDec3[k] ) )/3  - NormalizeTime( DeexHits[0] ), angles4[1] + angles4[0] );
							  RecoPos( AnniHitsDec3[i], AnniHitsDec3[j], AnniHitsDec3[k] );
							}
						}
					}
				      }  
			    }
		  }		
	  }
	  DeexHits.clear();
	  AnniHits.clear();
	  AnniHitsDec3.clear();
	
	
      }


    }
  } 
  else 
  {
	  return false;
  }
  return true;
}

bool EventCategorizer::terminate()
{
  //delete fBarrelMap;
  INFO("More than one hit Events done. Writing conrtrol histograms.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) {
    fOutputEvents->add<JPetEvent>(event);
  }
}

double CalcTOT( JPetHit Hit )
{
	double TOT = 0;
	  std::map<int, double> lead_timesA, lead_timesB, trail_timesA, trail_timesB;
	  lead_timesA = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	  lead_timesB = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
	  trail_timesA = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
	  trail_timesB = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);	  
	  for(int i=1;i<=Hit.getSignalA().getRecoSignal().getRawSignal().getNumberOfPoints(JPetSigCh::Leading);i++){
                    auto leadSearch = lead_timesA.find(i);
                    auto trailSearch = trail_timesA.find(i);
                    if (leadSearch != lead_timesA.end()
                        && trailSearch != trail_timesA.end())
                            TOT+=(trailSearch->second - leadSearch->second)/1000;
                }
          for(int i=1;i<=Hit.getSignalB().getRecoSignal().getRawSignal().getNumberOfPoints(JPetSigCh::Leading);i++){
                    auto leadSearch = lead_timesB.find(i);
                    auto trailSearch = trail_timesB.find(i);
                    if (leadSearch != lead_timesB.end()
                        && trailSearch != trail_timesB.end())
                            TOT+=(trailSearch->second - leadSearch->second)/1000;
                }
	return TOT;
}

double CalcScattAngle( JPetHit Hit1, JPetHit Hit2 )
{
	double scalarProd = Hit1.getPosX()*(Hit2.getPosX()-Hit1.getPosX()) + Hit1.getPosY()*(Hit2.getPosY()-Hit1.getPosY()) + Hit1.getPosZ()*(Hit2.getPosZ()-Hit1.getPosZ());
	double magProd = sqrt( ( pow(Hit1.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY(),2)
			+pow(Hit1.getPosZ(),2) )*( pow(Hit2.getPosX()-Hit1.getPosX(),2)
			+pow(Hit2.getPosY()-Hit1.getPosY(),2)
			+pow(Hit2.getPosZ()-Hit1.getPosZ(),2) ) );
	double ScattAngle = acos( scalarProd/magProd )*180/3.14159265;
	return ScattAngle;
}


double CalcDistanceBetweenHits( JPetHit Hit1, JPetHit Hit2 )
{
	float dist1 = sqrt(pow(Hit1.getPosX()-Hit2.getPosX(),2)  // Pos in cm
			+pow(Hit1.getPosY()-Hit2.getPosY(),2)
			+pow(Hit1.getPosZ()-Hit2.getPosZ(),2));
	return dist1;
}

double CalcAngle( JPetHit Hit1, JPetHit Hit2 )
{
	double scalarProd = Hit1.getPosX()*Hit2.getPosX() + Hit1.getPosY()*Hit2.getPosY() + Hit1.getPosZ()*Hit2.getPosZ();
	double magProd = sqrt( ( pow(Hit1.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY(),2)
			+pow(Hit1.getPosZ(),2) )*( pow(Hit2.getPosX(),2)
			+pow(Hit2.getPosY(),2)
			+pow(Hit2.getPosZ(),2) ) );
	double Angle = acos( scalarProd/magProd )*180/3.14159265;
	return Angle;
}

double CalcDistanceOfSurfaceAndZero( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	TVector3 vec1( Hit2.getPosX() - Hit1.getPosX(), Hit2.getPosY() - Hit1.getPosY(), Hit2.getPosZ() - Hit1.getPosZ() );
	TVector3 vec2( Hit3.getPosX() - Hit2.getPosX(), Hit3.getPosY() - Hit2.getPosY(), Hit3.getPosZ() - Hit2.getPosZ() );
	TVector3 crossProd  = vec1.Cross(vec2);
	double Dcoeef = -crossProd(0)*Hit2.getPosX() -crossProd(1)*Hit2.getPosY() -crossProd(2)*Hit2.getPosZ();
	double distanceFromZero = fabs(Dcoeef) / crossProd.Mag();
	return distanceFromZero;
}

int CheckIfScattered( JPetHit Hit1, JPetHit Hit2, double ErrorInterval, JPetStatistics& Stats )
{
	double TDiff = fabs(Hit2.getTime()/1000 - Hit1.getTime()/1000);
	TVector3 vec1( Hit2.getPosX() - Hit1.getPosX(), Hit2.getPosY() - Hit1.getPosY(), Hit2.getPosZ() - Hit1.getPosZ() );
	double Distance = vec1.Mag();
	double LightVel = 29.979246; 
	double ScattTime = Distance/LightVel;
	Stats.getHisto1D("Scatter_Test").Fill( TDiff - ScattTime );
	if( fabs( TDiff - ScattTime ) < ErrorInterval )
		return 1;
	else
		return 0;
	
}

double NormalizeTime( JPetHit Hit1 )
{
	TVector3 vec1( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	double Length0 = vec1.Mag();
	return Hit1.getTime()/1000 - (Length0)/29.979246;
}

double NormalizeTimeToPoint( JPetHit Hit1, TVector3 Point )
{
	TVector3 vec1( Hit1.getPosX() - Point(0), Hit1.getPosY() - Point(1), Hit1.getPosZ() - Point(2) );
	double Length0 = vec1.Mag();
	return Hit1.getTime()/1000 - (Length0)/29.979246;
}

TVector3 RecoPos2Hit( const JPetHit & Hit1, const JPetHit & Hit2)
{
	double tof = fabs( Hit1.getTime() - Hit2.getTime() )/1000;
	double VecLength = sqrt( pow(Hit1.getPosX()-Hit2.getPosX(),2)	// Pos in cm
			+pow(Hit1.getPosY()-Hit2.getPosY(),2)
			+pow(Hit1.getPosZ()-Hit2.getPosZ(),2) );
	double middleX = (Hit1.getPosX()+Hit2.getPosX() )/2;
	double middleY = (Hit1.getPosY()+Hit2.getPosY() )/2;
	double middleZ = (Hit1.getPosZ()+Hit2.getPosZ() )/2;
	double Fraction = 2*tof*29.979246/VecLength;
	TVector3 vec1(1000,1000,1000);
	if( Hit1.getTime() >= Hit2.getTime() )
	{
		//TVector3 vec1( middleX + Fraction*( Hit1.getPosX()-middleX ), middleY + Fraction*( Hit1.getPosY()-middleY ), middleZ + Fraction*( Hit1.getPosZ()-middleZ ) );
		vec1(0) = middleX + Fraction*( Hit1.getPosX()-middleX );
		vec1(1) = middleY + Fraction*( Hit1.getPosY()-middleY );
		vec1(2) = middleZ + Fraction*( Hit1.getPosZ()-middleZ );
		return vec1;
	}
	else
	{
		//TVector3 vec1( middleX + Fraction*( Hit2.getPosX()-middleX ), middleY + Fraction*( Hit2.getPosY()-middleY ), middleZ + Fraction*( Hit2.getPosZ()-middleZ ) );
		vec1(0) = middleX + Fraction*( Hit2.getPosX()-middleX );
		vec1(1) = middleY + Fraction*( Hit2.getPosY()-middleY );
		vec1(2) = middleZ + Fraction*( Hit2.getPosZ()-middleZ );
		return vec1;
	}
	return vec1;
}

void EventCategorizer::RecoPos( JPetHit Hit1, JPetHit Hit2, JPetHit Hit3 )
{
	TVector3 vecSurface;/*( (Hit2.getPosY() - Hit1.getPosY() )*(Hit3.getPosZ() - Hit1.getPosZ() ) - (Hit2.getPosZ() - Hit1.getPosZ() )*(Hit3.getPosY() - Hit1.getPosY() ), 
			     (Hit2.getPosZ() - Hit1.getPosZ() )*(Hit3.getPosX() - Hit1.getPosX() ) - (Hit2.getPosX() - Hit1.getPosX() )*(Hit3.getPosZ() - Hit1.getPosZ() ), 
			     (Hit2.getPosX() - Hit1.getPosX() )*(Hit3.getPosY() - Hit1.getPosY() ) - (Hit2.getPosY() - Hit1.getPosY() )*(Hit3.getPosX() - Hit1.getPosX() ) );*/

			     
	vecSurface.SetX( (Hit2.getPosY() - Hit1.getPosY() )*(Hit3.getPosZ() - Hit1.getPosZ() ) - (Hit2.getPosZ() - Hit1.getPosZ() )*(Hit3.getPosY() - Hit1.getPosY() ) );
	vecSurface(1) = (Hit2.getPosZ() - Hit1.getPosZ() )*(Hit3.getPosX() - Hit1.getPosX() ) - (Hit2.getPosX() - Hit1.getPosX() )*(Hit3.getPosZ() - Hit1.getPosZ() );
	vecSurface(2) = (Hit2.getPosX() - Hit1.getPosX() )*(Hit3.getPosY() - Hit1.getPosY() ) - (Hit2.getPosY() - Hit1.getPosY() )*(Hit3.getPosX() - Hit1.getPosX() );
	
	TVector3 vecPerpendicular( -vecSurface(1), vecSurface(0), 0 );
	vecPerpendicular = vecPerpendicular.Unit();
	
	double Theta = -acos( vecSurface(2)/vecSurface.Mag() );
	
	TVector3 RotationX( cos(Theta)+vecPerpendicular(0)*vecPerpendicular(0)*(1-cos(Theta)), vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(Theta)), vecPerpendicular(1)*sin(Theta) );
	TVector3 RotationY( vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(Theta)), cos(Theta)+vecPerpendicular(1)*vecPerpendicular(1)*(1-cos(Theta)), -vecPerpendicular(0)*sin(Theta) );
	TVector3 RotationZ( -vecPerpendicular(1)*sin(Theta), vecPerpendicular(0)*sin(Theta), cos(Theta) );

	TVector3 vecHit1( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 vecHit2( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 vecHit3( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );
	
	TVector3 P1( RotationX*vecHit1, RotationY*vecHit1, RotationZ*vecHit1 );
	TVector3 P2( RotationX*vecHit2, RotationY*vecHit2, RotationZ*vecHit2 );
	TVector3 P3( RotationX*vecHit3, RotationY*vecHit3, RotationZ*vecHit3 );

	//TVector3 Point = FindIntersection( P1, P2, P3, Hit2.getTime()/1000 - Hit1.getTime()/1000, Hit3.getTime()/1000 - Hit1.getTime()/1000 );
	TVector3 Point = FindIntersection2( P1, P2, P3, Hit2.getTime()/1000 - Hit1.getTime()/1000, Hit3.getTime()/1000 - Hit1.getTime()/1000 );
	
	
	if( Point(0) == 100. && Point(1) == 100. )
	  return;
	
	TVector3 RotationXr( cos(-Theta)+vecPerpendicular(0)*vecPerpendicular(0)*(1-cos(-Theta)), vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(-Theta)), vecPerpendicular(1)*sin(-Theta) );
	TVector3 RotationYr( vecPerpendicular(0)*vecPerpendicular(1)*(1-cos(-Theta)), cos(-Theta)+vecPerpendicular(1)*vecPerpendicular(1)*(1-cos(-Theta)), -vecPerpendicular(0)*sin(-Theta) );
	TVector3 RotationZr( -vecPerpendicular(1)*sin(-Theta), vecPerpendicular(0)*sin(-Theta), cos(-Theta) );
	
	TVector3 Pointr( RotationXr*Point, RotationYr*Point, RotationZr*Point );
	std::cout << std::endl;
	std::cout << "Points" << std::endl;
	std::cout << Pointr(0) << " " << Pointr(1) << " " << Pointr(2) << std::endl;
	std::cout << "Points" << std::endl;
	std::cout << std::endl;
	getStatistics().getHisto2D("4_hit_pos").Fill( Pointr(1), Pointr(0) );
	getStatistics().getHisto1D("4_hit_posZ").Fill( Pointr(2) );
	
	TVector3 Hit1Position( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 Hit2Position( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 Hit3Position( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );
	TVector3 AReco = AlekReconstruct( Hit1Position, Hit2Position, Hit3Position, Hit1.getTime()/1000, Hit2.getTime()/1000, Hit3.getTime()/1000 );
	
	
	if(AReco(0) != 1000 )
	{
		getStatistics().getHisto2D("4_hit_pos_Alek").Fill( AReco(1), AReco(0) );
		getStatistics().getHisto1D("4_hit_posZ_Alek").Fill( AReco(2) );
		//TVector3 Diff = AReco - Pointr;
		getStatistics().getHisto1D("4_hit_pos_abs_Alek_Diff").Fill( /*Diff.Mag()*/ AReco.Mag() - Pointr.Mag() );
	}
	
	TVector3 AReco2 = AlekReconstruct2( Hit1Position, Hit2Position, Hit3Position, Hit1.getTime()/1000, Hit2.getTime()/1000, Hit3.getTime()/1000 );
	
	if(AReco2(0) != 1000 )
	{
		getStatistics().getHisto2D("4_hit_pos_Alek2").Fill( AReco2(1), AReco2(0) );
		getStatistics().getHisto1D("4_hit_posZ_Alek2").Fill( AReco2(2) );
		//TVector3 Diff = AReco2 - Pointr;
		getStatistics().getHisto1D("4_hit_pos_abs_Alek_Diff2").Fill(/* Diff.Mag() */ AReco2.Mag() - Pointr.Mag());
	}  
}

TVector3 FindIntersection2( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t21, double t31 )
{
	/*TVector3 Hit1Pos( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	TVector3 Hit2Pos( Hit2.getPosX(), Hit2.getPosY(), Hit2.getPosZ() );
	TVector3 Hit3Pos( Hit3.getPosX(), Hit3.getPosY(), Hit3.getPosZ() );  */
	/*double t21 = Hit2.getTime()/1000 - Hit1.getTime()/1000;
	double t31 = Hit3.getTime()/1000 - Hit1.getTime()/1000;*/
  
	std::cout << t21 << " " << t31 << std::endl;
	
	double R21 = sqrt( pow(Hit2Pos(0) - Hit1Pos(0),2 ) + pow(Hit2Pos(1) - Hit1Pos(1),2 ) );
	double R32 = sqrt( pow(Hit3Pos(0) - Hit2Pos(0),2 ) + pow(Hit3Pos(1) - Hit2Pos(1),2 ) );
	double R13 = sqrt( pow(Hit1Pos(0) - Hit3Pos(0),2 ) + pow(Hit1Pos(1) - Hit3Pos(1),2 ) );
	
	double TDiffTOR1 = 0.;
	double TDiffTOR2 = t21;
	double TDiffTOR3 = t31;
	
	TDiffTOR2 = 29.979246*TDiffTOR2;
	TDiffTOR3 = 29.979246*TDiffTOR3;
	
	double R0 = 0.;
	
	if( R0 < (R21 - TDiffTOR2 )/2  )
	  R0 = (R21 - TDiffTOR2 )/2;
	if( R0 < (R32 - TDiffTOR2 - TDiffTOR3 )/2  )
	  R0 = (R32 - TDiffTOR2 - TDiffTOR3 )/2;
	if( R0 < (R13 - TDiffTOR3 )/2  )
	  R0 = (R13 - TDiffTOR3 )/2;
	
	double R1 = 0.;
	double R2 = 0.;
	double R3 = 0.;
	std::vector<double> temp, temp2;
	std::vector< std::vector<double> > Points;
	temp.push_back(0.);
	temp.push_back(0.);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	double Distance = 0.;
	double MinDistance = 0.;
	double PreviousDistance = 10000000.;
	
	temp.clear();
	
	int test = 1;
	while( test )
	{
		R1 = TDiffTOR1 + R0+1;
		R2 = TDiffTOR2 + R0+1;
		R3 = TDiffTOR2 + R0+1;
		Points = FindIntersectionPointsOfCircles( Hit1Pos, Hit2Pos, Hit3Pos, R1, R2, R3, R13, R21, R32 );
		
		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
		test++;
		if( test % 50 == 0 )
		{
			if( MinDistance == 1000000. )
			{
				temp.clear();
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				break;
			}
		}
		if( MinDistance > PreviousDistance )
			test = 0;
		else
		{
			PreviousDistance = MinDistance;
			temp2 = temp;
		}
		R0 += 1;
	}
	std::vector<double> R0s, Distances;
	if( MinDistance != 1000000. )
		test = 1;
	
	double MinnDistance = 1000000.;
	while( test )
	{
		R1 = TDiffTOR1 + R0+1;
		R2 = TDiffTOR2 + R0+1;
		R3 = TDiffTOR2 + R0+1;
		Points = FindIntersectionPointsOfCircles( Hit1Pos, Hit2Pos, Hit3Pos, R1, R2, R3, R13, R21, R32 );

		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
		if( MinDistance != 1000000. )
		{
			std::cout << R0 << " " << MinDistance << std::endl;
			R0s.push_back( R0 );
			Distances.push_back( MinDistance );
		}
		
		test++;
		if( test % 50 == 0 )
		{
			if( MinDistance == 1000000. )
			{
				temp.clear();
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				temp.push_back(100.);
				break;
			}
			test = 0;
		}
		if( MinDistance < MinnDistance )
		  MinnDistance = MinDistance;
		
		PreviousDistance = MinDistance;
		temp2 = temp;
		R0 -= 0.1;
	}
	
	if( MinnDistance != 1000000. )
	{
		double R0Min; 
		double minEle = *std::min_element( std::begin(Distances), std::end(Distances) );
		if( minEle == Distances[0] )
		{
			R0Min = R0s[0];
		}
		else if( minEle == Distances[ Distances.size() - 1 ] )
		{
			R0Min = R0s[ R0s.size() -1 ];	  
		}
		else
		{
			//R0Min = FindMinFromQuadraticFit( R0s, Distances );
			R0Min = FindMinFromDerrivative( R0s, Distances );
		}
		std::cout << R0Min << std::endl;
		R1 = TDiffTOR1 + R0Min+1;
		R2 = TDiffTOR2 + R0Min+1;
		R3 = TDiffTOR2 + R0Min+1;
		Points = FindIntersectionPointsOfCircles( Hit1Pos, Hit2Pos, Hit3Pos, R1, R2, R3, R13, R21, R32 );
		
		MinDistance = 1000000.;
		for( unsigned i=0; i<2; i++ )
		{
			for( unsigned j=0; j<2; j++ )
			{
				for( unsigned k=0; k<2; k++ )
				{
					Distance = sqrt( pow(Points[i][0] - Points[j+2][0], 2) + pow(Points[i][1] - Points[j+2][1],2 ) ) + sqrt( pow(Points[i][0] - Points[k+4][0], 2) + pow(Points[i][1] - Points[k+4][1],2 ) ) + sqrt( pow(Points[k+4][0] - Points[j+2][0], 2) + pow(Points[k+4][1] - Points[j+2][1],2 ) );
					if( Distance < MinDistance )
					{
						MinDistance = Distance;
						temp.clear();
						temp.push_back( Points[i][0] );
						temp.push_back( Points[i][1] );
						temp.push_back( Points[2+j][0] );
						temp.push_back( Points[2+j][1] );
						temp.push_back( Points[4+k][0] );
						temp.push_back( Points[4+k][1] );
					}
				}	
			}
		}
	
	}
	
	TVector3 RecoPoint( (temp[0]+temp[2]+temp[4])/3, (temp[1]+temp[3]+temp[5])/3, Hit1Pos(2) );
	
	return RecoPoint;
}


double FindMinFromDerrivative( std::vector<double> Arg, std::vector<double> Val )
{
	double Derr1 = Val[1] - Val[0];
	unsigned StopInd = 0;
	for( unsigned i=1; i<Val.size()-1; i++ )
	{
		if( Derr1 < 0 )
		{
			Derr1 = Val[i+1] - Val[i];
			if(Derr1 > 0)
			{
				StopInd = i;
				break;
			}
		}
	}
	double a = ( Val[StopInd+1] - Val[StopInd] - (Val[StopInd] - Val[StopInd-1]) )/( (Arg[StopInd+1] + Arg[StopInd])/2 - (Arg[StopInd] + Arg[StopInd-1])/2 );
	double b = Val[StopInd+1] - Val[StopInd] - a*(Arg[StopInd+1] + Arg[StopInd])/2;
	if( a )
		return -b/a;
	else
	{
		std::cout << "Error, wtf, error, no idea?!?" << std::endl;
		return 0;
	}
	
}

std::vector< std::vector<double> > FindIntersectionPointsOfCircles( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2, double R3, double R13, double R21, double R32 )
{
	std::vector< std::vector<double> > Points;
	std::vector<double> temp;
	temp.push_back(0);
	temp.push_back(0);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	Points.push_back(temp);
	
	Points[0][0] = (Hit1Pos(0) + Hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(0) - Hit1Pos(0) )/2/pow( R21,2 ) + 0.5*( Hit2Pos(1) - Hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[0][1] = (Hit1Pos(1) + Hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(1) - Hit1Pos(1) )/2/pow( R21,2 ) + 0.5*( Hit1Pos(0) - Hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[1][0] = (Hit1Pos(0) + Hit2Pos(0) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(0) - Hit1Pos(0) )/2/pow( R21,2 ) - 0.5*( Hit2Pos(1) - Hit1Pos(1) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	Points[1][1] = (Hit1Pos(1) + Hit2Pos(1) )/2 + (pow(R1,2) - pow(R2,2) )*( Hit2Pos(1) - Hit1Pos(1) )/2/pow( R21,2 ) - 0.5*( Hit1Pos(0) - Hit2Pos(0) )*sqrt( 2 * (pow(R1,2) + pow(R2,2) )/pow( R21,2 ) - pow( pow(R1,2) - pow(R2,2), 2 )/pow( R21, 4 ) - 1 );
	
	Points[2][0] = (Hit2Pos(0) + Hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(0) - Hit2Pos(0) )/2/pow( R32,2 ) + 0.5*( Hit3Pos(1) - Hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[2][1] = (Hit2Pos(1) + Hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(1) - Hit2Pos(1) )/2/pow( R32,2 ) + 0.5*( Hit2Pos(0) - Hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[3][0] = (Hit2Pos(0) + Hit3Pos(0) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(0) - Hit2Pos(0) )/2/pow( R32,2 ) - 0.5*( Hit3Pos(1) - Hit2Pos(1) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	Points[3][1] = (Hit2Pos(1) + Hit3Pos(1) )/2 + (pow(R2,2) - pow(R3,2) )*( Hit3Pos(1) - Hit2Pos(1) )/2/pow( R32,2 ) - 0.5*( Hit2Pos(0) - Hit3Pos(0) )*sqrt( 2 * (pow(R2,2) + pow(R3,2) )/pow( R32,2 ) - pow( pow(R2,2) - pow(R3,2), 2 )/pow( R32, 4 ) - 1 );
	
	Points[4][0] = (Hit1Pos(0) + Hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(0) - Hit3Pos(0) )/2/pow( R13,2 ) + 0.5*( Hit1Pos(1) - Hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[4][1] = (Hit1Pos(1) + Hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(1) - Hit3Pos(1) )/2/pow( R13,2 ) + 0.5*( Hit3Pos(0) - Hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[5][0] = (Hit1Pos(0) + Hit3Pos(0) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(0) - Hit3Pos(0) )/2/pow( R13,2 ) - 0.5*( Hit1Pos(1) - Hit3Pos(1) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	Points[5][1] = (Hit1Pos(1) + Hit3Pos(1) )/2 + (pow(R3,2) - pow(R1,2) )*( Hit1Pos(1) - Hit3Pos(1) )/2/pow( R13,2 ) - 0.5*( Hit3Pos(0) - Hit1Pos(0) )*sqrt( 2 * (pow(R3,2) + pow(R1,2) )/pow( R13,2 ) - pow( pow(R3,2) - pow(R1,2), 2 )/pow( R13, 4 ) - 1 );
	
	return Points;
}

TVector3 AlekReconstruct( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t1, double t2, double t3 )
{
  
  // find the decay plane
  TVector3 normal = ((Hit2Pos-Hit1Pos).Cross( Hit3Pos-Hit1Pos )).Unit();
  
  Double_t cvel = 29.979246;
  
  double times[3];
  times[0] = t1;
  times[1] = t2;
  times[2] = t3;
  // prepare transformation to the decay plane
  TVector3 z(0.,0.,1.);
  TVector3 rotAxis = normal.Cross( z );
  double angle = z.Angle( normal ); // radians
  
  TRotation rot;
  rot.Rotate(angle, rotAxis);
  
  // transform gamma hits to decay plane
  std::cout << "\n2D:" << std::endl;
  TVector3 gammas2D[3];
 /* for(int i=0;i<3;i++){
    gammas2D[i] = rot * getGammaHit(i);
  }*/
    gammas2D[0] = rot * Hit1Pos;
    gammas2D[1] = rot * Hit2Pos;
    gammas2D[2] = rot * Hit3Pos;
  
  // solve in 2D
  int combs[][2] = {{0,1}, {1,2}, {0,2}};
  double M[3][3];
  double D[3];
  
  // fill the matrix and constants vector
  int i,j;
  for(int k=0;k<3;++k){ // k - rows
    i = combs[k][0];
    j = combs[k][1];
    M[k][0] = 2.*( gammas2D[i].X() - gammas2D[j].X() );
    M[k][1] = 2.*( gammas2D[i].Y() - gammas2D[j].Y() );
    M[k][2] = 2.*cvel*cvel*( times[j] - times[i] );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - cvel*cvel*pow(times[i],2.)
      + cvel*cvel*pow(times[j],2.);
  }

  /*
  for(int k=0;k<3;++k){ // k - rows
    std::cout << "m1 = " << M[k][0] << std::endl;
    std::cout << "m2 = " << M[k][1] << std::endl;
    std::cout << "m3 = " << M[k][2] << std::endl;
    std::cout << "D = " << D[k] << std::endl;
  }
  */
  
  // use analytical solutions: x = Ex*t+Fx, y=Ey*t+Fy
  double Ex,Ey,Fx,Fy;
  Ex = ( M[0][2]*M[1][1]-M[0][1]*M[1][2] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fx = ( M[0][1]*D[1]-M[1][1]*D[0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  
  Ey = ( M[0][0]*M[1][2] - M[0][2]*M[1][0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fy = ( M[1][0]*D[0] - M[0][0]*D[1] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );       
  
  // find t - using ready analytical solutions
  double a,b,cc,delta;
  
  
  TVector3 testy(1000,1000,1000);
  TVector3 sol_hit[2];
  Double_t sol_time[2];
  
  a = Ex*Ex + Ey*Ey - cvel*cvel;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + cvel*cvel*times[0] );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - cvel*cvel*pow(times[0], 2.);
  delta = b*b - 4.*a*cc;
  
    if( delta < 0. )
    {
      return testy;
    }
    
    sol_time[0] = (-1.*b - sqrt(delta))/(2.*a);
    sol_time[1] = (-1.*b + sqrt(delta))/(2.*a);
    
    for(int i = 0; i<2;++i){
      TVector3 sol2Dv( Ex*sol_time[i]+Fx, Ey*sol_time[i]+Fy, gammas2D[0].Z() );
      
      // transform the solution back to 3D
      sol_hit[i] =  rot.Inverse() * sol2Dv;
      
    }
    
    // check solution 2 for reasonability
   /* if( errFlag == 0 ){
      if( sol_time[1] < -20000. || sol_time[1] > 20000.  ){
	errFlag = 2;
      }else if( sol_hit[1].Perp() > Hit2Pos.Perp() ||
		fabs( sol_hit[1].Z() ) > fStripLength/2. ){
	errFlag = 3;
      }else if( TMath::IsNaN( sol_time[1] ) ||
		TMath::IsNaN( sol_hit[1].X() ) ||
		TMath::IsNaN( sol_hit[1].Y() ) ||
		TMath::IsNaN( sol_hit[1].Z() )
		){
	errFlag = 4;
      }
    }*/
    
  
  return sol_hit[1];
}


TVector3 AlekReconstruct2( TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t1, double t2, double t3 )
{
  
  // find the decay plane
  TVector3 normal = ((Hit2Pos-Hit1Pos).Cross( Hit3Pos-Hit1Pos )).Unit();
  
  Double_t cvel = 29.979246;
  
  double times[3];
  times[0] = t1;
  times[1] = t2;
  times[2] = t3;
  // prepare transformation to the decay plane
  TVector3 z(0.,0.,1.);
  TVector3 rotAxis = normal.Cross( z );
  double angle = z.Angle( normal ); // radians
  
  TRotation rot;
  rot.Rotate(angle, rotAxis);
  
  // transform gamma hits to decay plane
  std::cout << "\n2D:" << std::endl;
  TVector3 gammas2D[3];
 /* for(int i=0;i<3;i++){
    gammas2D[i] = rot * getGammaHit(i);
  }*/
    gammas2D[0] = rot * Hit1Pos;
    gammas2D[1] = rot * Hit2Pos;
    gammas2D[2] = rot * Hit3Pos;
  
  // solve in 2D
  int combs[][2] = {{0,1}, {1,2}, {0,2}};
  double M[3][3];
  double D[3];
  
  // fill the matrix and constants vector
  int i,j;
  for(int k=0;k<3;++k){ // k - rows
    i = combs[k][0];
    j = combs[k][1];
    M[k][0] = 2.*( gammas2D[i].X() - gammas2D[j].X() );
    M[k][1] = 2.*( gammas2D[i].Y() - gammas2D[j].Y() );
    M[k][2] = 2.*cvel*cvel*( times[j] - times[i] );       
    D[k] = pow(gammas2D[i].X(),2.)
      - pow(gammas2D[j].X(),2.)
      + pow(gammas2D[i].Y(),2.)
      - pow(gammas2D[j].Y(),2.)
      - cvel*cvel*pow(times[i],2.)
      + cvel*cvel*pow(times[j],2.);
  }

  /*
  for(int k=0;k<3;++k){ // k - rows
    std::cout << "m1 = " << M[k][0] << std::endl;
    std::cout << "m2 = " << M[k][1] << std::endl;
    std::cout << "m3 = " << M[k][2] << std::endl;
    std::cout << "D = " << D[k] << std::endl;
  }
  */
  
  // use analytical solutions: x = Ex*t+Fx, y=Ey*t+Fy
  double Ex,Ey,Fx,Fy;
  Ex = ( M[0][2]*M[1][1]-M[0][1]*M[1][2] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fx = ( M[0][1]*D[1]-M[1][1]*D[0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  
  Ey = ( M[0][0]*M[1][2] - M[0][2]*M[1][0] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );
  Fy = ( M[1][0]*D[0] - M[0][0]*D[1] )/( M[0][1]*M[1][0]-M[0][0]*M[1][1] );       
  
  // find t - using ready analytical solutions
  double a,b,cc,delta;
  
  
  TVector3 testy(1000,1000,1000);
  TVector3 sol_hit[2];
  Double_t sol_time[2];
  
  a = Ex*Ex + Ey*Ey - cvel*cvel;
  b = 2.*( Ex*(Fx-gammas2D[0].X()) + Ey*(Fy-gammas2D[0].Y()) + cvel*cvel*times[0] );
  cc = pow(Fx-gammas2D[0].X(), 2.) + pow(Fy-gammas2D[0].Y(), 2.) - cvel*cvel*pow(times[0], 2.);
  delta = b*b - 4.*a*cc;
  
    if( delta < 0. )
    {
      return testy;
    }
    
    sol_time[0] = (-1.*b - sqrt(delta))/(2.*a);
    sol_time[1] = (-1.*b + sqrt(delta))/(2.*a);
    
    TVector3 Sol1( Ex*sol_time[0]+Fx, Ey*sol_time[0]+Fy, 0 );
    TVector3 Sol2( Ex*sol_time[1]+Fx, Ey*sol_time[1]+Fy, 0 );
    int BttSol = ( CalcDistFromCentres( Sol1, gammas2D[0], gammas2D[1], gammas2D[2] ) < CalcDistFromCentres( Sol2, gammas2D[0], gammas2D[1], gammas2D[2] ) ? 0 : 1 );
    
    for(int i = 0; i<2;++i){
      TVector3 sol2Dv( Ex*sol_time[i]+Fx, Ey*sol_time[i]+Fy, gammas2D[0].Z() );
      
      // transform the solution back to 3D
      sol_hit[i] =  rot.Inverse() * sol2Dv;
      
    }
    
    // check solution 2 for reasonability
   /* if( errFlag == 0 ){
      if( sol_time[1] < -20000. || sol_time[1] > 20000.  ){
	errFlag = 2;
      }else if( sol_hit[1].Perp() > Hit2Pos.Perp() ||
		fabs( sol_hit[1].Z() ) > fStripLength/2. ){
	errFlag = 3;
      }else if( TMath::IsNaN( sol_time[1] ) ||
		TMath::IsNaN( sol_hit[1].X() ) ||
		TMath::IsNaN( sol_hit[1].Y() ) ||
		TMath::IsNaN( sol_hit[1].Z() )
		){
	errFlag = 4;
      }
    }*/
    
  
  return sol_hit[BttSol];
}

double CalcDistFromCentres( TVector3 Sol1, TVector3 Gamma1, TVector3 Gamma2, TVector3 Gamma3 )
{
	double distance1 = 0.;
	distance1 = sqrt( pow(Sol1(0) - Gamma1(0),2) + pow(Sol1(1) - Gamma1(1),2) + pow(Sol1(2) - Gamma1(2),2) ) + sqrt( pow(Sol1(0) - Gamma2(0),2) + pow(Sol1(1) - Gamma2(1),2) + pow(Sol1(2) - Gamma2(2),2) ) + sqrt( pow(Sol1(0) - Gamma3(0),2) + pow(Sol1(1) - Gamma3(1),2) + pow(Sol1(2) - Gamma3(2),2) );		
	return distance1;
}

double CalcDistAdd( JPetHit Hit1 )
{
	TVector3 vec1( Hit1.getPosX(), Hit1.getPosY(), Hit1.getPosZ() );
	double LengthR = 40;
	double Length0 = vec1.Mag();
	return (Length0 - LengthR)/29.979246;
}
