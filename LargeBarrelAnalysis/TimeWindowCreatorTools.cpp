/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
 *  @file TimeWindowCreatorTools.cpp
 */

#include "TimeWindowCreatorTools.h"
#include "UniversalFileLoader.h"

using namespace std;

/**
 * Sorting method for Signal Channels by time value
 */
vector<JPetSigCh> TimeWindowCreatorTools::sortByValue(vector<JPetSigCh> input)
{
  auto sigChs(input);
  std::sort(sigChs.begin(), sigChs.end(),
    [] (JPetSigCh sigCh1, JPetSigCh sigCh2) {
      return sigCh1.getValue() < sigCh2.getValue();
    }
  );
  return sigChs;
}

/**
 * Building all Signal Chnnels from one TDC
 */
vector<JPetSigCh> TimeWindowCreatorTools::buildSigChs(
  TDCChannel* tdcChannel, JPetTOMBChannel tombChannel, double maxTime, double minTime,
  map<unsigned int, vector<double>> timeCalibrationMap,
  map<unsigned int, vector<double>> thresholdsMap,
  bool setTHRValuesFromChannels, JPetStatistics& stats, bool saveHistos
){
  vector<JPetSigCh> allTDCSigChs;
  // Loop over all entries on leading edge in current TOMBChannel and create SigCh
  for (int j = 0; j < tdcChannel->GetLeadHitsNum(); j++) {
    if (tdcChannel->GetLeadTime(j) > maxTime || tdcChannel->GetLeadTime(j) < minTime ) { continue; }
    auto leadSigCh = generateSigCh(
      tdcChannel->GetLeadTime(j), tombChannel, JPetSigCh::Leading,
      timeCalibrationMap, thresholdsMap, setTHRValuesFromChannels
    );
    allTDCSigChs.push_back(leadSigCh);
    if (saveHistos){
      stats.getHisto1D(Form("pm_occupation_thr%d", tombChannel.getLocalChannelNumber()))
        ->Fill(tombChannel.getPM().getID());
    }
  }
  // Loop over all entries on trailing edge in current TOMBChannel and create SigCh
  for (int j = 0; j < tdcChannel->GetTrailHitsNum(); j++) {
    if (tdcChannel->GetTrailTime(j) > maxTime || tdcChannel->GetTrailTime(j) < minTime ) { continue; }
    auto trailSigCh = generateSigCh(
      tdcChannel->GetTrailTime(j), tombChannel, JPetSigCh::Trailing,
      timeCalibrationMap, thresholdsMap, setTHRValuesFromChannels
    );
    allTDCSigChs.push_back(trailSigCh);
    if (saveHistos){
      stats.getHisto1D(Form("pm_occupation_thr%d", tombChannel.getLocalChannelNumber()))
        ->Fill(tombChannel.getPM().getID());
    }
  }
  return allTDCSigChs;
}

/**
 * Method for investigation of repetated Leading edges - setting RecoFlag for each SigCh
 */
vector<JPetSigCh> TimeWindowCreatorTools::flagSigChs(
  vector<JPetSigCh> inputSigChs, const JPetParamBank& paramBank,
  int numOfThresholds, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetSigCh> flaggedSigChs;
  auto mappedSigChs = sigChByPMTHR(inputSigChs, paramBank, numOfThresholds);
  for(auto pmSigCh : mappedSigChs){
    for(auto thrSigCh : pmSigCh){
      auto flggedTHRSigChs = flagTHRSigChs(thrSigCh, stats, saveHistos);
      flaggedSigChs.insert(flaggedSigChs.end(), flggedTHRSigChs.begin(), flggedTHRSigChs.end());
    }
  }
  return flaggedSigChs;
}

/**
* Sets up Signal Channel fields
*/
JPetSigCh TimeWindowCreatorTools::generateSigCh(
  double tdcChannelTime, const JPetTOMBChannel& channel, JPetSigCh::EdgeType edge,
  map<unsigned int, vector<double>> timeCalibrationMap,
  map<unsigned int, vector<double>> thresholdsMap,
  bool setTHRValuesFromChannels
) {
  JPetSigCh sigCh;
  sigCh.setValue(1000.*(tdcChannelTime
    + UniversalFileLoader::getConfigurationParameter(timeCalibrationMap, channel.getChannel())
  ));
  sigCh.setType(edge);
  sigCh.setTOMBChannel(channel);
  sigCh.setPM(channel.getPM());
  sigCh.setFEB(channel.getFEB());
  sigCh.setTRB(channel.getTRB());
  sigCh.setDAQch(channel.getChannel());
  sigCh.setThresholdNumber(channel.getLocalChannelNumber());
  if(setTHRValuesFromChannels) {
    sigCh.setThreshold(channel.getThreshold());
  } else {
    sigCh.setThreshold(
      UniversalFileLoader::getConfigurationParameter(thresholdsMap, channel.getChannel())
    );
  }
  return sigCh;
}

/**
 * Method distributes JPetSigCh by PMs and THR they belong to
 */
vector<vector<vector<JPetSigCh>>> TimeWindowCreatorTools::sigChByPMTHR(
  vector<JPetSigCh> inputSigChs, const JPetParamBank& paramBank, int numOfThresholds
){
  int pmNumber = paramBank.getPMsSize();
  vector<JPetSigCh> sigChVec;
  vector<vector<JPetSigCh>> thrVec(numOfThresholds, sigChVec);
  vector<vector<vector<JPetSigCh>>> pmThrVec(pmNumber, thrVec);
  for(auto sigCh : inputSigChs){
    int pmID = sigCh.getPM().getID();
    int thrNum = sigCh.getThresholdNumber();
    pmThrVec.at(pmID-1).at(thrNum-1).push_back(sigCh);
  }
  return pmThrVec;
}

/**
 * Submethod for RecoFlags setting
 */
vector<JPetSigCh> TimeWindowCreatorTools::flagTHRSigChs(
  vector<JPetSigCh> thrSigChs, JPetStatistics& stats, bool saveHistos
){
  auto sortedTHRSigChs = sortByValue(thrSigChs);
  for(uint i=0; i<sortedTHRSigChs.size(); i++) {
    auto sigCh1 = sortedTHRSigChs.at(i);
    if(sigCh1.getType()==JPetSigCh::Trailing) {
      sigCh1.setRecoFlag(JPetSigCh::Good);
      if(saveHistos){
        stats.getHisto1D("good_vs_bad_sigch")->Fill(1);
      }
      continue;
    }
    for(uint j=i+1; j<sortedTHRSigChs.size(); j++) {
      auto sigCh2 = sortedTHRSigChs.at(j);
      if (sigCh2.getType()==JPetSigCh::Leading) {
        sigCh1.setRecoFlag(JPetSigCh::Corrupted);
        if(saveHistos){
          stats.getHisto1D("good_vs_bad_sigch")->Fill(2);
          stats.getHisto1D("LL_per_PM")->Fill(sigCh1.getPM().getID());
          stats.getHisto1D("LL_per_THR")->Fill(sigCh1.getThresholdNumber());
          stats.getHisto1D("LL_time_diff")->Fill(sigCh2.getValue()-sigCh1.getValue());
        }
      } else if(sigCh2.getType()==JPetSigCh::Trailing) {
        sigCh1.setRecoFlag(JPetSigCh::Good);
        if(saveHistos){
          stats.getHisto1D("LT_time_diff")->Fill(sigCh2.getValue()-sigCh1.getValue());
        }
        break;
      }
    }
  }
  return sortedTHRSigChs;
}
