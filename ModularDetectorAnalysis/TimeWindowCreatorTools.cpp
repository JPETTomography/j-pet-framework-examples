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
 *  @file TimeWindowCreatorTools.cpp
 */

#include "TimeWindowCreatorTools.h"

using namespace std;

/**
 * Sorting method for Signal Channels by time
 */
void TimeWindowCreatorTools::sortByTime(vector<JPetSigCh>& input)
{
  std::sort(input.begin(), input.end(), [](JPetSigCh sigCh1, JPetSigCh sigCh2) { return sigCh1.getTime() < sigCh2.getTime(); });
}

/**
 * Building all Signal Chnnels from one TDC
 */
vector<JPetSigCh> TimeWindowCreatorTools::buildSigChs(TDCChannel* tdcChannel, const JPetChannel& channel, double maxTime, double minTime,
                                                      boost::property_tree::ptree& calibTree)
{
  vector<JPetSigCh> allSigChs;

  // Getting offsets for this channel
  // if calibrations are empty then default vaule is 0.0
  double offset = calibTree.get("channel_offests." + to_string(channel.getID()), 0.0);

  // Loop over all entries on leading edge in current TDCChannel and create SigCh
  for (int j = 0; j < tdcChannel->GetLeadHitsNum(); j++)
  {
    auto leadTime = tdcChannel->GetLeadTime(j);
    if (leadTime > maxTime || leadTime < minTime)
    {
      continue;
    }
    auto leadSigCh = generateSigCh(leadTime, channel, JPetSigCh::Leading, offset);
    allSigChs.push_back(leadSigCh);
  }

  // Loop over all entries on trailing edge in current TDCChannel and create SigCh
  for (int j = 0; j < tdcChannel->GetTrailHitsNum(); j++)
  {
    auto trailTime = tdcChannel->GetTrailTime(j);
    if (trailTime > maxTime || trailTime < minTime)
    {
      continue;
    }
    auto trailSigCh = generateSigCh(trailTime, channel, JPetSigCh::Trailing, offset);
    allSigChs.push_back(trailSigCh);
  }

  return allSigChs;
}

/**
 * Method for investigation of repetated edges - setting RecoFlag for each SigCh.
 * SigChs are flagged as GOOD if they appear in the sequence LTLTLTLT ->
 * in other words, each found LT pair is marked as GOOD. If sequence of one type of
 * the edge is encountered, repeated ones are flagged CORRUPTED. Examples:
 * edge type -> LTLTLT  LLT  LLTT  LLLLTTTT  LLTTLTLTTTLLLLTT
 * flag      -> GGGGGG  CGG  CGGC  CCCGGCCC  CGGCGGGGCCCCCGGC
 */
void TimeWindowCreatorTools::flagSigChs(vector<JPetSigCh>& inputSigChs, JPetStatistics& stats, bool saveHistos)
{
  for (unsigned int i = 0; i < inputSigChs.size(); i++)
  {
    if (i == inputSigChs.size() - 1)
    {
      inputSigChs.at(i).setRecoFlag(JPetSigCh::Good);
      break;
    }
    auto& sigCh1 = inputSigChs.at(i);
    auto& sigCh2 = inputSigChs.at(i + 1);
    // Explicit check for repeated edges
    if ((sigCh1.getType() == JPetSigCh::Leading && sigCh2.getType() == JPetSigCh::Trailing))
    {
      sigCh1.setRecoFlag(JPetSigCh::Good);
      sigCh2.setRecoFlag(JPetSigCh::Good);
      if (saveHistos)
      {
        stats.getHisto1D("filter_LT_tdiff")->Fill(sigCh2.getTime() - sigCh1.getTime());
      }
    }
    else if (sigCh1.getType() == JPetSigCh::Trailing && sigCh2.getType() == JPetSigCh::Leading)
    {
      if (sigCh1.getRecoFlag() == JPetSigCh::Unknown)
      {
        sigCh1.setRecoFlag(JPetSigCh::Good);
      }
    }
    else if (sigCh1.getType() == JPetSigCh::Leading && sigCh2.getType() == JPetSigCh::Leading)
    {
      sigCh1.setRecoFlag(JPetSigCh::Corrupted);
      if (saveHistos)
      {
        stats.getHisto1D("filter_LL_PM")->Fill(sigCh1.getChannel().getPM().getID());
        stats.getHisto1D("filter_LL_THR")->Fill(sigCh1.getChannel().getThresholdNumber());
        stats.getHisto1D("filter_LL_tdiff")->Fill(sigCh2.getTime() - sigCh1.getTime());
      }
    }
    else if (sigCh1.getType() == JPetSigCh::Trailing && sigCh2.getType() == JPetSigCh::Trailing)
    {
      if (sigCh1.getRecoFlag() == JPetSigCh::Unknown)
      {
        sigCh1.setRecoFlag(JPetSigCh::Corrupted);
      }
      sigCh2.setRecoFlag(JPetSigCh::Corrupted);
      if (saveHistos)
      {
        stats.getHisto1D("filter_TT_PM")->Fill(sigCh1.getChannel().getPM().getID());
        stats.getHisto1D("filter_TT_THR")->Fill(sigCh1.getChannel().getThresholdNumber());
        stats.getHisto1D("filter_TT_tdiff")->Fill(sigCh2.getTime() - sigCh1.getTime());
      }
    }
  }
}

/**
 * Sets up Signal Channel fields
 */
JPetSigCh TimeWindowCreatorTools::generateSigCh(double tdcChannelTime, const JPetChannel& channel, JPetSigCh::EdgeType edge, double offset)
{
  JPetSigCh sigCh;
  sigCh.setTime(1000.0 * tdcChannelTime - offset);
  sigCh.setType(edge);
  sigCh.setChannel(channel);
  sigCh.setRecoFlag(JPetSigCh::Unknown);
  return sigCh;
}
