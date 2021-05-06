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
                                                                      boost::property_tree::ptree& calibTree)
{
  vector<JPetMatrixSignal> allMtxSignals;
  // Iterating over whole map
  for (auto& rawSigScin : rawSigMtxMap)
  {
    for (auto& rawSigSide : rawSigScin.second)
    {
      auto mtxSignals = mergeRawSignalsOnSide(rawSigSide, mergingTime, calibTree);
      allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
    }
  }
  return allMtxSignals;
}

/**
 * Method iterates over all Raw Sigals on some SiPMs on the same matrix,
 * matching them into groups on max. 4 as a MatrixSignal
 */
vector<JPetMatrixSignal> SignalTransformerTools::mergeRawSignalsOnSide(vector<JPetRawSignal>& rawSigVec, double mergingTime,
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

    mtxSig.setTime(calculateAverageTime(mtxSig, calibTree));
    mtxSig.setTOT(calculateAverageTOT(mtxSig));
    rawSigVec.erase(rawSigVec.begin());
    mtxSigVec.push_back(mtxSig);
  }
  return mtxSigVec;
}

/**
 * Calculating time of Matrix Signal as an average of all leading edge times contained in all RawSignals
 */
double SignalTransformerTools::calculateAverageTime(JPetMatrixSignal& mtxSig, boost::property_tree::ptree& calibTree)
{
  double averageTime = 0.0;
  auto rawSignals = mtxSig.getRawSignals();
  int multiplicity = 0;
  for (auto rawSig : rawSignals)
  {
    auto leads = rawSig.second.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
    for (auto leadSigCh : leads)
    {
      double correction = 0.0;
      if (leadSigCh.getChannel().getPM().getSide() == JPetPM::SideB)
      {
        auto thr = leadSigCh.getChannel().getThresholdNumber();
        auto mtxPos = leadSigCh.getChannel().getPM().getMatrixPosition();
        auto scinID = leadSigCh.getChannel().getPM().getScin().getID();
        auto param = Form("%s.%d.%s%d%s%d.%s", "scin", scinID, "hit_tdiff_thr", thr, "_scin_mtx_pos_", mtxPos, "b_correction");
        correction = calibTree.get(param, 0.0);
      }
      averageTime += leadSigCh.getTime() - correction;
      multiplicity++;
    }
  }
  if (multiplicity != 0)
  {
    averageTime = averageTime / ((double)multiplicity);
  }
  return averageTime;
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
