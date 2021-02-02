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
 *  @file tof_synchro.C
 *
 *  @brief Script for reading histograms with selected annihilation and deexcitation events
 *  time differences and calculating constatns for Time of Flight synchronization,
 *  that can be used for correcting hit time.
 *
 *  This script uses histograms, that are produced by task "EventCategorizer".
 *  For more detailed description please refer to USAGE.md file.
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace bpt = boost::property_tree;

void tof_synchro(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
                 std::string resultDir = "./", int minScinID = 201, int maxScinID = 512)
{

  TFile* fileHitsTOF = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileHitsTOF->IsOpen())
  {

    TH2F* annhPerScin = dynamic_cast<TH2F*>(fileHitsTOF->Get("tdiff_annih_scin"));
    TH2F* deexPerScin = dynamic_cast<TH2F*>(fileHitsTOF->Get("tdiff_deex_scin"));

    TGraphErrors* tofCorrGraph = new TGraphErrors();
    tofCorrGraph->SetNameTitle("tof_corr", "TOF correction for hits in scintillators");
    tofCorrGraph->GetXaxis()->SetTitle("Scin ID");
    tofCorrGraph->GetYaxis()->SetTitle("correction [ps]");
    tofCorrGraph->SetMinimum(-6000.0);
    tofCorrGraph->SetMaximum(1000.0);
    unsigned graphIt = 0;

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {

      TH1D* annhHist = annhPerScin->ProjectionY(Form("anni_scin_%d", scinID), scinID - 200, scinID - 200);
      TH1D* deexHist = deexPerScin->ProjectionY(Form("deex_scin_%d", scinID), scinID - 200, scinID - 200);
      annhHist->Rebin(4);
      deexHist->Rebin(4);
      annhHist->SetLineColor(46);
      deexHist->SetLineColor(38);
      annhHist->SetLineWidth(2);
      deexHist->SetLineWidth(2);

      if (annhHist->GetEntries() < 100 || deexHist->GetEntries() < 100)
      {
        continue;
      }

      // Calculating limits for gaussian fit
      auto nBins = annhHist->GetNbinsX();
      int binRange = (int)(0.4 * nBins);

      auto aMax = annhHist->GetMaximumBin();
      auto aLow = annhHist->GetBinCenter(aMax - binRange);
      auto aUpp = annhHist->GetBinCenter(aMax + binRange);

      auto dMax = deexHist->GetMaximumBin();
      auto dLow = deexHist->GetBinCenter(dMax - binRange);
      auto dUpp = deexHist->GetBinCenter(dMax + binRange);

      annhHist->Fit("gaus", "", "", aLow, aUpp);
      auto aFitFun = annhHist->GetFunction("gaus");
      aFitFun->SetLineColor(2);
      aFitFun->SetLineWidth(2);

      deexHist->Fit("gaus", "", "", dLow, dUpp);
      auto dFitFun = deexHist->GetFunction("gaus");
      dFitFun->SetLineColor(4);
      dFitFun->SetLineWidth(2);

      // double tof_corr = 0.5 * (annhHist->GetMean() - deexHist->GetMean());
      // double tof_corr_error = 0.5 * (annhHist->GetMeanError(1) + deexHist->GetMeanError(1));
      double tofCorr = 0.5 * (aFitFun->GetParameter(1) - dFitFun->GetParameter(1));
      double tofCorrError = 0.5 * (aFitFun->GetParError(1) + dFitFun->GetParError(1));

      // If a constatnt for TOF correction already exists in the tree, adding to new one
      tofCorr += tree.get("scin." + to_string(scinID) + ".tof_correction", 0.0);

      // Writing result to the tree
      tree.put("scin." + to_string(scinID) + ".tof_correction", tofCorr);

      // Filling the graph
      tofCorrGraph->SetPoint(graphIt, (double)scinID, tofCorr);
      tofCorrGraph->SetPointError(graphIt, 0.0, tofCorrError);
      graphIt++;

      // Drawing canvases
      if (saveResult)
      {
        auto maxA = annhHist->GetMaximumBin();
        auto maxD = deexHist->GetMaximumBin();
        auto name = Form("tof_fit_scin_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        if (annhHist->GetBinContent(maxA) > deexHist->GetBinContent(maxD))
        {
          annhHist->Draw();
          deexHist->Draw("same");
        }
        else
        {
          deexHist->Draw();
          annhHist->Draw("same");
        }
        can->SaveAs(Form("%s/tof_corr_scin_%d.png", resultDir.c_str(), scinID));
      }
    }

    if (saveResult)
    {
      TCanvas* canTOF = new TCanvas("tof_corr_graph", "tof_corr_graph", 1200, 800);
      tofCorrGraph->Draw("AP*");
      canTOF->SaveAs(Form("%s/tof_corr_scin.png", resultDir.c_str()));
    }

    // Saving tree into json file
    bpt::write_json(calibJSONFileName, tree);
  }
}
