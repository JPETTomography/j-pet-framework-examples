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
 *  @file CalibrationTools.cpp
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "CalibrationTools.h"
#include <cstdlib>

CalibrationTools::CalibrationTools(std::string fileWithParameters, std::string calibrationOption) 
{
      fFileWithParameters = fileWithParameters;
      fCalibrationOption = calibrationOption;
};

CalibrationTools::~CalibrationTools() {}

void CalibrationTools::LoadCalibrationParameters()
{
  if (fFileWithParameters != "") {
    struct stat buffer;
    if (stat (fFileWithParameters.c_str(), &buffer) == 0) {
      namespace pt = boost::property_tree;                                        
      pt::ptree loadPtreeRoot;
      pt::read_json(fFileWithParameters, loadPtreeRoot);
      pt::ptree temp;
      
      if (fCalibrationOption == "single") {
        temp = loadPtreeRoot.get_child(kHistoFileSingleKey);
        fFileWithHistos = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kHistoOutputFileSingleKey);
        fFileWithHistosOut = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleKey);
        fFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleToVelocityKey);
        fFileWithABParametersToVelocity = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kHistoNameSingleKey);
        fHistoNameSingle = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kSaveDerivativesKey);
        fSaveDerivatives = temp.get_value<bool>();
        temp = loadPtreeRoot.get_child(kNumberOfThresholdsKey);
        fNumberOfThresholds = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfScintillatorsKey);
        fNumberOfScintillators = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfPointsToFilterKey);
        fNumberOfPointsToFilter = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kThresholdForDerivativeKey);
        fThresholdForDerivative = temp.get_value<int>();
      } else if (fCalibrationOption == "multi") {
        temp = loadPtreeRoot.get_child(kHistoFileMultiKey);
        fFileWithHistos = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kHistoOutputFileMultiKey);
        fFileWithHistosOut = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileMultiKey);
        fFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kAnniHistoNameMultiKey);
        fAnniHistoNameMulti = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kDeexHistoNameMultiKey);
        fDeexHistoNameMulti = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kSaveDerivativesKey);
        fSaveDerivatives = temp.get_value<bool>();
        temp = loadPtreeRoot.get_child(kNumberOfThresholdsKey);
        fNumberOfThresholds = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfScintillatorsKey);
        fNumberOfScintillators = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfPointsToFilterKey);
        fNumberOfPointsToFilter = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kThresholdForDerivativeKey);
        fThresholdForDerivative = temp.get_value<int>();
      } else if (fCalibrationOption == "final") {
        fFileWithHistos = "";
        temp = loadPtreeRoot.get_child(kConstantsFileFinalKey);
        fFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kVelocitiesFileFinalKey);
        fFileWithVelocities = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleKey);
        fFileWithABParameters = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleToVelocityKey);
        fFileWithABParametersToVelocity = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileMultiKey);
        fFileWithPALSParameters = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsOldFileKey);
        fOldFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kEffectiveLengthKey);
        fEffectiveLength = temp.get_value<int>();
      } else {
        std::cout << "Wrong calibration option: " << fCalibrationOption << 
                    ". Should be one of following: single, multi or final" << std::endl;
      }
    } else {
        std::cout << "No file with a given name: " << fFileWithParameters << " exiting the program." << std::endl;
    }
  } else {
    std::cout << "No file with parameters provided" << std::endl;  
  }
}

void CalibrationTools::Calibrate()
{
  if (fCalibrationOption == "single") {
    CalibrateSingleModule();
  } else if (fCalibrationOption == "multi") {
    CalibrateBetweenModules();
  } else if (fCalibrationOption == "final") {
    GenerateCalibrationFile();
  } else {
    std::cout << "Wrong calibration option: " << fCalibrationOption << ". Or a missing file with parameters." << std::endl;
  }
}


void CalibrationTools::CalibrateSingleModule()
{
  TFile* fileIn = new TFile(fFileWithHistos.c_str(), "READ" );
  std::vector<TH2F*> Histos;
  TH1D *projection_copy;

  TDirectory *dir;
  fileIn->GetObject("EventFinder subtask 0 stats", dir);

  TH2F* temp;
  for (int i=1; i<=fNumberOfThresholds; i++) {
    temp = (TH2F*) dir->Get((fHistoNameSingle + std::to_string(i)).c_str());
    if (temp)
      Histos.push_back(temp);
    else
      std::cout << "Error retrieving histogram with name " << fHistoNameSingle + std::to_string(i) << std::endl;
  }

  Parameter middleLeft, middleRight;
  double meanTemp;
  unsigned iterator = 0;
  std::string titleOfHistogram;
  std::vector<Parameter> tempContainerA, tempContainerB;

  TLatex lat1, lat2;
  TFile* fileOut = new TFile(fFileWithHistosOut.c_str(), "RECREATE" );
  TCanvas *c1 = new TCanvas( "c1", "", 710, 500 );
  c1 -> SetHighLightColor( 2 );
  c1 -> SetFillColor( 0 );
  c1 -> SetFrameBorderMode( 0 );
  c1 -> SetBorderSize( 2 );
  c1 -> SetFrameLineWidth( 2 );

  if ((int)Histos.size() == fNumberOfThresholds) {
    for (unsigned i=0; i<Histos.size(); i++) {
      tempContainerA.clear();
      tempContainerB.clear();
      for (int j=2; j<2+fNumberOfScintillators; j++) {
        iterator++;
        projection_copy = Histos[i]->ProjectionX("_px", j,j);
        titleOfHistogram = "thr" + std::to_string(i+1) + "_ID_nr" + std::to_string(j-1);
        
        meanTemp = projection_copy->GetMean(1);
        double maxCounts = projection_copy->GetMaximum();
        middleLeft = FindMiddle(fileOut, projection_copy, meanTemp - 15, meanTemp - 1, iterator, Side::Left, titleOfHistogram);
        middleRight = FindMiddle(fileOut, projection_copy, meanTemp + 1, meanTemp + 15, iterator, Side::Right, titleOfHistogram);

        tempContainerA.push_back(middleLeft);
        tempContainerB.push_back(middleRight);

        projection_copy -> Draw();
        projection_copy -> SetLineColor(kBlack);
        projection_copy -> GetXaxis()->SetTitle("TDiff B-A [ns]"); 
        projection_copy -> GetYaxis()->SetTitle("Counts");

        lat1.SetTextSize(0.020);
        lat1.SetTextAngle(270.);
        lat1.DrawLatex(middleLeft.Value, 1, Form("( %g )", middleLeft.Value));
        lat2.SetTextSize(0.020);
        lat2.SetTextAngle(270.);
        lat2.DrawLatex(middleRight.Value, 1, Form("( %g )", middleRight.Value));

        double minArgument = middleLeft.Value;
        double maxArgument = middleRight.Value;
        c1 -> Range(minArgument - 5, -0.1*maxCounts, maxArgument + 5, maxCounts + 0.1*maxCounts);
        TLine* line1 = new TLine(middleLeft.Value, -0.1*maxCounts, middleLeft.Value, maxCounts + 0.1*maxCounts);
        line1 -> SetLineColor(kBlue);
        line1 -> SetLineWidth(1);
        line1 -> SetLineStyle(kDashed);
        line1 -> Draw();
        
        TLine* line2 = new TLine(middleRight.Value, -0.1*maxCounts, middleRight.Value, maxCounts + 0.1*maxCounts);
        line2 -> SetLineColor(kBlue);
        line2 -> SetLineWidth(1);
        line2-> SetLineStyle(kDashed);
        line2 -> Draw();
        
        c1 -> Write( titleOfHistogram.c_str() );
      }
      fEdgesA.push_back(tempContainerA);
      fEdgesB.push_back(tempContainerB);
    }
    delete c1;
    fileOut->Close();
    fileIn->Close();

    std::ofstream results;
    if (fEdgesA.size() > 0) {
      results.open((fFileWithConstants).c_str());
      for (unsigned j=0; j<fEdgesA[0].size(); j++) {
        for (unsigned i=0; i<fEdgesA.size(); i++) {
          results << NumberToChar(fEdgesB[i][j].Value + fEdgesA[i][j].Value, 5) << "\t" 
                    << NumberToChar(sqrt(pow(fEdgesB[i][j].Uncertainty, 2) + pow(fEdgesA[i][j].Uncertainty, 2)), 5) << "\t";
        }
        results << std::endl;
      }
      results.close();
      results.open((fFileWithABParametersToVelocity).c_str());
      for (unsigned j=0; j<fEdgesA[0].size(); j++) {
        for (unsigned i=0; i<fEdgesA.size(); i++) {
          results << NumberToChar(fEdgesB[i][j].Value - fEdgesA[i][j].Value, 5) << "\t" 
                    << NumberToChar(sqrt(pow(fEdgesB[i][j].Uncertainty, 2) + pow(fEdgesA[i][j].Uncertainty, 2)), 5) << "\t";
        }
        results << std::endl;
      }
      results.close();
    } else {
      std::cout << "Wrong name of the histograms or the file with histograms in the parameters json file" << std::endl;
    }
  } else
    std::cout << "Number of histograms for Annihilation and Deexcitation does not match. Check their names" << std::endl;
}


void CalibrationTools::CalibrateBetweenModules()
{
  TFile* fileIn = new TFile(fFileWithHistos.c_str(), "READ" );
  std::vector<TH2F*> Histos;
  TH1D *projection_copy1, *projection_copy2, *temp_copy;

  TDirectory *dir;
  fileIn->GetObject("EventCategorizer subtask 0 stats", dir);

  TH2F* temp;
  for (int i=1; i<=fNumberOfThresholds; i++) {
    temp = (TH2F*) dir->Get((fAnniHistoNameMulti + std::to_string(i)).c_str());
    if (temp)
      Histos.push_back(temp);
    else
      std::cout << "Error retrieving histogram with name " << fAnniHistoNameMulti + std::to_string(i) << std::endl;
    temp = (TH2F*) dir->Get((fDeexHistoNameMulti + std::to_string(i)).c_str());
    if (temp)
      Histos.push_back(temp);
    else
      std::cout << "Error retrieving histogram with name " << fDeexHistoNameMulti + std::to_string(i) << std::endl;
  }

  Parameter middleAnni, middleDeex;
  double meanTemp;
  unsigned iterator = 0;
  std::string titleOfHistogram;
  std::vector<Parameter> tempContainerA, tempContainerB;

  TLatex lat1, lat2;
  TFile* fileOut = new TFile(fFileWithHistosOut.c_str(), "RECREATE" );
  TCanvas *c1 = new TCanvas( "c1", "", 710, 500 );
  c1 -> SetHighLightColor( 2 );
  c1 -> SetFillColor( 0 );
  c1 -> SetFrameBorderMode( 0 );
  c1 -> SetBorderSize( 2 );
  c1 -> SetFrameLineWidth( 2 );

  if ((int)Histos.size() == 2*fNumberOfThresholds) {
    for (unsigned i=0; i<Histos.size(); i+=2) {
      tempContainerA.clear();
      tempContainerB.clear();
      for (int j=2; j<2+fNumberOfScintillators; j++) {
        iterator++;
        projection_copy1 = Histos[i]->ProjectionX("_px", j,j);
        projection_copy2 = Histos[i+1]->ProjectionX("_px", j,j);
        titleOfHistogram = "thr" + std::to_string(i+1) + "_ID_nr" + std::to_string(j-1);

        meanTemp = projection_copy1->GetMean(1);
        middleAnni = FindMiddle(fileOut, projection_copy1, meanTemp - 5, meanTemp + 5, iterator, Side::MaxAnni, titleOfHistogram);
        meanTemp = projection_copy2->GetMean(1);
        middleDeex = FindMiddle(fileOut, projection_copy2, meanTemp - 5, meanTemp + 5, iterator, Side::MaxDeex, titleOfHistogram);

        tempContainerA.push_back(middleAnni);
        tempContainerB.push_back(middleDeex);

        double maxCounts = projection_copy1->GetMaximum();
        if (projection_copy1->GetMaximum() < projection_copy2->GetMaximum()) {
            maxCounts = projection_copy2->GetMaximum();
            temp_copy = projection_copy1;
            projection_copy1 = projection_copy2;
            projection_copy2 = temp_copy;
        }

        projection_copy1 -> Draw();
        projection_copy1 -> SetLineColor(kBlack);
        projection_copy1 -> GetXaxis()->SetTitle("TDiff PALS [ns]"); 
        projection_copy1 -> GetYaxis()->SetTitle("Counts");

        projection_copy2 -> Draw("same");
        projection_copy2 -> SetLineColor(kBlue);
        projection_copy2 -> GetXaxis()->SetTitle("TDiff PALS [ns]"); 
        projection_copy2 -> GetYaxis()->SetTitle("Counts");

        lat1.SetTextSize(0.020);
        lat1.SetTextAngle(270.);
        lat1.DrawLatex(middleAnni.Value, 1, Form("( %g )", middleAnni.Value));
        lat2.SetTextSize(0.020);
        lat2.SetTextAngle(270.);
        lat2.DrawLatex(middleDeex.Value, 1, Form("( %g )", middleDeex.Value));

        double minArgument = (middleAnni.Value > middleDeex.Value) ? middleDeex.Value : middleAnni.Value;
        double maxArgument = (middleAnni.Value > middleDeex.Value) ? middleAnni.Value : middleDeex.Value;
        c1 -> Range(minArgument - 5, -0.1*maxCounts, maxArgument + 5, maxCounts + 0.1*maxCounts);
        TLine* line1 = new TLine(middleAnni.Value, -0.1*maxCounts, middleAnni.Value, maxCounts + 0.1*maxCounts);
        line1 -> SetLineColor(kRed);
        line1 -> SetLineWidth(1);
        line1 -> SetLineStyle(kDashed);
        line1 -> Draw();
        
        TLine* line2 = new TLine(middleDeex.Value, -0.1*maxCounts, middleDeex.Value, maxCounts + 0.1*maxCounts);
        line2 -> SetLineColor(kRed);
        line2 -> SetLineWidth(1);
        line2 -> SetLineStyle(kDashed);
        line2 -> Draw();
        
        c1 -> Write( titleOfHistogram.c_str() );
      }
      fMaxAnnihilation.push_back(tempContainerA);
      fMaxDeexcitation.push_back(tempContainerB);
    }
    delete c1;
    fileOut->Close();
    fileIn->Close();
    
    std::vector<std::vector<double>> oldPALSParameters;
    std::ofstream results;
    if (fMaxAnnihilation.size() > 0) {
      struct stat buffer;
      if (stat (fFileWithConstants.c_str(), &buffer) == 0) {
        std::ifstream PALS_Corr(fFileWithConstants.c_str());
        if (PALS_Corr.is_open()) {
          double value, uncert;
          std::vector<double> tempContainer;
          oldPALSParameters.clear();
          while (PALS_Corr >> value >> uncert) {
            tempContainer.push_back(value);
            if ((int)tempContainer.size() == fNumberOfThresholds) {
                oldPALSParameters.push_back(tempContainer);
                tempContainer.clear();
            }
          }
        } else {
          std::cout << "No file - " << fFileWithConstants.c_str() << " - with corrections for calibration between modules" << std::endl;
        }
        results.open((fFileWithConstants).c_str());
        for (unsigned j=0; j<fMaxAnnihilation[0].size(); j++) {
          for (unsigned i=0; i<fMaxAnnihilation.size(); i++) {
            if (isnan(fMaxAnnihilation[i][j].Value) || isnan(fMaxDeexcitation[i][j].Value) 
                                                    || isnan(fMaxAnnihilation[i][j].Uncertainty) || isnan(fMaxAnnihilation[i][j].Uncertainty))
              results << NumberToChar(oldPALSParameters[j][i], 5) << "\t" << NumberToChar(0, 5) << "\t";
            else
              results << NumberToChar(fMaxAnnihilation[i][j].Value - fMaxDeexcitation[i][j].Value + oldPALSParameters[j][i], 5) << "\t" 
                      << NumberToChar(sqrt(pow(fMaxAnnihilation[i][j].Uncertainty, 2) + pow(fMaxDeexcitation[i][j].Uncertainty, 2)), 5) << "\t";
          }
          results << std::endl;
        }
        results.close();
      } else {
        results.open((fFileWithConstants).c_str());
        for (unsigned j=0; j<fMaxAnnihilation[0].size(); j++) {
          for (unsigned i=0; i<fMaxAnnihilation.size(); i++) {
            if (isnan(fMaxAnnihilation[i][j].Value) || isnan(fMaxDeexcitation[i][j].Value) 
                                                    || isnan(fMaxAnnihilation[i][j].Uncertainty) || isnan(fMaxAnnihilation[i][j].Uncertainty))
              results << NumberToChar(0, 5) << "\t" << NumberToChar(0, 5) << "\t";
            else
              results << NumberToChar(fMaxAnnihilation[i][j].Value - fMaxDeexcitation[i][j].Value, 5) << "\t" 
                      << NumberToChar(sqrt(pow(fMaxAnnihilation[i][j].Uncertainty, 2) + pow(fMaxDeexcitation[i][j].Uncertainty, 2)), 5) << "\t";
          }
          results << std::endl;
        }
        results.close();
      }
    } else {
      std::cout << "Wrong name of the histograms or the file with histograms in the parameters json file" << std::endl;
    }
  } else
    std::cout << "Number of histograms for Annihilation and Deexcitation does not match. Check their names" << std::endl;
}


void CalibrationTools::GenerateCalibrationFile()
{
  double value, uncert;
  std::ifstream oldCalib;

  oldCalib.open(fOldFileWithConstants.c_str(), std::ifstream::in);
  std::string tempLine;
  std::vector<double> temp_container_1, temp_container_2;
  std::vector<std::vector<double>> oldParams;
  std::vector<std::vector<double>> ABParams;
  std::vector<std::vector<double>> ABParamsUnc;
  std::vector<std::vector<double>> ABParamsToVelocities;
  std::vector<std::vector<double>> PALSParams;
  std::vector<std::vector<double>> PALSParamsUnc;
  if (oldCalib.is_open()) {
    while(std::getline(oldCalib, tempLine))
    {
      if(tempLine[0] == '#')
        continue;
      else {
        oldParams.push_back(GetParamsFromLine(tempLine));
      }
    }
  } else {
    std::cout << "No file - " << fOldFileWithConstants.c_str() << " - with old corrections" << std::endl;
    return;
  }
  oldCalib.close();
  
  std::ifstream AB_corr(fFileWithABParameters.c_str());
  if (AB_corr.is_open()) {
    ABParams.clear();
    while (AB_corr >> value >> uncert) {
      temp_container_1.push_back(value/2);
      temp_container_2.push_back(uncert);
      if ((int)temp_container_1.size() == fNumberOfThresholds) {
        ABParams.push_back(temp_container_1);
        ABParamsUnc.push_back(temp_container_2);
        temp_container_1.clear();
        temp_container_2.clear();
      }
    }
    temp_container_1.clear();
    temp_container_2.clear();
  } else {
    std::cout << "No file - " << fFileWithABParameters.c_str() << " - with corrections for calibration for a single module" << std::endl;
  }
  AB_corr.close();
  
  std::ifstream AB_corr_vel(fFileWithABParametersToVelocity.c_str());
  if (AB_corr_vel.is_open()) {
    ABParamsToVelocities.clear();
    while (AB_corr_vel >> value >> uncert) {
      temp_container_1.push_back(value/2);
      if ((int)temp_container_1.size() == fNumberOfThresholds) {
        ABParamsToVelocities.push_back(temp_container_1);
        temp_container_1.clear();
      }
    }
    temp_container_1.clear();
  } else {
    std::cout << "No file - " << fFileWithABParametersToVelocity.c_str() << " - with corrections for calibration for a single module" << std::endl;
  }
  AB_corr_vel.close();

  std::ifstream PALS_Corr(fFileWithPALSParameters.c_str());
  if (PALS_Corr.is_open()) {
    PALSParams.clear();
    while (PALS_Corr >> value >> uncert) {
      temp_container_1.push_back(value/4);
      temp_container_2.push_back(uncert);
      if ((int)temp_container_1.size() == fNumberOfThresholds) {
        PALSParams.push_back(temp_container_1);
        PALSParamsUnc.push_back(temp_container_2);
        temp_container_1.clear();
        temp_container_2.clear();
      }
    }
    temp_container_1.clear();
    temp_container_2.clear();
  } else {
    std::cout << "No file - " << fFileWithPALSParameters.c_str() << " - with corrections for calibration between modules" << std::endl;
  }
  PALS_Corr.close();
  
  std::ofstream CalibrationFile;
  CalibrationFile.open(fFileWithConstants.c_str());

  CalibrationFile << "# Time calibration constants" << std::endl;
  CalibrationFile << "# For side A we apply only the correction from Positron Lifetime Distribution calibration, for side B the correction is equal to the sum of the A-B" << std::endl;
  CalibrationFile << "# correction and the correction from Lifetime Distribution calibration. For side A we report the uncertainity from linear fit to the first derrivative";
  CalibrationFile << "in Positron Lifetime spectra, while for side B the same uncertainity is combined with uncertainity from linear fits to the second derrivative of TimeDifferenceAB";
  CalibrationFile << "distribution for right and left edge" << std::endl;
  CalibrationFile << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | offset_value_leading | offset_uncertainty_leading ";
  CalibrationFile << "| difference between old and new calibration | correction to B | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing" << std::endl;	

  int threshold = 0, scintillator = 0;;
  
  for (unsigned i=0; i<oldParams.size()/2-1; i++) {
    if ((i+1)%fNumberOfThresholds == 0)
      scintillator++;
    threshold = i%fNumberOfThresholds;
    CalibrationFile << NumberToChar(oldParams[2*i][0], 0) << "\t" << NumberToChar(oldParams[2*i][1], 0) << "\t"
                                                                << "A\t" << NumberToChar(threshold+1, 0) << "\t";
    CalibrationFile << NumberToChar(oldParams[2*i][3] - PALSParams[scintillator][threshold], 6) << "\t";
    CalibrationFile << NumberToChar(PALSParamsUnc[scintillator][threshold], 6) << "\t";
    CalibrationFile << NumberToChar(PALSParams[scintillator][threshold], 6) << "\t"
                            << NumberToChar(0/*oldParams[2*i][5]*/, 0) << "\t" << NumberToChar(0/*oldParams[2*i][6]*/, 0) << "\t";
    CalibrationFile << NumberToChar(0/* oldParams[2*i][8]*/, 0)  << "\t" << NumberToChar(0/*oldParams[2*i][9]*/, 0)  << "\t"
                                                                    << NumberToChar(0/*oldParams[2*i][10]*/, 0) << std::endl;
// Zeros to fit the old type of the calibration which required a lot of fitting and therefore more parameters were included
    CalibrationFile << NumberToChar(oldParams[2*i+1][0], 0) << "\t" << NumberToChar(oldParams[2*i+1][1], 0) << "\t" << "B\t"
                                                            << NumberToChar(threshold+1, 0) << "\t";
    CalibrationFile << NumberToChar(oldParams[2*i+1][3] - PALSParams[scintillator][threshold] -
                                                            ABParams[scintillator][threshold], 6) << "\t";
    CalibrationFile << NumberToChar(sqrt(pow(PALSParamsUnc[scintillator][threshold], 2) +
                                                        pow(ABParamsUnc[scintillator][threshold], 2)), 6) << "\t";
    CalibrationFile << NumberToChar(PALSParams[scintillator][threshold] + ABParams[scintillator][threshold], 6)
                                                << "\t" << NumberToChar(ABParams[scintillator][threshold], 6)
                                                << "\t" << NumberToChar( 0/*oldParams[2*i+1][7]*/, 0 ) << "\t";
    CalibrationFile << NumberToChar( 0/*oldParams[2*i+1][8]*/, 0 )  << "\t" << NumberToChar( 0/*oldParams[2*i+1][9]*/, 0 )
                                            << "\t" << NumberToChar( 0/*oldParams[2*i+1][10]*/, 0 ) << std::endl;
  }
  CalibrationFile.close();
  
  std::ofstream CalibrationFileVelocity;
  CalibrationFileVelocity.open(fFileWithVelocities.c_str());

  CalibrationFileVelocity << "# Calibrated Veclocities for each strip" << std::endl;
  CalibrationFileVelocity << "# Velocity is calculated as EffecitveLength divided by HalfWidth of TimeDifferenceAB distribution" << std::endl;
  CalibrationFileVelocity << "# Effective Length is assumed constant and equal to 48 cm, where the width of the TimeDifferenceAB distribution is" << std::endl;
  CalibrationFileVelocity << "# calculated as difference of middle of right edge and middle of left edge of TimeDifference AB distribution";
  CalibrationFileVelocity << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | velocity | velocity_uncertainity";
  CalibrationFileVelocity << "| parameter_to_fit_the_format1 | parameter_to_fit_the_format2 | parameter_to_fit_the_format3 | parameter_to_fit_the_format4 | parameter_to_fit_the_format5 | parameter_to_fit_the_format6" << std::endl;	

  scintillator = 0;
  for (unsigned i=0; i<oldParams.size()/2-1; i++) {
    if ((i+1)%fNumberOfThresholds == 0)
      scintillator++;
    threshold = i%fNumberOfThresholds;
    CalibrationFileVelocity << NumberToChar(oldParams[2*i][0], 0) << "\t" << NumberToChar(oldParams[2*i][1], 0) << "\t" << "A\t"
                                                << NumberToChar(threshold+1, 0) << "\t";
    CalibrationFileVelocity << NumberToChar(fEffectiveLength/ABParamsToVelocities[scintillator][threshold], 6) << "\t";
    CalibrationFileVelocity << NumberToChar(fEffectiveLength*ABParamsUnc[scintillator][threshold]/
                                    pow(ABParamsToVelocities[scintillator][threshold], 2 ), 6) << "\t"
                                    << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t";
    CalibrationFileVelocity << NumberToChar(0, 0)  << "\t" << NumberToChar(0, 0)  << "\t" << NumberToChar(0, 0) << std::endl;
    CalibrationFileVelocity << NumberToChar(oldParams[2*i+1][0], 0) << "\t" << NumberToChar(oldParams[2*i+1][1], 0) << "\t" << "B\t"
                                                                << NumberToChar(threshold+1, 0) << "\t";
    CalibrationFileVelocity << NumberToChar(fEffectiveLength/ABParamsToVelocities[scintillator][threshold], 6) << "\t";
    CalibrationFileVelocity << NumberToChar(fEffectiveLength*ABParamsUnc[scintillator][threshold]/
                                                    pow(ABParamsToVelocities[scintillator][threshold], 2) , 6) << "\t"
                                                << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t";
    CalibrationFileVelocity << NumberToChar(0, 0)  << "\t" << NumberToChar(0, 0)  << "\t" << NumberToChar(0, 0) << std::endl;
  }
  CalibrationFileVelocity.close();
}

Parameter CalibrationTools::FindMiddle(TFile* output, TH1D* histo, double firstBinCenter, double lastBinCenter, 
                                                                    unsigned iterator, Side side, std::string titleOfHistogram)
{
  int filterHalf = (int)(fNumberOfPointsToFilter/2);
  std::vector<double> Values, Arguments, temp;
  for (int i=0; i<histo->GetXaxis()->GetNbins(); i++) {
    if (firstBinCenter < histo->GetBinCenter(i) && histo->GetBinCenter(i) < lastBinCenter) {
      Values.push_back( histo->GetBinContent(i) );
      Arguments.push_back( histo->GetBinCenter(i) );
    }
  }
  double sumToFilter;
  temp = Values;
  for (unsigned i=0; i<Arguments.size(); i++) {
    sumToFilter = 0.;
    for (unsigned j=((i-filterHalf) >= 0 ? (i-filterHalf) : 0); 
         j<((i+filterHalf-1) < Values.size() ? (i+filterHalf-1) : Values.size()-1); j++) {
      sumToFilter += Values[j];
    }
    temp[i] = sumToFilter/(2*filterHalf + 1);
  }
  Values = temp;
  
  std::vector<double> FirstDerivative, SecondDerivative;
  FirstDerivative.push_back(Values[0]);
  for (unsigned i=0; i<Arguments.size()-1; i++) {
    FirstDerivative.push_back(Values[i+1] - Values[i]);
  }
  unsigned firstEstimationForExtremumBin;
  if (side == Side::MaxAnni || side == Side::MaxDeex) {
    firstEstimationForExtremumBin = FindMaximum(Values, Arguments, firstBinCenter, lastBinCenter);
    double argumentShift = Arguments[firstEstimationForExtremumBin+1] - Arguments[firstEstimationForExtremumBin];
    Parameter finalEstimatioOfExtremum = FindPeak(Arguments, FirstDerivative, 
                                             firstEstimationForExtremumBin-5, firstEstimationForExtremumBin+5);
    finalEstimatioOfExtremum.Value = finalEstimatioOfExtremum.Value - argumentShift;
    
    if (fSaveDerivatives) {
      std::string sideToTitle = (side == Side::MaxAnni) ? "anni" : "deex";
      TGraph *firstDerivative = new TGraph(FirstDerivative.size());
      for (unsigned i=0; i<FirstDerivative.size(); i++) {
        if ((int)i < 2*filterHalf || (int)i > (int)FirstDerivative.size() - 2*filterHalf - 1) {
          firstDerivative -> SetPoint(i, Arguments[i] - argumentShift, 0);
        }
        else {
          firstDerivative -> SetPoint(i, Arguments[i] - argumentShift, FirstDerivative[i]);
        }
      }
      firstDerivative -> GetXaxis() -> SetTitle("TDiff_B-A [ns]");
      firstDerivative -> GetYaxis() -> SetTitle("First Derrivative");
      firstDerivative -> SetMarkerStyle(20);
      output -> cd();
      firstDerivative -> Write((titleOfHistogram + "_firstDerivative_" + sideToTitle).c_str());
    }
    
    return finalEstimatioOfExtremum;
  } else {
    firstEstimationForExtremumBin = FindExtremum(FirstDerivative, filterHalf, fNumberOfPointsToFilter, side);

    SecondDerivative.push_back(FirstDerivative[0]);
    for( unsigned i=0; i<FirstDerivative.size()-1; i++ ) {
      SecondDerivative.push_back(FirstDerivative[i+1] - FirstDerivative[i]);
    }
    
    double argumentShift = Arguments[firstEstimationForExtremumBin+1] - Arguments[firstEstimationForExtremumBin];

    if (fSaveDerivatives) {;
      std::string sideToTitle = (side == Side::Right) ? "right" : "left";
      TGraph *firstDerivative = new TGraph(FirstDerivative.size());
      TGraph *secondDerivative = new TGraph(SecondDerivative.size());
      for (unsigned i=0; i<FirstDerivative.size(); i++) {
        if ((int)i < 2*filterHalf || (int)i > (int)FirstDerivative.size() - 2*filterHalf - 1) {
          firstDerivative -> SetPoint(i, Arguments[i] - argumentShift, 0);
          secondDerivative -> SetPoint(i, Arguments[i] - 2*argumentShift, 0);
        }
        else {
          firstDerivative -> SetPoint(i, Arguments[i] - argumentShift, FirstDerivative[i]);
          secondDerivative -> SetPoint(i, Arguments[i] - 2*argumentShift, SecondDerivative[i]);
        }
      }
      firstDerivative -> GetXaxis() -> SetTitle("TDiff_B-A [ns]");
      firstDerivative -> GetYaxis() -> SetTitle("First Derrivative");
      firstDerivative -> SetMarkerStyle(20);
      secondDerivative -> GetXaxis() -> SetTitle("TDiff_B-A [ns]");
      secondDerivative -> GetYaxis() -> SetTitle("Second Derrivative");
      secondDerivative -> SetMarkerStyle(20);
      output -> cd();
      firstDerivative -> Write((titleOfHistogram + "_firstDerivative_" + sideToTitle).c_str());
      secondDerivative -> Write((titleOfHistogram + "_secondDerivative_" + sideToTitle).c_str());
    }

    Parameter finalEstimatioOfExtremum = FindPeak(Arguments, SecondDerivative, 
                                             firstEstimationForExtremumBin-5, firstEstimationForExtremumBin+5);
    finalEstimatioOfExtremum.Value = finalEstimatioOfExtremum.Value - 2*argumentShift;
    return finalEstimatioOfExtremum;
  }
}

unsigned CalibrationTools::FindExtremum(std::vector<double> Vector, int filterHalf, unsigned shiftFromFilterHalf, Side side)
{
  unsigned extremum = 0;
  unsigned firstPoint = (side == Side::Right) ? Vector.size() - shiftFromFilterHalf : shiftFromFilterHalf;
  int sideParameter = (side == Side::Right) ? -1 : 1;
  while (sideParameter*Vector[firstPoint] < fThresholdForDerivative && firstPoint < Vector.size() && firstPoint > 0) {
    firstPoint += sideParameter;
  }
  
  double mean = 0, previousMean = 0;;
  for (int i=0; i<(filterHalf<(int)Vector.size() ? filterHalf : (int)Vector.size()); i++) {
    mean += Vector[firstPoint + sideParameter*i];
  }
  previousMean = mean + sideParameter;
  int iterator = firstPoint - sideParameter*fHalfRangeForExtremumEstimation;
  while ( ( (mean < previousMean && sideParameter == 1) || (mean > previousMean && sideParameter == -1))
                                        && iterator+fHalfRangeForExtremumEstimation < (int)Vector.size() 
                                                && iterator > fHalfRangeForExtremumEstimation) {
    previousMean = mean;
    for (int k=1; k<=fHalfRangeForExtremumEstimation; k++) {
      mean += sideParameter*Vector[iterator-k];
      mean += -1*sideParameter*Vector[iterator+k];
    }
    extremum = iterator;
    iterator += sideParameter*fHalfRangeForExtremumEstimation;
  }
  return extremum;
}

Parameter CalibrationTools::FindPeak(std::vector<double> Arguments, std::vector<double> Values, unsigned firstPoint, unsigned lastPoint)
{
  double XX=0, XY=0, YY=0, X=0, Y=0;
  unsigned size = lastPoint - firstPoint;
  for (unsigned k=firstPoint; k < lastPoint; k++) {
    XX += Arguments[k]*Arguments[k];
    XY += Arguments[k]*Values[k];
    YY += Values[k]*Values[k];
    X += Arguments[k];
    Y += Values[k];
  }
  double a = (size*XY-X*Y) / (size*XX - X*X);
  double b = (Y-a*X)/size;
  Parameter peak = {0., 0.};
  
  if(a) {
    peak.Value = -b / a;
    double Da2 = sqrt( ( size*( YY - a*XY - b*Y ) ) / ( ( size-2 )*( size*XX - X*X ) ) );
    double Db2 = sqrt( ( Da2*XX ) / size );
    double Dx0 = sqrt( Db2*Db2/(a*a) + b*b*Da2*Da2/(a*a*a*a) )/size;
    peak.Uncertainty = Dx0;
  }
  return peak;
}

unsigned FindMaximum(std::vector<double> Values, std::vector<double> Arguments, double minimalArgument, double maximalArgument)
{
  double maximum = 0.;
  unsigned maxID = 0;
  for(unsigned i=0; i<Values.size(); i++) {
    if (Values[i] > maximum && Arguments[i] > minimalArgument && Arguments[i] < maximalArgument) {
      maxID = i;
      maximum = Values[i];
    }
  }
  return maxID;
}

std::string NumberToChar(double number, int precision)		//Converting Numbers to char with given preicision
{
  std::ostringstream conv;
  conv << std::fixed << std::setprecision(precision);
  conv << number;
  return conv.str();
}

std::vector<double> GetParamsFromLine(std::string line)
{
	std::vector<double> ParametersLine;
	char sideTemp;
	for( unsigned k=0; k<11; k++ )
	{
		ParametersLine.push_back(0.);
	}
	std::istringstream StringToNumbers(line);
	StringToNumbers >> ParametersLine.at(0) >> ParametersLine.at(1) >> sideTemp >> ParametersLine.at(2) >> ParametersLine.at(3) >> 
                        ParametersLine.at(4) >> ParametersLine.at(5) >> ParametersLine.at(6) >> ParametersLine.at(7) >> 
                        ParametersLine.at(8) >> ParametersLine.at(9) >> ParametersLine.at(10);
	return ParametersLine;
}
