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
 *  @file SignalTransformerTools.cpp
 */

#include "SignalTransformerTools.h"

using namespace std;

/**
 * Map Raw Signals from all SiPMs according to matrix and side they belong to.
 * In side vector "A" is first element, "B" second
 */
const map<int, vector<vector<JPetRawSignal>>>
SignalTransformerTools::getRawSigMtxMap(
   const JPetTimeWindow* timeWindow, JPetStatistics& stats, bool saveHistos
){
  map<int, vector<vector<JPetRawSignal>>> rawSigMtxMap;

  if (!timeWindow) {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return rawSigMtxMap;
  }

  const unsigned int nRawSigs = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nRawSigs; i++) {
    auto rawSig = dynamic_cast<const JPetRawSignal&>(timeWindow->operator[](i));

    if(saveHistos){
      stats.getHisto1D("rawsig_per_pm")->Fill(rawSig.getPM().getID());
      stats.getHisto1D("rawsig_per_scin")->Fill(rawSig.getPM().getScin().getID());
    }

    auto scinID = rawSig.getPM().getScin().getID();
    auto pmSide = rawSig.getPM().getSide();
    auto search = rawSigMtxMap.find(scinID);

    if (search == rawSigMtxMap.end()) {
      // There is no map element with searched scin ID in this map, adding
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

vector<JPetMatrixSignal> SignalTransformerTools::mergeSignalsAllSiPMs(
   const map<int, vector<vector<JPetRawSignal>>>& rawSigMtxMap,
   double mergingTime, JPetStatistics& stats, bool saveHistos
) {
  vector<JPetMatrixSignal> allMtxSignals;
  // Iterating over whole map
  for (auto& rawSigScin : rawSigMtxMap) {
    for (auto& rawSigSide : rawSigScin.second){
      auto mtxSignals = mergeRawSignalsOnSide(
        rawSigSide, mergingTime, stats, saveHistos
      );
      allMtxSignals.insert(allMtxSignals.end(), mtxSignals.begin(), mtxSignals.end());
    }
  }
  return allMtxSignals;
}

vector<JPetMatrixSignal> SignalTransformerTools::mergeRawSignalsOnSide(
  const vector<JPetRawSignal>& rawSigVec, double mergingTime,
  JPetStatistics& stats, bool saveHistos
) {
  vector<JPetMatrixSignal> mtxSigSiPMs;

  int pmID = -1;
  int scinID = -1;
  JPetPM::Side side = JPetPM::SideA;
  if(rawSigVec.size()>0){
    pmID = rawSigVec.at(0).getPM().getID();
    scinID = rawSigVec.at(0).getPM().getScin().getID();
    side = rawSigVec.at(0).getPM().getSide();
  } else {
    return mtxSigSiPMs;
  }

  // Creating vector for each SiPM by matrix position
  vector<JPetRawSignal> tmp;
  vector<vector<JPetRawSignal>> rawSigsPerMtx;
  rawSigsPerMtx.push_back(tmp);
  rawSigsPerMtx.push_back(tmp);
  rawSigsPerMtx.push_back(tmp);
  rawSigsPerMtx.push_back(tmp);

  for (auto rawSig : rawSigVec) {
    auto mtxPos = rawSig.getPM().getMatrixPosition();
    rawSigsPerMtx.at(mtxPos-1).push_back(rawSig);
  }

  // Very redundant, but lets start with that
  // Explicit checking for next SiPMs position 1 with positions 2,3,4
  while(rawSigsPerMtx.at(0).size() > 0){
    JPetMatrixSignal mtxSignal(side, scinID);
    // Add first as default
    mtxSignal.addRawSignal(rawSigsPerMtx.at(0).at(0));
    auto time1 = rawSigsPerMtx.at(0).at(0)
      .getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();

    int pos2It = 0;
    int pos3It = 0;
    int pos4It = 0;
    bool pos2done = false;
    bool pos3done = false;
    bool pos4done = false;
    auto time2 = 0;
    auto time3 = 0;
    auto time4 = 0;

    while(!(pos2done && pos3done && pos4done)){
      // Checking if other vectors are non-zero
      if(rawSigsPerMtx.at(1).size() > pos2It){
        time2 = rawSigsPerMtx.at(1).at(pos2It).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
      } else {
        pos2done = true;
        break;
      }
      if(rawSigsPerMtx.at(2).size() > pos3It){
        time3 = rawSigsPerMtx.at(2).at(pos3It).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
      } else {
        pos3done = true;
        break;
      }
      if(rawSigsPerMtx.at(3).size() > pos4It){
        time4 = rawSigsPerMtx.at(3).at(pos4It).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
      } else {
        pos4done = true;
        break;
      }

      // check with SiPM position 2
      if(!pos2done && fabs(time2-time1)<mergingTime){
        if(saveHistos) {
          stats.getHisto1D("mtxsig_timediff")->Fill(fabs(time2-time1));
        }
        mtxSignal.addRawSignal(rawSigsPerMtx.at(1).at(pos2It));
        rawSigsPerMtx.at(1).erase(rawSigsPerMtx.at(1).begin()+pos2It);
        pos2done = true;
      } else if(!pos2done) {
        if((time2-time1) > mergingTime){
          // too far ahead in time, we are done
          pos2done = true;
        } else if((time1-time2) > mergingTime) {
          // too far back in time, mabye need to check next iteration
          pos2It++;
        }
      }

      // check with SiPM position 3
      if(!pos3done && fabs(time3-time1)<mergingTime){
        if(saveHistos) {
          stats.getHisto1D("mtxsig_timediff")->Fill(fabs(time3-time1));
        }
        mtxSignal.addRawSignal(rawSigsPerMtx.at(2).at(pos3It));
        rawSigsPerMtx.at(2).erase(rawSigsPerMtx.at(2).begin()+pos3It);
      } else if(!pos3done) {
        if((time3-time1) > mergingTime){
          // too far ahead in time, we are done
          pos3done = true;
        } else if((time1-time3) > mergingTime) {
          // too far back in time, mabye need to check next iteration
          pos3It++;
        }
      }

      // check with SiPM position 4
      if(!pos4done && fabs(time4-time1)<mergingTime){
        if(saveHistos) {
          stats.getHisto1D("mtxsig_timediff")->Fill(fabs(time4-time1));
        }
        mtxSignal.addRawSignal(rawSigsPerMtx.at(3).at(pos4It));
        rawSigsPerMtx.at(3).erase(rawSigsPerMtx.at(3).begin()+pos4It);
      } else if(!pos4done) {
        if((time4-time1) > mergingTime){
          // too far ahead in time, we are done
          pos4done = true;
        } else if((time1-time4) > mergingTime) {
          // too far back in time, mabye need to check next iteration
          pos4It++;
        }
      }
    }

    // After matching, check if MtxSig multiplicity grater than 1
    if(mtxSignal.getRawSignals().size() > 1) {
      mtxSigSiPMs.push_back(mtxSignal);
    }
    rawSigsPerMtx.at(0).erase(rawSigsPerMtx.at(0).begin());
  } // end of check 1 vs 2,3,4

  // Explicit checking for next SiPMs position 2 with positions 3,4
  while(rawSigsPerMtx.at(1).size() > 0){
    JPetMatrixSignal mtxSignal(side, scinID);
    // Add first as default
    mtxSignal.addRawSignal(rawSigsPerMtx.at(1).at(0));
    auto time2 = rawSigsPerMtx.at(1).at(0)
      .getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();

    int pos3It = 0;
    int pos4It = 0;
    bool pos3done = false;
    bool pos4done = false;
    auto time3 = 0;
    auto time4 = 0;

    while(!(pos3done && pos4done)){
      // Checking if other vectors are non-zero
      if(rawSigsPerMtx.at(2).size() > pos3It){
        time3 = rawSigsPerMtx.at(2).at(pos3It).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
      } else {
        pos3done = true;
        break;
      }
      if(rawSigsPerMtx.at(3).size() > pos4It){
        time4 = rawSigsPerMtx.at(3).at(pos4It).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
      } else {
        pos4done = true;
        break;
      }

      // check with SiPM position 3
      if(!pos3done && fabs(time3-time2)<mergingTime){
        if(saveHistos) {
          stats.getHisto1D("mtxsig_timediff")->Fill(fabs(time3-time2));
        }
        mtxSignal.addRawSignal(rawSigsPerMtx.at(2).at(pos3It));
        rawSigsPerMtx.at(2).erase(rawSigsPerMtx.at(2).begin()+pos3It);
      } else if(!pos3done) {
        if((time3-time2) > mergingTime){
          // too far ahead in time, we are done
          pos3done = true;
        } else if((time2-time3) > mergingTime) {
          // too far back in time, mabye need to check next iteration
          pos3It++;
        }
      }

      // check with SiPM position 4
      if(!pos4done && fabs(time4-time2)<mergingTime){
        if(saveHistos) {
          stats.getHisto1D("mtxsig_timediff")->Fill(fabs(time4-time2));
        }
        mtxSignal.addRawSignal(rawSigsPerMtx.at(3).at(pos4It));
        rawSigsPerMtx.at(3).erase(rawSigsPerMtx.at(3).begin()+pos4It);
      } else if(!pos4done) {
        if((time4-time2) > mergingTime){
          // too far ahead in time, we are done
          pos4done = true;
        } else if((time2-time4) > mergingTime) {
          // too far back in time, mabye need to check next iteration
          pos4It++;
        }
      }
    }

    // After matching, check if MtxSig multiplicity grater than 1
    if(mtxSignal.getRawSignals().size() > 1) {
      mtxSigSiPMs.push_back(mtxSignal);
    }
    rawSigsPerMtx.at(1).erase(rawSigsPerMtx.at(1).begin());
  } // end of check 2 vs 3,4

  // Explicit checking for next SiPMs position 3 with position 4
  while(rawSigsPerMtx.at(2).size() > 0){
    JPetMatrixSignal mtxSignal(side, scinID);
    // Add first as default
    mtxSignal.addRawSignal(rawSigsPerMtx.at(2).at(0));
    auto time3 = rawSigsPerMtx.at(2).at(0)
      .getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();

    int pos4It = 0;
    bool pos4done = false;
    auto time4 = 0;

    while(!pos4done){
      // Checking if other vector is non-zero
      if(rawSigsPerMtx.at(3).size() > pos4It){
        time4 = rawSigsPerMtx.at(3).at(pos4It).getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getTime();
      } else {
        pos4done = true;
        break;
      }

      // check with SiPM position 4
      if(!pos4done && fabs(time4-time3)<mergingTime){
        if(saveHistos) {
          stats.getHisto1D("mtxsig_timediff")->Fill(fabs(time4-time3));
        }
        mtxSignal.addRawSignal(rawSigsPerMtx.at(3).at(pos4It));
        rawSigsPerMtx.at(3).erase(rawSigsPerMtx.at(3).begin()+pos4It);
      } else if(!pos4done) {
        if((time4-time3) > mergingTime){
          // too far ahead in time, we are done
          pos4done = true;
        } else if((time3-time4) > mergingTime) {
          // too far back in time, mabye need to check next iteration
          pos4It++;
        }
      }
    }

    // After matching, check if MtxSig multiplicity grater than 1
    if(mtxSignal.getRawSignals().size() > 1) {
      mtxSigSiPMs.push_back(mtxSignal);
    } else if (saveHistos){
      stats.getHisto1D("unused_rawsig_per_pm")->Fill(pmID);
      if(side==JPetPM::SideA){
        stats.getHisto1D("unused_rawsig_per_scin_sideA")->Fill(scinID);
      } else if(side==JPetPM::SideB){
        stats.getHisto1D("unused_rawsig_per_scin_sideB")->Fill(scinID);
      }
    }

    rawSigsPerMtx.at(2).erase(rawSigsPerMtx.at(2).begin());
  } // end of check 3 vs 4

  return mtxSigSiPMs;
}
