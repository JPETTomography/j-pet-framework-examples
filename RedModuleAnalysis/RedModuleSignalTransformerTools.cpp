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

    auto mtxID = pmSig.getPM().getMatrix().getID();
    auto side = pmSig.getPM().getMatrix().getSide();

    auto search = mappedSignals.at(side).find(mtxID);
    if (search == mappedSignals.at(side).end())
    {
      vector<JPetPMSignal> tmpSigVec;
      tmpSigVec.push_back(pmSig);
      mappedSignals.at(side)[mtxID] = tmpSigVec;
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

  // WLS part - preparing vector of signals for merging
  // Mapping signals by SiPM ID
  map<int, vector<JPetPMSignal>> sigsBySiPMID;
  for (auto& pmSigs : mappedSignals[JPetMatrix::WLS])
  {
    for (auto& pmSig : pmSigs.second)
    {
      auto pmID = pmSig.getPM().getID();
      auto search = sigsBySiPMID.find(pmID);
      if (search == sigsBySiPMID.end())
      {
        vector<JPetPMSignal> tmpSigVec;
        tmpSigVec.push_back(pmSig);
        sigsBySiPMID[pmID] = tmpSigVec;
      }
      else
      {
        search->second.push_back(pmSig);
      }
    }
  }

  // Iterating over matrices in the ParamBank to look for conincdences
  for (auto matrixPair : paramBank.getMatrices())
  {
    auto matrix = matrixPair.second;

    if (matrix->getSide() == JPetMatrix::WLS)
    {
      auto signalsOnMatrix = getPMSignalsOnWLSMatrix(sigsBySiPMID, wlsConfig, paramBank.getMatrix(matrix->getID()));
      if (signalsOnMatrix.size() != 0)
      {
        // cout << "mtx id " << matrix->getID() << " pm vec size " << signalsOnMatrix.size() << endl;
        auto mtxSignals = SignalTransformerTools::mergePMSignalsOnSide(signalsOnMatrix, mergingTime, calibTree);
        // auto mtxSignals = mergePMSignalsOnWLS(signalsOnMatrix, mergingTime, calibTree, paramBank.getMatrix(matrix->getID()));
        allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
      }
    }
  }

  return allMtxSignals;
}

vector<JPetPMSignal> RedModuleSignalTransformerTools::getPMSignalsOnWLSMatrix(map<int, vector<JPetPMSignal>>& sigsBySiPMID,
                                                                              boost::property_tree::ptree& wlsConfig, const JPetMatrix& matrix)
{
  vector<JPetPMSignal> signalsOnMatrix;

  // Reading the configuration to get the IDs of the SiPM that are attached to this matrix
  for (auto& item : wlsConfig.get_child("wls_matrix." + to_string(matrix.getID()) + ".matrix_sipm_ids"))
  {
    auto siPMID = item.second.get_value<int>(-1);
    if (siPMID != -1)
    {
      // for (auto& pmSig : sigsBySiPMID[siPMID])
      // {
      //   cout << "check mtx id old " << pmSig.getPM().getMatrix().getID() << " new " << matrix.getID() << endl;
      //   JPetPMSignal newSig(pmSig);
      //   newSig.setMatrix(matrix);
      //   cout << " new sig " << newSig.getPM().getMatrix().getID() << endl;
      //   signalsOnMatrix.push_back(newSig);
      // }
      signalsOnMatrix.insert(signalsOnMatrix.end(), sigsBySiPMID[siPMID].begin(), sigsBySiPMID[siPMID].end());
    }
  }
  return signalsOnMatrix;
}

// vector<JPetMatrixSignal> RedModuleSignalTransformerTools::mergePMSignalsOnWLS(vector<JPetPMSignal>& pmSigVec, double mergingTime,
//                                                                               boost::property_tree::ptree& calibTree, const JPetMatrix& matrix)
// {
//   cout << "method mtx id " << matrix.getID() << " pm vec size " << pmSigVec.size() << endl;
//
//   vector<JPetMatrixSignal> mtxSigVec;
//   SignalTransformerTools::sortByTime(pmSigVec);
//
//   while (pmSigVec.size() > 0)
//   {
//     // Create Matrix Signal and add first PM Signal by default
//     JPetMatrixSignal mtxSig;
//     mtxSig.setMatrix(matrix);
//     if (!mtxSig.addPMSignal(pmSigVec.at(0)))
//     {
//       ERROR("Problem with adding the first signal to new object.");
//       break;
//     }
//
//     unsigned int nextIndex = 1;
//     while (true)
//     {
//       if (pmSigVec.size() <= nextIndex)
//       {
//         // nothing left to check
//         break;
//       }
//
//       cout << "sig 0 " << pmSigVec.at(0).getTime() << " sig next " << pmSigVec.at(nextIndex).getTime() << endl;
//
//       // signal matching condidion
//       if (fabs(pmSigVec.at(nextIndex).getTime() - pmSigVec.at(0).getTime()) < mergingTime)
//       {
//         // mathing signal found
//         if (mtxSig.addPMSignal(pmSigVec.at(nextIndex)))
//         {
//           // added succesfully
//           pmSigVec.erase(pmSigVec.begin() + nextIndex);
//         }
//         else
//         {
//           // this mtx pos is already occupied, check the next one
//           nextIndex++;
//         }
//       }
//       else
//       {
//         // next signal is too far from reference one, this MtxSig is finished
//         break;
//       }
//     }
//   }
//
//   return mtxSigVec;
// }
