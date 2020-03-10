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
 *  @file SignalTransformerTools.cpp
 */

#include "SignalTransformerTools.h"

using namespace std;

/**
 * Map Raw Signals from all SiPMs according to matrix and side they belong to.
 * Returns map< scin ID, side < signals >>.
 * Side A is the first element int he vector, Side B is the second one.
 */
const map<int, vector<vector<JPetRawSignal>>>
SignalTransformerTools::getRawSigMtxMap(const JPetTimeWindow* timeWindow){
  map<int, vector<vector<JPetRawSignal>>> rawSigMtxMap;

  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return rawSigMtxMap;
  }

  const unsigned int nRawSigs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nRawSigs; i++) {
    auto rawSig = dynamic_cast<const JPetRawSignal&>(timeWindow->operator[](i));

    auto scinID = rawSig.getPM().getScin().getID();
    auto pmSide = rawSig.getPM().getSide();
    auto search = rawSigMtxMap.find(scinID);

    if (search == rawSigMtxMap.end()) {
      // There is no element with searched scin ID in this map, adding new one
      vector<JPetRawSignal> tmpVec;
      vector<vector<JPetRawSignal>> tmpVecVec;
      tmpVecVec.push_back(tmpVec);
      tmpVecVec.push_back(tmpVec);
      if(pmSide==JPetPM::SideA){
        tmpVecVec.at(0).push_back(rawSig);
      }else if(pmSide==JPetPM::SideB){
        tmpVecVec.at(1).push_back(rawSig);
      }
      rawSigMtxMap.insert(pair<int, vector<vector<JPetRawSignal>>>(scinID, tmpVecVec));
    } else {
      if(pmSide==JPetPM::SideA){
        search->second.at(0).push_back(rawSig);
      }else if(pmSide==JPetPM::SideB){
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
vector<JPetMatrixSignal> SignalTransformerTools::mergeSignalsAllSiPMs(
   map<int, vector<vector<JPetRawSignal>>>& rawSigMtxMap,
   double mergingTime, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetMatrixSignal> allMtxSignals;
  // Iterating over whole map
  for (auto& rawSigScin : rawSigMtxMap) {
    for (auto& rawSigSide : rawSigScin.second){

      // if(saveHistos) {
        // Iterate over all signals, to get consecutive time difference
        // for (unsigned int i=0; i<rawSigSide.size()-1; i++){
        //   if(i>=rawSigSide.size()){ break; }

          // auto pos1 = rawSigSide.at(i).getPM().getMatrixPosition();
          // auto pos2 = rawSigSide.at(i+1).getPM().getMatrixPosition();
          // if(pos1 == pos2) { continue; }
          //
          // auto scinID = rawSigSide.at(i).getPM().getScin().getID();
          // auto t1 = getRawSigBaseTime(rawSigSide.at(i));
          // auto t2 = getRawSigBaseTime(rawSigSide.at(i+1));

          // if(rawSigSide.at(i).getPM().getSide()==JPetPM::SideA){
          //   stats.getHisto1D(Form("tdiff_%d_A_%d_%d", scinID, pos1, pos2))->Fill(t2-t1);
          // } else if(rawSigSide.at(i).getPM().getSide()==JPetPM::SideB){
          //   stats.getHisto1D(Form("tdiff_%d_B_%d_%d", scinID, pos1, pos2))->Fill(t2-t1);
          // }
        // }
      // }

      auto mtxSignals = mergeRawSignalsOnSide(
        rawSigSide, mergingTime, stats, saveHistos
      );
      allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
    }
  }
  return allMtxSignals;
}

/**
 * Method iterates over all Raw Sigals on some SiPMs on the same matrix,
 * matching them into groups on max. 4 as a MatrixSignal
 */
vector<JPetMatrixSignal> SignalTransformerTools::mergeRawSignalsOnSide(
  vector<JPetRawSignal>& rawSigVec, double mergingTime,
  JPetStatistics& stats, bool saveHistos
) {
  vector<JPetMatrixSignal> mtxSigVec;
  sortByTime(rawSigVec);

  while (rawSigVec.size() > 0) {
    // Create Matrix Signal and add first Raw Signal by default
    JPetMatrixSignal mtxSig;
    mtxSig.setPM(rawSigVec.at(0).getPM());
    if(!mtxSig.addRawSignal(rawSigVec.at(0))){
      ERROR("Problem with adding the first signal to new object.");
      break;
    }
    mtxSig.setTime(getRawSigBaseTime(rawSigVec.at(0)));

    unsigned int nextIndex = 1;
    while(true){

      if(rawSigVec.size() <= nextIndex){
        // nothing left to check
        break;
      }

      // signal matching condidion
      if(fabs(getRawSigBaseTime(rawSigVec.at(nextIndex))
        -getRawSigBaseTime(rawSigVec.at(0))) < mergingTime
      ){
        // mathing signal found
        if(mtxSig.addRawSignal(rawSigVec.at(nextIndex))){
          // added succesfully
          rawSigVec.erase(rawSigVec.begin()+nextIndex);
        } else {
          // this mtx pos is already occupied, check the next one
          nextIndex++;
        }
      } else {
        // next signal is too far from reference one, this MtxSig is finished
        break;
      }
    }
    rawSigVec.erase(rawSigVec.begin());
    mtxSigVec.push_back(mtxSig);
  }
  return mtxSigVec;
}

/**
 * Returning time of leading Signal Channel on the first threshold from Raw Signal
 */
float SignalTransformerTools::getRawSigBaseTime(JPetRawSignal& rawSig)
{
  return rawSig.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
}

/**
 * Sorting method for Raw Signals, based on time of leading THR1 Signal Channel
 */
void SignalTransformerTools::sortByTime(vector<JPetRawSignal>& input)
{
  std::sort(
    input.begin(), input.end(), [] (JPetRawSignal rawSig1, JPetRawSignal rawSig2) {
      return getRawSigBaseTime(rawSig1) < getRawSigBaseTime(rawSig2);
    }
  );
}
