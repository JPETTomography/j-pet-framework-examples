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
 *  @file time_walks.C
 *
 *  @brief Script for reading histograms with time walk histograms
 *
 *  This script uses histograms, that are produced by task EventCAtegorizer.
 *  More detailed description is being created, so stay tuned.
 *
 *  Basic usage:
 *  root> .L time_walks.C
 *  root> time_walks("file_with_calib_histos.root")
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

int fStep = 4;
bool fIgnoreFirst = false;

void time_walks(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
                std::string resultDir = "./")
{
  gStyle->SetOptStat(0);

  TFile* fileTimeWalk = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileTimeWalk->IsOpen())
  {
    map<string, TH2F*> histoMap;
    histoMap["thr1_mtx1"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr1_mtx_1"));
    histoMap["thr1_mtx2"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr1_mtx_2"));
    histoMap["thr1_mtx3"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr1_mtx_3"));
    histoMap["thr1_mtx4"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr1_mtx_4"));
    histoMap["thr2_mtx1"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr2_mtx_1"));
    histoMap["thr2_mtx2"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr2_mtx_2"));
    histoMap["thr2_mtx3"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr2_mtx_3"));
    histoMap["thr2_mtx4"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_thr2_mtx_4"));
    histoMap["sum_mtx1"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_sum_mtx_1"));
    // histoMap["sum_mtx2"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_sum_mtx_2"));
    // histoMap["sum_mtx3"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_sum_mtx_3"));
    // histoMap["sum_mtx4"] = dynamic_cast<TH2F*>(fileTimeWalk->Get("time_walk_sum_mtx_4"));

    for (auto elem : histoMap)
    {
      auto name = elem.first;
      auto hist = elem.second;

      TGraphErrors* gr1 = new TGraphErrors();
      int graphIt = 0;

      for (int bin = fStep; bin < hist->GetNbinsY(); bin += fStep)
      {
        auto projX = hist->ProjectionX("_px", bin - fStep, bin);
        if (projX->GetEntries() > 100)
        {
          if (fIgnoreFirst)
          {
            fIgnoreFirst = false;
            continue;
          }

          auto nBins = projX->GetNbinsX();
          int binRange = (int)(0.2 * nBins);

          auto max = projX->GetMaximumBin();
          auto low = projX->GetBinCenter(max - binRange);
          auto upp = projX->GetBinCenter(max + binRange);

          projX->Fit("gaus", "", "", low, upp);
          auto fitGaus = projX->GetFunction("gaus");
          fitGaus->SetLineColor(kGreen);
          fitGaus->SetLineWidth(2);

          gr1->SetPoint(graphIt, hist->GetYaxis()->GetBinCenter(bin - ((int)fStep / 2)), fitGaus->GetParameter(1));
          gr1->SetPointError(graphIt, 0.0, fitGaus->GetParameter(2));
          graphIt++;
        }
      }

      gr1->Fit("pol1");
      auto fun = gr1->GetFunction("pol1");
      auto chi2 = fun->GetChisquare();
      auto ndf = fun->GetNDF();
      auto p0 = fun->GetParameter(0);
      auto e0 = fun->GetParError(0);
      auto p1 = fun->GetParameter(1);
      auto e1 = fun->GetParError(1);

      tree.put(Form("%s.%s.%s", "time_walk", name.c_str(), "param_a"), p1);
      tree.put(Form("%s.%s.%s", "time_walk", name.c_str(), "param_b"), p0);

      if (saveResult)
      {
        TCanvas can(hist->GetName(), hist->GetName(), 1200, 720);
        gr1->SetNameTitle(hist->GetName(), hist->GetName());
        gr1->GetXaxis()->SetTitle("Projection range center [1/ps]");
        gr1->GetYaxis()->SetTitle("Projection Mean [ps]");
        gr1->Draw("ap");

        auto legend = new TLegend(0.1, 0.7, 0.45, 0.9);
        legend->AddEntry(fun, "fit ax+b", "l");
        legend->AddEntry((TObject*)0, Form("a = %f +- %f", p1, e1), "");
        legend->AddEntry((TObject*)0, Form("b = %f +- %f", p0, e0), "");
        legend->AddEntry((TObject*)0, Form("Chi2 = %f     ndf = %i", chi2, ndf), "");
        legend->Draw();

        can.SaveAs(Form("%s/fit_%s%s", resultDir.c_str(), hist->GetName(), ".png"));

        TCanvas can2(Form("can_%s%s", name.c_str(), ".png"), Form("can_%s%s", name.c_str(), ".png"), 1300, 700);
        hist->Draw("colz");

        auto minY = gr1->GetX()[1];
        auto maxY = gr1->GetX()[gr1->GetN() - 1];

        auto minX = gr1->GetY()[1];
        auto maxX = gr1->GetY()[gr1->GetN() - 1];

        auto line = new TLine(minX, minY, maxX, maxY);
        line->SetLineColor(kRed);
        line->SetLineWidth(2);
        line->Draw("same");

        can2.SaveAs(Form("%s/tw_%s%s", resultDir.c_str(), hist->GetName(), ".png"));
      }
    }
  }

  // Saving tree into json file
  bpt::write_json(calibJSONFileName, tree);
}
