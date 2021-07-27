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
 *  @file SignalTransformerTools.cpp
 */

#include "SignalTransformerTools.h"

using namespace std;

/**
 * Map Raw Signals from all SiPMs according to matrix and side they belong to.
 * Returns map< scin ID, side < signals >>.
 * Side A is the first element int he vector, Side B is the second one.
 */
const map<int, vector<vector<JPetRawSignal>>> SignalTransformerTools::getRawSigMtxMap(const JPetTimeWindow* timeWindow, int selectMatrixPos)
{
  map<int, vector<vector<JPetRawSignal>>> rawSigMtxMap;

  if (!timeWindow)
  {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return rawSigMtxMap;
  }

  const unsigned int nRawSigs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nRawSigs; i++)
  {
    auto rawSig = dynamic_cast<const JPetRawSignal&>(timeWindow->operator[](i));

    auto leads = rawSig.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue);
    auto trails = rawSig.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrValue);

    // Saving only signals with lead-trail pair on threshold
    if (leads.size() != 2 && trails.size() != 2)
    {
      continue;
    }

    auto scinID = rawSig.getPM().getScin().getID();
    auto pmSide = rawSig.getPM().getSide();
    auto pmMtxPos = rawSig.getPM().getMatrixPosition();

    if (selectMatrixPos != -1 && pmMtxPos != selectMatrixPos)
    {
      continue;
    }

    auto search = rawSigMtxMap.find(scinID);
    if (search == rawSigMtxMap.end())
    {
      // There is no element with searched scin ID in this map, adding new one
      vector<JPetRawSignal> tmpVec;
      vector<vector<JPetRawSignal>> tmpVecVec;
      tmpVecVec.push_back(tmpVec);
      tmpVecVec.push_back(tmpVec);
      if (pmSide == JPetPM::SideA)
      {
        tmpVecVec.at(0).push_back(rawSig);
      }
      else if (pmSide == JPetPM::SideB)
      {
        tmpVecVec.at(1).push_back(rawSig);
      }
      rawSigMtxMap.insert(pair<int, vector<vector<JPetRawSignal>>>(scinID, tmpVecVec));
    }
    else
    {
      if (pmSide == JPetPM::SideA)
      {
        search->second.at(0).push_back(rawSig);
      }
      else if (pmSide == JPetPM::SideB)
      {
        search->second.at(1).push_back(rawSig);
      }
    }
  }
  return rawSigMtxMap;
}

/**
 * Method iterates over all matrices in the detector with signals,
 * calling merging procedure for each
 */
vector<JPetMatrixSignal> SignalTransformerTools::mergeSignalsAllSiPMs(map<int, vector<vector<JPetRawSignal>>>& rawSigMtxMap, double mergingTime,
                                                                      int thrSelect, boost::property_tree::ptree& calibTree)
{
  vector<JPetMatrixSignal> allMtxSignals;
  // Iterating over whole map
  for (auto& rawSigScin : rawSigMtxMap)
  {
    for (auto& rawSigSide : rawSigScin.second)
    {
      auto mtxSignals = mergeRawSignalsOnSide(rawSigSide, mergingTime, thrSelect, calibTree);
      allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
    }
  }
  return allMtxSignals;
}

/**
 * Method iterates over all Raw Sigals on some SiPMs on the same matrix,
 * matching them into groups on max. 4 as a MatrixSignal
 */
vector<JPetMatrixSignal> SignalTransformerTools::mergeRawSignalsOnSide(vector<JPetRawSignal>& rawSigVec, double mergingTime, int thrSelect,
                                                                       boost::property_tree::ptree& calibTree)
{
  vector<JPetMatrixSignal> mtxSigVec;
  sortByTime(rawSigVec);

  while (rawSigVec.size() > 0)
  {
    // Create Matrix Signal and add first Raw Signal by default
    JPetMatrixSignal mtxSig;
    mtxSig.setPM(rawSigVec.at(0).getPM());
    if (!mtxSig.addRawSignal(rawSigVec.at(0)))
    {
      ERROR("Problem with adding the first signal to new object.");
      break;
    }

    unsigned int nextIndex = 1;
    while (true)
    {

      if (rawSigVec.size() <= nextIndex)
      {
        // nothing left to check
        break;
      }

      // signal matching condidion
      if (fabs(rawSigVec.at(nextIndex).getTime() - rawSigVec.at(0).getTime()) < mergingTime)
      {
        // mathing signal found
        if (mtxSig.addRawSignal(rawSigVec.at(nextIndex)))
        {
          // added succesfully
          rawSigVec.erase(rawSigVec.begin() + nextIndex);
        }
        else
        {
          // this mtx pos is already occupied, check the next one
          nextIndex++;
        }
      }
      else
      {
        // next signal is too far from reference one, this MtxSig is finished
        break;
      }
    }

    mtxSig.setTOT(calculateAverageTOT(mtxSig));
    mtxSig.setTime(calculateAverageTime(mtxSig, thrSelect, calibTree));
    rawSigVec.erase(rawSigVec.begin());
    mtxSigVec.push_back(mtxSig);
  }
  return mtxSigVec;
}

/**
 * Calculating time of Matrix Signal as an average of all leading edge times contained in all RawSignals
 */
double SignalTransformerTools::calculateAverageTime(JPetMatrixSignal& mtxSig, int thrSelect, boost::property_tree::ptree& calibTree)
{
  double averageTime = 0.0;
  auto rawSignals = mtxSig.getRawSignals();
  int multiplicity = 0;
  for (auto rawSig : rawSignals)
  {
    auto leads = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    for (auto leadSigCh : leads)
    {
      if (thrSelect != -1 && leadSigCh.getChannel().getThresholdNumber() == thrSelect)
      {
        // Time walk correction per threshold

        double corrTHR = calibTree.get("time_walk.thr" + to_string(leadSigCh.getChannel().getThresholdNumber()) + ".param_a", 0.0);
        averageTime += leadSigCh.getTime() - corrTHR;
        multiplicity++;
      }
    }
  }
  if (multiplicity != 0)
  {
    averageTime = averageTime / ((double)multiplicity);
  }
  auto scinID = mtxSig.getPM().getScin().getID();

  // Applying b-side correction for A-B time difference synchronization
  double bCorrection = 0.0;
  if (mtxSig.getPM().getSide() == JPetPM::SideB)
  {
    bCorrection = calibTree.get("scin." + to_string(scinID) + ".b_correction", 0.0);
  }

  // Applying time walk correction
  double timeWalkCorrection = 0.0;
  if (mtxSig.getTOT() != 0.0)
  {
    double p0 = calibTree.get("scin." + to_string(scinID) + ".time_walk_ab_param_0", 0.0);
    double p1 = calibTree.get("scin." + to_string(scinID) + ".time_walk_ab_param_1", 0.0);

    if (p0 == 0.0 && p1 == 0.0)
    {
      p1 = calibTree.get("time_walk.param_a", 0.0);
    }
    timeWalkCorrection = p1 / mtxSig.getTOT();
  }

  return averageTime - bCorrection - timeWalkCorrection;
}

/**
 * Calculating average TOT of Matrix Signal
 */
double SignalTransformerTools::calculateAverageTOT(JPetMatrixSignal& mtxSig)
{
  double tot = 0.0;
  int multiplicity = 0;
  auto rawSignals = mtxSig.getRawSignals();
  for (auto rawSig : rawSignals)
  {
    if (rawSig.second.getTOT() > 0.0)
    {
      tot += rawSig.second.getTOT();
      multiplicity++;
    }
  }
  return tot / ((double)multiplicity);
}

/**
 * Sorting method for Raw Signals, based on time of leading THR1 Signal Channel
 */
void SignalTransformerTools::sortByTime(vector<JPetRawSignal>& input)
{
  std::sort(input.begin(), input.end(), [](JPetRawSignal rawSig1, JPetRawSignal rawSig2) { return rawSig1.getTime() < rawSig2.getTime(); });
}
