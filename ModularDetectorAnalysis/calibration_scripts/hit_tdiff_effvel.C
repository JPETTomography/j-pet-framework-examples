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
 *  @file hit_tdiff_effvel.C
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
#include <TGraph.h>
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

const double fScinActiveLenght = 50.0;
const int fNumberOfPointsToFilter = 6;
const int fThresholdForDerivative = 80;
const int fHalfRangeForExtremumEstimation = 2;

enum Side
{
  Right,
  Left
};

struct Edge
{
  Point extremum;
  TGraph* firstDevGraph;
  TGraph* secondDevGraph;
};

TGraph* makeGraph(std::vector<double> x, std::vector<double> y)
{
  int i = 0;
  TGraph* graph = new TGraph();

  for (int i = 0; i < x.size(); ++i)
  {
    graph->SetPoint(i, x.at(i), y.at(i));
  }

  graph->SetMarkerStyle(gRandom->Uniform() * 5 + 20);
  graph->SetMarkerSize(0.6);
  return graph;
}

Point findPeak(Points& points)
{
  double sumX = 0, sumY = 0, sumX2 = 0, sumY2 = 0, sumXY = 0;
  int size = (int)points.first.size();

  for (unsigned i = 0; i < size; i++)
  {
    sumX += points.first.at(i);
    sumY += points.second.at(i);
    sumX2 += points.first.at(i) * points.first.at(i);
    sumY2 += points.second.at(i) * points.second.at(i);
    sumXY += points.first.at(i) * points.second.at(i);
  }

  double a = (size * sumXY - sumX * sumY) / (size * sumX2 - sumX * sumX);
  double b = (sumY - a * sumX) / size;

  double value = 0, uncertainty = 0;
  if (a != 0)
  {
    value = -b / a;
    double da2 = TMath::Sqrt((size * (sumY2 - a * sumXY - b * sumY)) / ((size - 2) * (size * sumX2 - sumX * sumX)));
    double db2 = TMath::Sqrt((da2 * sumX2) / size);
    double dx0 = TMath::Sqrt(db2 * db2 / (a * a) + b * b * da2 * da2 / (a * a * a * a)) / size;
    uncertainty = dx0;
  }
  return std::make_pair(value, uncertainty);
}

unsigned findExtremumBin(std::vector<double> values, int sideParam)
{
  int filterHalf = (int)(fNumberOfPointsToFilter / 2);
  int halfRange = fNumberOfPointsToFilter;

  unsigned firstPoint = (sideParam == -1) ? values.size() - halfRange : halfRange;

  while (sideParam * values.at(firstPoint) < fThresholdForDerivative && firstPoint < values.size() && firstPoint > 0)
  {
    firstPoint += sideParam;
    if (firstPoint >= values.size())
      break;
  }

  double mean = 0, previousMean = 0;
  ;
  for (int i = 0; i < (filterHalf < (int)values.size() ? filterHalf : (int)values.size()); i++)
  {
    if (firstPoint + sideParam * i >= values.size())
      break;
    mean += values.at(firstPoint + sideParam * i);
  }
  previousMean = mean - sideParam;

  unsigned extremumBin = 0;
  int iterator = firstPoint + sideParam * fHalfRangeForExtremumEstimation;
  while (((mean > previousMean && sideParam == 1) || (mean < previousMean && sideParam == -1)) &&
         iterator + fHalfRangeForExtremumEstimation < (int)values.size() && iterator > fHalfRangeForExtremumEstimation)
  {
    previousMean = mean;
    for (int k = 1; k <= fHalfRangeForExtremumEstimation; k++)
    {
      mean += -1 * sideParam * values.at(iterator - k);
      mean += sideParam * values.at(iterator + k);
    }
    extremumBin = iterator;
    iterator += sideParam * fHalfRangeForExtremumEstimation;
  }
  return extremumBin;
}

std::vector<double> getDerivative(std::vector<double> values)
{
  std::vector<double> derivative;
  derivative.push_back(values.at(0));
  for (unsigned i = 0; i < values.size() - 1; i++)
  {
    derivative.push_back(values.at(i + 1) - values.at(i));
  }
  return derivative;
}

Edge findEdge(Points points, Side side)
{
  auto firstDerivative = getDerivative(points.second);
  auto firstDevGraph = makeGraph(points.first, firstDerivative);
  firstDevGraph->SetMarkerColor(kOrange);
  firstDevGraph->SetLineColor(kOrange);
  unsigned extremumBin = findExtremumBin(firstDerivative, (side == Side::Right) ? -1 : 1);
  auto secondDerivative = getDerivative(firstDerivative);
  auto secondDevGraph = makeGraph(points.first, secondDerivative);
  secondDevGraph->SetMarkerColor(kTeal);
  secondDevGraph->SetLineColor(kTeal);

  std::vector<double> subArgs(points.first.begin() + extremumBin - 2, points.first.begin() + extremumBin + 3);
  std::vector<double> subValues(secondDerivative.begin() + extremumBin - 2, secondDerivative.begin() + extremumBin + 3);

  Points peakPoints = std::make_pair(subArgs, subValues);
  auto extremum = findPeak(peakPoints);
  double corrExtr = extremum.first - 2 * (points.first.at(extremumBin + 1) - points.first.at(extremumBin));
  Edge edge = {make_pair(corrExtr, extremum.second), firstDevGraph, secondDevGraph};
  return edge;
}

Points getSubset(TH1F* histo, double min, double max)
{
  int filterHalf = (int)(fNumberOfPointsToFilter / 2);

  std::vector<double> x_vec, y_vec, temp;
  for (int i = 0; i < histo->GetXaxis()->GetNbins(); i++)
  {
    if (histo->GetBinCenter(i) > min && histo->GetBinCenter(i) < max)
    {
      x_vec.push_back(histo->GetBinCenter(i));
      y_vec.push_back(histo->GetBinContent(i));
    }
  }

  temp = y_vec;
  for (int i = 0; i < y_vec.size(); i++)
  {
    double sumToFilter = 0.0;

    for (unsigned j = ((i - filterHalf) >= 0 ? (i - filterHalf) : 0);
         j < ((i + filterHalf - 1) < y_vec.size() ? (i + filterHalf - 1) : y_vec.size() - 1); j++)
    {
      sumToFilter += y_vec.at(j);
    }
    temp[i] = sumToFilter / (2 * filterHalf + 1);
  }
  y_vec = temp;

  return std::make_pair(x_vec, y_vec);
}

void hit_tdiff_effvel(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false,
                      std::string resultDir = "./", int minScinID = 201, int maxScinID = 512)
{

  TFile* fileHitsAB = new TFile(fileName.c_str(), "READ");

  bpt::ptree tree;
  ifstream file(calibJSONFileName.c_str());
  if (file.good())
  {
    bpt::read_json(calibJSONFileName, tree);
  }

  if (fileHitsAB->IsOpen())
  {

    TGraphErrors* bCorrGraph = new TGraphErrors();
    bCorrGraph->SetNameTitle("b_corr", "B side signals correction for scintillators");
    bCorrGraph->GetXaxis()->SetTitle("Scin ID");
    bCorrGraph->GetYaxis()->SetTitle("correction [ps]");

    TGraphErrors* effVelGraph = new TGraphErrors();
    effVelGraph->SetNameTitle("eff_vel", "Effective light velocity in scintillators");
    effVelGraph->GetXaxis()->SetTitle("Scin ID");
    effVelGraph->GetYaxis()->SetTitle("velocity [cm/ps]");

    unsigned graphIt = 0;

    for (int scinID = minScinID; scinID <= maxScinID; ++scinID)
    {

      TH1F* ab_tdiff = dynamic_cast<TH1F*>(fileHitsAB->Get(Form("ab_tdiff_scin_%d", scinID)));
      ab_tdiff->SetLineColor(kBlue);
      ab_tdiff->SetLineWidth(2);

      if (ab_tdiff->GetEntries() < 1000)
      {
        continue;
      }

      double mean = ab_tdiff->GetMean();

      auto leftSide = getSubset(ab_tdiff, mean - 8000, mean - 2000);
      auto rightSide = getSubset(ab_tdiff, mean + 2000, mean + 8000);

      auto leftEdge = findEdge(leftSide, Side::Left);
      auto rightEdge = findEdge(rightSide, Side::Right);

      auto leftLine = new TLine(leftEdge.extremum.first, ab_tdiff->GetMinimum(), leftEdge.extremum.first, ab_tdiff->GetMaximum());
      leftLine->SetLineColor(kRed);
      leftLine->SetLineWidth(2);
      auto rightLine = new TLine(rightEdge.extremum.first, ab_tdiff->GetMinimum(), rightEdge.extremum.first, ab_tdiff->GetMaximum());
      rightLine->SetLineColor(kRed);
      rightLine->SetLineWidth(2);

      // B side correction in [ps] and effective velocity in [cm/ps]
      double b_corr = 0.5 * (leftEdge.extremum.first + rightEdge.extremum.first);
      double b_corr_error = 0.5 * (leftEdge.extremum.second + rightEdge.extremum.second);
      double eff_vel = 2.0 * fScinActiveLenght / (fabs(rightEdge.extremum.first - leftEdge.extremum.first));
      double eff_vel_error = 2.0 * fScinActiveLenght * (leftEdge.extremum.second - rightEdge.extremum.second) /
                             pow((rightEdge.extremum.first - leftEdge.extremum.first), 2);

      tree.put("scin." + to_string(scinID) + ".b_correction", b_corr);
      tree.put("scin." + to_string(scinID) + ".eff_velocity", eff_vel);

      // Filling the graph
      bCorrGraph->SetPoint(graphIt, (double)scinID, b_corr);
      bCorrGraph->SetPointError(graphIt, 0.0, 0.0);
      effVelGraph->SetPoint(graphIt, (double)scinID, eff_vel);
      effVelGraph->SetPointError(graphIt, 0.0, 0.0);
      graphIt++;

      if (saveResult)
      {
        // Drawing canvas with spectra, derivatives and extremums marked with lines
        auto name = Form("edges_scin_%d", scinID);
        TCanvas* can = new TCanvas(name, name, 1200, 800);
        ab_tdiff->SetMinimum(-1000.0);
        ab_tdiff->Draw();
        leftEdge.firstDevGraph->Draw("LPsame");
        leftEdge.secondDevGraph->Draw("LPsame");
        rightEdge.firstDevGraph->Draw("LPsame");
        rightEdge.secondDevGraph->Draw("LPsame");
        leftLine->Draw("same");
        rightLine->Draw("same");
        // Saving canvas in the specified directory
        can->SaveAs(Form("%s/edges_scin_%d.png", resultDir.c_str(), scinID));
      }
    }

    if (saveResult)
    {
      TCanvas* canBcorr = new TCanvas("b_corr_graph", "b_corr_graph", 1200, 800);
      bCorrGraph->Draw("AP*");
      canBcorr->SaveAs(Form("%s/b_corrections.png", resultDir.c_str()));

      TCanvas* canEffVel = new TCanvas("eff_vel_graph", "eff_vel_graph", 1200, 800);
      effVelGraph->Draw("AP*");
      canEffVel->SaveAs(Form("%s/eff_velocity.png", resultDir.c_str()));
    }
  }

  // Saving tree into json file
  bpt::write_json(calibJSONFileName, tree);
}
