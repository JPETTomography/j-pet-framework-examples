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
 *  @file hit_tdiff_collim.C
 *
 *  @brief Script for reading histograms with hit time differences to and produced
 *  calibraiton constants for A-B centering and estimation of effective light velocity
 *  in scintillators
 *
 *  This script uses histograms, that are produced by task HitFinder.
 *  More detailed description is being created, so stay tuned.
 *
 *  Basic usage:
 *  root> .L hit_tdiff_effvel.C
 *  root> hit_tdiff_effvel("file_with_hits_tdiff.root")
 *  -- this will produce file "calibration_constants.json" with the results. If the
 *  file exists, the result of this calibration will be appended to the existing tree.
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLine.h>
#include <TMath.h>
#include <TRandom.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace bpt = boost::property_tree;

typedef std::pair<std::vector<double>, std::vector<double>> Points;
typedef std::pair<double, double> Point;

const int fScinOffset = 200;

void hit_tdiff(std::string fileName, std::string calibJSONFileName = "calibration_constants.json",
               std::string velocityCalibFile = "with_velocity.json", bool saveResult = false, std::string resultDir = "./", int minScinID = 201,
               int maxScinID = 512)
{
  TFile* fileHitsAB = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  // File with additional information with light velocities
  bpt::ptree velocityTree;
  ifstream velocityFile(velocityCalibFile.c_str());
  if (velocityFile.good())
  {
    bpt::read_json(velocityFile, velocityTree);
  }

  if (fileHitsAB->IsOpen())
  {
    TH2F* hitTDiffAll = dynamic_cast<TH2F*>(fileHitsAB->Get("hit_tdiff_scin"));

    TGraphErrors* bCorrGraph = new TGraphErrors();
    bCorrGraph->SetNameTitle("b_corr", "B side signals correction for scintillators");
    bCorrGraph->GetXaxis()->SetTitle("Scin ID");
    bCorrGraph->GetYaxis()->SetTitle("correction [ps]");

    unsigned graphIt = 0;

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {
      TH1D* ab_tdiff = hitTDiffAll->ProjectionY(Form("ab_tdiff_scin_%d", scinID), scinID - fScinOffset, scinID - fScinOffset);
      ab_tdiff->SetLineColor(kBlue);
      ab_tdiff->SetLineWidth(2);

      if (ab_tdiff->GetEntries() < 1000)
      {
        continue;
      }

      // Calculating limits for gaussian fit
      auto max = ab_tdiff->GetMaximumBin();
      auto nBins = ab_tdiff->GetNbinsX();
      int binRange = (int)(0.05 * nBins);
      auto low = ab_tdiff->GetBinCenter(max - binRange);
      auto upp = ab_tdiff->GetBinCenter(max + binRange);

      ab_tdiff->Fit("gaus", "", "", low, upp);
      auto fitFun = ab_tdiff->GetFunction("gaus");
      fitFun->SetLineColor(kRed);

      double b_corr = fitFun->GetParameter(1);
      double b_corr_error = fitFun->GetParError(1);

      tree.put("scin." + to_string(scinID) + ".b_correction", b_corr);
      bCorrGraph->SetPoint(graphIt, (double)scinID, b_corr);
      bCorrGraph->SetPointError(graphIt, 0.0, b_corr_error);
      graphIt++;

      // copying velocity value read from provided file
      double velocity = velocityTree.get("scin." + to_string(scinID) + ".eff_velocity", 0.0);
      tree.put("scin." + to_string(scinID) + ".eff_velocity", velocity);

      if (saveResult)
      {
        auto name = Form("fit_result_scin_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        ab_tdiff->Draw();

        auto legend = new TLegend(0.1, 0.7, 0.35, 0.9);
        legend->AddEntry(ab_tdiff, Form("Measurement with collimator - hits time difference scin %d", scinID), "l");
        legend->AddEntry((TObject*)0, Form("mean = %f +- %f", fitFun->GetParameter(1), fitFun->GetParError(1)), "");
        legend->AddEntry((TObject*)0, Form("sigma = %f +- %f", fitFun->GetParameter(2), fitFun->GetParError(2)), "");
        legend->AddEntry((TObject*)0, Form("Chi2 = %f    ndf = %i", fitFun->GetChisquare(), fitFun->GetNDF()), "");
        legend->Draw("same");

        // Saving canvas in the specified directory
        can->SaveAs(Form("%s/hit_tdiff_scin_%d.png", resultDir.c_str(), scinID));
      }
    }

    if (saveResult)
    {
      TCanvas* canBcorr = new TCanvas("b_corr_graph", "b_corr_graph", 1200, 800);
      bCorrGraph->Draw("AP*");
      canBcorr->SaveAs(Form("%s/b_corrections.png", resultDir.c_str()));
    }
  }

  // Saving tree into json file
  bpt::write_json(calibJSONFileName, tree);
}
