/**
 * @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file tot_norm_hit.C
 *
 * @brief Script for reading histograms with ToT spectra for scintillators and performing
 * procedure of finding minima of derivative of distribution, that are equivalent to
 * Compton edges.
 *
 * This script uses histograms, that are produced by task "HitFinder".
 * For more detailed description please refer to README.md file.
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

/**
 * The parameters for the edges - depend on the ToT calculation method and units
 */
const double fNominalAnnihilationEdge = 4000000.0;
const double fNominalDeexcitationEdge = 8000000.0;

/**
 * Produce the derivative graph for input histogram
 */
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

/**
 * Iterating the bins of the graph of the histogram derivative
 * to find annihilation and deexcitation edges and then return calculated a,b factors
 */
pair<double, double> getEdges(TH1D* totHist)
{
  double anniEdge = 399000.0, deexEdge = 399000.0;
  double dMinA = 100000.0, dMinB = 10000.0;
  int anniBin = totHist->GetMaximumBin();
  TGraph* devGraph = getDerivativeGraph(totHist);
  devGraph->Draw();
  for (int bin = totHist->GetMaximumBin(); bin < totHist->GetNbinsX() - 5; ++bin)
  {
    double deriv_x = devGraph->GetX()[bin];
    double deriv_y = devGraph->GetY()[bin];
    if (deriv_y < dMinA)
    {
      dMinA = deriv_y;
      anniEdge = deriv_x;
      anniBin = bin;
    }
  }
  for (int bin = anniBin + 15; bin < totHist->GetNbinsX() - 2; ++bin)
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

/**
 * Tool for drawing the result and saving to PNG image
 */
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

/**
 * The main method of the macro, that iterates over the projections of the input histogram
 * and searches for the Compton edges. The output is printed into the JSON file
 */
void tot_norm(string fileName, string calibJSONFileName = "calibration_constants.json", bool saveResult = false, string resultDir = "./",
              int minScinID = 201, int maxScinID = 512)
{
  TFile* fileScinTOT = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileScinTOT->IsOpen())
  {
    TH2D* allScins = dynamic_cast<TH2D*>(fileScinTOT->Get("hit_tot_scin"));

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {
      TH1D* totHist = allScins->ProjectionY(Form("tot_scin_%d", scinID), scinID - 200, scinID - 200);
      totHist->SetLineWidth(2);
      // Rebbining histogram, so procedure that looks for a minimum
      // of a derivative does not get stuck on fluctuations of spectra
      totHist->Rebin(2);

      // Skip this scin if not enought entries
      if (totHist->GetEntries() > 100)
      {
        auto factors = getEdges(totHist);
        double temp = fNominalAnnihilationEdge * factors.first / (factors.second - factors.first);
        double factorA = temp / factors.first;
        double factorB = fNominalDeexcitationEdge - temp;
        tree.put("scin." + to_string(scinID) + ".tot_factor_a", factorA);
        tree.put("scin." + to_string(scinID) + ".tot_factor_b", factorB);
        if (saveResult)
        {
          savePlotPNG(totHist, factors.first, factors.second, resultDir);
        }
      }
    }

    // Saving tree into json file
    bpt::write_json(calibJSONFileName, tree);
  }
  fileScinTOT->Close();
}
