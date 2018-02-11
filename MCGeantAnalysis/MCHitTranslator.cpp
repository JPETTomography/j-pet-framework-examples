/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file MCHitTranslator.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "MCHitTranslator.h"

using namespace jpet_options_tools;

MCHitTranslator::MCHitTranslator(const char* name) : JPetUserTask(name) { }

MCHitTranslator::~MCHitTranslator() { }

bool MCHitTranslator::init()
{
//  fOutputEvents = new JPetTimeWindow("JPetHit");
//  auto opts = getOptions();
//
//  if (isOptionSet(opts, fVelocityCalibFileParamKey))
//    fVelocityCalibrationFilePath = getOptionAsString(opts, fVelocityCalibFileParamKey);
//
//  INFO("Reading velocities.");
//  fVelocityMap = readVelocityFile();
//
//  getStatistics().createHistogram(
//    new TH1F("hits_per_time_window",
//             "Number of Hits in Time Window",
//             101, -0.5, 100.5
//            )
//  );
//
//  getStatistics().createHistogram(
//    new TH2F("time_diff_per_scin",
//             "Signals Time Difference per Scintillator ID",
//             200, -20000.0, 20000.0,
//             192, 1.0, 193.0));
//
//  getStatistics().createHistogram(
//    new TH2F("hit_pos_per_scin",
//             "Hit Position per Scintillator ID",
//             200, -150.0, 150.0,
//             192, 1.0, 193.0));
//
//  if (isOptionSet(opts, fTimeWindowWidthParamKey)) {
//    kTimeWindowWidth = getOptionAsFloat(opts, fTimeWindowWidthParamKey);
//  }
//
//  INFO("Hit finding started.");

  return true;
}

bool MCHitTranslator::exec()
{
//  if (auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
//    uint n = timeWindow->getNumberOfEvents();
//    for (uint i = 0; i < n; ++i) {
//      fillSignalsMap(dynamic_cast<const JPetPhysSignal&>(timeWindow->operator[](i)));
//    }
//
//    std::vector<JPetHit> hits = fHitTools.createHits(
//                                  getStatistics(),
//                                  fAllSignalsInTimeWindow,
//                                  kTimeWindowWidth,
//                                  fVelocityMap);
//    saveHits(hits);
//    getStatistics().getHisto1D("hits_per_time_window")->Fill(hits.size());
//    fAllSignalsInTimeWindow.clear();
//  } else {
//    return false;
//  }
  return true;
}



bool MCHitTranslator::terminate()
{
  INFO("Hit finding ended.");
  return true;
}

//
//void MCHitTranslator::saveHits(const std::vector<JPetHit>& hits)
//{
//  auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(hits);
//
//  for (const auto& hit : sortedHits) {
//    fOutputEvents->add<JPetHit>(hit);
//  }
//}
//
//void MCHitTranslator::fillSignalsMap(const JPetPhysSignal& signal)
//{
//  auto scinId = signal.getPM().getScin().getID();
//  switch (signal.getPM().getSide()) {
//  case JPetPM::SideA:
//    fAllSignalsInTimeWindow[scinId].first.push_back(signal); //if element in map do not exists, map is creating new element with default constructor
//    break;
//  case JPetPM::SideB:
//    fAllSignalsInTimeWindow[scinId].second.push_back(signal);
//    break;
//  default:
//    ERROR("Unknow side in signal, should never happend");
//    break;
//  }
//}
//
//MCHitTranslatorTools::VelocityMap MCHitTranslator::readVelocityFile()
//{
//  MCHitTranslatorTools::VelocityMap velocitiesMap;
//  std::ifstream input;
//  input.open(fVelocityCalibrationFilePath);
//  if (input.is_open())
//    INFO("Velocity file " + fVelocityCalibrationFilePath + " opened correctly, reading data..");
//  else {
//    INFO("Error opening velocity file " + fVelocityCalibrationFilePath);
//    return velocitiesMap;
//  }
//
//  int slot = 0;
//  double vel = 0.0, error = 0.0;
//
//  while (!input.eof()) {
//    input >> slot >> vel >> error;
//    velocitiesMap[slot] = std::make_pair(vel, error);
//  }
//
//  return velocitiesMap;
//}
