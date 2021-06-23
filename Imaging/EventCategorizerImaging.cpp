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
 *  @file EventCategorizerImaging.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <iostream>

#include "../ModularDetectorAnalysis/EventCategorizerTools.h"
#include "EventCategorizerImaging.h"

using namespace jpet_options_tools;
using namespace std;

EventCategorizerImaging::EventCategorizerImaging(const char* name) : JPetUserTask(name) {}

bool EventCategorizerImaging::init()
{
  INFO("Imaging stream started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Event categorization parameters
  // Angle
  if (isOptionSet(fParams.getOptions(), k2GammaMaxAngleParamKey))
  {
    f2GammaMaxAngle = getOptionAsDouble(fParams.getOptions(), k2GammaMaxAngleParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2GammaMaxAngleParamKey.c_str(), f2GammaMaxAngle));
  }
  if (isOptionSet(fParams.getOptions(), k3GammaMinAngleParamKey))
  {
    f3GammaMinAngle = getOptionAsDouble(fParams.getOptions(), k3GammaMinAngleParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k3GammaMinAngleParamKey.c_str(), f3GammaMinAngle));
  }

  // TOT cut
  if (isOptionSet(fParams.getOptions(), kMinAnnihTOTParamKey))
  {
    fMinAnnihTOT = getOptionAsDouble(fParams.getOptions(), kMinAnnihTOTParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinAnnihTOTParamKey.c_str(), fMinAnnihTOT));
  }
  if (isOptionSet(fParams.getOptions(), kMaxAnnihTOTParamKey))
  {
    fMaxAnnihTOT = getOptionAsDouble(fParams.getOptions(), kMaxAnnihTOTParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxAnnihTOTParamKey.c_str(), fMaxAnnihTOT));
  }

  // Time diffs
  if (isOptionSet(fParams.getOptions(), kScatterTimeDiffParamKey))
  {
    fScatterTOFTimeDiff = getOptionAsDouble(fParams.getOptions(), kScatterTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kScatterTimeDiffParamKey.c_str(),
                 fScatterTOFTimeDiff));
  }
  if (isOptionSet(fParams.getOptions(), k2GammaMaxTOFParamKey))
  {
    f2GammaMaxTOF = getOptionAsDouble(fParams.getOptions(), k2GammaMaxTOFParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k2GammaMaxTOFParamKey.c_str(), f2GammaMaxTOF));
  }
  if (isOptionSet(fParams.getOptions(), k3GammaMaxTimeDiffParamKey))
  {
    f3GammaMaxTimeDiff = getOptionAsDouble(fParams.getOptions(), k3GammaMaxTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", k3GammaMaxTimeDiffParamKey.c_str(),
                 f3GammaMaxTimeDiff));
  }

  if (isOptionSet(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey))
  {
    fDPCenterDist = getOptionAsDouble(fParams.getOptions(), kMaxDistOfDecayPlaneFromCenterParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxDistOfDecayPlaneFromCenterParamKey.c_str(),
                 fDPCenterDist));
  }

  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveLORsOnlyParamKey))
  {
    fSaveLORsOnly = getOptionAsBool(fParams.getOptions(), kSaveLORsOnlyParamKey);
    if (fSaveLORsOnly)
    {
      INFO("Imaging stream is set to save only events with 2 hits that meet LOR selection conditions.");
    }
  }

  if (fSaveHistos)
  {
    initialiseHistograms();
  }

  return true;
}

bool EventCategorizerImaging::exec()
{
  vector<JPetEvent> eventVec;
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      if (fSaveLORsOnly)
      {
        auto lors =
            EventCategorizerTools::getLORs(event, getStatistics(), fSaveHistos, f2GammaMaxTOF, fScatterTOFTimeDiff, fMinAnnihTOT, fMaxAnnihTOT);
        events.insert(events.end(), lors.begin(), lors.end());
      }
      else
      {
        JPetEvent categorizedEvent(event);
        if (EventCategorizerTools::stream2Gamma(event, getStatistics(), fSaveHistos, f2GammaMaxAngle, f2GammaMaxTOF, fScatterTOFTimeDiff))
        {
          categorizedEvent.addEventType(JPetEventType::k2Gamma);
        }
        if (EventCategorizerTools::stream3Gamma(event, getStatistics(), fSaveHistos, f3GammaMinAngle, f3GammaMaxTimeDiff, fDPCenterDist,
                                                fScatterTOFTimeDiff))
        {
          categorizedEvent.addEventType(JPetEventType::k3Gamma);
        }
        if (categorizedEvent.isTypeOf(JPetEventType::k2Gamma) || categorizedEvent.isTypeOf(JPetEventType::k3Gamma))
        {
          events.push_back(categorizedEvent);
        }
        // Checking event type for control histograms
        if (fSaveHistos)
        {
          if (categorizedEvent.isTypeOf(JPetEventType::kUnknown))
          {
            getStatistics().fillHistogram("event_tags", 1);
          }
          if (categorizedEvent.isTypeOf(JPetEventType::k2Gamma))
          {
            getStatistics().fillHistogram("event_tags", 2);
          }
          if (categorizedEvent.isTypeOf(JPetEventType::k3Gamma))
          {
            getStatistics().fillHistogram("event_tags", 3);
          }
        }
      }
    }
    saveEvents(events);
  }
  else
  {
    return false;
  }
  return true;
}

bool EventCategorizerImaging::terminate()
{
  INFO("Imaging streaming ended.");
  return true;
}

void EventCategorizerImaging::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

void EventCategorizerImaging::initialiseHistograms()
{
  double totUppLimit = 10000000.0;
  getStatistics().createHistogramWithAxes(new TH1D("2g_tot", "2 gamma event - average TOT scaled", 201, 0.0, totUppLimit), "Time over Threshold [ps]",
                                          "Number of Hits");

  getStatistics().createHistogramWithAxes(
      new TH1D("2g_tof", "2 gamma event - TOF calculated by convention", 201, -5 * f2GammaMaxTOF, 5 * f2GammaMaxTOF), "Time of Flight [ps]",
      "Number of Hits Pairs");

  getStatistics().createHistogramWithAxes(new TH2D("ap_yx", "YX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Y position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_zx", "ZX position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "X position [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("ap_zy", "ZY position of annihilation point (bin 0.5 cm)", 202, -50.5, 50.5, 202, -50.5, 50.5),
                                          "Z position [cm]", "Y position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_yx_zoom", "YX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Y position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_zx_zoom", "ZX position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "X position [cm]");

  getStatistics().createHistogramWithAxes(
      new TH2D("ap_zy_zoom", "ZY position of annihilation point (bin 0.25 cm)", 132, -16.5, 16.5, 132, -16.5, 16.5), "Z position [cm]",
      "Y position [cm]");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_pass", "Scatter test passed by two hits", 200, 0.0, 5.0 * fScatterTOFTimeDiff),
                                          "Difference of Scatter TOF and Hits time difference [ps]", "Number of Hits Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("scatter_test_fail", "Scatter test failed by two hits", 200, 0.0, 5.0 * fScatterTOFTimeDiff),
                                          "Difference of Scatter TOF and Hits time difference [ps]", "Number of Hits Pairs");

  getStatistics().createHistogramWithAxes(
      new TH2D("scatter_angle_tot_primary", "Scattering angle vs. Primary Hit TOT", 181, -0.5, 180.5, 201, 0.0, totUppLimit),
      "Scattering Angle [deg]", "Time over Threshold [ps]");

  getStatistics().createHistogramWithAxes(
      new TH2D("scatter_angle_tot_scatter", "Scattering angle vs. Scattered Hit TOT", 181, -0.5, 180.5, 201, 0.0, totUppLimit),
      "Scattering Angle [deg]", "Time over Threshold [ps]");

  getStatistics().createHistogramWithAxes(new TH1D("stream2g_tdiff", "2 Gamma Hits Time Difference", 200, 0.0, 10000.0), "Hits time difference [ns]",
                                          "Number of paris");

  getStatistics().createHistogramWithAxes(new TH1D("stream2g_dlor_dist", "Delta LOR distance", 200, 0.0, 50.0), "Delta LOR [cm]", "Number LORs");

  getStatistics().createHistogramWithAxes(new TH1D("stream2g_theta_diff", "2 Gamma Hits angles", 180, -0.5, 179.5), "Hits theta diff [deg]",
                                          "Number of hit pairs");

  getStatistics().createHistogramWithAxes(new TH2D("stream3g_thetas", "3 Gamma Thetas plot", 251, -0.5, 250.5, 201, -0.5, 200.5),
                                          "Transformed thetas 1-2 [deg]", "Transformed thetas 2-3 [deg]");

  getStatistics().createHistogramWithAxes(new TH1D("stream3g_plane_dist", "3 Gamma Plane Distance to Center", 100, 0.0, 5 * fDPCenterDist),
                                          "Distance [cm]", "Events");

  getStatistics().createHistogramWithAxes(new TH1D("stream3g_tdiff", "3 gamma last and first hit time difference", 200, 0.0, 10000.0),
                                          "Time Difference [ps]", "Events");

  getStatistics().createHistogramWithAxes(new TH1D("event_tags", "Result of event categorization", 5, 0.5, 5.5), "Signal label", "Number of SigChs");
  std::vector<std::pair<unsigned, std::string>> binLabels = {make_pair(1, "Unknown"), make_pair(2, "2 gamma"), make_pair(3, "3 gamma"),
                                                             make_pair(4, "Prompt"), make_pair(5, "Cosmics")};
  getStatistics().setHistogramBinLabel("event_tags", getStatistics().AxisLabel::kXaxis, binLabels);

  // Histograms for scattering category
  // getStatistics().createHistogram(
  //   new TH1F("ScatterTOF_TimeDiff", "Difference of Scatter TOF and hits time difference",
  //     200, 0.0, 3.0*fScatterTOFTimeDiff));
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetXaxis()->SetTitle("Scat_TOF & time diff [ps]");
  // getStatistics().getHisto1D("ScatterTOF_TimeDiff")->GetYaxis()->SetTitle("Number of Hit Pairs");

  // getStatistics().createHistogram(
  //    new TH2F("ScatterAngle_PrimaryTOT", "Angle of scattering vs. TOT of primary hits",
  //     181, -0.5, 180.5, 200, 0.0, 40000.0));
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetXaxis()->SetTitle("Scattering Angle");
  // getStatistics().getHisto2D("ScatterAngle_PrimaryTOT")->GetYaxis()->SetTitle("TOT of primary hit [ps]");
}
