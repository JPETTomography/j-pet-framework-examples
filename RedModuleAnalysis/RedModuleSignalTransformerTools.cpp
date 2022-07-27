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
                                                                               boost::property_tree::ptree& wlsConfig, const JPetParamBank& paramBank)
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

  for (auto matrixPair : paramBank.getMatrices())
  {
    auto matrix = matrixPair.second;
    if (matrix->getSide() == JPetMatrix::WLS)
    {
      // vector<int> sipmIDs;
      vector<JPetPMSignal> signalsOnMatrix;
      for (auto& item : wlsConfig.get_child("wls_matrix." + to_string(matrix->getID()) + ".matrix_sipm_ids"))
      {
        auto siPMID = item.second.get_value<int>(-1);
        if (siPMID != -1)
        {
          signalsOnMatrix.insert(signalsOnMatrix.end(), mappedSignals[JPetMatrix::WLS][siPMID].begin(), mappedSignals[JPetMatrix::WLS][siPMID].end());
        }
      }
      auto mtxSignals = mergePMSignalsOnWLS(signalsOnMatrix, mergingTime, calibTree, paramBank.getMatrix(matrix->getID()));
      allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
    }
  }

  return allMtxSignals;
}

vector<JPetMatrixSignal> RedModuleSignalTransformerTools::mergePMSignalsOnWLS(vector<JPetPMSignal>& pmSigVec, double mergingTime,
                                                                              boost::property_tree::ptree& calibTree, const JPetMatrix& matrix)
{
  vector<JPetMatrixSignal> mtxSigVec;
  SignalTransformerTools::sortByTime(pmSigVec);

  while (pmSigVec.size() > 0)
  {
    // Create Matrix Signal and add first PM Signal by default
    JPetMatrixSignal mtxSig;
    mtxSig.setMatrix(matrix);
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
  }

  return mtxSigVec;
}
