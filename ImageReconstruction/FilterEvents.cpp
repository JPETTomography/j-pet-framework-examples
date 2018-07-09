/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
 *  @file FilterEvents.cpp
 */

#include "FilterEvents.h"
#include <TH3D.h>
#include <TH1I.h>
#include "./JPetOptionsTools/JPetOptionsTools.h"
using namespace jpet_options_tools;

FilterEvents::FilterEvents(const char* name) : JPetUserTask(name) {}

FilterEvents::~FilterEvents() {}

bool FilterEvents::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH1I("number_of_events",
                                  "Number of events with n hits",
                                  kNumberOfHitsInEventHisto, 0, kNumberOfHitsInEventHisto));
  getStatistics().createHistogram(new TH1I("number_of_hits_filtered_by_condition",
                                  "Number of hits filtered by condition",
                                  kNumberOfConditions, 0, kNumberOfConditions));

  //it is not really nessesery, but it is creating labels in given order
  getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on Z", 1);
  getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on LOR distance", 1);
  getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on delta angle", 1);
  getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on first hit TOT", 1);
  getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on second hit TOT", 1);
  getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on annihilation point Z", 1);

  return true;
}

bool FilterEvents::exec()
{
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      auto numberOfHits = event.getHits().size();
      if (numberOfHits <= 1)
        continue;
      else {
        auto hits = event.getHits();
        for (unsigned int i = 0; i < hits.size() - 1; i++) {
          if (!checkConditions(hits[i], hits[i + 1]))
            continue;
          fOutputEvents->add<JPetEvent>(event);
        }
      }
    }
  } else {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool FilterEvents::terminate()
{
  return true;
}

bool FilterEvents::checkConditions(const JPetHit& first, const JPetHit& second)
{
  if (!cutOnZ(first, second)) {
    getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on Z", 1);
    return false;
  }
  if (!cutOnLORDistanceFromCenter(first, second)) {
    getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on LOR distance", 1);
    return false;
  }
  if (angleDelta(first, second) < fAngleDeltaMinValue) {
    getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on delta angle", 1);
    return false;
  }

  double totOfFirstHit = calculateSumOfTOTsOfHit(first);
  if (totOfFirstHit < fTOTMinValueInNs || totOfFirstHit > fTOTMaxValueInNs) {
    getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on first hit TOT", 1);
    return false;
  }

  double totOfSecondHit = calculateSumOfTOTsOfHit(second);
  if (totOfSecondHit < fTOTMinValueInNs || totOfSecondHit > fTOTMaxValueInNs) {
    getStatistics().getObject<TH1I>("number_of_hits_filtered_by_condition")->Fill("Cut on second hit TOT", 1);
    return false;
  }

  return true;
}

bool FilterEvents::cutOnZ(const JPetHit& first, const JPetHit& second)
{
  return (std::fabs(first.getPosZ()) < fCutOnZValue) && (fabs(second.getPosZ()) < fCutOnZValue);
}

bool FilterEvents::cutOnLORDistanceFromCenter(const JPetHit& first, const JPetHit& second)
{
  double x_a = first.getPosX();
  double x_b = second.getPosX();

  double y_a = first.getPosY();
  double y_b = second.getPosY();

  double a = (y_a - y_b) / (x_a - x_b);
  double c = y_a - ((y_a - y_b) / (x_a - x_b)) * x_a;
  return (std::fabs(c) / std::sqrt(a * a + 1)) < fCutOnLORDistanceFromCenter; //b is 1 and b*b is 1
}

float FilterEvents::angleDelta(const JPetHit& first, const JPetHit& second)
{
  float delta = fabs(first.getBarrelSlot().getTheta() - second.getBarrelSlot().getTheta());
  return std::min(delta, (float)360 - delta);
}

double FilterEvents::calculateSumOfTOTsOfHit(const JPetHit& hit)
{
  return calculateSumOfTOTs(hit.getSignalA()) + calculateSumOfTOTs(hit.getSignalB());
}

double FilterEvents::calculateSumOfTOTs(const JPetPhysSignal& signal)
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

void FilterEvents::setUpOptions()
{
  auto opts = getOptions();

  if (isOptionSet(opts, kCutOnZValueKey)) {
    fCutOnZValue = getOptionAsFloat(opts, kCutOnZValueKey);
  }
  if (isOptionSet(opts, kCutOnLORDistanceKey)) {
    fCutOnLORDistanceFromCenter = getOptionAsFloat(opts, kCutOnLORDistanceKey);
  }
  if (isOptionSet(opts, kCutOnTOTMinValueKey)) {
    fTOTMinValueInNs = getOptionAsFloat(opts, kCutOnTOTMinValueKey);
  }
  if (isOptionSet(opts, kCutOnTOTMaxValueKey)) {
    fTOTMaxValueInNs = getOptionAsFloat(opts, kCutOnTOTMaxValueKey);
  }
  if (isOptionSet(opts, kCutOnAngleDeltaMinValueKey)) {
    fAngleDeltaMinValue = getOptionAsFloat(opts, kCutOnAngleDeltaMinValueKey);
  }
}
