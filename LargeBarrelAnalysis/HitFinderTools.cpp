/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file HitFinderTools.cpp
 */

#include "HitFinderTools.h"
#include "UniversalFileLoader.h"
#include <TMath.h>
#include <cmath>
#include <map>
#include <vector>

using namespace tot_energy_converter;
using namespace std;

/**
 * Helper method for sotring signals in vector
 */
void HitFinderTools::sortByTime(vector<JPetPhysSignal>& sigVec)
{
  sort(sigVec.begin(), sigVec.end(), [](const JPetPhysSignal& sig1, const JPetPhysSignal& sig2) { return sig1.getTime() < sig2.getTime(); });
}

/**
 * Method distributing Signals according to Scintillator they belong to
 */
map<int, vector<JPetPhysSignal>> HitFinderTools::getSignalsBySlot(const JPetTimeWindow* timeWindow, bool useCorrupts)
{
  map<int, vector<JPetPhysSignal>> signalSlotMap;
  if (!timeWindow)
  {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return signalSlotMap;
  }
  const unsigned int nSignals = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSignals; i++)
  {
    auto physSig = dynamic_cast<const JPetPhysSignal&>(timeWindow->operator[](i));
    if (!useCorrupts && physSig.getRecoFlag() == JPetBaseSignal::Corrupted)
    {
      continue;
    }
    int slotID = physSig.getBarrelSlot().getID();
    auto search = signalSlotMap.find(slotID);
    if (search == signalSlotMap.end())
    {
      vector<JPetPhysSignal> tmp;
      tmp.push_back(physSig);
      signalSlotMap.insert(pair<int, vector<JPetPhysSignal>>(slotID, tmp));
    }
    else
    {
      search->second.push_back(physSig);
    }
  }
  return signalSlotMap;
}

/**
 * Loop over all Scins invoking matching procedure
 */
vector<JPetHit> HitFinderTools::matchAllSignals(map<int, vector<JPetPhysSignal>>& allSignals, const map<unsigned int, vector<double>>& velocitiesMap,
                                                double timeDiffAB, int refDetScinId, bool convertToT, const ToTEnergyConverter& totConverter,
                                                JPetStatistics& stats, bool saveHistos)
{
  vector<JPetHit> allHits;
  for (auto& slotSigals : allSignals)
  {
    // Loop for Reference Detector ID
    if (slotSigals.first == refDetScinId)
    {
      for (auto refSignal : slotSigals.second)
      {
        auto refHit = createDummyRefDetHit(refSignal);
        allHits.push_back(refHit);
      }
      continue;
    }
    // Loop for other slots than reference one
    auto slotHits = matchSignals(slotSigals.second, velocitiesMap, timeDiffAB, convertToT, totConverter, stats, saveHistos);
    allHits.insert(allHits.end(), slotHits.begin(), slotHits.end());
  }
  return allHits;
}

/**
 * Method matching signals on the same Scintillator
 */
vector<JPetHit> HitFinderTools::matchSignals(vector<JPetPhysSignal>& slotSignals, const map<unsigned int, vector<double>>& velocitiesMap,
                                             double timeDiffAB, bool convertToT, const ToTEnergyConverter& totConverter, JPetStatistics& stats,
                                             bool saveHistos)
{
  vector<JPetHit> slotHits;
  vector<JPetPhysSignal> remainSignals;
  sortByTime(slotSignals);
  while (slotSignals.size() > 0)
  {
    auto physSig = slotSignals.at(0);
    if (slotSignals.size() == 1)
    {
      remainSignals.push_back(physSig);
      break;
    }
    for (unsigned int j = 1; j < slotSignals.size(); j++)
    {
      if (slotSignals.at(j).getTime() - physSig.getTime() < timeDiffAB)
      {
        if (physSig.getPM().getSide() != slotSignals.at(j).getPM().getSide())
        {
          auto hit = createHit(physSig, slotSignals.at(j), velocitiesMap, convertToT, totConverter, stats, saveHistos);
          slotHits.push_back(hit);
          slotSignals.erase(slotSignals.begin() + j);
          slotSignals.erase(slotSignals.begin() + 0);
          break;
        }
        else
        {
          if (j == slotSignals.size() - 1)
          {
            remainSignals.push_back(physSig);
            slotSignals.erase(slotSignals.begin() + 0);
            break;
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        if (saveHistos && physSig.getPM().getSide() != slotSignals.at(j).getPM().getSide())
        {
          stats.fillHistogram("remain_signals_tdiff", slotSignals.at(j).getTime() - physSig.getTime());
        }
        remainSignals.push_back(physSig);
        slotSignals.erase(slotSignals.begin() + 0);
        break;
      }
    }
  }
  if (remainSignals.size() > 0 && saveHistos)
  {
    stats.fillHistogram("remain_signals_per_scin", (float)(remainSignals.at(0).getPM().getScin().getID()), remainSignals.size());
  }
  return slotHits;
}

/**
 * Method for Hit creation - setting all fields, that make sense here
 */
JPetHit HitFinderTools::createHit(const JPetPhysSignal& signal1, const JPetPhysSignal& signal2,
                                  const map<unsigned int, vector<double>>& velocitiesMap, bool convertToT, const ToTEnergyConverter& totConverter,
                                  JPetStatistics& stats, bool saveHistos)
{
  JPetPhysSignal signalA;
  JPetPhysSignal signalB;
  if (signal1.getPM().getSide() == JPetPM::SideA)
  {
    signalA = signal1;
    signalB = signal2;
  }
  else
  {
    signalA = signal2;
    signalB = signal1;
  }
  auto radius = signalA.getPM().getBarrelSlot().getLayer().getRadius();
  auto theta = TMath::DegToRad() * signalA.getPM().getBarrelSlot().getTheta();
  auto velocity = UniversalFileLoader::getConfigurationParameter(velocitiesMap, getProperChannel(signalA));
  checkTheta(theta);

  JPetHit hit;
  hit.setSignalA(signalA);
  hit.setSignalB(signalB);
  hit.setTime((signalA.getTime() + signalB.getTime()) / 2.0);
  hit.setQualityOfTime(-1.0);
  hit.setTimeDiff(signalB.getTime() - signalA.getTime());
  hit.setQualityOfTimeDiff(-1.0);
  hit.setScintillator(signalA.getPM().getScin());
  hit.setBarrelSlot(signalA.getPM().getBarrelSlot());
  hit.setPosX(radius * cos(theta));
  hit.setPosY(radius * sin(theta));
  hit.setPosZ(velocity * hit.getTimeDiff() / 2000.0);

  if (convertToT)
  {
    auto tot = calculateTOT(hit);
    /// Checking if provided conversion function accepts calculated value of ToT
    if (tot > totConverter.getRange().first && tot < totConverter.getRange().second)
    {
      auto energy = totConverter(tot);
      if (!isnan(energy))
      {
        hit.setEnergy(energy);
        stats.fillHistogram("conv_tot_range", tot);
        stats.fillHistogram("conv_dep_energy", energy);
        stats.fillHistogram("conv_dep_energy_vs_tot", energy, tot);
      }
      else
      {
        hit.setEnergy(-1.0);
      }
    }
    else
    {
      hit.setEnergy(-1.0);
    }
  }
  else
  {
    hit.setEnergy(-1.0);
  }
  hit.setQualityOfEnergy(-1.0);

  if (signalA.getRecoFlag() == JPetBaseSignal::Good && signalB.getRecoFlag() == JPetBaseSignal::Good)
  {
    hit.setRecoFlag(JPetHit::Good);
    if (saveHistos)
    {
      stats.fillHistogram("good_vs_bad_hits", 1);
      stats.fillHistogram("time_diff_per_scin", hit.getTimeDiff(), (float)(hit.getScintillator().getID()));
      stats.fillHistogram("hit_pos_per_scin", hit.getPosZ(), (float)(hit.getScintillator().getID()));
    }
  }
  else if (signalA.getRecoFlag() == JPetBaseSignal::Corrupted || signalB.getRecoFlag() == JPetBaseSignal::Corrupted)
  {
    hit.setRecoFlag(JPetHit::Corrupted);
    if (saveHistos)
    {
      stats.fillHistogram("good_vs_bad_hits", 2);
    }
  }
  else
  {
    hit.setRecoFlag(JPetHit::Unknown);
    if (saveHistos)
    {
      stats.fillHistogram("good_vs_bad_hits", 3);
    }
  }
  return hit;
}

/**
 * Method for Hit creation in case of reference detector.
 * Setting only some necessary fields.
 */
JPetHit HitFinderTools::createDummyRefDetHit(const JPetPhysSignal& signalB)
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
  return hit;
}

/**
 * Helper method for getting TOMB channel
 */
int HitFinderTools::getProperChannel(const JPetPhysSignal& signal)
{
  auto someSigCh = signal.getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue)[0];
  return someSigCh.getTOMBChannel().getChannel();
}

/**
 * Helper method for checking if theta is in radians
 */
void HitFinderTools::checkTheta(const double& theta)
{
  if (theta > 2 * TMath::Pi())
  {
    WARNING("Probably wrong values of Barrel Slot theta - conversion to radians failed. Check please.");
  }
}

/**
 * Calculation of the total TOT of the hit - Time over Threshold:
 * the weighted sum of the TOTs on all of the thresholds (1-4) and on the both sides (A,B)
 */

HitFinderTools::TOTCalculationType HitFinderTools::getTOTCalculationType(const std::string& type)
{
  if (type == "rectangular")
  {
    return TOTCalculationType::kThresholdRectangular;
  }
  else if (type == "trapeze")
  {
    return TOTCalculationType::kThresholdTrapeze;
  }
  else if (type == "standard")
  {
    return TOTCalculationType::kSimplified;
  }
  else
  {
    WARNING("Undefinied type for the calculation of the TOTs. Probably missing option in userParams, typo, or mistake.");
    return TOTCalculationType::kSimplified;
  }
}

double HitFinderTools::calculateTOT(const JPetHit& hit, TOTCalculationType type)
{
  double tot = 0.0;

  std::map<int, double> thrToTOT_sideA = hit.getSignalA().getRecoSignal().getRawSignal().getTOTsVsThresholdValue();
  std::map<int, double> thrToTOT_sideB = hit.getSignalB().getRecoSignal().getRawSignal().getTOTsVsThresholdValue();

  tot += calculateTOTside(thrToTOT_sideA, type);
  tot += calculateTOTside(thrToTOT_sideB, type);
  return tot;
}

double HitFinderTools::calculateTOTside(const std::map<int, double>& thrToTOT_side, TOTCalculationType type)
{
  double tot = 0., weight = 1.;
  if (!thrToTOT_side.empty())
  {
    double firstThr = thrToTOT_side.begin()->first;
    tot += weight * thrToTOT_side.begin()->second;
    if (thrToTOT_side.size() > 1)
    {
      for (auto it = std::next(thrToTOT_side.begin(), 1); it != thrToTOT_side.end(); it++)
      {
        switch (type)
        {
        case TOTCalculationType::kSimplified:
          weight = 1.;
          break;
        case TOTCalculationType::kThresholdRectangular:
          weight = (it->first - std::prev(it, 1)->first) / firstThr;
          break;
        case TOTCalculationType::kThresholdTrapeze:
          weight = (it->first - std::prev(it, 1)->first) / firstThr;
          tot += weight * fabs(std::prev(it, 1)->second - it->second) / 2; // fabs, because we do not know which TOT is lower
          break;
        }
        tot += weight * it->second;
      }
    }
  }
  else
    return 0;
  return tot;
}
void HitFinderTools::saveTOTsync(std::vector<JPetHit>& hits, std::string TOTCalculationType, const boost::property_tree::ptree& syncTree)
{
  for (auto& hit : hits)
  {
    auto tot = HitFinderTools::calculateTOT(hit, HitFinderTools::getTOTCalculationType(TOTCalculationType));
    tot = HitFinderTools::syncTOT(hit, tot, syncTree);
    hit.setEnergy(tot);
  }
}
double HitFinderTools::syncTOT(const JPetHit& hit, double TOT, const boost::property_tree::ptree& syncTree)
{
  double syncTOT = 0;
  double factorA = syncTree.get("scin." + to_string(hit.getScintillator().getID()) + ".tot_scaling_factor_a",
                                1.0); // default 1.f
  double factorB = syncTree.get("scin." + to_string(hit.getScintillator().getID()) + ".tot_scaling_factor_b",
                                0.0); // default 0.f

  syncTOT = TOT * factorA + factorB;
  return syncTOT;
}
