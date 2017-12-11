/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file HitFinderTools.h
 */

#include "HitFinderTools.h"
#include <cmath> /// std::sin(), std::cos()
#include <TMath.h> /// DegToRad()

HitFinderTools::HitFinderTools(const JPetStatistics& statistics) : fStats(statistics)
{
  fStats.createHistogram(
    new TH2F("time_diff_per_scin",
             "Signals Time Difference per Scintillator ID",
             200, -20000.0, 20000.0,
             192, 1.0, 193.0));

  fStats.createHistogram(
    new TH2F("hit_pos_per_scin",
             "Hit Position per Scintillator ID",
             200, -150.0, 150.0,
             192, 1.0, 193.0));
}

JPetHit HitFinderTools::createDummyRefDefHit(const JPetPhysSignal& signalB,
    const VelocityMap& velMap)
{
  JPetHit hit;
  hit.setSignalB(signalB);
  hit.setTime(signalB.getTime());
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(0.0);
  hit.setQualityOfTimeDiff(-1.0);
  hit.setEnergy(-1.0);
  hit.setQualityOfEnergy(-1.0);
  hit.setScintillator(signalB.getPM().getScin());
  hit.setBarrelSlot(signalB.getPM().getBarrelSlot());
  setHitXYPosition(hit);

  auto search = velMap.find(hit.getBarrelSlot().getID());
  if (search != velMap.end()) hit.setPosZ(-1000000.0);

  return hit;
}

void HitFinderTools::addIfReferenceSignal(
  std::vector<JPetHit>& hits,
  const std::vector<JPetPhysSignal>& sideA,
  const std::vector<JPetPhysSignal>& sideB,
  const VelocityMap& velMap)
{
  if (sideA.size() == 0 && sideB.size() > 0) {
    auto scinID = sideB.at(0).getPM().getScin().getID();
    auto layerID = sideB.at(0).getBarrelSlot().getLayer().getID();
    if (scinID == 193 && layerID == 4) {
      for (auto signalB : sideB) {
        hits.push_back(createDummyRefDefHit(signalB, velMap));
      }
    }
  }
}

void HitFinderTools::sortByTime(std::vector<JPetPhysSignal>& side)
{
  std::sort(side.begin(),
            side.end(),
            [](const JPetPhysSignal & h1,
  const JPetPhysSignal & h2) {
    return h1.getTime() < h2.getTime();
  });
}

void HitFinderTools::setHitZPosition(JPetHit& hit, const VelocityMap& velMap)
{
  auto search = velMap.find(hit.getBarrelSlot().getID());
  if (search != velMap.end()) {
    double vel = search->second.first;
    double position = vel * hit.getTimeDiff() / 2000.;
    hit.setPosZ(position);
  } else {
    hit.setPosZ(-1000000.0);
  }
}

void HitFinderTools::setHitXYPosition(JPetHit& hit)
{
  auto radius = hit.getBarrelSlot().getLayer().getRadius();
  auto theta = TMath::DegToRad() * hit.getBarrelSlot().getTheta();
  hit.setPosX(radius * std::cos(theta));
  hit.setPosY(radius * std::sin(theta));
}

JPetHit HitFinderTools::createHit(const JPetPhysSignal& signalA,
                                  const JPetPhysSignal& signalB,
                                  const VelocityMap& velMap)
{
  JPetHit hit;
  hit.setSignalA(signalA);
  hit.setSignalB(signalB);
  hit.setTime((signalA.getTime() + signalB.getTime()) / 2.0);
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(signalB.getTime() - signalA.getTime());
  hit.setQualityOfTimeDiff(-1.0);
  hit.setEnergy(-1.0);
  hit.setQualityOfEnergy(-1.0);
  hit.setScintillator(signalA.getPM().getScin());
  hit.setBarrelSlot(signalA.getPM().getBarrelSlot());

  setHitXYPosition(hit);
  setHitZPosition(hit, velMap);

  return hit;
}

std::vector<JPetHit> HitFinderTools::createHits(
  const SignalsContainer& allSignalsInTimeWindow,
  const double timeDifferenceWindow,
  const VelocityMap& velMap)
{
  std::vector<JPetHit> hits;

  for (auto scintillator : allSignalsInTimeWindow) {

    auto sideA = scintillator.second.first;
    auto sideB = scintillator.second.second;

    // Handling the special case of reference detector signals,
    // which are defined as coming only from sideB,
    // scintillator no. 193 in layer no. 4
    // and do not correspond to a full hit
    addIfReferenceSignal(hits, sideA, sideB, velMap);

    if (sideA.size() > 0 && sideB.size() > 0) {

      sortByTime(sideA);
      sortByTime(sideB);

      for (auto signalA : sideA) {
        for (auto signalB : sideB) {

          if ((signalB.getTime() - signalA.getTime())
              > timeDifferenceWindow)
            break;

          if (fabs(signalA.getTime() - signalB.getTime())
              < timeDifferenceWindow) {

            //Creating hit for successfully matched pair of Phys singlas
            //Setting meaningless parameters of Energy, Position, quality
            JPetHit hit = createHit(signalA, signalB, velMap);
            hits.push_back(hit);

            fStats.getHisto2D("time_diff_per_scin")
            .Fill(hit.getTimeDiff(),
                  (float) (hit.getScintillator().getID()));

            fStats.getHisto2D("hit_pos_per_scin")
            .Fill(hit.getPosZ(),
                  (float) (hit.getScintillator().getID()));
          }
        }
      }
    }
  }
  if (!checkIsDegreeOrRad(hits))
    WARNING("ALL barrel slots have theta < then 7, check is they have correct degree theta");
  return hits;
}

bool HitFinderTools::checkIsDegreeOrRad(const std::vector<JPetHit>& hits)
{
  bool isDegree = false;
  for (const JPetHit& hit : hits) {
    if (hit.getBarrelSlot().getTheta() > 7.) {
      isDegree = true;
      break;
    }
  }
  return isDegree;
}