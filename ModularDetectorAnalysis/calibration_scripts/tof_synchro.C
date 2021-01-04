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

#include <TDirectory.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TFile.h>
#include <TH1D.h>

#include <iostream>
#include <fstream>
#include <vector>

namespace bpt = boost::property_tree;

void tof_synchro(
  std::string fileName, std::string calibJSONFileName = "calibration_constants.json",
  bool saveResult = false, std::string resultDir = "./", int minScinID = 201, int maxScinID = 512
) {

  TFile* fileHitsTOF = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if(file.good()){
    bpt::read_json(calibJSONFileName, tree);
  }

  if(fileHitsTOF->IsOpen()){

    TGraphErrors* tofCorrGraph = new TGraphErrors();
    tofCorrGraph->SetNameTitle("tof_corr", "TOF correction for hits in scintillators");
    tofCorrGraph->GetXaxis()->SetTitle("Scin ID");
    tofCorrGraph->GetYaxis()->SetTitle("correction [ps]");

    unsigned graphIt = 0;

    for(int scinID = minScinID; scinID <= maxScinID; ++scinID) {

      TH1F* adHist = (TH1F*) fileHitsTOF->Get(Form("tdiff_a_d_scin_%d", scinID));
      TH1F* daHist = (TH1F*) fileHitsTOF->Get(Form("tdiff_d_a_scin_%d", scinID));
      adHist->SetLineColor(46);
      daHist->SetLineColor(36);
      adHist->SetLineWidth(2);
      daHist->SetLineWidth(2);

      if(adHist->GetEntries()==0 || daHist->GetEntries()==0) { continue; }

      // Simplified way of getting correction constant
      double tof_corr = 0.5*(adHist->GetMean()-daHist->GetMean());
      double tof_corr_error = 0.5*(adHist->GetMeanError(1)+daHist->GetMeanError(1));

      // Writing result to the tree
      tree.put("scin."+to_string(scinID)+".tof_correction", tof_corr);

      // Filling the graph
      tofCorrGraph->SetPoint(graphIt, (double) scinID, tof_corr);
      tofCorrGraph->SetPointError(graphIt, 0.0, tof_corr_error);
      graphIt++;

      // Drawing canvases
      if(saveResult){
        auto maxA = adHist->GetMaximumBin();
        auto maxD = daHist->GetMaximumBin();
        auto name = Form("tof_fit_scin_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        if(adHist->GetBinContent(maxA) > daHist->GetBinContent(maxD)){
          adHist->Draw();
          daHist->Draw("same");
        } else {
          daHist->Draw();
          adHist->Draw("same");
        }
        can->SaveAs(Form("%s/tof_fit_scin_%d.png", resultDir.c_str(), scinID));
      }
    }

    if(saveResult){
      TCanvas* canTOF = new TCanvas("tof_corr_graph", "tof_corr_graph", 1200, 800);
      tofCorrGraph->Draw("AP*");
      canTOF->SaveAs(Form("%s/tof_corrections.png", resultDir.c_str()));
    }
  }
}
