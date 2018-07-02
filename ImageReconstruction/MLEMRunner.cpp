/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file MLEMRunner.cpp
 */

#include "MLEMRunner.h"
#include "JPetHit/JPetHit.h"
#include <iomanip> //std::setprecision
#include "JPetParamBank/JPetParamBank.h"
#include "JPetGeomMapping/JPetGeomMapping.h"

using namespace jpet_options_tools;

using Point = PET2D::Point<float>;

MLEMRunner::MLEMRunner(const char* name) : JPetUserTask(name) {}

MLEMRunner::~MLEMRunner() {}

bool MLEMRunner::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  return true;
}

bool MLEMRunner::exec()
{
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    const unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      const auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      parseEvent(event);
    }
  }
  return true;
}

bool MLEMRunner::terminate()
{
  fOutputStream.close();
  return true;
}

bool MLEMRunner::parseEvent(const JPetEvent& event)
{
  const auto hits = event.getHits();
  if (hits.size() != 2) {
    return false;
  }
  float x1 = hits[0].getPosX();
  float y1 = hits[0].getPosY();
  float z1 = hits[0].getPosZ();
  float t1 = hits[0].getTime();

  float x2 = hits[1].getPosX();
  float y2 = hits[1].getPosY();
  float z2 = hits[1].getPosZ();
  float t2 = hits[1].getTime();

  fOutputStreamTest << std::setprecision(30) << x1 << "\t" << y1 << "\t" << z1 << "\t" << t1 << "\t"
                    << x2 << "\t" << y2 << "\t" << z2 << "\t" << t2 << std::endl;

  if (std::abs(z1 * cm) > fHalfStripLenght || std::abs(z2 * cm) > fHalfStripLenght) {
    return false;
  }

  int d1 = -1, d2 = -1;
  for (size_t i = 0; i < fScanner.size(); ++i) {
    if (fScanner[i].contains(Point(x1 * cm, y1 * cm), EPSILON))
      d1 = i;
    if (fScanner[i].contains(Point(x2 * cm, y2 * cm), EPSILON))
      d2 = i;
  }

  if (d1 < 0 || d2 < 0) {
    return false;
  }

  if (d1 < d2) {
    std::swap(d1, d2);
    std::swap(z1, z2);
    std::swap(t1, t2);
  }
  double dl = (t1 - t2) * speed_of_light_m_per_ps;
  fOutputStream << d1 << " " << d2 << " " << z1* cm << " "
                << z2* cm << " " << dl
                << "\n";
  return true;
}

void MLEMRunner::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey)) {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  fOutputStream.open(fOutFileName, std::ofstream::out);
  fOutputStreamTest.open(fOutFileNameTest, std::ofstream::out);
  std::vector<float> radius;
  radius.reserve(3);
  std::vector<int> scintillators;
  scintillators.reserve(3);

  const std::vector<float> rotation{0.f, 0.5f, 0.5f}; // rotation for BigBarrel

  const JPetParamBank& bank = getParamBank();
  const JPetGeomMapping mapping(bank);
  for (unsigned int i = 1; i < mapping.getLayersCount(); i++) {
    radius.push_back(mapping.getRadiusOfLayer(i) * cm);
    scintillators.push_back(static_cast<int>(mapping.getSlotsCount(i)));
  }

  const auto scin = bank.getScintillator(1);

  const float detectorWidth = scin.getScinSize(JPetScin::Dimension::kWidth);
  const float detectorHeight = scin.getScinSize(JPetScin::Dimension::kHeight);
  fHalfStripLenght = scin.getScinSize(JPetScin::Dimension::kLength) / 2.f;
  const float detectorD = 0.f;
  const float fowRadius = 0.4f;

  fScanner = ScannerBuilder<SquareScanner>::build_multiple_rings(__PET2D_BARREL(
               radius,         //radius
               rotation,       //rotation
               scintillators,  //n-detectors
               detectorWidth,  //w-detector
               detectorHeight, //h-detector
               detectorD,      //should be d-detector
               fowRadius       //fow radius
             ));
}
