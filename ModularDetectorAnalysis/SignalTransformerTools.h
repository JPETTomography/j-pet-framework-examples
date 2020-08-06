/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
  * Contains methods merging Raw Signals from matrix of SiPMs to a Physical Signal
  */

 #include <JPetStatistics/JPetStatistics.h>
 #include <JPetTimeWindow/JPetTimeWindow.h>
 #include <JPetMatrixSignal/JPetMatrixSignal.h>
 #include <JPetRawSignal/JPetRawSignal.h>
 #include <JPetPM/JPetPM.h>
 #include <utility>
 #include <vector>

 class SignalTransformerTools
 {
 public:
   static const std::map<int, std::vector<std::vector<JPetRawSignal>>> getRawSigMtxMap(const JPetTimeWindow* timeWindow);

   static std::vector<JPetMatrixSignal> mergeSignalsAllSiPMs(
     std::map<int, std::vector<std::vector<JPetRawSignal>>>& rawSigMtxMap,
     double mergingTime, JPetStatistics& stats, bool saveHistos
   );

   static std::vector<JPetMatrixSignal> mergeRawSignalsOnSide(
     std::vector<JPetRawSignal>& rawSigVec, double mergingTime,
     JPetStatistics& stats, bool saveHistos
   );
   static double getRawSigBaseTime(JPetRawSignal& rawSig);

 private:
   static double calculateAverageTime(JPetMatrixSignal& mtxSig);
   static void sortByTime(std::vector<JPetRawSignal>& input);
 };
 #endif /* !SIGNALTRANSFORMERTOOLS_H */
