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

    TH2F* tof2Hist = dynamic_cast<TH2F*>(fileHitsTOF->Get("ap_tof_scin"));

    TGraphErrors* tofCorrGraph = new TGraphErrors();
    tofCorrGraph->SetNameTitle("tof_corr", "Secondary TOF correction for hits in scintillators");
    tofCorrGraph->GetXaxis()->SetTitle("Scin ID");
    tofCorrGraph->GetYaxis()->SetTitle("correction [ps]");
    tofCorrGraph->SetMinimum(-15000.0);
    tofCorrGraph->SetMaximum(15000.0);

    unsigned graphIt = 0;

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {
      TH1D* proj = tof2Hist->ProjectionY(Form("tof2_%d", scinID), scinID - 200, scinID - 200);
      proj->Rebin(2);
      proj->SetLineColor(46);
      proj->SetLineWidth(2);

      if (proj->GetEntries() < 100)
      {
        continue;
      }

      // Calculating limits for gaussian fit
      auto nBins = proj->GetNbinsX();
      int binRange = (int)(0.075 * nBins);

      auto aMax = proj->GetMaximumBin();
      auto aLow = proj->GetBinCenter(aMax - binRange);
      auto aUpp = proj->GetBinCenter(aMax + binRange);

      proj->Fit("gaus", "", "", aLow, aUpp);
      auto fitFun = proj->GetFunction("gaus");
      fitFun->SetLineColor(2);
      fitFun->SetLineWidth(2);

      // double tofCorr = (fitFun->GetParameter(1) - fitFun->GetParameter(1)) / 2.0;
      // double tofCorrError = (fitFun->GetParError(1) + fitFun->GetParError(1)) / 2.0;

      // Writing result to the tree
      double tofCorr = fitFun->GetParameter(1) / 2.0;
      if (scinID > 356)
      {
        tofCorr = -1.0 * fitFun->GetParameter(1) / 2.0;
      }

      // If a constatnt for TOF correction already exists in the tree, adding to new one
      tofCorr += tree.get("scin." + to_string(scinID) + ".tof_correction_2", 0.0);
      tree.put("scin." + to_string(scinID) + ".tof_correction_2", tofCorr);

      double tofCorrError = fitFun->GetParError(1) / 2.0;
      tofCorrGraph->SetPoint(graphIt, (double)scinID, tofCorr);
      tofCorrGraph->SetPointError(graphIt, 0.0, tofCorrError);

      graphIt++;

      // Drawing canvases
      if (saveResult)
      {
        auto name = Form("tof2_fit_scin_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        proj->Draw();
        // can->SaveAs(Form("%s/tof2_corr_scin_%d.png", resultDir.c_str(), scinID));
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
