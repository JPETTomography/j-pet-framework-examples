/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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

#include "EventCategorizer.h"
#include "CalibrationTools.h"
#include "EventCategorizerTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <cmath>

using namespace jpet_options_tools;
using namespace std;

EventCategorizer::EventCategorizer(const char* name) : JPetUserTask(name) {}

EventCategorizer::~EventCategorizer() {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");

  // Reading user parameters
  if (isOptionSet(fParams.getOptions(), k2gThetaDiffParamKey))
  {
    f2gThetaDiff = getOptionAsDouble(fParams.getOptions(), k2gThetaDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2gThetaDiffParamKey.c_str(), f2gThetaDiff));
  }

  if (isOptionSet(fParams.getOptions(), k2gTimeDiffParamKey))
  {
    f2gTimeDiff = getOptionAsDouble(fParams.getOptions(), k2gTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2gTimeDiffParamKey.c_str(), f2gTimeDiff));
  }

  // Reading ToT cut values
  if (isOptionSet(fParams.getOptions(), kToTCutAnniMinParamKey))
  {
    fToTCutAnniMin = getOptionAsDouble(fParams.getOptions(), kToTCutAnniMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutAnniMinParamKey.c_str(), fToTCutAnniMin));
  }

  if (isOptionSet(fParams.getOptions(), kToTCutAnniMaxParamKey))
  {
    fToTCutAnniMax = getOptionAsDouble(fParams.getOptions(), kToTCutAnniMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutAnniMaxParamKey.c_str(), fToTCutAnniMax));
  }

  if (isOptionSet(fParams.getOptions(), kToTCutDeexMinParamKey))
  {
    fToTCutDeexMin = getOptionAsDouble(fParams.getOptions(), kToTCutDeexMinParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutDeexMinParamKey.c_str(), fToTCutDeexMin));
  }

  if (isOptionSet(fParams.getOptions(), kToTCutDeexMaxParamKey))
  {
    fToTCutDeexMax = getOptionAsDouble(fParams.getOptions(), kToTCutDeexMaxParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kToTCutDeexMaxParamKey.c_str(), fToTCutDeexMax));
  }

  // For plotting ToT histograms
  if (isOptionSet(fParams.getOptions(), kToTHistoUpperLimitParamKey))
  {
    fToTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kToTHistoUpperLimitParamKey);
  }

  // Cuts around source position
  if (isOptionSet(fParams.getOptions(), kSourceDistCutXYParamKey))
  {
    fSourceDistXYCut = getOptionAsDouble(fParams.getOptions(), kSourceDistCutXYParamKey);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kSourceDistCutXYParamKey.c_str(), fSourceDistXYCut));
  }

  if (isOptionSet(fParams.getOptions(), kSourceDistCutZParamKey))
  {
    fSourceDistZCut = getOptionAsDouble(fParams.getOptions(), kSourceDistCutZParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kSourceDistCutZParamKey.c_str(), fSourceDistZCut));
  }

  // LOR cuts
  if (isOptionSet(fParams.getOptions(), kLORAngleCutParamKey))
  {
    fLORAngleCut = getOptionAsDouble(fParams.getOptions(), kLORAngleCutParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLORAngleCutParamKey.c_str(), fLORAngleCut));
  }

  if (isOptionSet(fParams.getOptions(), kLORPosZCutParamKey))
  {
    fLORPosZCut = getOptionAsDouble(fParams.getOptions(), kLORPosZCutParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kLORPosZCutParamKey.c_str(), fLORPosZCut));
  }

  // Source position
  if (isOptionSet(fParams.getOptions(), kSourcePosXParamKey) && isOptionSet(fParams.getOptions(), kSourcePosYParamKey) &&
      isOptionSet(fParams.getOptions(), kSourcePosZParamKey))
  {
    auto x = getOptionAsDouble(fParams.getOptions(), kSourcePosXParamKey);
    auto y = getOptionAsDouble(fParams.getOptions(), kSourcePosYParamKey);
    auto z = getOptionAsDouble(fParams.getOptions(), kSourcePosZParamKey);
    fSourcePos.SetXYZ(x, y, z);
    INFO(Form("Source position is: %lf, %lf, %lf", x, y, z));
  }
  else
  {
    fSourcePos.SetXYZ(0.0, 0.0, 0.0);
    INFO("Source is positioned in (0, 0, 0).");
  }

  // Reading file with constants to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey))
  {
    fScatterTOFTimeDiff = getOptionAsDouble(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kScatterTOFTimeDiffParamKey.c_str(),
                 fScatterTOFTimeDiff));
  }

  // Time variable used as +- axis limits for histograms with time spectra
  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey))
  {
    fMaxTimeDiff = getOptionAsDouble(fParams.getOptions(), kMaxTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }

  // Variable used in measurements with Trento setup - rotation of Z axis with respect of
  // vertical direction in Earth frame
  if (isOptionSet(fParams.getOptions(), kDetectorYRotation))
  {
    fDetectorYRotationDeg = getOptionAsDouble(fParams.getOptions(), kDetectorYRotation);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kDetectorYRotation.c_str(), fDetectorYRotationDeg));
  }

  if (isOptionSet(fParams.getOptions(), kCosmicMaxThetaDeg))
  {
    fCosmicMaxThetaDiffDeg = getOptionAsDouble(fParams.getOptions(), kCosmicMaxThetaDeg);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kCosmicMaxThetaDeg.c_str(), fCosmicMaxThetaDiffDeg));
  }

  // Getting bools for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Initialise hisotgrams
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }

  return true;
}

bool EventCategorizer::checkTOT(const JPetPhysRecoHit& hit){


  int id = hit.getScin().getID() - 200;
  id = id % 13;
  if(id == 1 || id == 0 || id == 2 || id == 12
     ){
    return false;
  }
  
  getStatistics().fillHistogram("tot_initial", hit.getEnergy()/1000.);
  
  if(hit.getEnergy() > fToTCutAnniMin && hit.getEnergy() < fToTCutAnniMax){
    return true;
  }
  return false;
  
  return true;
}

bool EventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
    {
      vector<JPetEvent> events;
      for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
        {
          const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));          
          int nhits = event.getHits().size();
          fIndicesToRemove.clear();

            /**************************************************************/
            /* Scatter test                                               */
            /**************************************************************/
            for(int j=0; j<nhits-1; j++){

              const auto& hit1 = dynamic_cast<const JPetPhysRecoHit&>(*event.getHits().at(j));
              if(!checkTOT(hit1)){
                fIndicesToRemove.insert(j);
                continue;
              }

              for(int k=j+1; k<nhits; k++){

                const auto& hit2 = dynamic_cast<const JPetPhysRecoHit&>(*event.getHits().at(k));
                if(!checkTOT(hit2)){
                  fIndicesToRemove.insert(k);
                  continue;
                }
                
                
                double d = (hit1.getPos() - hit2.getPos()).Mag();
                double dt = (hit1.getTime() - hit2.getTime()) / 1000.;
                
                double st = d -  fabs(dt)*29.9792;
                getStatistics().fillHistogram("scatter_test", st);
                getStatistics().fillHistogram("scatter_test_time", d/29.9792 - fabs(dt));
                getStatistics().fillHistogram("scatter_test_2d", fabs(dt), d);
                
                if(st < 40.0 ){
                  if(fabs(dt) < 0.1){
                    fIndicesToRemove.insert(j);
                    fIndicesToRemove.insert(k);
                  }
                  if(dt < 0){
                    fIndicesToRemove.insert(k);
                  }
                  if(dt > 0){
                    fIndicesToRemove.insert(j);
                  }                
                }
                                
              } 
            }
          
          if(nhits - fIndicesToRemove.size() != 2){
            continue;
          }
          
          JPetEvent newEvent;
          for(int j=0; j<nhits; ++j){
            if(fIndicesToRemove.count(j) == 0){
              newEvent.addHit(event.getHits().at(j));
            }else{
              getStatistics().fillHistogram("tot_scatters", event.getHits().at(j)->getEnergy()/1000.);
            }
          }
          
          if(newEvent.getHits().size() != 2){
            continue;
          }

          /****************************************************************/
          /* Angle and TOF                                                */
          /****************************************************************/
          const auto& hit1 = dynamic_cast<const JPetPhysRecoHit&>(*event.getHits().at(0));
          const auto& hit2 = dynamic_cast<const JPetPhysRecoHit&>(*event.getHits().at(1));          

          TVector3 hit1_xy_pos = hit1.getPos();
          hit1_xy_pos.SetZ(0.);
          TVector3 hit2_xy_pos = hit2.getPos();
          hit2_xy_pos.SetZ(0.);
          
          double theta = TMath::RadToDeg() * hit1_xy_pos.Angle(hit2_xy_pos);

          // TOF calculated by convention
          double tof = 0.;
          if (hit1.getScin().getSlot().getTheta() < hit2.getScin().getSlot().getTheta())
            {
              tof = hit1.getTime() - hit2.getTime();
            }
          else
            {
              tof = hit2.getTime() - hit1.getTime();
            }
          tof /= 1000.; // [ps -> ns]
          
          getStatistics().fillHistogram("2g_angle_tof", fabs(tof), theta);

          // Cut on 2D angle and TOF
          double x = tof;
          double y = theta - 180.0;
          double a = 1.5;
          double b = 40.0;
          if( pow(x/a, 2.) + pow(y/b, 2.) > 1 ){
            continue;
          }
          getStatistics().fillHistogram("2g_angle_tof_after_cut", fabs(tof), theta);          
          
          // fill calibration control histograms
          int scin1ID = hit1.getScin().getID();
          int scin2ID = hit2.getScin().getID();
          getStatistics().fillHistogram("2g_tof_scin", scin1ID, tof);
          getStatistics().fillHistogram("2g_tof_scin", scin2ID, tof);        
          getStatistics().fillHistogram("2g_ab_tdiff_scin", scin1ID, hit1.getTimeDiff()/1000.);
          getStatistics().fillHistogram("2g_ab_tdiff_scin", scin2ID, hit2.getTimeDiff()/1000.);
                    
          /****************************************************************/
          /* Fill annihilation points                                     */
          /****************************************************************/
          TVector3 ap = EventCategorizerTools::calculateAnnihilationPoint(&hit1, &hit2);             
          getStatistics().fillHistogram("ap_xy", ap.X(), ap.Y());
          getStatistics().fillHistogram("ap_xz", ap.X(), ap.Z());
          getStatistics().fillHistogram("ap_zy", ap.Z(), ap.Y());
          getStatistics().fillHistogram("ap_3d", ap.X(), ap.Y(), ap.Z());
          
          if(fabs(ap.Z()) < 2.0){
            getStatistics().fillHistogram("ap_xy_z0", ap.X(), ap.Y());
          }
          
          /**********************************************************/
          /* Store event                                            */
          /**********************************************************/
          fOutputEvents->add<JPetEvent>(newEvent);
          
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
  INFO("Event categorization completed.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

void EventCategorizer::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // Imaging preselection
  getStatistics().createHistogramWithAxes(new TH1D("scatter_test", "scatter test for 2-hit pairs", 200, -100.0, 100.0),
                                          "d-c*t [cm]");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_time", "scatter test for 2-hit pairs", 200, -10.0, 10.0),
                                          "d/c-t [ns]");

  getStatistics().createHistogramWithAxes(
                                          new TH2D("scatter_test_2d", "2D scatter test", 200, 0.0, 4.0,
                                                   240, 0., 120.0
                                                   ),
                                          "#Delta t [ns]",
                                          "d [cm]"
                                          );
  
  getStatistics().createHistogramWithAxes(new TH1D("tot_initial", "TOT - initial all hits", 250, 0.0, 250.0),
                                          "TOT [ns]");
  
  getStatistics().createHistogramWithAxes(new TH1D("tot_scatters", "TOT - hits identified as scatters", 250, 0.0, 250.0),
                                          "TOT [ns]");
  
  
  // Events after cut - defined as annihilation event
  getStatistics().createHistogramWithAxes(
                                          new TH2D("2g_angle_tof", "2g events, 2D angle vs TOF", 250, 0.0, 5.0,
                                                   360, -0.25, 180-0.25
                                                   ),
                                          "TOF [ns]",
                                          "2D theta difference [deg]"
                                          );

  getStatistics().createHistogramWithAxes(
                                          new TH2D("2g_angle_tof_after_cut", "2g events, 2D angle vs TOF; after cut on that",
                                                   250, 0.0, 5.0,
                                                   360, -0.25, 180-0.25
                                                   ),
                                          "TOF [ns]",
                                          "2D theta difference [deg]"
                                          );

  // Annihilation point distributions
  getStatistics().createHistogramWithAxes(
                                          new TH2D("ap_xy", "Annihilation points, XY plane", 
                                                   160, -40.-0.25, 40.-0.25, 160, -40.-0.25, 40.-0.25
                                                   ),
                                          "X [cm]",
                                          "Y [cm]"
                                          );
  
  getStatistics().createHistogramWithAxes(
                                          new TH2D("ap_xy_z0", "Annihilation points, XY plane, |Z|<2cm", 
                                                   160, -40.-0.25, 40.-0.25, 160, -40.-0.25, 40.-0.25
                                                   ),
                                          "X [cm]",
                                          "Y [cm]"
                                          );

  getStatistics().createHistogramWithAxes(
                                          new TH2D("ap_xz", "Annihilation points, XZ plane (top view)", 
                                                   160, -40.-0.25, 40.-0.25, 160, -40.-0.25, 40.-0.25
                                                   ),
                                          "X [cm]",
                                          "Z [cm]"
                                          );

  getStatistics().createHistogramWithAxes(
                                          new TH2D("ap_zy", "Annihilation points, ZY plane (side view)", 
                                                   160, -40.-0.25, 40.-0.25, 160, -40.-0.25, 40.-0.25
                                                   ),
                                          "Z [cm]",
                                          "Y [cm]"
                                          );

  getStatistics().createHistogramWithAxes(
                                          new TH3D("ap_3d", "Annihilation points in 3D", 
                                                   160, -40.-0.25, 40.-0.25, 160, -40.-0.25, 40.-0.25,
                                                   160, -40.-0.25, 40.-0.25
                                                   ),
                                          "X [cm]",
                                          "Y [cm]",
                                          "Z [cm]"
                                          );
  
  // calibration control histograms
  getStatistics().createHistogramWithAxes(new TH2D("2g_tof_scin", "2 gamma event - TOF calculated by convention per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff/1000., fMaxTimeDiff/1000.),
                                          "Scintillator ID", "Time of Flight [ns]");
    
  getStatistics().createHistogramWithAxes(new TH2D("2g_ab_tdiff_scin", "2 gamma event - hits A-B time difference per scintillator",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 201, -fMaxTimeDiff/1000., fMaxTimeDiff/1000.),
                                          "Scintillator ID", "A-B Signal Time Difference [ns]");
  
  
}

