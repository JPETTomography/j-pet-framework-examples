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
                 std::string resultDir = "./", int iteration = 0, int minScinID = 201, int maxScinID = 512)
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

    TH2D* annhPerScin = dynamic_cast<TH2D*>(fileHitsTOF->Get("tdiff_anni_scin"));
    TH2D* deexPerScin = dynamic_cast<TH2D*>(fileHitsTOF->Get("tdiff_deex_scin"));

    TGraphErrors* tofCorrGraph = new TGraphErrors();
    tofCorrGraph->SetNameTitle("tof_corr", "TOF correction for hits in scintillators");
    tofCorrGraph->GetXaxis()->SetTitle("Scin ID");
    tofCorrGraph->GetYaxis()->SetTitle("correction [ps]");
    tofCorrGraph->SetMinimum(-15000.0);
    tofCorrGraph->SetMaximum(15000.0);

    TGraphErrors* peakDiffGraph = new TGraphErrors();
    peakDiffGraph->SetNameTitle("peak_diff_graph", "Difference of peaks means vs. sci ID");
    peakDiffGraph->GetXaxis()->SetTitle("Scin ID");
    peakDiffGraph->GetYaxis()->SetTitle("time diff [ps]");
    peakDiffGraph->SetMinimum(-8000.0);
    peakDiffGraph->SetMaximum(8000.0);

    TGraphErrors* peakDiffGraphZoom = new TGraphErrors();
    peakDiffGraphZoom->SetNameTitle("peak_diff_graph_zoom", "Difference of peaks means vs. sci ID, smaler scale");
    peakDiffGraphZoom->GetXaxis()->SetTitle("Scin ID");
    peakDiffGraphZoom->GetYaxis()->SetTitle("time diff [ps]");
    peakDiffGraphZoom->SetMinimum(-1000.0);
    peakDiffGraphZoom->SetMaximum(1000.0);

    TGraphErrors* meanGraph = new TGraphErrors();
    meanGraph->SetNameTitle("mean_graph", "Mean of distributions vs. scin ID");
    meanGraph->GetXaxis()->SetTitle("Scin ID");
    meanGraph->GetYaxis()->SetTitle("mean [ps]");
    meanGraph->SetMinimum(0.0);
    meanGraph->SetMaximum(7000.0);

    unsigned graphIt = 0;

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {
      TH1D* annhHist = annhPerScin->ProjectionY(Form("anni_scin_%d", scinID), scinID - 200, scinID - 200);
      TH1D* deexHist = deexPerScin->ProjectionY(Form("deex_scin_%d", scinID), scinID - 200, scinID - 200);
      annhHist->Rebin(2);
      deexHist->Rebin(2);
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
      int binRange = (int)(0.05 * nBins);

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

      double tofCorr = (fabs(aFitFun->GetParameter(1)) - fabs(dFitFun->GetParameter(1))) / 2.0;
      double tofCorrError = (aFitFun->GetParError(1) + dFitFun->GetParError(1)) / 2.0;

      peakDiffGraph->SetPoint(graphIt, (double)scinID, tofCorr);
      peakDiffGraph->SetPointError(graphIt, 0.0, tofCorrError);
      peakDiffGraphZoom->SetPoint(graphIt, (double)scinID, tofCorr);
      peakDiffGraphZoom->SetPointError(graphIt, 0.0, tofCorrError);

      meanGraph->SetPoint(graphIt, (double)scinID, (aFitFun->GetParameter(1) + dFitFun->GetParameter(1)) / 2.0);
      meanGraph->SetPointError(graphIt, 0.0, tofCorrError);

      // If a constatnt for TOF correction already exists in the tree, adding to new one
      tofCorr += tree.get("scin." + to_string(scinID) + ".tof_correction", 0.0);

      // Writing result to the tree
      tree.put("scin." + to_string(scinID) + ".tof_correction", tofCorr);

      tofCorrGraph->SetPoint(graphIt, (double)scinID, tofCorr);
      tofCorrGraph->SetPointError(graphIt, 0.0, tofCorrError);

      graphIt++;

      // Drawing canvases
      if (saveResult)
      {
        auto maxA = annhHist->GetMaximumBin();
        auto maxD = deexHist->GetMaximumBin();
        auto name = Form("tof_fit_scin_%d_%d", scinID, iteration);
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
        can->SaveAs(Form("%s/tof_corr_scin_%d_%d.png", resultDir.c_str(), scinID, iteration));
      }
    }

    if (saveResult)
    {
      TCanvas* canTOF = new TCanvas("tof_corr_graph", "tof_corr_graph", 1200, 800);
      tofCorrGraph->Draw("AP*");
      canTOF->SaveAs(Form("%s/tof_corr_scin_%d.png", resultDir.c_str(), iteration));

      TCanvas* canPeakDiff = new TCanvas("peak_diff_graph", "peak_diff_graph", 1200, 800);
      string rmsTitle1 = "Iteration " + to_string(iteration) + ":   RMS_X=" + to_string(peakDiffGraph->GetRMS(1)) +
                         "   RMS_Y=" + to_string(peakDiffGraph->GetRMS(2));
      peakDiffGraph->SetTitle(rmsTitle1.c_str());
      peakDiffGraph->Draw("AP*");
      canPeakDiff->SaveAs(Form("%s/peak_diff_scin_%d.png", resultDir.c_str(), iteration));

      TCanvas* canPeakDiffZoom = new TCanvas("peak_diff_scin_zoom", "peak_diff_scin_zoom", 1200, 800);
      string rmsTitle2 = "Iteration " + to_string(iteration) + ":   RMS_X=" + to_string(peakDiffGraphZoom->GetRMS(1)) +
                         "   RMS_Y=" + to_string(peakDiffGraphZoom->GetRMS(2));
      peakDiffGraphZoom->SetTitle(rmsTitle2.c_str());
      peakDiffGraphZoom->Draw("AP*");
      canPeakDiffZoom->SaveAs(Form("%s/peak_diff_scin_zoom_%d.png", resultDir.c_str(), iteration));

      TCanvas* canMean = new TCanvas("mean_graph", "mean_graph", 1200, 800);
      meanGraph->Draw("AP*");
      canMean->SaveAs(Form("%s/mean_scin_%d.png", resultDir.c_str(), iteration));
    }

    // Saving tree into json file
    bpt::write_json(calibJSONFileName, tree);
    bpt::write_json(Form("%s/tof_per_it_%d.json", resultDir.c_str(), iteration), tree);
  }
}
