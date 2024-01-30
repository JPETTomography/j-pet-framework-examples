/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
 *  @file channel_offests.C
 *
 *  @brief Script for reading histograms with channel offsets and producing calibraiton json file
 *
 *  This script uses histograms, that are produced by task SignalTransformer.
 *  Channels that belong to SiPM in the same mattix are synchronized to channel
 *  on THR1 of SiPM with matrix position 1.
 *
 *  Basic usage:
 *  root> .L channel_offsets.C
 *  root> channel_offests("file_with_calib_histos.root")
 *  -- this will produce file "calibration_constants.json" with the results. If the
 *  file exists, the result of this calibration will be appended to the existing tree.
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLine.h>
#include <TMath.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace bpt = boost::property_tree;

void channel_offsets(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
                     std::string resultDir = "./", int minChannelID = 2100, int maxChannelID = 7140)
{

  TFile* fileMtxSynchro = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileMtxSynchro->IsOpen())
  {
    TH2D* channelOffsets = dynamic_cast<TH2D*>(fileMtxSynchro->Get("mtx_channel_offsets"));

    for (int channelID = minChannelID; channelID <= maxChannelID; ++channelID)
    {

      TH1D* offsetHist =
          channelOffsets->ProjectionY(Form("offset_channel_%d", channelID), channelID - minChannelID + 1, channelID - minChannelID + 1);
      if (offsetHist->GetEntries() < 100)
      {
        continue;
      }

      // Offset is a time indicated by bin with highest number of counts
      double offset = offsetHist->GetBinCenter(offsetHist->GetMaximumBin());
      tree.put("channel_offests." + to_string(channelID), offset);

      if (saveResult)
      {
        auto name = Form("offset_result_channel_%d", channelID);

        TCanvas* can = new TCanvas(name, name, 900, 720);
        offsetHist->Draw();

        TLine* line = new TLine(offset, offsetHist->GetMinimum(), offset, offsetHist->GetMaximum());
        line->SetLineWidth(2);
        line->SetLineColor(kRed);
        line->Draw("same");

        can->SaveAs(Form("%s/offset_channel_%d.png", resultDir.c_str(), channelID));
      }
    }
  }

  // Saving tree into json file
  bpt::write_json(calibJSONFileName, tree);
}
