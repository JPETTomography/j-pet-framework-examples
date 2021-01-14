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

    TH2F *tofPerScin = dynamic_cast<TH2F*>(fileHitsTOF->Get("ap_tof_conv_corr_scin"));

    TGraphErrors* tofMeanGraph = new TGraphErrors();
    tofMeanGraph->SetNameTitle("tof_corr", "TOF correction per Scin (mean from fit)");
    tofMeanGraph->GetXaxis()->SetTitle("Scin ID");
    tofMeanGraph->GetYaxis()->SetTitle("Correction [ps]");

    TGraphErrors* tofStdDevGraph = new TGraphErrors();
    tofStdDevGraph->SetNameTitle("tof_stddev_scin", "TOF StdDev from gaussian fit per Scin");
    tofStdDevGraph->GetXaxis()->SetTitle("Scin ID");
    tofStdDevGraph->GetYaxis()->SetTitle("TOF StdDev [ps]");

    TGraphErrors* tofConstGraph = new TGraphErrors();
    tofConstGraph->SetNameTitle("tof_const_scin", "TOF const param from gaussian fit per Scin");
    tofConstGraph->GetXaxis()->SetTitle("Scin ID");
    tofConstGraph->GetYaxis()->SetTitle("N");

    unsigned graphIt = 0;

    for(int scinID = minScinID; scinID <= maxScinID; ++scinID) {

      TH1D* tofHist = tofPerScin->ProjectionY(Form("tof_scin_%d", scinID), scinID-200, scinID-200);
      if(tofHist->GetEntries() < 100) { continue; }

      // Calculating limits for gaussian fit
      auto max = tofHist->GetMaximumBin();
      auto nBins = tofHist->GetNbinsX();
      int binRange = (int)(0.08*nBins);
      auto low = tofHist->GetBinCenter(max-binRange);
      auto upp = tofHist->GetBinCenter(max+binRange);

      tofHist->Fit("gaus", "", "", low, upp);
      auto fitFun = tofHist->GetFunction("gaus");
      fitFun->SetLineColor(2);

      // Writing result to the tree
      tree.put("scin."+to_string(scinID)+".tof_correction", fitFun->GetParameter(1));

      // Filling graphs
      tofConstGraph->SetPoint(graphIt, (double) scinID, fitFun->GetParameter(0));
      tofConstGraph->SetPointError(graphIt, 0.0, fitFun->GetParError(0));
      tofMeanGraph->SetPoint(graphIt, (double) scinID, fitFun->GetParameter(1));
      tofMeanGraph->SetPointError(graphIt, 0.0, fitFun->GetParError(1));
      tofStdDevGraph->SetPoint(graphIt, (double) scinID, fitFun->GetParameter(2));
      tofStdDevGraph->SetPointError(graphIt, 0.0, fitFun->GetParError(2));
      graphIt++;

      if(saveResult){
        auto name = Form("tof_fit_scin_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        tofHist->Draw();

        auto legend = new TLegend(0.1, 0.6, 0.4, 0.9);
        legend->AddEntry(tofHist, Form("TOF offset Scin ID %d with fit", scinID), "l");
        legend->AddEntry((TObject*)0, Form("mean = %f +- %f", fitFun->GetParameter(1), fitFun->GetParError(1)), "");
        legend->AddEntry((TObject*)0, Form("sigma = %f +- %f", fitFun->GetParameter(2), fitFun->GetParError(2)), "");
        legend->AddEntry((TObject*)0, Form("Chi2 = %f    ndf = %i", fitFun->GetChisquare(), fitFun->GetNDF()), "");
        legend->Draw("same");

        can->SaveAs(Form("%s/tof_fit_scin_%d.png", resultDir.c_str(), scinID));
      }
    }

    if(saveResult){
      TCanvas* can1 = new TCanvas("tof_const_graph", "tof_const_graph", 1200, 800);
      tofConstGraph->Draw("AP*");
      can1->SaveAs(Form("%s/tof_const_param.png", resultDir.c_str()));

      TCanvas* can2 = new TCanvas("tof_corr_graph", "tof_corr_graph", 1200, 800);
      tofMeanGraph->Draw("AP*");
      can2->SaveAs(Form("%s/tof_corrections.png", resultDir.c_str()));

      TCanvas* can3 = new TCanvas("tof_stddev_graph", "tof_stddev_graph", 1200, 800);
      tofStdDevGraph->Draw("AP*");
      can3->SaveAs(Form("%s/tof_stddev.png", resultDir.c_str()));
    }

    // Saving tree into json file
    bpt::write_json(calibJSONFileName, tree);
  }
}
