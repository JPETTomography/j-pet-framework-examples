/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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

#include "EventCategorizerTOTvsEdep.h"
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include <JPetWriter/JPetWriter.h>
#include <TH3F.h>
#include <TMath.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace jpet_options_tools;

EventCategorizer::EventCategorizer(const char* name) : JPetUserTask(name) {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if (isOptionSet(fParams.getOptions(), kTOTvsEdepAnniSelUpperParamKey)){
    fAnniSelUpper = getOptionAsFloat(fParams.getOptions(), kTOTvsEdepAnniSelUpperParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kTOTvsEdepAnniSelUpperParamKey.c_str(), fAnniSelUpper
    ));
  }

  if (isOptionSet(fParams.getOptions(), kTOTvsEdepDeexSelUpperParamKey)){
    fDeexSelUpper = getOptionAsFloat(fParams.getOptions(), kTOTvsEdepDeexSelUpperParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kTOTvsEdepDeexSelUpperParamKey.c_str(), fDeexSelUpper
    ));
  }
  
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

bool EventCategorizer::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    uint n = timeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      if (fSaveControlHistos)
      {
        getStatistics().fillHistogram("Hit Multiplicity", event.getHits().size());
      }
      if (event.getHits().size() == 3)
      {
        vector<JPetEvent> vec = analyseThreeHitEvent(&event);
      }
      fEventNumber++;
    }
  }
  return true;
}

bool EventCategorizer::terminate()
{
  INFO("More than one hit Events done. Writing conrtrol histograms.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}

// Three hit events
vector<JPetEvent> EventCategorizer::analyseThreeHitEvent(const JPetEvent* event)
{

  vector<JPetHit> hits = event->getHits();
  for (unsigned int i = 0; i < hits.size(); i++)
  {
    for (unsigned int j = i + 1; j < hits.size(); j++)
    {
      for (unsigned int k = j + 1; k < hits.size(); k++)
      {
        JPetHit firstHit = event->getHits().at(i);
        JPetHit secondHit = event->getHits().at(j);
        JPetHit thirdHit = event->getHits().at(k);

        vector<JPetHit> orderedHits;
        orderedHits.push_back(firstHit);
        orderedHits.push_back(secondHit);
        orderedHits.push_back(thirdHit);
        orderedHits = reorderHits(orderedHits);

        getStatistics().fillHistogram("Time difference 2-1 BOrdering", TMath::Abs(secondHit.getTime() - firstHit.getTime()) * kPsToNs);
        getStatistics().fillHistogram("Time difference 3-2 BOrdering", TMath::Abs(thirdHit.getTime() - secondHit.getTime()) * kPsToNs);
        getStatistics().fillHistogram("Time difference 3-1 BOrdering", TMath::Abs(thirdHit.getTime() - firstHit.getTime()) * kPsToNs);

        // Ordered in time after corrections
        JPetHit firstHit2 = orderedHits.at(0);
        JPetHit secondHit2 = orderedHits.at(1);
        JPetHit thirdHit2 = orderedHits.at(2);

        // Time difference after ordering

        getStatistics().fillHistogram("Time difference 2-1", TMath::Abs(secondHit2.getTime() - firstHit2.getTime()) * kPsToNs);
        getStatistics().fillHistogram("Time difference 3-2", TMath::Abs(thirdHit2.getTime() - secondHit2.getTime()) * kPsToNs);
        getStatistics().fillHistogram("Time difference 3-1", TMath::Abs(thirdHit2.getTime() - firstHit2.getTime()) * kPsToNs);

        // ORDERING DONE (not beautifully however), NOW NEW HITS ARE NAMED AS firstHit2, secondHit2, thirdHit2
        // New hits with ordered in time will be used for the conditions only.
        vector<double> angles;
        angles.push_back(calcAngle(firstHit2, secondHit2));
        angles.push_back(calcAngle(secondHit2, thirdHit2));
        angles.push_back(calcAngle(thirdHit2, firstHit2));
        sort(angles.begin(), angles.end());

        // 3-D angles
        getStatistics().fillHistogram("3_hit_angles", angles[0] + angles[1], angles[1] - angles[0]);

        double Angle_3D_1_2_B = TMath::Abs(calcAngle(firstHit, secondHit));
        double Angle_3D_2_3_B = TMath::Abs(calcAngle(secondHit, thirdHit));
        double Angle_3D_3_1_B = TMath::Abs(calcAngle(thirdHit, firstHit));

        deexcitationSelection(angles, firstHit2, secondHit2, thirdHit2);
        annihilationSelection(angles, firstHit2, secondHit2, thirdHit2);
      }
    }
  }
  vector<JPetEvent> eventVector;
  return eventVector;
}

// de-excitation gamma selection
void EventCategorizer::deexcitationSelection(const vector<double>& angles, const JPetHit& firstHit2, const JPetHit& secondHit2,
                                             const JPetHit& thirdHit2)
{
  if ((angles[0] + angles[1]) < 178 || (angles[0] + angles[1]) > 182)
  {
    double time_diff_check = (thirdHit2.getTime() - firstHit2.getTime()) * kPsToNs;
    double deex_selection = HitFinderTools::calculateTOT(firstHit2, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs;
    getStatistics().fillHistogram("3_hit_angles after cut", angles[0] + angles[1], angles[1] - angles[0]);
    getStatistics().fillHistogram("De-exci time criteria t3-t1", time_diff_check);

    int firstHitScinID = firstHit2.getScintillator().getID();
    int secondHitScinID = secondHit2.getScintillator().getID();
    int thirdHitScinID = thirdHit2.getScintillator().getID();

    // TimeDiff_From && TimeDiff_To used to put time difference between prompt photon and annihilation photon  registration, values are fixed in
    // EventCategorizerTOTvsEdep.h
    // Only upper limit was applied on TOT values for the selection of prompt and annihilation photons fixed in EventCategorizationTOTvsEdep.h

    // Make it sure the first hit is from center
    if (time_diff_check > TimeDiff_From && time_diff_check < TimeDiff_To && deex_selection < fDeexSelUpper &&
        TMath::Abs(firstHit2.getPosZ()) < 23.0 && TMath::Abs(secondHit2.getPosZ()) < 23.0 && TMath::Abs(thirdHit2.getPosZ()) < 23.0 &&
        firstHitScinID != secondHitScinID && firstHitScinID != thirdHitScinID && secondHitScinID != thirdHitScinID)
    {
      fTotal3HitEvents++;
      vector<double> vec0 = scatterAnalysis(firstHit2, secondHit2, thirdHit2, 1274.6);
      getStatistics().fillHistogram("2 hit assignment", vec0.at(5));

      if (vec0.at(9) < 4)
        return;

      if (vec0.at(5) > -0.5 && vec0.at(5) < 0.75)
      {
        writeSelected(firstHit2, secondHit2, vec0, true, 1274.6, firstHit2.getPosZ(), firstHitScinID);
        getStatistics().fillHistogram("De-exci time accepted t3-t1", time_diff_check);
      }
    }
  }
}

void EventCategorizer::annihilationSelection(const vector<double>& angles, const JPetHit& firstHit2, const JPetHit& secondHit2,
                                             const JPetHit& thirdHit2)
{
  // b2b gammas selection
  getStatistics().fillHistogram("Time difference 2-1", TMath::Abs(secondHit2.getTime() - firstHit2.getTime()) * kPsToNs);
  getStatistics().fillHistogram("Time difference 3-2", TMath::Abs(thirdHit2.getTime() - secondHit2.getTime()) * kPsToNs);

  int firstHitScinID = firstHit2.getScintillator().getID();
  int secondHitScinID = secondHit2.getScintillator().getID();
  int thirdHitScinID = thirdHit2.getScintillator().getID();

  if ((angles[0] + angles[1]) > 178 && (angles[0] + angles[1]) < 182 &&
      (TMath::Abs(firstHit2.getTime() - secondHit2.getTime()) * kPsToNs) < 0.2 // Time diff. criteria between hits will cut elongation of scatter test
      && TMath::Abs(firstHit2.getPosZ()) < 23.0 && TMath::Abs(secondHit2.getPosZ()) < 23.0 && TMath::Abs(thirdHit2.getPosZ()) < 23.0 &&
      HitFinderTools::calculateTOT(firstHit2, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs < fAnniSelUpper &&
      HitFinderTools::calculateTOT(secondHit2, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs < fAnniSelUpper &&
      firstHitScinID != secondHitScinID && firstHitScinID != thirdHitScinID && secondHitScinID != thirdHitScinID)
  {

    fTotal3HitEvents++;

    double Angle_3D_1_2 = TMath::Abs(calcAngle(firstHit2, secondHit2));
    double Angle_3D_2_3 = TMath::Abs(calcAngle(secondHit2, thirdHit2));
    double Angle_3D_3_1 = TMath::Abs(calcAngle(thirdHit2, firstHit2));

    getStatistics().fillHistogram("3_hit_angles after cut BTB", angles[0] + angles[1], angles[1] - angles[0]);

    vector<double> vec1 = scatterAnalysis(firstHit2, thirdHit2, secondHit2, 511.0);
    vector<double> vec2 = scatterAnalysis(secondHit2, thirdHit2, firstHit2, 511.0);

    if (TMath::Abs(vec1.at(13) > 2))
      return;
    if (vec1.at(9) < 4 || vec1.at(9) < 4)
      return;

    getStatistics().fillHistogram("3rd hit assignment_wc", vec1.at(5), vec2.at(5));
    getStatistics().fillHistogram("Annihilation points XY position", vec1.at(10), vec1.at(11));

    // Implementation of ellip. cuts :

    // Calculate the equations:
    double x = vec1.at(5), y = vec2.at(5), angle = 0.785398, a = 1.50, b = 0.82;
    double Cut_sca_13 =
        pow((((x - 0.4324) * cos(angle) + (y + 2.266) * sin(angle)) / a), 2) + pow((((x - 0.4324) * sin(angle) - (y + 2.266) * cos(angle)) / b), 2);
    double Cut_sca_23 =
        pow((((x + 2.498) * cos(angle) + (y - 0.4124) * sin(angle)) / a), 2) + pow((((x + 2.498) * sin(angle) - (y - 0.4124) * cos(angle)) / b), 2);

    if (Cut_sca_13 < 1)
    {
      writeSelected(firstHit2, thirdHit2, vec1, true, 511, firstHit2.getPosZ(), firstHitScinID);
      getStatistics().fillHistogram("3rd hit assignment", vec1.at(5), vec2.at(5));
      getStatistics().fillHistogram("Scatt_ZHit", firstHit2.getPosZ(), vec1.at(2));
      getStatistics().fillHistogram("scatter_angle_sel_511", vec1.at(2));
      getStatistics().fillHistogram("Energy_dep_sel_511", 511 * (1 - (1 / (1 + ((511 / 511) * (1 - cos(vec1.at(2) * TMath::Pi() / 180)))))));
    }
    else if (Cut_sca_23 < 1)
    {
      writeSelected(secondHit2, thirdHit2, vec2, true, 511, secondHit2.getPosZ(), secondHitScinID);
      getStatistics().fillHistogram("3rd hit assignment", vec1.at(5), vec2.at(5));
      getStatistics().fillHistogram("Scatt_ZHit", secondHit2.getPosZ(), vec2.at(2));
      getStatistics().fillHistogram("scatter_angle_sel_511", vec2.at(2));
      getStatistics().fillHistogram("Energy_dep_sel_511", 511 * (1 - (1 / (1 + ((511 / 511) * (1 - cos(vec2.at(2) * TMath::Pi() / 180)))))));
    }
  }
}

vector<JPetHit> EventCategorizer::reorderHits(vector<JPetHit> hits)
{
  vector<JPetHit> reorderedHits;
  auto first = new JPetHit(hits.at(0));
  auto second = new JPetHit(hits.at(1));
  auto third = new JPetHit(hits.at(2));

  // Corrected time is a original time in pico seconds
  // minus value of time of flight from the center - distance/speed of light
  // Setting the time of new hits as corrected ones
  first->setTime(hits.at(0).getTime() - hits.at(0).getPos().Mag() * kNsToPs / kLightVelocity_cm_ns);
  second->setTime(hits.at(1).getTime() - hits.at(1).getPos().Mag() * kNsToPs / kLightVelocity_cm_ns);
  third->setTime(hits.at(2).getTime() - hits.at(2).getPos().Mag() * kNsToPs / kLightVelocity_cm_ns);

  reorderedHits.push_back(*first);
  reorderedHits.push_back(*second);
  reorderedHits.push_back(*third);

  return JPetAnalysisTools::getHitsOrderedByTime(reorderedHits);
}

// Scatter Analysis
vector<double> EventCategorizer::scatterAnalysis(const JPetHit& primary1, const JPetHit& scatter, const JPetHit& primary2, double gamma)
{
  vector<double> values;
  // cm/nsec
  TVector3 primary1Vec, scatterVec;
  double xcor, ycor, zcor, distanceFromCenter;

  if (gamma == 511.0)
  {
    TVector3 anniPoint = EventCategorizerTools::calculateAnnihilationPoint(primary1, primary2);
    xcor = anniPoint.X();
    ycor = anniPoint.Y();
    zcor = anniPoint.Z();
    distanceFromCenter = sqrt(pow(xcor, 2) + pow(ycor, 2));
    primary1Vec = primary1.getPos() - primary2.getPos();
    scatterVec = scatter.getPos() - primary1.getPos();
  }
  else if (gamma == 1274.6)
  {
    primary1Vec = primary1.getPos();
    scatterVec = scatter.getPos() - primary1.getPos();
  }

  double distance = scatterVec.Mag();
  double primary1Theta = primary1.getBarrelSlot().getTheta();
  double scatterTheta = scatter.getBarrelSlot().getTheta();
  double PhiAngle = TMath::Abs(scatterTheta - primary1Theta);

  // Follow dot product recipe
  double scatterAngle = TMath::RadToDeg() * scatterVec.Angle(primary1Vec);
  getStatistics().fillHistogram("scatter_angle_all", scatterAngle);

  double thetaDiff = 0.0;
  if (primary1Theta > scatterTheta && primary1Theta - scatterTheta < 180.0)
  {
    thetaDiff = primary1Theta - scatterTheta;
  }
  else if (primary1Theta > scatterTheta && primary1Theta - scatterTheta > 180.0)
  {
    thetaDiff = 360.0 - primary1Theta + scatterTheta;
  }
  else if (scatterTheta > primary1Theta && scatterTheta - primary1Theta < 180.0)
  {
    thetaDiff = scatterTheta - primary1Theta;
  }
  else if (scatterTheta > primary1Theta && scatterTheta - primary1Theta > 180.0)
  {
    thetaDiff = 360.0 - scatterTheta + primary1Theta;
  }

  // Distance b/w hits
  double computedDistance = (primary1.getPos() - scatter.getPos()).Mag();

  // pico to nano scatter conversion
  double hitTimeDiff = (scatter.getTime() - primary1.getTime()) * kPsToNs;
  double evalSpeedLight = distance / TMath::Abs(hitTimeDiff);

  // Using scattered angle, calculate theoretical value of energy deposition
  double Edep = gamma * (1 - (1 / (1 + ((gamma / 511) * (1 - cos(scatterAngle * TMath::Pi() / 180))))));

  if (gamma == 511)
  {
    getStatistics().fillHistogram("EDEP_VS_SCATTER_511", scatterAngle, Edep);
    getStatistics().fillHistogram("scatter_angle_all_511", scatterAngle);
  }
  if (gamma == 1274.6)
  {
    getStatistics().fillHistogram("EDEP_VS_SCATTER_1274", scatterAngle, Edep);
    getStatistics().fillHistogram("scatter_angle_all_1275", scatterAngle);
  }

  // FILLING VALUES
  // 0: distance, 1: thetaDiff, 2: scatterAngle, 3: distance-computedDistance
  // 4: speed of light constrain 5: hit-assignemnet 6:angle-edep
  // 7: Primary Scint ID, 8: Scint_Id of scintillator with scattering
  // 9: Phi angle of scatter, 10,11,12 - annihilation point coordinates,..
  values.push_back(distance);
  values.push_back(thetaDiff);
  values.push_back(scatterAngle);
  values.push_back(TMath::Abs(distance - computedDistance));
  values.push_back(evalSpeedLight);
  values.push_back(hitTimeDiff - distance / kLightVelocity_cm_ns);
  values.push_back(Edep);
  values.push_back(primary1.getScintillator().getID());
  values.push_back(scatter.getScintillator().getID());
  values.push_back(PhiAngle);
  values.push_back(xcor);
  values.push_back(ycor);
  values.push_back(zcor);
  values.push_back(distanceFromCenter);
  return values;
}

// ANGLE CALCULATION
double EventCategorizer::calcAngle(const JPetHit& primary, const JPetHit& scatter)
{
  return TMath::RadToDeg() * primary.getPos().Angle(scatter.getPos());
}

// Write selected histogram section
void EventCategorizer::writeSelected(JPetHit orig, JPetHit /*scatter*/, vector<double> values, bool isScatter, double Gamma_inv, double hitZ,
                                     int /*ScinID*/
)
{
  if (isScatter)
  {
    // gerenal fill
    getStatistics().fillHistogram("scatter_angle_sel", values.at(2));
    getStatistics().fillHistogram("TOT_EDEP", values.at(6),
                                  HitFinderTools::calculateTOT(orig, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs);
    getStatistics().fillHistogram("TOT_Spectra_3-Hit",
                                  HitFinderTools::calculateTOT(orig, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs);
  }

  // 511 keV fill
  if (Gamma_inv == 511)
  {
    getStatistics().fillHistogram("TOT_EDEP 511", values.at(6),
                                  HitFinderTools::calculateTOT(orig, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs);
  }

  // 1274 fill
  if (Gamma_inv == 1274.6)
  {
    double primary2_Scatt_angle = values.at(2);
    double primary2_eng_dep_511 = 511 * (1 - (1 / (1 + ((511 / 511) * (1 - cos(primary2_Scatt_angle * TMath::Pi() / 180))))));
    double primary2_TOT_val = -2332.32 + 632.1 * log(primary2_eng_dep_511 + 606.909) - 42.0769 * pow(log(primary2_eng_dep_511 + 606.909), 2);
    double primary2_eng_dep_1275 = Gamma_inv * (1 - (1 / (1 + ((Gamma_inv / 511) * (1 - cos(primary2_Scatt_angle * TMath::Pi() / 180))))));
    getStatistics().fillHistogram("TOT_EDEP 1274", values.at(6),
                                  HitFinderTools::calculateTOT(orig, HitFinderTools::getTOTCalculationType("standard")) * kPsToNs);
    getStatistics().fillHistogram("scatter_angle_sel_1275", values.at(2));
    getStatistics().fillHistogram("TOT_EDEP_dummy", primary2_eng_dep_1275, primary2_TOT_val);
  }
}

void EventCategorizer::initialiseHistograms()
{
  // 3-HIT ANGLES
  getStatistics().createHistogramWithAxes(new TH2D("3_hit_angles", "3 Hit angles", 360, 0, 360, 360, 0, 360),
                                          "(Smallest angle + Second smallest angle) [deg]", "(Second smallest angle - Smallest angle) [deg]");

  getStatistics().createHistogramWithAxes(new TH2D("3_hit_angles after cut BTB", "3 Hit angles", 360, 0, 360, 360, 0, 360),
                                          "(Smallest angle + Second smallest angle) [deg]", "(Second smallest angle - Smallest angle) [deg]");

  // 3-HIT ANGLES NOT B2B
  getStatistics().createHistogramWithAxes(new TH2D("3_hit_angles after cut", "3 Hit angles", 360, 0, 360, 360, 0, 360),
                                          "(Smallest angle + Second smallest angle) [deg]", "(Second smallest angle - Smallest angle) [deg]");
  // Scatter TEST
  // Scatter test without Cut
  getStatistics().createHistogramWithAxes(new TH2D("3rd hit assignment_wc", "3rd hit assignment_wc", 90, -8.5, 9.5, 90, -8.5, 9.5),
                                          "(t3-t2) - L32/c [ns]", "(t3-t1) - L31/c [ns]");

  // Scatter test after Cut
  getStatistics().createHistogramWithAxes(new TH2D("3rd hit assignment", "3rd hit assignment", 90, -8.5, 9.5, 90, -8.5, 9.5), "(t3-t2) - L32/c [ns]",
                                          "(t3-t1) - L31/c [ns]");

  // ANGLE DIFFERENCES ends
  getStatistics().createHistogramWithAxes(
      new TH1D("Time difference 2-1 BOrdering", "Time difference b/w primary1 and scatter BOrdering", 320, -19.5, 300.5), "time [ns]", "counts[a.u]");

  getStatistics().createHistogramWithAxes(
      new TH1D("Time difference 3-2 BOrdering", "Time difference b/w scatter and primary2 BOrdering", 320, -19.5, 300.5), "time [ns]", "counts[a.u]");

  getStatistics().createHistogramWithAxes(
      new TH1D("Time difference 3-1 BOrdering", "Time difference b/w primary1 and primary2 BOrdering", 320, -19.5, 300.5), "time [ns]",
      "counts[a.u]");

  // Hit Time difference 2-1 before ordered
  // Hit time diffference after ordered
  getStatistics().createHistogramWithAxes(new TH1D("Time difference 2-1", "Time difference b/w primary1 and scatter", 320, -19.5, 300.5), "time [ns]",
                                          "counts[a.u]");

  getStatistics().createHistogramWithAxes(new TH1D("Time difference 3-2", "Time difference b/w scatter and primary2", 320, -19.5, 300.5), "time [ns]",
                                          "counts[a.u]");

  getStatistics().createHistogramWithAxes(new TH1D("Time difference 3-1", "Time difference b/w scatter and primary2", 320, -19.5, 300.5), "time [ns]",
                                          "counts[a.u]");

  // 2 HIT EVENTS ANALYSIS HISTOGRAMS
  // 2ND HIT SELECTED
  getStatistics().createHistogramWithAxes(new TH1D("2 hit assignment", "criteria for 2 hit assignment", 4000, -19.5, 60.5), "counts [a.u]",
                                          "( t2 - t1 ) - Distance12/c [ns]");

  // COMMON HISTOGRAMS
  // More TOT spectra
  // after selected cuts of 511

  // TOT sum spectra 3-hit events
  getStatistics().createHistogramWithAxes(new TH1D("TOT_Spectra_3-Hit", "Time over Thresholds", 1500, 0.0, 150.0), " TOT [ns]", "counts [a.u]");

  // SCATTERED ANGLE ALL
  getStatistics().createHistogramWithAxes(new TH1D("scatter_angle_all", "Scattered Angles", 180, -0.5, 179.5), "Scattering angles [deg]",
                                          "counts [a.u]");

  // Scatter Angle All 511
  getStatistics().createHistogramWithAxes(new TH1D("scatter_angle_all_511", "Scattered Angles all 511", 180, -0.5, 179.5), "Scattering angles [deg]",
                                          "counts [a.u]");

  // Scatter Angle All 1275
  getStatistics().createHistogramWithAxes(new TH1D("scatter_angle_all_1275", "Scattered Angles all 1275", 180, -0.5, 179.5),
                                          "Scattering angles [deg]", "counts [a.u]");

  // SCATTERED ANGLES SELECTED
  getStatistics().createHistogramWithAxes(new TH1D("scatter_angle_sel", "Selected Scatter Angle", 180, -0.5, 179.5), "Scattering angles [deg]",
                                          "counts [a.u]");

  // 511 keV
  getStatistics().createHistogramWithAxes(new TH1D("scatter_angle_sel_511", "Selected Scatter Angle", 180, -0.5, 179.5), "Scatter angles [deg]",
                                          "counts [a.u]");

  // 1275 keV
  getStatistics().createHistogramWithAxes(new TH1D("scatter_angle_sel_1275", "Selected Scatter Angle", 180, -0.5, 179.5), "Scattering angles [deg]",
                                          "counts [a.u]");

  // TOT vs Edep total
  getStatistics().createHistogramWithAxes(new TH2D("TOT_EDEP", "TOT vs. Energy Deposition", 3600, 0, 1200, 1500, 0.0, 150.0),
                                          "Time over Thresholds [ns]", "Energy Deposition [keV]");

  // TOT vs Edep total primary2 to find entry fo low energy photon in 1274.6 keV loop
  getStatistics().createHistogramWithAxes(new TH2D("TOT_EDEP_dummy", "TOT vs. Energy Deposition dummy", 3600, 0, 1200, 1500, 0, 150.),
                                          "Time over Thresholds [ns]", "Energy Deposition [keV]");

  //+++++++++++ Hit wise filling TOT vs Edep ++++++++++++++++++++++++++++++

  // TOT vs Edep for 511 keV
  getStatistics().createHistogramWithAxes(new TH2D("TOT_EDEP 511", "TOT vs. Energy Deposition for 511", 1500, 0, 500, 1500, 0, 150.),
                                          "Time over Thresholds [ns]", "Energy Deposition [keV]");

  // TOT vs Edep for 1274.6 keV
  getStatistics().createHistogramWithAxes(new TH2D("TOT_EDEP 1274", "TOT vs. Energy Deposition", 3600, 0, 1200, 1500, 0, 150.),
                                          "Time over Thresholds [ns]", "Energy Deposition [keV]");

  // SPEED OF LIGHT AND HIT-CRITERION
  // energy dep for 511 keV scatter photons 2 august 2018
  getStatistics().createHistogramWithAxes(new TH1D("Energy_dep_sel_511", "Energy deposition by 511 keV photons", 380, 0, 380), "Energy [keV]",
                                          "Number of events");

  // Threshold check and filling FIRED
  // Compton eng deposition 1274.6 keV
  getStatistics().createHistogramWithAxes(new TH2D("EDEP_VS_SCATTER_1274", "TOT vs. Energy Deposition", 181, -0.5, 180.5, 1080, 0.0, 1080.0),
                                          "Energy Deposition [keV]", "Scatter Angle [deg]");

  // Compton eng deposition 511 keV
  getStatistics().createHistogramWithAxes(new TH2D("EDEP_VS_SCATTER_511", "TOT vs. Energy Deposition", 181, -0.5, 180.5, 380, 0.0, 380),
                                          "Energy Deposition [keV]", "Scatter Angle [deg]");

  getStatistics().createHistogramWithAxes(new TH1D("De-exci time criteria t3-t1", "Time difference between hit 3 and 1", 310, -10, 300),
                                          "time difference t3-t1 [ns]", "counts [ au ]");

  getStatistics().createHistogramWithAxes(new TH1D("De-exci time accepted t3-t1", "Time difference accepted between hit 3 and 1", 310, -10, 300),
                                          "time difference t3-t1 [ns]", "counts [ au ]");

  // Added on 29 April 2018

  getStatistics().createHistogramWithAxes(new TH2D("Scatt_ZHit", "Hit Z vs. Scattered Angle", 56, -28, 28, 360, 0.0, 360.0), "Scatter Angle [deg]",
                                          "Z Hit [cm]");

  // Annihilation point
  // Reconstructed image of annihilation point in 3 D after looping
  // Annihilation point 2D
  getStatistics().createHistogramWithAxes(
      new TH2D("Annihilation points XY position", "Reconstruction of annihilation point", 60, -29.5, 30.5, 60, -29.5, 30.5), "Y coordinate [cm]",
      "X coordinate [cm]");

  // Hit multiplicity
  getStatistics().createHistogramWithAxes(new TH1D("Hit Multiplicity", "Event wise hit multiplicity", 100, -0.5, 99.5), "Hits", "counts [a.u]");
}
