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
 *  @file matrix_offests.C
 *
 *  @brief Script for reading histograms with SiPM offsets and producing calibraiton json file
 *
 *  This script uses histograms, that are produced by task SignalTransformer.
 *  More detailed description is being created, so stay tuned.
 *
 *  Basic usage:
 *  root> .L matrix_offsets.C
 *  root> matrix_offests("file_with_calib_histos.root")
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

void matrix_offsets(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
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

    TH2D* offsetsHisto = dynamic_cast<TH2D*>(fileMtxSynchro->Get("evtcat_channel_offsets"));

    for (int channelID = minChannelID; channelID <= maxChannelID; ++channelID)
    {

      TH1D* channelHisto =
          offsetsHisto->ProjectionY(Form("offset_channel_%d", channelID), channelID - minChannelID + 1, channelID - minChannelID + 1);
      if (channelHisto->GetEntries() < 100)
      {
        continue;
      }

      // Calculating limits for gaussian fit
      auto max = channelHisto->GetMaximumBin();
      auto nBins = channelHisto->GetNbinsX();
      int binRange = (int)(0.1 * nBins);
      auto low = channelHisto->GetBinCenter(max - binRange);
      auto upp = channelHisto->GetBinCenter(max + binRange);

      channelHisto->Fit("gaus", "", "", low, upp);
      auto fitFun = channelHisto->GetFunction("gaus");
      fitFun->SetLineColor(2);

      double offset = fitFun->GetParameter(1);
      offset += tree.get("channel_offests." + to_string(channelID), 0.0);
      tree.put("channel_offests." + to_string(channelID), offset);

      if (saveResult)
      {
        auto name = Form("fit_result_channel_%d", channelID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        channelHisto->Draw();

        auto legend = new TLegend(0.1, 0.7, 0.35, 0.9);
        legend->AddEntry(channelHisto, Form("Fit for offset of Channel ID %d", channelID), "l");
        legend->AddEntry((TObject*)0, Form("mean = %f +- %f", fitFun->GetParameter(1), fitFun->GetParError(1)), "");
        legend->AddEntry((TObject*)0, Form("sigma = %f +- %f", fitFun->GetParameter(2), fitFun->GetParError(2)), "");
        legend->AddEntry((TObject*)0, Form("Chi2 = %f    ndf = %i", fitFun->GetChisquare(), fitFun->GetNDF()), "");
        legend->Draw("same");

        can->SaveAs(Form("%s/offset_channel_%d.png", resultDir.c_str(), channelID));
      }
    }
  }

  // Saving tree into json file
  bpt::write_json(calibJSONFileName, tree);
}
