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

const double fNominalAnnihilationEdge = 140000.0;
const double fNominalDeexcitationEdge = 200000.0;

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

void tot_norm(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
              std::string resultDir = "./", int minScinID = 201, int maxScinID = 512)
{
  TFile* fileHitsTOT = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileHitsTOT->IsOpen())
  {
    TH2F* totPerScin = dynamic_cast<TH2F*>(fileHitsTOT->Get("hit_tot_scin"));

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {
      TH1D* totHist = totPerScin->ProjectionY(Form("tot_scin_%d", scinID), scinID - 200, scinID - 200);
      // Rebbining histogram from 200 bins to 100, so procedure that looks for a minimum
      // of a derivative does not get stuck on fluctuations of spectra
      totHist->Rebin(2);
      totHist->SetLineWidth(2);

      // Skip this scin if not enought entries
      if (totHist->GetEntries() < 100)
      {
        continue;
      }

      // Iterating graph of histogram derivative to find annihilation and deexcitation edges
      double anniEdge = 399000.0, deexEdge = 399000.0;
      double dMinA = 10000.0, dMinB = 10000.0;
      TGraph* devGraph = getDerivativeGraph(totHist);
      for (int bin = 0; bin < totHist->GetNbinsX() - 2; ++bin)
      {
        double deriv_x, deriv_y;
        devGraph->GetPoint(bin, deriv_x, deriv_y);
        if (deriv_y < dMinA)
        {
          dMinA = deriv_y;
          anniEdge = deriv_x;
        }
        if (deriv_x > 1.5 * anniEdge && deriv_y < dMinB)
        {
          dMinB = deriv_y;
          deexEdge = deriv_x;
        }
      }

      double temp = fNominalAnnihilationEdge * anniEdge / (deexEdge - anniEdge);
      double factorA = temp / anniEdge;
      double factorB = fNominalDeexcitationEdge - temp;

      // Writing result to the tree
      tree.put("scin." + to_string(scinID) + ".tot_scaling_factor_a", factorA);
      tree.put("scin." + to_string(scinID) + ".tot_scaling_factor_b", factorB);

      if (saveResult)
      {
        int max = totHist->GetMaximum();
        TLine* l1 = new TLine(anniEdge, 0., anniEdge, max);
        TLine* l2 = new TLine(deexEdge, 0., deexEdge, max);
        l1->SetLineWidth(2);
        l2->SetLineWidth(2);
        l1->SetLineColor(kRed);
        l2->SetLineColor(kRed);

        auto name = Form("tot_edges_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        totHist->Draw();
        l1->Draw("same");
        l2->Draw("same");
        can->SaveAs(Form("%s/tot_edges_%d.png", resultDir.c_str(), scinID));
      }
    }

    // Saving tree into json file
    bpt::write_json(calibJSONFileName, tree);
  }
  fileHitsTOT->Close();
}
