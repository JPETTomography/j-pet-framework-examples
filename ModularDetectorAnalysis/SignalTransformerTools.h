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
 *  @file SignalTransformerTools.h
 */

#ifndef SIGNALTRANSFORMERTOOLS_H
#define SIGNALTRANSFORMERTOOLS_H

/**
 * @brief Set of tools for Signal Transformer task
 *
 * Contains methods merging PM Signals from matrix of SiPMs to a Matrix Signal
 */

#include <JPetMatrixSignal/JPetMatrixSignal.h>
#include <JPetPMSignal/JPetPMSignal.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include <vector>

class SignalTransformerTools
{
public:
  static const std::map<JPetMatrix::Side, std::map<int, std::vector<JPetPMSignal>>> getPMSigMtxMap(const JPetTimeWindow* timeWindow);
  static std::vector<JPetMatrixSignal> mergeSignalsAllSiPMs(std::map<JPetMatrix::Side, std::map<int, std::vector<JPetPMSignal>>>& pmSigMtxMap,
                                                            double mergingTime, boost::property_tree::ptree& calibTree);
  static std::vector<JPetMatrixSignal> mergePMSignalsOnSide(std::vector<JPetPMSignal>& pmSigVec, double mergingTime,
                                                            boost::property_tree::ptree& calibTree);

private:
  static double calculateAverageTime(JPetMatrixSignal& mtxSig, boost::property_tree::ptree& calibTree);
  static double calculateAverageTOT(JPetMatrixSignal& mtxSig);
  static void sortByTime(std::vector<JPetPMSignal>& input);
};
#endif /* !SIGNALTRANSFORMERTOOLS_H */
