/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RedModuleSignalTransformerTools.cpp
 */

#include "RedModuleSignalTransformerTools.h"
#include "../ModularDetectorAnalysis/SignalTransformerTools.h"
#include "JPetLoggerInclude.h"

using namespace std;

/**
 * Map PM Signals from all SiPMs according to matrix and side they belong to.
 * Returns map< scin ID, map < side < signals >>>.
 * Side A is the first element int the vector, Side B is the second one.
 */
const map<JPetMatrix::Side, map<int, vector<JPetPMSignal>>> RedModuleSignalTransformerTools::getPMSigMtxMap(const JPetTimeWindow* timeWindow)
{
  map<JPetMatrix::Side, map<int, vector<JPetPMSignal>>> mappedSignals;
  map<int, vector<JPetPMSignal>> tmpMapA;
  map<int, vector<JPetPMSignal>> tmpMapB;
  map<int, vector<JPetPMSignal>> tmpMapWLS;

  if (!timeWindow)
  {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return mappedSignals;
  }

  mappedSignals[JPetMatrix::SideA] = tmpMapA;
  mappedSignals[JPetMatrix::SideB] = tmpMapB;
  mappedSignals[JPetMatrix::WLS] = tmpMapWLS;

  const unsigned int nPMSigs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nPMSigs; i++)
  {
    auto pmSig = dynamic_cast<const JPetPMSignal&>(timeWindow->operator[](i));

    auto scinID = pmSig.getPM().getMatrix().getScin().getID();
    auto side = pmSig.getPM().getMatrix().getSide();

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
vector<JPetMatrixSignal> RedModuleSignalTransformerTools::mergeSignalsAllSiPMs(map<JPetMatrix::Side, map<int, vector<JPetPMSignal>>>& mappedSignals,
                                                                               double mergingTime, boost::property_tree::ptree& calibTree,
                                                                               boost::property_tree::ptree& wlsConfig)
{
  vector<JPetMatrixSignal> allMtxSignals;
  for (auto& signals : mappedSignals[JPetMatrix::SideA])
  {
    auto mtxSignals = SignalTransformerTools::mergePMSignalsOnSide(signals.second, mergingTime, calibTree);
    allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
  }
  for (auto& signals : mappedSignals[JPetMatrix::SideB])
  {
    auto mtxSignals = SignalTransformerTools::mergePMSignalsOnSide(signals.second, mergingTime, calibTree);
    allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
  }
  for (auto& signals : mappedSignals[JPetMatrix::WLS])
  {
    // Method for WLS
    auto mtxSignals = mergePMSignalsOnWLS(signals.second, mergingTime, calibTree, wlsConfig);
    allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
  }
  return allMtxSignals;
}

vector<JPetMatrixSignal> RedModuleSignalTransformerTools::mergePMSignalsOnWLS(vector<JPetPMSignal>& pmSigVec, double mergingTime,
                                                                              boost::property_tree::ptree& calibTree,
                                                                              boost::property_tree::ptree& wlsConfig)
{
  vector<JPetMatrixSignal> mtxSigVec;
  for (auto pmSig : pmSigVec)
  {
    JPetMatrixSignal mtxSig;
    mtxSig.setMatrix(pmSigVec.at(0).getPM().getMatrix());
    if (!mtxSig.addPMSignal(pmSigVec.at(0)))
    {
      ERROR("Problem with adding the first signal to new object.");
      break;
    }
    mtxSigVec.push_back(mtxSig);
  }

  return mtxSigVec;
}
