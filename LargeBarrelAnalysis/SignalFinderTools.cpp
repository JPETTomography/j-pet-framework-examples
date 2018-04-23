/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file SignalFinderToolsTools.cpp
 */

#include "SignalFinderTools.h"
using namespace std;

map<int, vector<JPetSigCh>> SignalFinderTools::getSigChsPMMapById(const JPetTimeWindow* timeWindow)
{
  map<int, vector<JPetSigCh>> sigChsPMMap;
  if (!timeWindow) {
    WARNING("timeWindow pointer is not set");
    return sigChsPMMap;
  }

  //map Signal Channels in this Time window according to PM they belong to
  const unsigned int nSigChs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nSigChs; i++) {
    JPetSigCh sigCh = dynamic_cast<const JPetSigCh&>(timeWindow->operator[](i));
    int pmt_id = sigCh.getPM().getID();
    auto search = sigChsPMMap.find(pmt_id);
    if (search == sigChsPMMap.end()) {
      vector<JPetSigCh> tmp;
      tmp.push_back(sigCh);
      sigChsPMMap.insert(make_pair(pmt_id, tmp));
    } else {
      search->second.push_back(sigCh);
    }
  }

  return sigChsPMMap;
}

//method with loop of building raw signals for whole PM map
vector<JPetRawSignal> SignalFinderTools::buildAllSignals(
  map<int, vector<JPetSigCh>> sigChsPMMap,
  unsigned int numOfThresholds,
  JPetStatistics& stats,
  bool saveControlHistos,
  double sigChEdgeMaxTime,
  double sigChLeadTrailMaxTime)
{

  vector<JPetRawSignal> allSignals;

  for (auto& sigChPair : sigChsPMMap) {
    vector<JPetRawSignal> currentSignals = buildRawSignals(sigChPair.second, numOfThresholds, stats, saveControlHistos, sigChEdgeMaxTime, sigChLeadTrailMaxTime);
    allSignals.insert(allSignals.end(), currentSignals.begin(), currentSignals.end());
  }

  return allSignals;
}

//sorting method
bool sortByTimeValue(JPetSigCh sig1, JPetSigCh sig2)
{
  return (sig1.getValue() < sig2.getValue());
}

//method creating Raw signals form vector of Signal Channels
vector<JPetRawSignal> SignalFinderTools::buildRawSignals(
  const vector<JPetSigCh>& sigChFromSamePM,
  unsigned int numOfThresholds,
  JPetStatistics& stats,
  bool saveControlHistos,
  double sigChEdgeMaxTime,
  double sigChLeadTrailMaxTime)
{
  vector<JPetRawSignal> rawSigVec;

  //Threshold number check - fixed number equal 4
  if (numOfThresholds != 4) {
    ERROR("This function is ment to work with 4 thresholds only!");
    return rawSigVec;
  }

  vector<JPetSigCh> tmp;
  vector<vector<JPetSigCh>> thresholdSigCh(2 * numOfThresholds, tmp);

  bool errorOccured = false;

  //division into subvectors according to threshold number:
  //0-3 leading, 4-7 trailing
  for (const JPetSigCh& sigCh : sigChFromSamePM) {
    auto threshNum = sigCh.getThresholdNumber();
    if ((threshNum <= 0) || (threshNum > 2 * numOfThresholds)) {
      ERROR("Threshold number out of range:" + std::to_string(threshNum));
      errorOccured = true;
      break;
    }

    if (sigCh.getType() == JPetSigCh::Leading) {
      thresholdSigCh.at(sigCh.getThresholdNumber() - 1).push_back(sigCh);
    } else if (sigCh.getType() == JPetSigCh::Trailing) {
      thresholdSigCh.at(sigCh.getThresholdNumber() + numOfThresholds - 1)
      .push_back(sigCh);
    }
  }

  if (errorOccured) {
    return rawSigVec;
  }

  //probably not needed vector sorting according to Signal channel time values
  for (auto thrVec : thresholdSigCh) {
    sort(thrVec.begin(), thrVec.end(), sortByTimeValue);
  }

  assert(thresholdSigCh.size() > 0);
  while (thresholdSigCh.at(0).size() > 0) {

    JPetRawSignal rawSig;
    rawSig.setPM(thresholdSigCh.at(0).at(0).getPM());
    rawSig.setBarrelSlot(thresholdSigCh.at(0).at(0).getPM().getBarrelSlot());

    //first leading added by default
    rawSig.addPoint(thresholdSigCh.at(0).at(0));

    //looking for points from other thresholds that belong to the same leading edge
    //and search for equivalent trailing edge points
    //first thr trailing

    assert(thresholdSigCh.at(0).size() > 0);
    assert(thresholdSigCh.size() > 4);

    int closestTrailingSigCh0 = findTrailingSigCh(
                                  thresholdSigCh.at(0).at(0),
                                  thresholdSigCh.at(4),
                                  sigChLeadTrailMaxTime);

    if (closestTrailingSigCh0 != -1) {
      rawSig.addPoint(thresholdSigCh.at(4).at(closestTrailingSigCh0));
      thresholdSigCh.at(4).erase(thresholdSigCh.at(4).begin()
                                 + closestTrailingSigCh0);
    }

    //second thr leading and trailing
    int nextThrSigChIndex1 = findSigChOnNextThr(
                               thresholdSigCh.at(0).at(0).getValue(),
                               thresholdSigCh.at(1),
                               sigChEdgeMaxTime);

    if (nextThrSigChIndex1 != -1) {

      int closestTrailingSigCh1 = findTrailingSigCh(
                                    thresholdSigCh.at(0).at(0),
                                    thresholdSigCh.at(5),
                                    sigChLeadTrailMaxTime);

      if (closestTrailingSigCh1 != -1) {
        rawSig.addPoint(thresholdSigCh.at(5).at(closestTrailingSigCh1));
        thresholdSigCh.at(5).erase(thresholdSigCh.at(5).begin()
                                   + closestTrailingSigCh1);
      }

      rawSig.addPoint(thresholdSigCh.at(1).at(nextThrSigChIndex1));
      thresholdSigCh.at(1).erase(thresholdSigCh.at(1).begin()
                                 + nextThrSigChIndex1);
    }

    //third thr leading and trailing
    int nextThrSigChIndex2 = findSigChOnNextThr(
                               thresholdSigCh.at(0).at(0).getValue(),
                               thresholdSigCh.at(2),
                               sigChEdgeMaxTime);

    if (nextThrSigChIndex2 != -1) {

      int closestTrailingSigCh2 = findTrailingSigCh(
                                    thresholdSigCh.at(0).at(0),
                                    thresholdSigCh.at(6),
                                    sigChLeadTrailMaxTime);

      if (closestTrailingSigCh2 != -1) {
        rawSig.addPoint(thresholdSigCh.at(6).at(closestTrailingSigCh2));
        thresholdSigCh.at(6).erase(thresholdSigCh.at(6).begin()
                                   + closestTrailingSigCh2);
      }

      rawSig.addPoint(thresholdSigCh.at(2).at(nextThrSigChIndex2));
      thresholdSigCh.at(2).erase(thresholdSigCh.at(2).begin()
                                 + nextThrSigChIndex2);
    }

    //fourth thr leading and trailing
    int nextThrSigChIndex3 = findSigChOnNextThr(
                               thresholdSigCh.at(0).at(0).getValue(),
                               thresholdSigCh.at(3),
                               sigChEdgeMaxTime);

    if (nextThrSigChIndex3 != -1) {

      int closestTrailingSigCh3 = findTrailingSigCh(
                                    thresholdSigCh.at(0).at(0),
                                    thresholdSigCh.at(7),
                                    sigChLeadTrailMaxTime);

      if (closestTrailingSigCh3 != -1) {
        rawSig.addPoint(thresholdSigCh.at(7).at(closestTrailingSigCh3));
        thresholdSigCh.at(7).erase(thresholdSigCh.at(7).begin()
                                   + closestTrailingSigCh3);
      }

      rawSig.addPoint(thresholdSigCh.at(3).at(nextThrSigChIndex3));
      thresholdSigCh.at(3).erase(thresholdSigCh.at(3).begin()
                                 + nextThrSigChIndex3);
    }

    //adding created Raw Signal to vector
    rawSigVec.push_back(rawSig);
    thresholdSigCh.at(0).erase(thresholdSigCh.at(0).begin() + 0);
  }

  //filling controll histograms
  if (saveControlHistos) {
    stats.getHisto1D("remainig_leading_sig_ch_per_thr")
    ->Fill(1, thresholdSigCh.at(0).size());
    stats.getHisto1D("remainig_leading_sig_ch_per_thr")
    ->Fill(2, thresholdSigCh.at(1).size());
    stats.getHisto1D("remainig_leading_sig_ch_per_thr")
    ->Fill(3, thresholdSigCh.at(2).size());
    stats.getHisto1D("remainig_leading_sig_ch_per_thr")
    ->Fill(4, thresholdSigCh.at(3).size());
    stats.getHisto1D("remainig_trailing_sig_ch_per_thr")
    ->Fill(1, thresholdSigCh.at(4).size());
    stats.getHisto1D("remainig_trailing_sig_ch_per_thr")
    ->Fill(2, thresholdSigCh.at(5).size());
    stats.getHisto1D("remainig_trailing_sig_ch_per_thr")
    ->Fill(3, thresholdSigCh.at(6).size());
    stats.getHisto1D("remainig_trailing_sig_ch_per_thr")
    ->Fill(4, thresholdSigCh.at(7).size());
  }

  return rawSigVec;
}


//method of finding Signal Channels that belong to the same leading edge
//not more than sigChEdgeMaxTime away. Defined in ps.
int SignalFinderTools::findSigChOnNextThr(Double_t sigChValue, const vector<JPetSigCh>& sigChVec, double sigChEdgeMaxTime)
{
  for (size_t i = 0; i < sigChVec.size(); i++) {
    if (fabs(sigChValue - sigChVec.at(i).getValue()) < sigChEdgeMaxTime) {
      return i;
    }
  }
  return -1;
}

//method of finding trailing edge SigCh that suits certian leading edge SigCh
//not further away than sigChLeadTrailMaxTime in ps
//if more than one trailing edge SigCh found, returning one with the smallest index
//that is equivalent of SigCh earliest in time
int SignalFinderTools::findTrailingSigCh(const JPetSigCh& leadingSigCh, const vector<JPetSigCh>& trailingSigChVec, double sigChLeadTrailMaxTime)
{
  vector<int> trailingFoundIdices;
  for (size_t i = 0; i < trailingSigChVec.size(); i++) {
    if (fabs(leadingSigCh.getValue() - trailingSigChVec.at(i).getValue()) < sigChLeadTrailMaxTime)
      trailingFoundIdices.push_back(i);
  }
  if (trailingFoundIdices.size() == 0) return -1;
  sort(trailingFoundIdices.begin(), trailingFoundIdices.end());
  return trailingFoundIdices.at(0);
}
