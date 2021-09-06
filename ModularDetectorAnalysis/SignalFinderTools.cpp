/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file SignalFinderTools.cpp
 */

#include "SignalFinderTools.h"
#include <TRandom.h>

using namespace std;

/**
 * Method returns a map of vectors of JPetChannelSignal ordered by photomultiplier ID
 */
const map<int, vector<JPetChannelSignal>> SignalFinderTools::getChannelSignalsByPM(const JPetTimeWindow* timeWindow, bool useCorruptedChannelSignal)
{
  map<int, vector<JPetChannelSignal>> chSigsPMMap;
  if (!timeWindow)
  {
    WARNING("Pointer of Time Window object is not set, returning empty map");
    return chSigsPMMap;
  }
  // Map Signal Channels according to PM they belong to
  const unsigned int nChannelSignals = timeWindow->getNumberOfEvents();
  for (unsigned int i = 0; i < nChannelSignals; i++)
  {
    auto chSig = dynamic_cast<const JPetChannelSignal&>(timeWindow->operator[](i));
    if (!useCorruptedChannelSignal && chSig.getRecoFlag() == JPetRecoSignal::Corrupted)
    {
      continue;
    }
    auto pmtID = chSig.getChannel().getPM().getID();
    auto search = chSigsPMMap.find(pmtID);
    if (search == chSigsPMMap.end())
    {
      vector<JPetChannelSignal> tmp;
      tmp.push_back(chSig);
      chSigsPMMap.insert(pair<int, vector<JPetChannelSignal>>(pmtID, tmp));
    }
    else
    {
      search->second.push_back(chSig);
    }
  }
  return chSigsPMMap;
}

/**
 * Method invoking PM Signal building method for each PM separately
 */
vector<JPetPMSignal> SignalFinderTools::buildAllSignals(const map<int, vector<JPetChannelSignal>>& chSigByPM, double chSigEdgeMaxTime,
                                                        double chSigLeadTrailMaxTime, int numberOfThrs, JPetStatistics& stats, bool saveHistos,
                                                        SignalFinderTools::ToTCalculationType type, boost::property_tree::ptree& calibTree)
{
  vector<JPetPMSignal> allSignals;
  for (auto& chSigPair : chSigByPM)
  {
    auto signals = buildPMSignals(chSigPair.second, chSigEdgeMaxTime, chSigLeadTrailMaxTime, numberOfThrs, stats, saveHistos, type, calibTree);
    allSignals.insert(allSignals.end(), signals.begin(), signals.end());
  }
  return allSignals;
}

/**
 * @brief Reconstruction of PM Signals based on Signal Channels on the same PM
 *
 * PMSignal is created with all Leading ChannelSignals that are found within first
 * time window (chSigEdgeMaxTime parameter) and all Trailing ChannelSignals that conform
 * to second time window (chSigLeadTrailMaxTime parameter).
 */
vector<JPetPMSignal> SignalFinderTools::buildPMSignals(const vector<JPetChannelSignal>& chSigByPM, double chSigEdgeMaxTime,
                                                       double chSigLeadTrailMaxTime, int numberOfThrs, JPetStatistics& stats, bool saveHistos,
                                                       SignalFinderTools::ToTCalculationType type, boost::property_tree::ptree& calibTree)
{
  vector<JPetPMSignal> pmSigVec;

  vector<JPetChannelSignal> tmpVec;
  vector<vector<JPetChannelSignal>> thrLeadingChannelSignal(numberOfThrs, tmpVec);
  vector<vector<JPetChannelSignal>> thrTrailingChannelSignal(numberOfThrs, tmpVec);

  for (const JPetChannelSignal& chSig : chSigByPM)
  {
    if (chSig.getEdgeType() == JPetChannelSignal::Leading)
    {
      thrLeadingChannelSignal.at(chSig.getChannel().getThresholdNumber() - 1).push_back(chSig);
    }
    else if (chSig.getEdgeType() == JPetChannelSignal::Trailing)
    {
      thrTrailingChannelSignal.at(chSig.getChannel().getThresholdNumber() - 1).push_back(chSig);
    }
  }

  assert(thrLeadingChannelSignal.size() > 0);
  while (thrLeadingChannelSignal.at(0).size() > 0)
  {
    // pmSig.setPM(thrLeadingChannelSignal.at(0).at(0).getChannel().getPM());
    // double totTHR1 = 0.0, totTHR2 = 0.0, thr1Val = 0.0, thr2Val = 0.0;

    // // First THR leading added by default
    // pmSig.addPoint(thrLeadingChannelSignal.at(0).at(0));
    //
    // addLeadTrailPair
    //
    // // Establishing Reco flag same as of the first added Channel Signal
    // if (thrLeadingChannelSignal.at(0).at(0).getRecoFlag() == JPetRecoSignal::Good)
    // {
    //   pmSig.setRecoFlag(JPetRecoSignal::Good);
    // }
    // else if (thrLeadingChannelSignal.at(0).at(0).getRecoFlag() == JPetRecoSignal::Corrupted)
    // {
    //   pmSig.setRecoFlag(JPetRecoSignal::Corrupted);
    // }

    // pmSig.setTime(thrLeadingChannelSignal.at(0).at(0).getTime());
    // thr1Val = thrLeadingChannelSignal.at(0).at(0).getChannel().getThresholdValue();

    // PM Signal is created if the Lead-Trail pair is found at THR 1
    int closestTrailingChannelSignal =
        findTrailingChannelSignal(thrLeadingChannelSignal.at(0).at(0), chSigLeadTrailMaxTime, thrTrailingChannelSignal.at(0));
    if (closestTrailingChannelSignal != -1)
    {
      JPetPMSignal pmSig;
      pmSig.setRecoFlag(JPetRecoSignal::Good);

      // pmSig.addPoint(thrTrailingChannelSignal.at(0).at(closestTrailingChannelSignal));
      pmSig.addLeadTrailPair(thrLeadingChannelSignal.at(0).at(0), thrTrailingChannelSignal.at(0).at(closestTrailingChannelSignal));

      // Modifying flag if needed
      if (thrLeadingChannelSignal.at(0).at(0).getRecoFlag() == JPetRecoSignal::Corrupted ||
          thrTrailingChannelSignal.at(0).at(closestTrailingChannelSignal).getRecoFlag() == JPetRecoSignal::Corrupted)
      {
        pmSig.setRecoFlag(JPetRecoSignal::Corrupted);
      }

      // totTHR1 = thrTrailingChannelSignal.at(0).at(closestTrailingChannelSignal).getTime() - thrLeadingChannelSignal.at(0).at(0).getTime();
      thrTrailingChannelSignal.at(0).erase(thrTrailingChannelSignal.at(0).begin() + closestTrailingChannelSignal);

      // Adding Lead-Trail pair if found on THR 2
      int nextThrChannelSignalIndex =
          findChannelSignalOnNextThr(thrLeadingChannelSignal.at(0).at(0).getTime(), chSigEdgeMaxTime, thrLeadingChannelSignal.at(1));
      if (nextThrChannelSignalIndex != -1)
      {
        closestTrailingChannelSignal =
            findTrailingChannelSignal(thrLeadingChannelSignal.at(0).at(0), chSigLeadTrailMaxTime, thrTrailingChannelSignal.at(1));
        if (closestTrailingChannelSignal != -1)
        {

          // pmSig.addPoint(thrTrailingChannelSignal.at(1).at(closestTrailingChannelSignal));
          pmSig.addLeadTrailPair(thrLeadingChannelSignal.at(1).at(nextThrChannelSignalIndex),
                                 thrTrailingChannelSignal.at(1).at(closestTrailingChannelSignal));

          // Modifying flag if needed
          if (thrLeadingChannelSignal.at(1).at(nextThrChannelSignalIndex).getRecoFlag() == JPetRecoSignal::Corrupted ||
              thrTrailingChannelSignal.at(1).at(closestTrailingChannelSignal).getRecoFlag() == JPetRecoSignal::Corrupted)
          {
            pmSig.setRecoFlag(JPetRecoSignal::Corrupted);
          }

          // totTHR2 = thrTrailingChannelSignal.at(1).at(closestTrailingChannelSignal).getTime() -
          //           thrLeadingChannelSignal.at(1).at(nextThrChannelSignalIndex).getTime();
          thrTrailingChannelSignal.at(1).erase(thrTrailingChannelSignal.at(1).begin() + closestTrailingChannelSignal);
          thrLeadingChannelSignal.at(1).erase(thrLeadingChannelSignal.at(1).begin() + nextThrChannelSignalIndex);
        }

        // pmSig.addPoint(thrLeadingChannelSignal.at(1).at(nextThrChannelSignalIndex));

        // Modifying flag if needed
        // if (thrLeadingChannelSignal.at(1).at(nextThrChannelSignalIndex).getRecoFlag() == JPetRecoSignal::Corrupted)
        // {
        //   pmSig.setRecoFlag(JPetRecoSignal::Corrupted);
        // }
        // thr2Val = thrLeadingChannelSignal.at(1).at(nextThrChannelSignalIndex).getChannel().getThresholdValue();
      }

      auto tot = calculatePMSignalToT(pmSig, type, calibTree);
      pmSig.setToT(tot);
      // Adding created PM Signal to vector
      pmSigVec.push_back(pmSig);

      // Filling control histograms
      if (saveHistos && gRandom->Uniform() < 0.001)
      {
        if (pmSig.getRecoFlag() == JPetRecoSignal::Good)
        {
          stats.fillHistogram("reco_flags_pmsig", 1);
        }
        else if (pmSig.getRecoFlag() == JPetRecoSignal::Corrupted)
        {
          stats.fillHistogram("reco_flags_pmsig", 2);
        }
        else
        {
          stats.fillHistogram("reco_flags_pmsig", 3);
        }

        for (int jj = 0; jj < numberOfThrs; jj++)
        {
          for (auto chSig : thrLeadingChannelSignal.at(jj))
          {
            stats.fillHistogram("unused_chsig_thr", 2 * chSig.getChannel().getThresholdNumber() - 1);
            stats.fillHistogram("unused_chsig_sipm", chSig.getChannel().getPM().getID());
          }
          for (auto chSig : thrTrailingChannelSignal.at(jj))
          {
            stats.fillHistogram("unused_chsig_thr", 2 * chSig.getChannel().getThresholdNumber());
            stats.fillHistogram("unused_chsig_sipm", chSig.getChannel().getPM().getID());
          }
        }
      }
    }

    thrLeadingChannelSignal.at(0).erase(thrLeadingChannelSignal.at(0).begin());
  }

  return pmSigVec;
}

/**
 * Method finds Signal Channels that belong to the same leading edge
 */
int SignalFinderTools::findChannelSignalOnNextThr(double chSigValue, double chSigEdgeMaxTime, const vector<JPetChannelSignal>& chSigVec)
{
  for (size_t i = 0; i < chSigVec.size(); i++)
  {
    if (fabs(chSigValue - chSigVec.at(i).getTime()) < chSigEdgeMaxTime)
    {
      return i;
    }
  }
  return -1;
}

/**
 * Method finds trailing edge Signal Channel that suits certian leading edge
 * Signal Channel, if more than one trailing edge Signal Channel found,
 * returning the one with the smallest index, that is equivalent of ChannelSignal
 * earliest in time
 */
int SignalFinderTools::findTrailingChannelSignal(const JPetChannelSignal& leadingChannelSignal, double chSigLeadTrailMaxTime,
                                                 const vector<JPetChannelSignal>& trailingChannelSignalVec)
{
  vector<int> trailingFoundIdices;
  for (size_t i = 0; i < trailingChannelSignalVec.size(); i++)
  {
    double timeDiff = trailingChannelSignalVec.at(i).getTime() - leadingChannelSignal.getTime();
    if (timeDiff > 0.0 && timeDiff < chSigLeadTrailMaxTime)
    {
      trailingFoundIdices.push_back(i);
    }
  }
  if (trailingFoundIdices.size() == 0)
  {
    return -1;
  }
  sort(trailingFoundIdices.begin(), trailingFoundIdices.end());
  return trailingFoundIdices.at(0);
}

double SignalFinderTools::calculatePMSignalToT(JPetPMSignal& pmSignal, SignalFinderTools::ToTCalculationType type,
                                               boost::property_tree::ptree& calibTree)
{
  double tot = 0.0;
  auto signals = pmSignal.getLeadTrailPairs();

  for (unsigned int thr = 0; thr < signals.size(); ++thr)
  {
    double prevTHR = (thr > 0) ? signals.at(thr - 1).first.getChannel().getThresholdValue() : 0.0;
    double thisTHR = signals.at(thr).first.getChannel().getThresholdValue();
    double thrDiff = thisTHR - prevTHR;
    double thisTDiff = signals.at(thr).second.getTime() - signals.at(thr).first.getTime();
    double prevTDiff = (thr > 0) ? signals.at(thr - 1).second.getTime() - signals.at(thr - 1).first.getTime() : thisTDiff;

    switch (type)
    {
    case kSimplified:
      tot += signals.at(thr).second.getTime() - signals.at(thr).first.getTime();
      break;
    case kThresholdRectangular:
      tot += thrDiff * thisTDiff;
      break;
    case kThresholdTrapeze:
      tot += 0.5 * (thisTDiff + prevTDiff) * thrDiff;
    }
  }

  // Applying ToT normalization constatns
  double totNormA = calibTree.get("sipm." + to_string(pmSignal.getPM().getID()) + ".tot_factor_a", 1.0);
  double totNormB = calibTree.get("sipm." + to_string(pmSignal.getPM().getID()) + ".tot_factor_b", 0.0);
  tot = tot * totNormA + totNormB;
  return tot;
}
