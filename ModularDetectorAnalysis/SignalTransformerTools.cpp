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
#include "JPetLoggerInclude.h"

using namespace std;

/**
 * Map PM Signals from all SiPMs according to matrix and side they belong to.
 * Returns map< scin ID, map < side < signals >>>.
 * Side A is the first element int the vector, Side B is the second one.
 */
const map<JPetMatrix::Side, map<int, vector<JPetPMSignal>>> SignalTransformerTools::getPMSigMtxMap(const JPetTimeWindow* timeWindow)
{
  map<JPetMatrix::Side, map<int, vector<JPetPMSignal>>> mappedSignals;
  map<int, vector<JPetPMSignal>> tmpMapA;
  map<int, vector<JPetPMSignal>> tmpMapB;

  mappedSignals[JPetMatrix::SideA] = tmpMapA;
  mappedSignals[JPetMatrix::SideB] = tmpMapB;

  if (!timeWindow)
  {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return mappedSignals;
  }

  const unsigned int nPMSigs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nPMSigs; i++)
  {
    auto pmSig = dynamic_cast<const JPetPMSignal&>(timeWindow->operator[](i));

    auto scinID = pmSig.getPM().getMatrix().getScin().getID();
    auto side = pmSig.getPM().getMatrix().getSide();
    auto pmMtxPos = pmSig.getPM().getMatrixPosition();

    if (side != JPetMatrix::SideA || side != JPetMatrix::SideB)
    {
      continue;
    }

    auto search = mappedSignals.at(side).find(scinID);
    if (search == mappedSignals.at(side).end())
    {
      vector<JPetPMSignal> tmpSigVec;
      tmpSigVec.push_back(pmSig);
      mappedSignals.at(side)[scinID] = tmpSigVec;
    }
    else
    {
      search->second.push_back(pmSig);
    }
  }
  return mappedSignals;
}

/**
 * Method iterates over all matrices in the detector with signals,
 * calling merging procedure for each
 */
vector<JPetMatrixSignal> SignalTransformerTools::mergeSignalsAllSiPMs(map<JPetMatrix::Side, map<int, vector<JPetPMSignal>>>& mappedSignals,
                                                                      double mergingTime, boost::property_tree::ptree& calibTree)
{
  vector<JPetMatrixSignal> allMtxSignals;
  for (auto& sideSigs : mappedSignals)
  {
    for (auto& signals : sideSigs.second)
    {
      auto mtxSignals = mergePMSignalsOnSide(signals.second, mergingTime, calibTree);
      allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
    }
  }
  return allMtxSignals;
}

/**
 * Method iterates over all PM Sigals on some SiPMs on the same matrix,
 * matching them into groups on max. 4 as a MatrixSignal
 */
vector<JPetMatrixSignal> SignalTransformerTools::mergePMSignalsOnSide(vector<JPetPMSignal>& pmSigVec, double mergingTime,
                                                                      boost::property_tree::ptree& calibTree)
{
  vector<JPetMatrixSignal> mtxSigVec;
  sortByTime(pmSigVec);

  while (pmSigVec.size() > 0)
  {
    // Create Matrix Signal and add first PM Signal by default
    JPetMatrixSignal mtxSig;
    mtxSig.setMatrix(pmSigVec.at(0).getPM().getMatrix());
    if (!mtxSig.addPMSignal(pmSigVec.at(0)))
    {
      ERROR("Problem with adding the first signal to new object.");
      break;
    }

    unsigned int nextIndex = 1;
    while (true)
    {
      if (pmSigVec.size() <= nextIndex)
      {
        // nothing left to check
        break;
      }

      // signal matching condidion
      if (fabs(pmSigVec.at(nextIndex).getTime() - pmSigVec.at(0).getTime()) < mergingTime)
      {
        // mathing signal found
        if (mtxSig.addPMSignal(pmSigVec.at(nextIndex)))
        {
          // added succesfully
          pmSigVec.erase(pmSigVec.begin() + nextIndex);
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

    // mtxSig.setToT(calculateAverageToT(mtxSig));
    mtxSig.setTime(calculateAverageTime(mtxSig, calibTree));
    pmSigVec.erase(pmSigVec.begin());
    mtxSigVec.push_back(mtxSig);
  }
  return mtxSigVec;
}

/**
 * Calculating time of Matrix Signal as an average of all leading edge times contained in all PMSignals
 */
double SignalTransformerTools::calculateAverageTime(JPetMatrixSignal& mtxSig, boost::property_tree::ptree& calibTree)
{
  double averageTime = 0.0;
  auto pmSignals = mtxSig.getPMSignals();
  int multiplicity = 0;
  for (auto pmSig : pmSignals)
  {
    averageTime += pmSig.second.getTime();
  }
  if (pmSignals.size() != 0)
  {
    averageTime = averageTime / ((double)pmSignals.size());
  }

  // Applying b-side correction for A-B time difference synchronization
  auto scinID = mtxSig.getMatrix().getScin().getID();
  double bCorrection = 0.0;
  if (mtxSig.getMatrix().getSide() == JPetMatrix::SideB)
  {
    bCorrection = calibTree.get("scin." + to_string(scinID) + ".b_correction", 0.0);
  }

  // Applying time walk correction
  double timeWalkCorrection = 0.0;
  auto tot = mtxSig.getToT();
  if (tot != 0.0)
  {
    auto p1 = calibTree.get("time_walk.param_a", 0.0);
    timeWalkCorrection = p1 / mtxSig.getToT();
  }

  // Returning average time corrected by calibration constatns, that can be zero.
  return averageTime - bCorrection - timeWalkCorrection;
}

/**
 * Sorting method for PM Signals, based on time of leading THR1 Signal Channel
 */
void SignalTransformerTools::sortByTime(vector<JPetPMSignal>& input)
{
  std::sort(input.begin(), input.end(), [](JPetPMSignal pmSig1, JPetPMSignal pmSig2) { return pmSig1.getTime() < pmSig2.getTime(); });
}
