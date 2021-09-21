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
 *  @file SignalTransformer.cpp
 */

#include "SignalTransformer.h"
#include "SignalTransformerTools.h"
#include <boost/property_tree/json_parser.hpp>

using namespace jpet_options_tools;

SignalTransformer::SignalTransformer(const char* name) : JPetUserTask(name) {}

SignalTransformer::~SignalTransformer() {}

bool SignalTransformer::init()
{
  INFO("Signal Transformer started: PM to Matrix Signal");
  fOutputEvents = new JPetTimeWindow("JPetMatrixSignal");

  // Getting bools for saving control and calibration histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  // Reading file with Side B signals correction to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  // Signal merging time parameter
  if (isOptionSet(fParams.getOptions(), kMergeSignalsTimeParamKey))
  {
    fMergingTime = getOptionAsDouble(fParams.getOptions(), kMergeSignalsTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMergeSignalsTimeParamKey.c_str(), fMergingTime));
  }

  // For plotting ToT histograms
  if (isOptionSet(fParams.getOptions(), kToTHistoUpperLimitParamKey))
  {
    fToTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kToTHistoUpperLimitParamKey);
  }

  // Control histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

bool SignalTransformer::exec()
{
  // Getting the data from event in an apropriate format
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    // Distribute PM Signals per Matrices
    auto pmSigMtxMap = SignalTransformerTools::getPMSigMtxMap(timeWindow);

    // Merging max. 4 PM Signals into a MatrixSignal
    auto mergedSignals = SignalTransformerTools::mergeSignalsAllSiPMs(pmSigMtxMap, fMergingTime, fConstansTree);

    // Saving method invocation
    if (mergedSignals.size() > 0)
    {
      saveMatrixSignals(mergedSignals);
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool SignalTransformer::terminate()
{
  INFO("Signal Transformer finished");
  return true;
}

/**
 * Save objects and make some histograms
 */
void SignalTransformer::saveMatrixSignals(const std::vector<JPetMatrixSignal>& mtxSigVec)
{
  if (mtxSigVec.size() > 0 && fSaveControlHistos)
  {
    getStatistics().fillHistogram("mtxsig_tslot", mtxSigVec.size());
  }
  for (auto& mtxSig : mtxSigVec)
  {
    fOutputEvents->add<JPetMatrixSignal>(mtxSig);

    if (fSaveControlHistos)
    {
      auto scinID = mtxSig.getMatrix().getScin().getID();
      getStatistics().fillHistogram("mtxsig_multi", mtxSig.getPMSignals().size());
      if (mtxSig.getMatrix().getSide() == JPetMatrix::SideA)
      {
        getStatistics().fillHistogram("mtxsig_scin_sideA", scinID);
        getStatistics().fillHistogram("mtxsig_sideA_tot", scinID, mtxSig.getToT());
      }
      else if (mtxSig.getMatrix().getSide() == JPetMatrix::SideB)
      {
        getStatistics().fillHistogram("mtxsig_scin_sideB", scinID);
        getStatistics().fillHistogram("mtxsig_sideB_tot", scinID, mtxSig.getToT());
      }
    }

    if (fSaveCalibHistos)
    {
      // Filling histograms gor each channel in Matrix SiPMs to produce
      // channel offsets with the respect to channel on 1st THR of SiPM mtx pos 1
      auto sigMap = mtxSig.getPMSignals();
      if (sigMap.find(1) != sigMap.end())
      {
        // auto t_1_1 = sigMap.at(1).getPoints(JPetSigCh::Leading, JPetPMSignal::ByThrNum).at(0).getTime();
        auto t_1_1 = sigMap.at(1).getLeadTrailPairs().at(0).first.getTime();

        for (auto pmSig : sigMap)
        {
          auto pairs = pmSig.second.getLeadTrailPairs();
          for (auto pair : pairs)
          {
            auto t_ch_i = pair.first.getTime();
            auto channelID = pair.first.getChannel().getID();
            if (t_1_1 == t_ch_i)
            {
              continue;
            }
            getStatistics().fillHistogram("mtx_channel_offsets", channelID, t_ch_i - t_1_1);
          }
        }
      }
    }
  }
}

void SignalTransformer::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // MatrixSignal multiplicity
  getStatistics().createHistogramWithAxes(new TH1D("mtxsig_multi", "Multiplicity of matched MatrixSignals", 5, 0.5, 5.5),
                                          "Number of PM Signals in Matrix Signal", "Number of Matrix Signals");

  getStatistics().createHistogramWithAxes(new TH1D("mtxsig_tslot", "Number of Matrix Signals in Time Window", 100, 0.5, 100.5),
                                          "Number of Matrix Signals in Time Window", "Number of Time Windows");

  getStatistics().createHistogramWithAxes(
      new TH1D("mtxsig_scin_sideA", "Number of Matrix Signals per scintillator side A", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scin ID", "Number of Matrix Signals");

  getStatistics().createHistogramWithAxes(
      new TH1D("mtxsig_scin_sideB", "Number of Matrix Signals per scintillator side B", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scin ID", "Number of Matrix Signals");

  getStatistics().createHistogramWithAxes(new TH2D("mtxsig_sideA_tot", "Matrix Signal ToT - Side A per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 200, 0.0, fToTHistoUpperLimit),
                                          "Scin ID", "ToT [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("mtxsig_sideB_tot", "Matrix Signal ToT - Side B per scintillator", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 200, 0.0, fToTHistoUpperLimit),
                                          "Scin ID", "ToT [ps]");

  // SiPM offsets if needed
  if (fSaveCalibHistos)
  {
    auto minChannelID = getParamBank().getChannels().begin()->first;
    auto maxChannelID = getParamBank().getChannels().rbegin()->first;

    getStatistics().createHistogramWithAxes(new TH2D("mtx_channel_offsets", "Offset of Channel in Matrix vs. Channel ID",
                                                     maxChannelID - minChannelID + 1, minChannelID - 0.5, maxChannelID + 0.5, 200, -fMergingTime,
                                                     fMergingTime),
                                            "Channel ID", "Offset");
  }
}
