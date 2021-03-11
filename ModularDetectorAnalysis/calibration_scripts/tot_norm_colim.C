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

#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace bpt = boost::property_tree;

const double fNominalAnnihilationEdge = 75000.0;
const double fNominalDeexcitationEdge = 130000.0;

TGraph* getDerivativeGraph(TH1D* histo)
{
  TGraph* derivGraph = new TGraph();
  for (int bin = 2; bin < histo->GetNbinsX(); ++bin)
  {
    double dh = histo->GetBinCenter(bin) - histo->GetBinCenter(bin - 1);
    double df = histo->GetBinContent(bin) - histo->GetBinContent(bin - 1);
    double deriv_x = 0.5 * (histo->GetBinCenter(bin) + histo->GetBinCenter(bin - 1));
    double deriv_y = df / dh;
    derivGraph->SetPoint(bin - 2, deriv_x, deriv_y);
  }
  return derivGraph;
}

// Iterating graph of histogram derivative to find annihilation and deexcitation edges
// and then calculate a,b factors
pair<double, double> getEdges(TH1D* totHist)
{
  double anniEdge = 399000.0, deexEdge = 399000.0;
  double dMinA = 100000.0, dMinB = 10000.0;
  int anniBin = totHist->GetMaximumBin();
  TGraph* devGraph = getDerivativeGraph(totHist);
  devGraph->Draw();
  for (int bin = totHist->GetMaximumBin(); bin < totHist->GetNbinsX() - 2 && bin < anniBin + 5; ++bin)
  {
    double deriv_x = devGraph->GetX()[bin];
    double deriv_y = devGraph->GetY()[bin];
    if (deriv_y < dMinA)
    {
      dMinA = deriv_y;
      anniEdge = deriv_x;
      anniBin = bin;
    }
    // else
    // {
    //   break;
    // }
  }
  for (int bin = anniBin; bin < totHist->GetNbinsX() - 2; ++bin)
  {
    double deriv_x = devGraph->GetX()[bin];
    double deriv_y = devGraph->GetY()[bin];
    if (deriv_x > 1.25 * anniEdge && deriv_y < dMinB)
    {
      dMinB = deriv_y;
      deexEdge = deriv_x;
    }
  }

  return make_pair(anniEdge, deexEdge);
}

void savePlotPNG(TH1D* totHist, double anniEdge, double deexEdge, string resultDir)
{
  int max = totHist->GetMaximum();

  TLine* l1 = new TLine(anniEdge, 0.0, anniEdge, max);
  TLine* l2 = new TLine(deexEdge, 0.0, deexEdge, max);

  l1->SetLineWidth(2);
  l2->SetLineWidth(2);
  l1->SetLineColor(kRed);
  l2->SetLineColor(kOrange);

  auto name = totHist->GetName();
  TCanvas* can = new TCanvas(name, name, 1200, 800);
  totHist->Draw();
  l1->Draw("same");
  l2->Draw("same");
  can->SaveAs(Form("%s/%s.png", resultDir.c_str(), name));
}

void tot_norm(string fileName, string calibJSONFileName = "calibration_constants.json", bool saveResult = false, string resultDir = "./",
              int minSiPMID = 401, int maxSiPMID = 2896)
{
  TFile* fileSiPMTOT = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileSiPMTOT->IsOpen())
  {
    TH2F* allTHR1 = dynamic_cast<TH2F*>(fileSiPMTOT->Get("tot_sipm_id_thr1"));
    TH2F* allTHR2 = dynamic_cast<TH2F*>(fileSiPMTOT->Get("tot_sipm_id_thr2"));

    for (int sipmID = minSiPMID; sipmID <= maxSiPMID; ++sipmID)
    {
      TH1D* totTHR1 = allTHR1->ProjectionY(Form("tot_thr1_sipm_%d", sipmID), sipmID - minSiPMID + 1, sipmID - minSiPMID + 1);
      TH1D* totTHR2 = allTHR2->ProjectionY(Form("tot_thr2_sipm_%d", sipmID), sipmID - minSiPMID + 1, sipmID - minSiPMID + 1);
      totTHR1->SetLineWidth(2);
      totTHR2->SetLineWidth(2);
      // Rebbining histogram, so procedure that looks for a minimum
      // of a derivative does not get stuck on fluctuations of spectra
      totTHR1->Rebin(4);
      totTHR2->Rebin(4);

      // Skip this sipm if not enought entries
      if (totTHR1->GetEntries() > 100)
      {
        auto factors = getEdges(totTHR1);
        double temp = fNominalAnnihilationEdge * factors.first / (factors.second - factors.first);
        double factorA = temp / factors.first;
        double factorB = fNominalDeexcitationEdge - temp;
        tree.put("sipm." + to_string(sipmID) + ".tot_factor_thr1_a", factorA);
        tree.put("sipm." + to_string(sipmID) + ".tot_factor_thr1_b", factorB);
        if (saveResult)
        {
          savePlotPNG(totTHR1, factors.first, factors.second, resultDir);
        }
      }

      if (totTHR2->GetEntries() > 100)
      {
        auto factors = getEdges(totTHR2);
        double temp = fNominalAnnihilationEdge * factors.first / (factors.second - factors.first);
        double factorA = temp / factors.first;
        double factorB = fNominalDeexcitationEdge - temp;
        tree.put("sipm." + to_string(sipmID) + ".tot_factor_thr2_a", factorA);
        tree.put("sipm." + to_string(sipmID) + ".tot_factor_thr2_b", factorB);
        if (saveResult)
        {
          savePlotPNG(totTHR2, factors.first, factors.second, resultDir);
        }
      }
    }

    // Saving tree into json file
    bpt::write_json(calibJSONFileName, tree);
  }
  fileSiPMTOT->Close();
}
