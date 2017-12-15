
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
 *  @file ImageReco.cpp
 */

#include "ImageReco.h"
#include <TH3D.h>
#include <TH1I.h>
#include "./JPetOptionsTools/JPetOptionsTools.h"
using namespace jpet_options_tools;

const int numberOfBins = 100;
const int xRange = 100;
const int yRange = 100;
const int zRange = 50;

const int numberOfHitsInEventHisto = 10;
const int numberOfConditions = 6;

int CUT_ON_Z_VALUE = 23;
int CUT_ON_LOR_DISTANCE_FROM_CENTER = 25;
int ANNIHILATION_POINT_Z = 23;
int TOT_MIN_VALUE_IN_NS = 15;
int TOT_MAX_VALUE_IN_NS = 25;
int ANGLE_DELTA_MIN_VALUE = 20;

ImageReco::ImageReco(const char* name) : JPetUserTask(name) {}

ImageReco::~ImageReco() {}

bool ImageReco::init()
{
  auto opts = getOptions();
  CUT_ON_Z_VALUE = getOptionAsInt(opts, "ImageReco_CUT_ON_Z_VALUE_int");
  CUT_ON_LOR_DISTANCE_FROM_CENTER = getOptionAsInt(opts, "ImageReco_CUT_ON_LOR_DISTANCE_FROM_CENTER_int");
  ANNIHILATION_POINT_Z = getOptionAsInt(opts, "ImageReco_ANNIHILATION_POINT_Z_int");
  TOT_MIN_VALUE_IN_NS = getOptionAsInt(opts, "ImageReco_TOT_MIN_VALUE_IN_NS_int");
  TOT_MAX_VALUE_IN_NS = getOptionAsInt(opts, "ImageReco_TOT_MAX_VALUE_IN_NS_int");
  ANGLE_DELTA_MIN_VALUE = getOptionAsInt(opts, "ImageReco_ANGLE_DELTA_MIN_VALUE_int");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH3D("hits_pos",
                                  "Reconstructed hit pos",
                                  numberOfBins, -xRange, xRange,
                                  numberOfBins, -yRange, yRange,
                                  numberOfBins, -zRange, zRange));
  getStatistics().createHistogram(new TH1I("number_of_events",
                                  "Number of events with n hits",
                                  numberOfHitsInEventHisto, 0, numberOfHitsInEventHisto));
  getStatistics().createHistogram(new TH1I("number_of_hits_filtered_by_condition",
                                  "Number of hits filtered by condition",
                                  numberOfConditions, 0, numberOfConditions));

  getStatistics().createHistogram(new TH1D("distance_from_center",
                                  "Distance from center",
                                  zRange, -zRange, zRange));

  getStatistics().createHistogram(new TH1D("cut_on_Z",
                                  "Cut on Z",
                                  zRange, -zRange, zRange));

  getStatistics().createHistogram(new TH1D("angle_delta",
                                  "Angle delta",
                                  360, 0, 360));

  getStatistics().createHistogram(new TH1D("first_hit_TOT",
                                  "Cut on first hit TOT",
                                  100, 0, 100));

  getStatistics().createHistogram(new TH1D("second_hit_TOT",
                                  "Cut on second hit TOT",
                                  100, 0, 100));

  getStatistics().createHistogram(new TH1D("annihilation_point_z",
                                  "Annihilation point Z",
                                  zRange, -zRange, zRange));

  //it is not really nessesery, but it is creating labels in given order
  getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on Z", 1);
  getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on LOR distance", 1);
  getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on delta angle", 1);
  getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on first hit TOT", 1);
  getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on second hit TOT", 1);
  getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on annihilation point Z", 1);

  return true;
}

bool ImageReco::exec()
{
  if (auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      auto numberOfHits = event.getHits().size();
      getStatistics().getHisto<TH1I>("number_of_events").Fill(numberOfHits);
      if (numberOfHits <= 1)
        continue;
      else {
        auto hits = event.getHits();
        for (unsigned int i = 0; i < hits.size() - 1; i++) {
          if (!checkConditions(hits[i], hits[i + 1]))
            continue;
          calculateReconstructedPosition(hits[i], hits[i + 1]);
        }
      }
    }
  } else {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool ImageReco::terminate()
{
  return true;
}

bool ImageReco::checkConditions(const JPetHit& first, const JPetHit& second)
{
  if (!cutOnZ(first, second)) {
    getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on Z", 1);
    return false;
  }
  if (!cutOnLORDistanceFromCenter(first, second)) {
    getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on LOR distance", 1);
    return false;
  }
  if (angleDelta(first, second) < ANGLE_DELTA_MIN_VALUE) {
    getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on delta angle", 1);
    return false;
  }

  double totOfFirstHit = calculateSumOfTOTsOfHit(first);
  getStatistics().getHisto<TH1D>("first_hit_TOT").Fill(totOfFirstHit);
  if (totOfFirstHit < TOT_MIN_VALUE_IN_NS || totOfFirstHit > TOT_MAX_VALUE_IN_NS) {
    getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on first hit TOT", 1);
    return false;
  }

  double totOfSecondHit = calculateSumOfTOTsOfHit(second);
  getStatistics().getHisto<TH1D>("second_hit_TOT").Fill(totOfSecondHit);
  if (totOfSecondHit < TOT_MIN_VALUE_IN_NS || totOfSecondHit > TOT_MAX_VALUE_IN_NS) {
    getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on second hit TOT", 1);
    return false;
  }

  return true;
}

bool ImageReco::cutOnZ(const JPetHit& first, const JPetHit& second)
{
  getStatistics().getHisto<TH1D>("cut_on_Z").Fill(std::fabs(first.getPosZ()));
  getStatistics().getHisto<TH1D>("cut_on_Z").Fill(std::fabs(second.getPosZ()));
  return (std::fabs(first.getPosZ()) < CUT_ON_Z_VALUE) && (fabs(second.getPosZ()) < CUT_ON_Z_VALUE);
}

bool ImageReco::cutOnLORDistanceFromCenter(const JPetHit& first, const JPetHit& second)
{
  double x_a = first.getPosX();
  double x_b = second.getPosX();

  double y_a = first.getPosY();
  double y_b = second.getPosY();

  double a = (y_a - y_b) / (x_a - x_b);
  double c = y_a - ((y_a - y_b) / (x_a - x_b)) * x_a;
  getStatistics().getHisto<TH1D>("distance_from_center").Fill((std::fabs(c) / std::sqrt(a * a + 1)));
  return (std::fabs(c) / std::sqrt(a * a + 1)) < CUT_ON_LOR_DISTANCE_FROM_CENTER; //b is 1 and b*b is 1
}

float ImageReco::angleDelta(const JPetHit& first, const JPetHit& second)
{
  float delta = fabs(first.getBarrelSlot().getTheta() - second.getBarrelSlot().getTheta());
  getStatistics().getHisto<TH1D>("angle_delta").Fill(std::min(delta, (float)360 - delta));
  return std::min(delta, (float)360 - delta);
}

double ImageReco::calculateSumOfTOTsOfHit(const JPetHit& hit)
{
  return calculateSumOfTOTs(hit.getSignalA()) + calculateSumOfTOTs(hit.getSignalB());
}

double ImageReco::calculateSumOfTOTs(const JPetPhysSignal& signal)
{
  double tot = 0.;
  std::map<int, double> leadingPoints, trailingPoints;
  leadingPoints = signal.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  trailingPoints = signal.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
  for (int i = 1; i < 5; i++) {
    auto leadSearch = leadingPoints.find(i);
    auto trailSearch = trailingPoints.find(i);

    if (leadSearch != leadingPoints.end() && trailSearch != trailingPoints.end())
      tot += (trailSearch->second - leadSearch->second);
  }
  return tot / 1000.;
}

bool ImageReco::calculateReconstructedPosition(const JPetHit& firstHit, const JPetHit& secondHit)
{
  double s1_a = static_cast<double>(firstHit.getSignalA().getTime());
  double s1_b = static_cast<double>(firstHit.getSignalB().getTime());
  double s2_a = static_cast<double>(secondHit.getSignalA().getTime());
  double s2_b = static_cast<double>(secondHit.getSignalB().getTime());

  double t_s1_ab = s1_a - s1_b;
  double t_s2_ab = s2_a - s2_b;

  double s1_z = (t_s1_ab * 11) / 2.0;
  double s2_z = (t_s2_ab * 11) / 2.0;

  double vdx = secondHit.getPosX() - firstHit.getPosX();
  double vdz = s2_z - s1_z;
  double vdy = secondHit.getPosY() - firstHit.getPosY();

  double dd = std::sqrt(vdx * vdx + vdz * vdz + vdy * vdy);

  double mtof_a = 0;
  if (firstHit.getPosY() > secondHit.getPosY()) {
    mtof_a = ((((s1_a + s1_b) / 2.0) - ((s2_a + s2_b) / 2.0)) * 30);
  } else {
    mtof_a = ((((s2_a + s2_b) / 2.0) - ((s1_a + s1_b) / 2.0)) * 30);
  }
  double x, y, z;
  x = firstHit.getPosX() + ((vdx / 2.0) + (vdx / dd * mtof_a));
  y = firstHit.getPosY() + ((vdy / 2.0) + (vdy / dd * mtof_a));
  z = s1_z + ((vdz / 2.0) + (vdz / dd * mtof_a));
  //x > -xRange && x < xRange && y > -yRange && y < yRange &&
  getStatistics().getHisto<TH1D>("annihilation_point_z").Fill(z);
  if (z > -ANNIHILATION_POINT_Z && z < ANNIHILATION_POINT_Z) {
    getStatistics().getHisto<TH3D>("hits_pos").Fill(x, y, z);
    return true;
  } else {
    getStatistics().getHisto<TH1I>("number_of_hits_filtered_by_condition").Fill("Cut on annihilation point Z", 1);
  }
  return false;
}
