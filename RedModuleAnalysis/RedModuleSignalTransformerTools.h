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
 *  @file RedModuleSignalTransformerTools.h
 */

#ifndef REDMODULESIGNALTRANSFORMERTOOLS_H
#define REDMODULESIGNALTRANSFORMERTOOLS_H

/**
 * @brief Set of tools for Signal Transformer task
 *
 * Contains methods merging PM Signals from matrix of SiPMs to a Matrix Signal
 */

#include "../ModularDetectorAnalysis/SignalTransformerTools.h"
#include <JPetParamBank/JPetParamBank.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <Signals/JPetMatrixSignal/JPetMatrixSignal.h>
#include <Signals/JPetPMSignal/JPetPMSignal.h>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include <vector>

class RedModuleSignalTransformerTools
{
public:
  static const std::map<JPetMatrix::Side, std::map<int, std::vector<JPetPMSignal>>> getPMSigMtxMap(const JPetTimeWindow* timeWindow);

  static std::vector<JPetMatrixSignal> mergeSignalsAllSiPMs(std::map<JPetMatrix::Side, std::map<int, std::vector<JPetPMSignal>>>& pmSigMtxMap,
                                                            double mergingTime, boost::property_tree::ptree& calibTree,
                                                            boost::property_tree::ptree& wlsConfig, const JPetParamBank& paramBank);

  static std::vector<JPetMatrixSignal> mergePMSignalsOnWLS(std::vector<JPetPMSignal>& pmSigVec, double mergingTime,
                                                           boost::property_tree::ptree& calibTree, const JPetMatrix& matrix);
};
#endif /* !REDMODULESIGNALTRANSFORMERTOOLS_H */
