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

using namespace std;

vector<JPetHit> HitFinderTools::createHits(JPetStatistics& stats,
    const SignalsContainer& allSignalsInTimeWindow,
    const double timeDifferenceWindow,
    const std::map<int, std::vector<double>> velMap)
{
  vector<JPetHit> hits;

  for (auto scintillator : allSignalsInTimeWindow) {

    auto sideA = scintillator.second.first;
    auto sideB = scintillator.second.second;

    if (sideA.size() > 0 && sideB.size() > 0) {

      std::sort(sideA.begin(),
                sideA.end(),
                [] (const JPetPhysSignal & h1,
      const JPetPhysSignal & h2) {
        return h1.getTime() < h2.getTime();
      });

      std::sort(sideB.begin(),
                sideB.end(),
                [] (const JPetPhysSignal & h1,
      const JPetPhysSignal & h2) {
        return h1.getTime() < h2.getTime();
      });

      for (auto signalA : sideA) {
        for (auto signalB : sideB) {

          if ((signalB.getTime() - signalA.getTime())
              > timeDifferenceWindow)
            break;

          if (fabs(signalA.getTime() - signalB.getTime())
              < timeDifferenceWindow) {

            //Creating hit for successfully matched pair of Phys singlas
            //Setting meaningless parameters of Energy, Position, quality
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
            auto radius = hit.getBarrelSlot().getLayer().getRadius();
            auto theta = TMath::DegToRad() * hit.getBarrelSlot().getTheta();
            hit.setPosX(radius * std::cos(theta));
            hit.setPosY(radius * std::sin(theta));
            auto search = velMap.find(hit.getBarrelSlot().getID());
            if (search != velMap.end()) {
              double vel = search->second.at(0);
              double position = vel * hit.getTimeDiff() / 2000;
              hit.setPosZ(position);
            } else {
              hit.setPosZ(-1000000.0);
            }

            hits.push_back(hit);

            stats.getHisto2D("time_diff_per_scin")
            .Fill(hit.getTimeDiff(),
                  (float) (hit.getScintillator().getID()));

            stats.getHisto2D("hit_pos_per_scin")
            .Fill(hit.getPosZ(),
                  (float) (hit.getScintillator().getID()));
          }
        }
      }
    }
  }
  return hits;
}
