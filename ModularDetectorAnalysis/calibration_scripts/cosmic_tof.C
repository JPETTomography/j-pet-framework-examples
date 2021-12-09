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
 *  @file cosmic_tof.C
 *
 *  @brief Script for reading histograms with SiPM offsets and producing calibraiton json file
 *
 *  This script uses histograms, that are produced by task SignalTransformer.
 *  More detailed description is being created, so stay tuned.
 *
 *  Basic usage:
 *  root> .L cosmic_tof.C
 *  root> cosmic_tof("file_with_calib_histos.root")
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

vector<vector<int>> calibrationIDs = {{214, 201, 201, 1},  {215, 202, 202, 1},  {216, 203, 203, 1},  {217, 204, 204, 1},  {218, 205, 205, 1},
                                      {219, 206, 206, 1},  {220, 207, 207, 1},  {221, 208, 208, 1},  {222, 209, 209, 1},  {223, 210, 210, 1},
                                      {224, 211, 211, 1},  {225, 212, 212, 1},  {226, 213, 213, 1},  {214, 202, 214, -1}, {215, 203, 215, -1},
                                      {216, 204, 216, -1}, {217, 205, 217, -1}, {218, 206, 218, -1}, {219, 207, 219, -1}, {221, 207, 221, -1},
                                      {222, 208, 222, -1}, {223, 209, 223, -1}, {224, 210, 224, -1}, {225, 211, 225, -1}, {226, 212, 226, -1}};

void cosmic_tof(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
                std::string resultDir = "./", int iteration = 0)
{
  TFile* fileCosmicToF = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (!fileCosmicToF->IsOpen())
  {
    std::cout << "Cannot open file " << fileName << std::endl;
    return;
  }

  TGraphErrors* offsetCorrGraph = new TGraphErrors();
  offsetCorrGraph->SetNameTitle("offset_corr_graph", "Difference of peaks means vs. scin ID");
  offsetCorrGraph->GetXaxis()->SetTitle("Scin ID");
  offsetCorrGraph->GetYaxis()->SetTitle("time diff [ps]");
  offsetCorrGraph->SetMinimum(-8000.0);
  offsetCorrGraph->SetMaximum(8000.0);

  TGraphErrors* offsetCorrGraphZoom = new TGraphErrors();
  offsetCorrGraphZoom->SetNameTitle("offset_corr_graph_zoom", "Difference of peaks means vs. sci ID, smaler scale");
  offsetCorrGraphZoom->GetXaxis()->SetTitle("Scin ID");
  offsetCorrGraphZoom->GetYaxis()->SetTitle("time diff [ps]");
  offsetCorrGraphZoom->SetMinimum(-1000.0);
  offsetCorrGraphZoom->SetMaximum(1000.0);

  unsigned graphIt = 0;

  for (auto ids : calibrationIDs)
  {
    int refHistoID = ids.at(0);
    int refScinID = ids.at(1);
    int calibScinID = ids.at(2);

    TH2D* refHisto = dynamic_cast<TH2D*>(fileCosmicToF->Get(Form("cosmic_tof_offset_scin_%d_all", refHistoID)));

    TH1D* offsetHisto =
        refHisto->ProjectionY(Form("offset_scin_%d_%d_%d", refHistoID, refScinID, calibScinID), refScinID - 201 + 1, refScinID - 201 + 1);
    if (offsetHisto->GetEntries() < 1000)
    {
      std::cout << "Not enough entries in the histogram to perform fitting." << std::endl;
      return;
    }

    // Calculating limits for gaussian fit
    offsetHisto->Rebin(2);
    auto max = offsetHisto->GetMaximumBin();
    auto nBins = offsetHisto->GetNbinsX();
    int binRange = (int)(0.5 * nBins / 2);
    auto low = offsetHisto->GetBinCenter(max - binRange);
    auto upp = offsetHisto->GetBinCenter(max + binRange);

    offsetHisto->Fit("gaus", "", "", low, upp);
    auto fitFun = offsetHisto->GetFunction("gaus");
    fitFun->SetLineColor(2);

    // Offset correction (with each iteration should be cloer to zero)
    double offset = fitFun->GetParameter(1) * ids.at(3);
    double offsetError = fitFun->GetParError(1);

    offsetCorrGraph->SetPoint(graphIt, (double)calibScinID, offset);
    offsetCorrGraph->SetPointError(graphIt, 0.0, offsetError);
    offsetCorrGraphZoom->SetPoint(graphIt, (double)calibScinID, offset);
    offsetCorrGraphZoom->SetPointError(graphIt, 0.0, offsetError);

    // Adding result to previous value and  writing to the tree
    offset += tree.get("scin." + to_string(calibScinID) + ".tof_correction", 0.0);
    tree.put("scin." + to_string(calibScinID) + ".tof_correction", offset);

    graphIt++;

    if (saveResult)
    {
      auto name = Form("tof_fit_scin_%d_%d", calibScinID, iteration);

      TCanvas* can = new TCanvas(name, name, 1200, 800);
      offsetHisto->Draw();

      auto legend = new TLegend(0.1, 0.7, 0.35, 0.9);
      legend->AddEntry(offsetHisto, Form("Fit for offset of Scin ID %d", calibScinID), "l");
      legend->AddEntry((TObject*)0, Form("mean = %f +- %f", fitFun->GetParameter(1), fitFun->GetParError(1)), "");
      legend->AddEntry((TObject*)0, Form("sigma = %f +- %f", fitFun->GetParameter(2), fitFun->GetParError(2)), "");
      legend->AddEntry((TObject*)0, Form("Chi2 = %f    ndf = %i", fitFun->GetChisquare(), fitFun->GetNDF()), "");
      legend->Draw("same");

      can->SaveAs(Form("%s/tof_corr_scin_%d_%d.png", resultDir.c_str(), calibScinID, iteration));
    }
  }

  if (saveResult)
  {
    TCanvas* canOffsetCorr = new TCanvas("offset_corr_graph", "offset_corr_graph", 1200, 800);
    string rmsTitle1 = "Iteration " + to_string(iteration) + ":   RMS_X=" + to_string(offsetCorrGraph->GetRMS(1)) +
                       "   RMS_Y=" + to_string(offsetCorrGraph->GetRMS(2));
    offsetCorrGraph->SetTitle(rmsTitle1.c_str());
    offsetCorrGraph->Draw("AP*");
    canOffsetCorr->SaveAs(Form("%s/offset_corr_scin_%d.png", resultDir.c_str(), iteration));

    TCanvas* canOffsetCorrZoom = new TCanvas("offset_corr_scin_zoom", "offset_corr_scin_zoom", 1200, 800);
    string rmsTitle2 = "Iteration " + to_string(iteration) + ":   RMS_X=" + to_string(offsetCorrGraphZoom->GetRMS(1)) +
                       "   RMS_Y=" + to_string(offsetCorrGraphZoom->GetRMS(2));
    offsetCorrGraphZoom->SetTitle(rmsTitle2.c_str());
    offsetCorrGraphZoom->Draw("AP*");
    canOffsetCorrZoom->SaveAs(Form("%s/offset_corr_scin_zoom_%d.png", resultDir.c_str(), iteration));
  }

  // Saving tree into json file
  bpt::write_json(calibJSONFileName, tree);
}
