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
 *  @file TimeWindowCreatorTools.cpp
 */

#include "TimeWindowCreatorTools.h"

using namespace std;

/**
 * Sorting method for Channel Signals by time
 */
void TimeWindowCreatorTools::sortByTime(vector<JPetChannelSignal>& input)
{
  std::sort(input.begin(), input.end(), [](JPetChannelSignal chSig1, JPetChannelSignal chSig2) { return chSig1.getTime() < chSig2.getTime(); });
}

/**
 * Building all Channel Signals from one TDC
 */
vector<JPetChannelSignal> TimeWindowCreatorTools::buildChannelSignals(TDCChannel* tdcChannel, const JPetChannel& channel, double maxTime,
                                                                      double minTime, boost::property_tree::ptree& calibTree)
{
  vector<JPetChannelSignal> allChSigs;

  // Getting offsets for this channel
  // if calibrations are empty then default vaule is 0.0
  double offset = calibTree.get("channel_offests." + to_string(channel.getID()), 0.0);

  // Loop over all entries on leading edge in current TDCChannel and create ChSig
  for (int j = 0; j < tdcChannel->GetLeadHitsNum(); j++)
  {
    auto leadTime = tdcChannel->GetLeadTime(j);
    if (leadTime > maxTime || leadTime < minTime)
    {
      continue;
    }
    auto leadChSig = generateChannelSignal(leadTime, channel, JPetChannelSignal::Leading, offset);
    allChSigs.push_back(leadChSig);
  }

  // Loop over all entries on trailing edge in current TDCChannel and create ChSig
  for (int j = 0; j < tdcChannel->GetTrailHitsNum(); j++)
  {
    auto trailTime = tdcChannel->GetTrailTime(j);
    if (trailTime > maxTime || trailTime < minTime)
    {
      continue;
    }
    auto trailChSig = generateChannelSignal(trailTime, channel, JPetChannelSignal::Trailing, offset);
    allChSigs.push_back(trailChSig);
  }

  return allChSigs;
}

/**
 * Method for investigation of repetated edges - setting RecoFlag for each ChSig.
 * ChSigs are flagged as GOOD if they appear in the sequence LTLTLTLT ->
 * in other words, each found LT pair is marked as GOOD. If sequence of one type of
 * the edge is encountered, repeated ones are flagged CORRUPTED. Examples:
 * edge type -> LTLTLT  LLT  LLTT  LLLLTTTT  LLTTLTLTTTLLLLTT
 * flag      -> GGGGGG  CGG  CGGC  CCCGGCCC  CGGCGGGGCCCCCGGC
 */
void TimeWindowCreatorTools::flagChannelSignals(vector<JPetChannelSignal>& inputChSigs, JPetStatistics& stats, bool saveHistos)
{
  for (unsigned int i = 0; i < inputChSigs.size(); i++)
  {
    if (i == inputChSigs.size() - 1)
    {
      inputChSigs.at(i).setRecoFlag(JPetRecoSignal::Good);
      break;
    }
    auto& chSig1 = inputChSigs.at(i);
    auto& chSig2 = inputChSigs.at(i + 1);
    // Explicit check for repeated edges
    if ((chSig1.getEdgeType() == JPetChannelSignal::Leading && chSig2.getEdgeType() == JPetChannelSignal::Trailing))
    {
      chSig1.setRecoFlag(JPetRecoSignal::Good);
      chSig2.setRecoFlag(JPetRecoSignal::Good);
      if (saveHistos)
      {
        stats.fillHistogram("filter_LT_tdiff", chSig2.getTime() - chSig1.getTime());
      }
    }
    else if (chSig1.getEdgeType() == JPetChannelSignal::Trailing && chSig2.getEdgeType() == JPetChannelSignal::Leading)
    {
      if (chSig1.getRecoFlag() == JPetRecoSignal::Unknown)
      {
        chSig1.setRecoFlag(JPetRecoSignal::Good);
      }
    }
    else if (chSig1.getEdgeType() == JPetChannelSignal::Leading && chSig2.getEdgeType() == JPetChannelSignal::Leading)
    {
      chSig1.setRecoFlag(JPetRecoSignal::Corrupted);
      if (saveHistos)
      {
        stats.fillHistogram("filter_LL_PM", chSig1.getChannel().getPM().getID());
        stats.fillHistogram("filter_LL_THR", chSig1.getChannel().getThresholdNumber());
        stats.fillHistogram("filter_LL_tdiff", chSig2.getTime() - chSig1.getTime());
      }
    }
    else if (chSig1.getEdgeType() == JPetChannelSignal::Trailing && chSig2.getEdgeType() == JPetChannelSignal::Trailing)
    {
      if (chSig1.getRecoFlag() == JPetRecoSignal::Unknown)
      {
        chSig1.setRecoFlag(JPetRecoSignal::Corrupted);
      }
      chSig2.setRecoFlag(JPetRecoSignal::Corrupted);
      if (saveHistos)
      {
        stats.fillHistogram("filter_TT_PM", chSig1.getChannel().getPM().getID());
        stats.fillHistogram("filter_TT_THR", chSig1.getChannel().getThresholdNumber());
        stats.fillHistogram("filter_TT_tdiff", chSig2.getTime() - chSig1.getTime());
      }
    }
  }
}

/**
 * Sets up Signal Channel fields
 */
JPetChannelSignal TimeWindowCreatorTools::generateChannelSignal(double tdcChannelTime, const JPetChannel& channel, JPetChannelSignal::EdgeType edge,
                                                                double offset)
{
  JPetChannelSignal chSig;
  chSig.setTime(1000.0 * tdcChannelTime - offset);
  chSig.setEdgeType(edge);
  chSig.setChannel(channel);
  chSig.setRecoFlag(JPetRecoSignal::Unknown);
  return chSig;
}
