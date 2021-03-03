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
 *  @file CalibrationTools.cpp
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "CalibrationTools.h"
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TROOT.h>
#include <TAxis.h>
#include <TFile.h>
#include <TLine.h>
#include <cstdlib>

CalibrationTools::CalibrationTools()
{
  fFileWithParameters = "";
  fCalibrationOption = "";
  fThresholdForDerivative = 10;
  fNumberOfPointsToFilter = 2;
}

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
      
      temp = loadPtreeRoot.get_child(kConstantsOldFileKey);
      fOldFileWithConstants = temp.get_value<std::string>();
      temp = loadPtreeRoot.get_child(kEffectiveLengthKey);
      fEffectiveLength = temp.get_value<float>();
      if (fCalibrationOption == "single" || fCalibrationOption == "multi") {
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
        }
        temp = loadPtreeRoot.get_child(kSaveDerivativesKey);
        fSaveDerivatives = temp.get_value<bool>();
        temp = loadPtreeRoot.get_child(kNumberOfThresholdsKey);
        fNumberOfThresholds = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kMinScintillatorIDKey);
        fMinScintillatorID = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kMaxScintillatorIDKey);
        fMaxScintillatorID = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfPointsToFilterKey);
        fNumberOfPointsToFilter = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kThresholdForDerivativeKey);
        fThresholdForDerivative = temp.get_value<int>();
      }
       else if (fCalibrationOption == "final") {
        fFileWithHistos = "";
        temp = loadPtreeRoot.get_child(kConstantsFileFinalKey);
        fFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleKey);
        fFileWithABParameters = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleToVelocityKey);
        fFileWithABParametersToVelocity = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileMultiKey);
        fFileWithPALSParameters = temp.get_value<std::string>();
      } else {
        std::cerr << "Wrong calibration option: " << fCalibrationOption << ". Should be one of following: single, multi or final" << std::endl;
      }
    } else {
        fCalibrationOption = "";
        std::cerr << "No file with a given name: " << fFileWithParameters << " exiting the program." << std::endl;
    }
  } else {
    fCalibrationOption = "";
    std::cerr << "No file with parameters provided" << std::endl;  
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
    std::cerr << "Wrong calibration option: " << fCalibrationOption << ". Or a missing file with parameters." << std::endl;
  }
}

void CalibrationTools::CalibrateSingleModule()
{
  TFile* fileIn = new TFile(fFileWithHistos.c_str(), "READ" );
  std::vector<TH2D*> Histos = GetHistosFromFile(fileIn, fNumberOfThresholds, fCalibrationOption, fHistoNameSingle, "");
  if (Histos.size() == 0) {
    std::cerr << "No histos were found in given file." << std::endl;
    return;
  }

  if ((int)Histos.size() == fNumberOfThresholds) {
    FindEdges(Histos);
    fileIn->Close();
    
    if (fEdgesA.size() > 0) {
      SaveABEdgesResults(fOldFileWithConstants, fFileWithConstants, fFileWithABParametersToVelocity, fEffectiveLength, fEdgesA, fEdgesB);
    } else {
      std::cerr << "Wrong name of the histograms or the file with histograms in the parameters json file" << std::endl;
    }
  } else
    std::cerr << "Number of histograms for Annihilation and Deexcitation does not match. Check their names" << std::endl;
  delete fileIn;
}

void CalibrationTools::CalibrateBetweenModules()
{
  TFile* fileIn = new TFile(fFileWithHistos.c_str(), "READ" );
  std::vector<TH2D*> Histos = GetHistosFromFile(fileIn, fNumberOfThresholds, fCalibrationOption, fAnniHistoNameMulti, fDeexHistoNameMulti);

  if ((int)Histos.size() == 2*fNumberOfThresholds) {
    FindPeaks(Histos);
    fileIn->Close();

    if (fMaxAnnihilation.size() > 0) {
      SavePALSPeakResults(fOldFileWithConstants, fFileWithConstants, fMaxAnnihilation, fMaxDeexcitation);
    } else {
      std::cerr << "Wrong name of the histograms or the file with histograms in the parameters json file" << std::endl;
    }
  } else
    std::cerr << "Number of histograms for Annihilation and Deexcitation does not match. Check their names" << std::endl;
  delete fileIn;
}

void CalibrationTools::GenerateCalibrationFile()
{
  std::ifstream oldCalib;

  oldCalib.open(fOldFileWithConstants.c_str(), std::ifstream::in);
  std::string tempLine;
  std::vector<std::vector<double>> oldParams;
  std::vector<std::vector<double>> ABParams;
  std::vector<std::vector<double>> PALSParams;
  if (oldCalib.is_open()) {
    while(std::getline(oldCalib, tempLine))
    {
      if(tempLine.at(0) == '#')
        continue;
      else {
        oldParams.push_back(GetParamsFromLine(tempLine));
      }
    }
  } else {
    std::cerr << "No file - " << fOldFileWithConstants.c_str() << " - with old corrections" << std::endl;
    return;
  }
  oldCalib.close();
  
  std::ifstream AB_corr(fFileWithABParameters.c_str());
  if (AB_corr.is_open()) {
    while(std::getline(AB_corr, tempLine))
    {
      if(tempLine.at(0) == '#')
        continue;
      else {
        ABParams.push_back(GetParamsFromLine(tempLine));
      }
    }
  } else {
    std::cerr << "No file - " << fFileWithABParameters.c_str() << " - with corrections for calibration for a single module" << std::endl;
  }
  AB_corr.close();

  std::ifstream PALS_Corr(fFileWithPALSParameters.c_str());
  if (PALS_Corr.is_open()) {
    while(std::getline(PALS_Corr, tempLine))
    {
      if(tempLine.at(0) == '#')
        continue;
      else {
        PALSParams.push_back(GetParamsFromLine(tempLine));
      }
    }
  } else {
    std::cerr << "No file - " << fFileWithPALSParameters.c_str() << " - with corrections for calibration between modules" << std::endl;
  }
  PALS_Corr.close();
  
  std::ofstream CalibrationFile;
  CalibrationFile.open(fFileWithConstants.c_str());

  CalibrationFile << "# Time calibration constants for effective length: " << fEffectiveLength << std::endl;
  CalibrationFile << "# For side A we apply only the correction from Positron Lifetime Distribution calibration, for side B the correction is equal to the sum of the A-B" << std::endl;
  CalibrationFile << "# correction and the correction from Lifetime Distribution calibration. For side A we report the uncertainity from linear fit to the first derrivative";
  CalibrationFile << "in Positron Lifetime spectra, while for side B the same uncertainity is combined with uncertainity from linear fits to the second derrivative of TimeDifferenceAB";
  CalibrationFile << "distribution for right and left edge" << std::endl;
  CalibrationFile << "# Description of the parameters: layer | slot | side | threshold | offset_value_leading | offset_uncertainty_leading ";
  CalibrationFile << "| RSquared from PALS correction | Fisher Parameter from PALS correction | blankToFit | blankToFit |" ;
  CalibrationFile << "blankToFit | blankToFit" << std::endl;	
  
  std::string side, sideOld;
  int layer, slot, threshold;
  unsigned iteratorForOldParams;
  bool test = true;
  for (unsigned i=0; i<PALSParams.size(); i++) {
    layer = (int)PALSParams.at(i).at(0);
    slot = (int)PALSParams.at(i).at(1);
    side = (char)PALSParams.at(i).at(2);
    threshold = (int)PALSParams.at(i).at(3);
    test = true;
    iteratorForOldParams = 0;
    while (test && iteratorForOldParams < oldParams.size()) {
      sideOld = (char)oldParams.at(iteratorForOldParams).at(2);
      if (oldParams.at(iteratorForOldParams).at(0) == layer && oldParams.at(iteratorForOldParams).at(1) == slot 
          && sideOld == PALSParams.at(i).at(2) && oldParams.at(iteratorForOldParams).at(3) == threshold)
        test = false;
      else
        iteratorForOldParams++;
    }
    CalibrationFile << NumberToChar(PALSParams.at(i).at(0), 0) << "\t" << NumberToChar(PALSParams.at(i).at(1), 0) << "\t";
    CalibrationFile << side << "\t" << NumberToChar(PALSParams.at(i).at(3), 0) << "\t";
    if (side == 'A') {
      CalibrationFile << NumberToChar(oldParams.at(iteratorForOldParams).at(4) - PALSParams.at(i).at(4), 6) << "\t" << NumberToChar(PALSParams.at(i).at(5), 6) << "\t";
    } else if (side == 'B') {
      CalibrationFile << NumberToChar(oldParams.at(iteratorForOldParams).at(4) - PALSParams.at(i).at(4) - ABParams.at(i).at(4), 6) << "\t"; 
      CalibrationFile << NumberToChar(sqrt(pow(PALSParams.at(i).at(5), 2) + pow(ABParams.at(i).at(5), 2)), 6) << "\t";
    } else {
      std::cerr << "Wrong side while reading old clibration file" << std::endl;
    }
    CalibrationFile << NumberToChar(PALSParams.at(i).at(6), 6) << "\t" << NumberToChar(PALSParams.at(i).at(7), 6) << "\t";
    CalibrationFile << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0)  << "\t" << NumberToChar(0, 0)  << std::endl;
  }

  CalibrationFile.close();
}

std::vector<TH2D*> GetHistosFromFile(TFile* fileIn, int numberOfThresholds, std::string calibrationOption, std::string histoName, std::string histoName2)
{
  std::vector<TH2D*> Histos;
  TDirectory *dir = gDirectory;
  if (calibrationOption == "single") {
    fileIn->GetObject("EventFinder subtask 0 stats", dir);
  } else if (calibrationOption == "multi") {
    fileIn->GetObject("PALSCalibrationTask subtask 0 stats", dir);
  }
  if (!dir) {
    std::cerr << "No directory EventFinder or PALSCalibrationTask in a given file or wrong calibration option (single or multi)" << std::endl;
  } else {
    TH2D* tempHisto;
    for (int i=1; i<=numberOfThresholds; i++) {
      tempHisto = dynamic_cast<TH2D*>(dir->Get((histoName + std::to_string(i)).c_str()));
      if (tempHisto)
        Histos.push_back(tempHisto);
      else
        std::cerr << "Error retrieving histogram with name " << histoName + std::to_string(i) << std::endl;
    }
    if (calibrationOption == "multi") {
      for (int i=1; i<=numberOfThresholds; i++) {
        tempHisto = dynamic_cast<TH2D*>(dir->Get((histoName2 + std::to_string(i)).c_str()));
        if (tempHisto)
          Histos.push_back(tempHisto);
        else
          std::cerr << "Error retrieving histogram with name " << histoName2 + std::to_string(i) << std::endl;
      }
    }
  }
  return Histos;
}

void CalibrationTools::FindEdges(std::vector<TH2D*> Histos)
{
  TH1D *projection_copy;
  Parameter middleLeft, middleRight;
  double meanTemp, rangeParameter;
  unsigned iterator = 0;
  std::string titleOfHistogram, titleOfDirectory;
  std::vector<Parameter> tempContainerA, tempContainerB;
  
  TFile* fileOut = new TFile(fFileWithHistosOut.c_str(), "RECREATE" );
  
  for (unsigned i=0; i<Histos.size(); i++) {
    tempContainerA.clear();
    tempContainerB.clear();
//Loop starts from the first bin - zero bin is underflow bin
    for (int j=1; j<=fMaxScintillatorID - fMinScintillatorID + 1; j++) {
      iterator++;
      projection_copy = Histos.at(i)->ProjectionX("_px", j, j);
      titleOfHistogram = "thr" + std::to_string(i+1) + "_ID_nr" + std::to_string(j-1 + fMinScintillatorID);
      
      fileOut->cd();
      titleOfDirectory = "ID_nr" + std::to_string(j-1 + fMinScintillatorID) + "/";
      fileOut->mkdir(titleOfDirectory.c_str());
      fileOut->cd(titleOfDirectory.c_str());
      
      meanTemp = projection_copy->GetMean(1);
      rangeParameter = 4*projection_copy->GetStdDev();
      if (projection_copy -> GetEntries() > 0) {
        middleLeft = FindMiddle(projection_copy, meanTemp - rangeParameter, meanTemp, Side::Left, titleOfHistogram);
        middleRight = FindMiddle(projection_copy, meanTemp, meanTemp + rangeParameter, Side::Right, titleOfHistogram);

        tempContainerA.push_back(middleLeft);
        tempContainerB.push_back(middleRight);
        
        DrawEdgesOnHistogram(projection_copy, middleLeft, middleRight, titleOfHistogram);
      } else {
        std::cout << "No data in histogram from scintillator nr " << j-1 + fMinScintillatorID << std::endl;
        Parameter temp;
        tempContainerA.push_back(temp);
        tempContainerB.push_back(temp); 
      }
    }
    fEdgesA.push_back(tempContainerA);
    fEdgesB.push_back(tempContainerB);
  }
  fileOut->Close();
  delete fileOut;
}

void CalibrationTools::FindPeaks(std::vector<TH2D*> Histos)
{
  TH1D *projection_copy1, *projection_copy2;
  Parameter middleAnni, middleDeex;
  double meanTemp, rangeParameter;
  unsigned iterator = 0;
  std::string titleOfHistogram, titleOfDirectory;
  std::vector<Parameter> tempContainerA, tempContainerB;
  
  TFile* fileOut = new TFile(fFileWithHistosOut.c_str(), "RECREATE" );
  
  for (unsigned i=0; i<Histos.size()/2; i++) {
    tempContainerA.clear();
    tempContainerB.clear();
//Loop starts from the first bin - zero bin is underflow bin
    for (int j=1; j<=fMaxScintillatorID - fMinScintillatorID + 1; j++) {
      iterator++;
      projection_copy1 = Histos.at(i)->ProjectionX("_px", j, j);
      projection_copy2 = Histos.at(i+Histos.size()/2)->ProjectionX("_px", j, j);
      titleOfHistogram = "thr" + std::to_string(i+1) + "_ID_nr" + std::to_string(j-1 + fMinScintillatorID);
      
      fileOut->cd();
      titleOfDirectory = "ID_nr" + std::to_string(j-1 + fMinScintillatorID) + "/";
      fileOut->mkdir(titleOfDirectory.c_str());
      fileOut->cd(titleOfDirectory.c_str());
      
      if (projection_copy1 -> GetEntries() > 0 && projection_copy2 -> GetEntries() > 0) {
        meanTemp = projection_copy1->GetMean(1);
        rangeParameter = 4*projection_copy1->GetStdDev();
        middleAnni = FindMiddle(projection_copy1, meanTemp - rangeParameter, meanTemp + rangeParameter, Side::MaxAnni, titleOfHistogram);
        meanTemp = projection_copy2->GetMean(1);
        rangeParameter = 4*projection_copy2->GetStdDev();
        middleDeex = FindMiddle(projection_copy2, meanTemp - rangeParameter, meanTemp + rangeParameter, Side::MaxDeex, titleOfHistogram);

        tempContainerA.push_back(middleAnni);
        tempContainerB.push_back(middleDeex);
        
        DrawPeaksOnHistogram(projection_copy1, projection_copy2, middleAnni, middleDeex, titleOfHistogram);
      } else {
        std::cerr << "No data in histogram from scintillator nr " << j-1 + fMinScintillatorID << std::endl;
        Parameter temp;
        tempContainerA.push_back(temp);
        tempContainerB.push_back(temp); 
      }
    }
    fMaxAnnihilation.push_back(tempContainerA);
    fMaxDeexcitation.push_back(tempContainerB);
  }
  fileOut->cd();
  Parameter meanCorrection;
  meanCorrection = CalculateMeanCorrection(fMaxAnnihilation, fMaxDeexcitation);
  std::cout << "Mean correction: " << meanCorrection.Value << " with mean uncertainty: " << meanCorrection.Uncertainty << std::endl;
  PlotCorrectionHistos(fMaxAnnihilation, fMaxDeexcitation, meanCorrection.Uncertainty);
  fileOut->Close();
  delete fileOut;
}

Parameter CalibrationTools::FindMiddle(TH1D* histo, double firstBinCenter, double lastBinCenter, Side side, std::string titleOfHistogram)
{
  Parameter finalEstimatioOfExtremum;
  int filterHalf = (int)(fNumberOfPointsToFilter/2);
  std::vector<double> Values, Arguments, temp;
  for (int i=0; i<histo->GetXaxis()->GetNbins(); i++) {
    if (firstBinCenter < histo->GetBinCenter(i) && histo->GetBinCenter(i) < lastBinCenter) {
      Values.push_back( histo->GetBinContent(i) );
      Arguments.push_back( histo->GetBinCenter(i) );
    }
  }
//Smoothing of distribution to decrease influence of statistical fluctuations
  Values = SmoothByLinearFilter(Values, filterHalf);
  Values = ReduceBoundaryEffect(Values, filterHalf);
  Arguments = ReduceBoundaryEffect(Arguments, filterHalf);
  
  std::vector<double> FirstDerivative, SecondDerivative;
  FirstDerivative = CalculateDerivative(Values);
  FirstDerivative = SmoothByLinearFilter(FirstDerivative, filterHalf);

  unsigned firstEstimationForExtremumBin;
  if (side == Side::MaxAnni || side == Side::MaxDeex) {
//Maximum estiamted as just a point with maximal value in some range (firstBinCenter, lastBinCenter)
    firstEstimationForExtremumBin = FindMaximum(Values, Arguments, firstBinCenter, lastBinCenter);
//For calculations of derivative argument should be a mean of two arguments between which derivative is calculated
//In order to not to creat additional argument vector for derivative, one can just apply argument shift coming from
//calculations of the derivative
    double argumentShift = Arguments.at(firstEstimationForExtremumBin+1) - Arguments.at(firstEstimationForExtremumBin);
//Peak estimated as a "zero" of the first derivative. Zero is esitmated from a linear fit in close range of the firstEstimationForExtremumBin
    finalEstimatioOfExtremum = FindPeak(Arguments, FirstDerivative, 
                                             firstEstimationForExtremumBin-5, firstEstimationForExtremumBin+5);
    finalEstimatioOfExtremum.Value = finalEstimatioOfExtremum.Value - argumentShift;
    
    if (fSaveDerivatives) {
      DrawDerivatives(titleOfHistogram, side, filterHalf, Arguments, FirstDerivative, temp);
    }
  } else {
//In case of the TDiff BA distributions maximum of the derivative corresponding to the edge on a given side is very sensitive
//to any fluctuations. Therefore maximum is estimated as a first point after which moving average is changing trend. 
//Change of the trend and direction of the changing the iterator depends on the side on which we want to estimate and edge
    firstEstimationForExtremumBin = EstimateExtremumBin(FirstDerivative, filterHalf, fNumberOfPointsToFilter, side);

    SecondDerivative = CalculateDerivative(FirstDerivative);
    SecondDerivative = SmoothByLinearFilter(SecondDerivative, filterHalf);
    
    double argumentShift = Arguments.at(firstEstimationForExtremumBin+1) - Arguments.at(firstEstimationForExtremumBin);

    if (fSaveDerivatives) {
      DrawDerivatives(titleOfHistogram, side, filterHalf, Arguments, FirstDerivative, SecondDerivative);
    }

    finalEstimatioOfExtremum = FindPeak(Arguments, SecondDerivative, 
                                             firstEstimationForExtremumBin-5, firstEstimationForExtremumBin+5);
    finalEstimatioOfExtremum.Value = finalEstimatioOfExtremum.Value - 2*argumentShift;

  }
  return finalEstimatioOfExtremum;
}

//Looking for a maximum in some range (minimalArgument, maximalArgument)
unsigned FindMaximum(std::vector<double> Values, std::vector<double> Arguments, double minimalArgument, double maximalArgument)
{
  double maximum = 0.;
  unsigned maxID = 0;
  for(unsigned i=0; i<Values.size(); i++) {
    if (Values.at(i) > maximum && Arguments.at(i) > minimalArgument && Arguments.at(i) < maximalArgument) {
      maxID = i;
      maximum = Values.at(i);
    }
  }
  return maxID;
}

//Finding bin with extremum (maximum or minimum) in order to proceed with more sophisticated methods of finding extremum.
//Extremum is estimated based on the change in the trend of the moving average
unsigned CalibrationTools::EstimateExtremumBin(std::vector<double> vector, int filterHalf, unsigned shiftFromFilterHalf, Side side)
{
  unsigned extremum = 0;
  unsigned firstPoint = (side == Side::Right) ? vector.size() - shiftFromFilterHalf - 1 : shiftFromFilterHalf;
//Side parameter allows to define one function for both edges. In case of the Right edge order of indicator increment
//should be descending (-1) where for the Left it should be increasing (+1)
  int sideParameter = (side == Side::Right) ? -1 : 1;
  
  while (sideParameter*vector.at(firstPoint) < fThresholdForDerivative && firstPoint < vector.size()-1 && firstPoint > 0) {
    firstPoint += sideParameter;
  }
  if (firstPoint == vector.size()-1 || firstPoint == 0) {
    std::cerr << "Probably the threshold is too high for looking at the maximum on the first derivative" << std::endl;
    return vector.size()/2;
  }
  
  double mean = 0, previousMean = 0;;
  for (int i=0; i<(filterHalf<(int)vector.size() ? filterHalf : (int)vector.size()); i++) {
    mean += vector.at(firstPoint + sideParameter*i);
  }
  
  previousMean = mean - sideParameter;
  int iterator = firstPoint + sideParameter*fHalfRangeForExtremumEstimation;
  while ( ( (mean > previousMean && sideParameter == 1) || (mean < previousMean && sideParameter == -1))
                                        && iterator+fHalfRangeForExtremumEstimation < (int)vector.size() 
                                                && iterator > fHalfRangeForExtremumEstimation) {
    previousMean = mean;
//Moving average loop. For side Left mean should go from left to right, for side Right from right to left
//It is like that because in many cases distribution of the derivative of the TDiff BA distribution is disturbed between the real edge
//of the strip and middle of the scintillator (because of the cos^2 distribution of the irradiation, statistical fluctuations and some
//electronic noise)
    for (int k=1; k<=fHalfRangeForExtremumEstimation; k++) {
      mean += -1*sideParameter*vector.at(iterator-k);
      mean += sideParameter*vector.at(iterator+k);
    }
    extremum = iterator;
    iterator += sideParameter*fHalfRangeForExtremumEstimation;
  }
  return extremum;
}

//Linear regression to estimate peak between firstPoint and lastPoint
Parameter CalibrationTools::FindPeak(std::vector<double> arguments, std::vector<double> values, unsigned firstPoint, unsigned lastPoint)
{
  unsigned size = lastPoint - firstPoint;
  double meanX=0, meanY=0;
  Parameter peak;
  if (size == 0) {
    std::cerr << "Empty vector for looking for a peak" << std::endl;
    return peak;
  }
  for (unsigned k=firstPoint; k<lastPoint; k++) {
    meanX += arguments.at(k);
    meanY += values.at(k);
  }
  meanX = meanX/size;
  meanY = meanY/size;
  
  double SSxx=0, SSyy=0, SSxy=0;
  for (unsigned k=firstPoint; k<lastPoint; k++) {
    SSxx += pow(arguments.at(k) - meanX, 2);
    SSyy += pow(values.at(k) - meanY, 2);
    SSxy += (arguments.at(k) - meanX)*(values.at(k) - meanY);
  }
  
  if (SSxx != 0 && size > 2) {
    double slope = SSxy/SSxx;
    double intercept = meanY - slope*meanX;
    double SDyx=0;
    for (unsigned k=firstPoint; k<lastPoint; k++) {
      SDyx += pow(values.at(k) - slope*arguments.at(k) - intercept, 2)/(size-2);
    }
    double SDslope = SDyx/SSxx;
    double SDintercept = SDyx*(1/size + (meanX*meanX)/SSxx);
    
    if (slope != 0) {
      peak.Value = -intercept/slope;
      if (intercept != 0) {
        peak.Uncertainty = fabs(intercept/slope)*(fabs(SDintercept/intercept) + fabs(SDslope/slope));
        peak.RSquared = (SSyy - SDyx*(size-2))/SSyy;
        peak.FisherParameter = (SSyy - SDyx*(size-2))/SDyx;
      }
    }
  } else {
    std::cerr << "Size of the vector for the linaer regression is too small ot something wrong with calculations" << std::endl;
    return peak;
  }
  return peak;
}

EffLengthTools::EffLengthTools(std::string fileWithParameters) 
{
      fFileWithParameters = fileWithParameters;
};

EffLengthTools::~EffLengthTools() {}

void EffLengthTools::LoadCalibrationParameters()
{
  if (fFileWithParameters != "") {
    struct stat buffer;
    if (stat (fFileWithParameters.c_str(), &buffer) == 0) {
      namespace pt = boost::property_tree;                                        
      pt::ptree loadPtreeRoot;
      pt::read_json(fFileWithParameters, loadPtreeRoot);
      pt::ptree temp;
      
      temp = loadPtreeRoot.get_child(kFileNumberKey);
      fNmbOfFiles = temp.get_value<int>();
      temp = loadPtreeRoot.get_child(kReferenceFileIDKey);
      fReferenceFileID = temp.get_value<int>();
      temp = loadPtreeRoot.get_child(kOutputRootFileKey);
      fOutputRootFile = temp.get_value<std::string>();
      if (fNmbOfFiles > 0 && fReferenceFileID < fNmbOfFiles) {
        std::string keyForFileName, keyForLength;
        std::string fileNameTemp;
        int lengthTemp;
        for (int i=1; i<=fNmbOfFiles; i++) {
          keyForFileName = kFileNamePattern1Key;
          keyForFileName.insert(fIteratorToNumberInFileName, std::to_string(i));
          keyForLength = kLengthPattern1Key;
          keyForLength.insert(fIteratorToNumberInLength, std::to_string(i));
          temp = loadPtreeRoot.get_child(keyForFileName);
          fileNameTemp = temp.get_value<std::string>();
          fFileNames.push_back(fileNameTemp);
          temp = loadPtreeRoot.get_child(keyForLength);
          lengthTemp = temp.get_value<int>();
          fLengths.push_back(lengthTemp);
        }
      } else if (fReferenceFileID < fNmbOfFiles) {
        std::cerr << "You had set zero or negative number of files to process in file with parameters given in input.";
        std::cout << "Or the parameter file does not have field with key : " <<  kFileNumberKey << std::endl;   
      } else {
        std::cerr << "Missing field with number of files to process ot the file reference ID is too high" << std::endl; 
      }
    } else {
        std::cerr << "No file with a given name: " << fFileWithParameters << " exiting the program." << std::endl;
    }
  } else {
    std::cerr << "No file with parameters provided" << std::endl;  
  }
}

void EffLengthTools::CalculateEffectiveLength()
{
  std::vector<std::vector<double>> meanDifferenceBetewenParams;
  fCorrDiff_vs_Length = LoadParametersFromFiles(fFileNames);
  if (fCorrDiff_vs_Length.size() > 2 || (int)fCorrDiff_vs_Length.size() >= fReferenceFileID) {
    std::vector<double> tempVector;
    for (unsigned i=0; i<fCorrDiff_vs_Length.size(); i++) {
      tempVector.clear();
      for (unsigned j=0; j<fCorrDiff_vs_Length.at(0).size(); j++ ) {
        if ((int)i == fReferenceFileID-1)
          tempVector.push_back(0);
        else
          tempVector.push_back(CalcMeanDiff(fCorrDiff_vs_Length.at(i).at(j), fCorrDiff_vs_Length.at(fReferenceFileID-1).at(j)));
      }
      meanDifferenceBetewenParams.push_back(tempVector);
    }
    double length = 0;
    std::vector<TGraph*> graphs;
    if (meanDifferenceBetewenParams.size() > 0) {
      for (unsigned i=0; i<meanDifferenceBetewenParams.at(0).size(); i++) {
        graphs.push_back(new TGraph(meanDifferenceBetewenParams.size()));
      }
      for (unsigned i=0; i<meanDifferenceBetewenParams.size(); i++) {
        if (i < fLengths.size())
          length = fLengths.at(i);
        else
          length = 0;
        for (unsigned j=0; j<meanDifferenceBetewenParams.at(0).size(); j++) {
          graphs.at(j) -> SetPoint(i, length, meanDifferenceBetewenParams.at(i).at(j));
        }
      }
      std::string titleOfGraph;
      TF1 *fitFunction = new TF1("quadraticFunction", "[0]*(x - [1])*(x - [1]) + [2]");
      TFile* fileInRoot = new TFile(fOutputRootFile.c_str(), "RECREATE" );
      fileInRoot -> cd();
      for (unsigned i=0; i<graphs.size(); i++) {
        std::cout << "Fitting quadratic function for the " << i+1 << " threshold. Estimated extremum:" << std::endl;
        titleOfGraph = "Length_vs_Correction_for_threshold_" + NumberToChar(i+1, 0);
        graphs.at(i) -> GetXaxis() -> SetTitle("Effective length [cm]");
        graphs.at(i) -> GetYaxis() -> SetTitle("Relative correction");
        graphs.at(i) -> SetMarkerStyle(20);
        graphs.at(i) -> Fit(fitFunction, "Q");
        graphs.at(i) -> Write(titleOfGraph.c_str());
        std::cout << fitFunction -> GetParameter(1) << "\t goodness of fit (Chisquared): " << fitFunction -> GetChisquare() << std::endl;
      }
      fileInRoot -> Close();
      delete fileInRoot;
      delete fitFunction;
    } else {
      std::cerr << "No data for calculations of the effective length" << std::endl;
    }
  } else {
    std::cerr << "Not enough files to calculate effective length or reference file ID was too high" << std::endl;
  }
}

std::vector<std::vector<std::vector<double>>> LoadParametersFromFiles(std::vector<std::string> filenames)
{
  double threshold, correction;
  std::string tempLine;
  std::vector<double> tempVector;
  std::vector<std::vector<double>> tempVector2D;
  std::vector<std::vector<std::vector<double>>> output;
  if (filenames.size() < 3) {
    std::cerr << "Empty or only one/two element in vector with filenames to calculate effective lengths. Not enough to calculate effective length" << std::endl;
  } else {
    for (unsigned i=0; i<filenames.size(); i++) {
      std::ifstream input(filenames.at(i).c_str());
      if (input.is_open()) {
        while(std::getline(input, tempLine)) {
          if(tempLine.at(0) == '#')
            continue;
          else {
            threshold = GetParamsFromLine(tempLine).at(3);
            correction = GetParamsFromLine(tempLine).at(4);
            if (tempVector2D.size() < threshold) {
              tempVector.push_back(correction);
              tempVector2D.push_back(tempVector);
              tempVector.clear();
            } else {
              tempVector2D.at(threshold-1).push_back(correction);
            }
          }
        }
      } else {
        std::cerr << "No file - " << filenames.at(i) << " - with corrections for for a given length" << std::endl;
      }
      output.push_back(tempVector2D);
      tempVector2D.clear();
      input.close();
    }
  }
  return output;
}

//For a better visual feeling, estimated edges (middleLeft, middleRight) on the time difference BA distribution are drawn as lines
void DrawEdgesOnHistogram(TH1D *projection_copy, Parameter middleLeft, Parameter middleRight, std::string titleOfHistogram)
{
  double maxCounts = projection_copy->GetMaximum();
  TLatex lat1, lat2;
  TCanvas *c1 = new TCanvas("c1", "", 710, 500);
  c1 -> SetHighLightColor(2);
  c1 -> SetFillColor(0);
  c1 -> SetFrameBorderMode(0);
  c1 -> SetBorderSize(2);
  c1 -> SetFrameLineWidth(2);
  
  projection_copy -> Draw();
  projection_copy -> SetLineColor(kBlack);
  projection_copy -> GetXaxis()->SetTitle("TDiff B-A [ps]"); 
  projection_copy -> GetYaxis()->SetTitle("Counts");

  double distributionWidth = projection_copy->GetStdDev();
  
  lat1.SetTextSize(0.020);
  lat1.SetTextAngle(270.);
  lat1.DrawLatex(middleLeft.Value - distributionWidth/2.5, maxCounts/2, Form("( %g )", middleLeft.Value));
// It has a bigger shift from the value middleLeft.Value than the below for middleRight.Value
// in order to draw at more or less the same distance from the line. It is because drawLatex is not so well designed
  lat2.SetTextSize(0.020);
  lat2.SetTextAngle(270.);
  lat2.DrawLatex(middleRight.Value + distributionWidth/4, maxCounts/2, Form("( %g )", middleRight.Value));

  double minArgument = (middleLeft.Value > middleRight.Value) ? middleRight.Value : middleLeft.Value;
  double maxArgument = (middleLeft.Value > middleRight.Value) ? middleLeft.Value : middleRight.Value;
  c1 -> Range(minArgument - fabs(maxArgument+minArgument)/2, -0.1*maxCounts, maxArgument + fabs(maxArgument+minArgument)/2, maxCounts + 0.1*maxCounts);
  TLine* line1 = new TLine(middleLeft.Value, -0.1*maxCounts, middleLeft.Value, maxCounts + 0.1*maxCounts);
  line1 -> SetLineColor(kBlue);
  line1 -> SetLineWidth(1);
  line1 -> SetLineStyle(kDashed);
  line1 -> Draw();

  TLine* line2 = new TLine(middleRight.Value, -0.1*maxCounts, middleRight.Value, maxCounts + 0.1*maxCounts);
  line2 -> SetLineColor(kBlue);
  line2 -> SetLineWidth(1);
  line2 -> SetLineStyle(kDashed);
  line2 -> Draw();

  c1 -> Write(titleOfHistogram.c_str());
  delete c1;
  delete line1;
  delete line2;
}

//For a better visual feeling, estimated peaks (middleAnni, middleDeex) on the aanihilation-deexcitation time difference distribution are drawn as lines
void DrawPeaksOnHistogram(TH1D *projection_copy1, TH1D *projection_copy2, Parameter middleAnni, Parameter middleDeex, std::string titleOfHistogram)
{
  TH1D *temp_copy;
  TLatex lat1, lat2;
  TCanvas *c1 = new TCanvas("c1", "", 710, 500);
  c1 -> SetHighLightColor(2);
  c1 -> SetFillColor(0);
  c1 -> SetFrameBorderMode(0);
  c1 -> SetBorderSize(2);
  c1 -> SetFrameLineWidth(2);
  
  double maxCounts = projection_copy1->GetMaximum();
  if (projection_copy1->GetMaximum() < projection_copy2->GetMaximum()) {
    maxCounts = projection_copy2->GetMaximum();
    temp_copy = projection_copy1;
    projection_copy1 = projection_copy2;
    projection_copy2 = temp_copy;
  }

  projection_copy1 -> Draw();
  projection_copy1 -> SetLineColor(kBlack);
  projection_copy1 -> GetXaxis()->SetTitle("TDiff PALS [ps]"); 
  projection_copy1 -> GetYaxis()->SetTitle("Counts");

  projection_copy2 -> Draw("same");
  projection_copy2 -> SetLineColor(kBlue);
  projection_copy2 -> GetXaxis()->SetTitle("TDiff PALS [ps]"); 
  projection_copy2 -> GetYaxis()->SetTitle("Counts");

  lat1.SetTextSize(0.020);
  lat1.SetTextAngle(270.);
  lat1.DrawLatex(middleAnni.Value, maxCounts/5, Form("( %g )", middleAnni.Value));

  lat2.SetTextSize(0.020);
  lat2.SetTextAngle(270.);
  lat2.DrawLatex(middleDeex.Value, maxCounts/5, Form("( %g )", middleDeex.Value));

  double minArgument = (middleAnni.Value > middleDeex.Value) ? middleDeex.Value : middleAnni.Value;
  double maxArgument = (middleAnni.Value > middleDeex.Value) ? middleAnni.Value : middleDeex.Value;
  c1 -> Range(minArgument - (maxArgument+minArgument)/2, -0.1*maxCounts, maxArgument + (maxArgument+minArgument)/2, maxCounts + 0.1*maxCounts);
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
  delete c1;
  delete line1;
  delete line2;
}

//For a better visual feeling derivatives are drawn on the primary distribution (TDiff BA or PALS), following Alek`s idea from presentation
void DrawDerivatives(std::string titleOfHistogram, Side side, int filterHalf, std::vector<double> arguments, 
                     std::vector<double> firstDerivativeVector, std::vector<double> secondDerivativeVector)
{
  if (arguments.size() < 2 || firstDerivativeVector.size() < 2) {
    std::cerr << "Empty or not sufficient (size < 2) vectors for drawing derivatives" << std::endl;
    return;
  }
  
  double argumentShift = arguments.at(1) - arguments.at(0);
  if (side == Side::MaxAnni || side == Side::MaxDeex) {
    std::string sideToTitle = (side == Side::MaxAnni) ? "anni" : "deex";
    
    TGraph *firstDerivative = new TGraph(firstDerivativeVector.size());
    for (unsigned i=0; i<firstDerivativeVector.size(); i++) {
      if ((int)i < 2*filterHalf || (int)i > (int)firstDerivativeVector.size() - 2*filterHalf - 1) {
        firstDerivative -> SetPoint(i, arguments.at(i) - argumentShift, 0);
      } else {
        firstDerivative -> SetPoint(i, arguments.at(i) - argumentShift, firstDerivativeVector.at(i));
      }
    }
    firstDerivative -> GetXaxis() -> SetTitle("TDiff PALS [ps]");
    firstDerivative -> GetYaxis() -> SetTitle("First Derrivative");
    firstDerivative -> SetMarkerStyle(20);
    firstDerivative -> Write((titleOfHistogram + "_firstDerivative_" + sideToTitle).c_str());
    delete firstDerivative;
  } else {
    std::string sideToTitle = (side == Side::Right) ? "right" : "left";
    TGraph *firstDerivative = new TGraph(firstDerivativeVector.size());
    TGraph *secondDerivative = new TGraph(secondDerivativeVector.size());
    for (unsigned i=0; i<firstDerivativeVector.size(); i++) {
      if ((int)i < 2*filterHalf || (int)i > (int)firstDerivativeVector.size() - 2*filterHalf - 1) {
        firstDerivative -> SetPoint(i, arguments.at(i) - argumentShift, 0);
        secondDerivative -> SetPoint(i, arguments.at(i) - 2*argumentShift, 0);
      }
      else {
        firstDerivative -> SetPoint(i, arguments.at(i) - argumentShift, firstDerivativeVector.at(i));
        secondDerivative -> SetPoint(i, arguments.at(i) - 2*argumentShift, secondDerivativeVector.at(i));
      }
    }
    firstDerivative -> GetXaxis() -> SetTitle("TDiff_B-A [ps]");
    firstDerivative -> GetYaxis() -> SetTitle("First Derrivative");
    firstDerivative -> SetMarkerStyle(20);
    secondDerivative -> GetXaxis() -> SetTitle("TDiff_B-A [ps]");
    secondDerivative -> GetYaxis() -> SetTitle("Second Derrivative");
    secondDerivative -> SetMarkerStyle(20);
    firstDerivative -> Write((titleOfHistogram + "_firstDerivative_" + sideToTitle).c_str());
    secondDerivative -> Write((titleOfHistogram + "_secondDerivative_" + sideToTitle).c_str());
    delete firstDerivative;
    delete secondDerivative;
  }
}

void PlotCorrectionHistos(std::vector<std::vector<Parameter>> vector1, std::vector<std::vector<Parameter>> vector2, double meanUnc)
{
  if (vector1.size() == 0 && vector2.size() == 0) {
    std::cerr << "Empty vectors for plotting correction histogram" << std::endl;
  } else if ((vector1.size() != vector2.size()) || (vector1.at(0).size() != vector2.at(0).size())) {
    std::cerr << "Vectors with correction have different sizes in plotting correction histogram" << std::endl;
  } else {
    TCanvas *c1 = new TCanvas("c1", "", 710, 500);
    c1 -> SetHighLightColor(2);
    c1 -> SetFillColor(0);
    c1 -> SetFrameBorderMode(0);
    c1 -> SetBorderSize(2);
    c1 -> SetFrameLineWidth(2);
      
    TLine* line1 = new TLine(-meanUnc, 0.5, -meanUnc, vector1.at(0).size() + 0.5);
    line1 -> SetLineColor(kRed);
    line1 -> SetLineWidth(1);
    line1 -> SetLineStyle(kDashed);
    TLine* line2 = new TLine(meanUnc, 0.5, meanUnc, vector1.at(0).size() + 0.5);
    line2 -> SetLineColor(kRed);
    line2 -> SetLineWidth(1);
    line2 -> SetLineStyle(kDashed);
    
    TH2D* tempHisto;
    double corr = 0.;
    for (unsigned i=0; i<vector1.size(); i++) {
      tempHisto = new TH2D(Form("PALS_correction_vs_ID_thr%d", i+1), Form("PALS correction vs scintillator ID for threshold %d", i+1), 
                                                200, -995, 1005, vector1.at(0).size() + 1, 0.5, vector1.at(0).size() + 0.5);
      tempHisto -> GetXaxis() -> SetTitle("Correction [ps]");
      tempHisto -> GetYaxis() -> SetTitle("Scintillator ID");
      for (unsigned j=0; j<vector1.at(0).size(); j++) {
        corr = (vector1.at(i).at(j).Value - vector2.at(i).at(j).Value);
        tempHisto -> Fill(corr, j+1);
      }
      gStyle->SetPalette(kRust);
      tempHisto -> Draw("colz");
      line1 -> Draw();
      line2 -> Draw();
      
      c1 -> Write( ("Correction_vs_ID_thr" + NumberToChar(i+1, 0)).c_str() );
    }
    delete c1;
    delete line1;
    delete line2;
  }
}

//Reading UniversalFileLoader convention. Used for reading old calibration file
std::vector<double> GetParamsFromLine(std::string line)
{
  std::vector<double> ParametersLine;
  char sideTemp;
  for (unsigned k=0; k<12; k++) {
    ParametersLine.push_back(0.);
  }
  std::istringstream StringToNumbers(line);
  StringToNumbers >> ParametersLine.at(0) >> ParametersLine.at(1) >> sideTemp >> ParametersLine.at(3) >> 
                     ParametersLine.at(4) >> ParametersLine.at(5) >> ParametersLine.at(6) >> ParametersLine.at(7) >> 
                     ParametersLine.at(8) >> ParametersLine.at(9) >> ParametersLine.at(10) >> ParametersLine.at(11);
  ParametersLine[2] = (double)sideTemp;
  return ParametersLine;
}

std::vector<double> CalculateDerivative(std::vector<double> values)
{
  std::vector<double> derivative;
  if (values.size() == 0)
    return derivative;
  
  if (values.size() > 1)
    derivative.push_back(values.at(1) - values.at(0));
  else
    derivative.push_back(0);
  for (unsigned i=0; i<values.size()-1; i++) {
    derivative.push_back(values.at(i+1) - values.at(i));
  }
  return derivative;
}

std::vector<double> SmoothByLinearFilter(std::vector<double> values, int filterHalf)
{
  double sumToFilter;
  std::vector<double> smoothed = values;
  unsigned minIterator = 0, maxIterator = 1;
  for (unsigned i=0; i<values.size(); i++) {
    sumToFilter = 0.;
    minIterator = ((i-filterHalf) >= 0 ? (i-filterHalf) : 0);
    maxIterator = ((i+filterHalf) < values.size() ? (i+filterHalf) : values.size()-1);
    if (i - minIterator < maxIterator - i) {
      maxIterator = i + (i - minIterator);
    } else if (i - minIterator > maxIterator - i) {
      minIterator = i - (maxIterator - i);
    }
    for (unsigned j=minIterator; j<maxIterator; j++) {
      sumToFilter += values.at(j);
    }
    smoothed.at(i) = sumToFilter/(maxIterator-minIterator+1);
  }
  return smoothed;
}

std::vector<double> ReduceBoundaryEffect(std::vector<double> values, int filterHalf)
{
  std::vector<double> reduced;
  if ((int)values.size() < 2*filterHalf+1) {
    std::cout << "Could not reduce the boundaries effect after smoothing. Vector is too small to proceed. Vector size: " << values.size() << std::endl;
    reduced = values;
  } else {
    for (unsigned i=filterHalf; i<=values.size()-filterHalf-1; i++) {
      reduced.push_back(values.at(i));
    }
  }
  return reduced;
}

Parameter CalculateMeanCorrection(std::vector<std::vector<Parameter>> vector1, std::vector<std::vector<Parameter>> vector2)
{
  Parameter meanCorrection;
  if (vector1.size() == 0 && vector2.size() == 0) {
    std::cerr << "Empty vectors for calculations of the mean corrrection" << std::endl;
  } else if ((vector1.size() != vector2.size()) || (vector1.at(0).size() != vector2.at(0).size())) {
    std::cerr << "Vectors with correction have different sizes in calculation of the mean correction. Returning zero." << std::endl;
  } else {
    double meanCorr = 0., meanUnc = 0.;
    double NumberOfChannels = vector1.size()*vector1.at(0).size();
    for (unsigned i=0; i<vector1.size(); i++) {
      for (unsigned j=0; j<vector1.at(0).size(); j++) {
        meanCorr += fabs(vector1.at(i).at(j).Value - vector2.at(i).at(j).Value)/NumberOfChannels;
        meanUnc += (vector1.at(i).at(j).Uncertainty + vector2.at(i).at(j).Uncertainty)/(2*NumberOfChannels);
      }
    }
    meanCorrection.Value = meanCorr;
    meanCorrection.Uncertainty = meanUnc;
  }
  return meanCorrection;
}

double CalcMeanDiff(std::vector<double> vector1, std::vector<double> vector2)
{
  int size = 0;
  double mean = 0;
  for (unsigned i=0; i<vector1.size() && i<vector2.size(); i++) {
    size++;
    mean += vector1.at(i) - vector2.at(i);
  }
  if (size != 0)
    return mean/size;
  else
    return 0;
}

void SaveABEdgesResults(std::string oldFileWithConstants, std::string fileWithConstants, std::string fileWithABParametersToVelocity, 
                        float effectiveLength, std::vector<std::vector<Parameter>> edgesA, std::vector<std::vector<Parameter>> edgesB)
{
  std::ifstream oldCalib;
  oldCalib.open(oldFileWithConstants.c_str(), std::ifstream::in);
  std::string tempLine;
  std::vector<std::vector<double>> oldParams;
  if (oldCalib.is_open()) {
    while(std::getline(oldCalib, tempLine))
    {
      if(tempLine.at(0) == '#')
        continue;
      else {
        oldParams.push_back(GetParamsFromLine(tempLine));
      }
    }
  } else {
    std::cerr << "No file - " << oldFileWithConstants.c_str() << " - with old corrections" << std::endl;
    return;
  }
  oldCalib.close();
    
  std::ofstream results;
  results.open((fileWithConstants).c_str());
  
  results << "# Time calibration BA constants. Correction is calculated as a mean value of the edges on a time difference B-A distribution." << std::endl;
  results << "# Edge is estimated as a 'zero' of the second derivative of Time difference distribution. Correction should be applied only to side B (convention)" << std::endl;
  results << "# Description of the parameters: layer | slot | side | threshold | offset_value_leading | offset_uncertainty_leading ";
  results << "| Mean RSquared of the linear fit | Mean Fisher Parameter | blankToFit | blankToFit | blankToFit | blankToFit" << std::endl;
  
  int threshold, slot, previousSlot;
  unsigned iterator = 0;
  std::string side;
  for (unsigned i=0; i<oldParams.size(); i++) {
    if (i == 0) {
      slot = (int)oldParams.at(i).at(1);
      previousSlot = slot;
    } else
      slot = (int)oldParams.at(i).at(1);
    
    if (previousSlot != slot ) {
      iterator++;
      previousSlot = slot;
    }
    
    side = (char)oldParams.at(i).at(2);
    threshold = (int)oldParams.at(i).at(3) - 1;
    if (side == 'A' || side == 'B') {
      results << NumberToChar(oldParams.at(i).at(0), 0) << "\t" << NumberToChar(oldParams.at(i).at(1), 0) << "\t"
                                                                  << side <<"\t" << NumberToChar(threshold+1, 0) << "\t";
      if (threshold<(int)edgesB.size() && side == 'B') {
        if (iterator<edgesB.at(threshold).size() && threshold >= 0) {
          results << NumberToChar((edgesB.at(threshold).at(iterator).Value + edgesA.at(threshold).at(iterator).Value)/2, 6) << "\t";
          results << NumberToChar(sqrt(pow(edgesB.at(threshold).at(iterator).Uncertainty, 2) + pow(edgesA.at(threshold).at(iterator).Uncertainty, 2))/2, 6) << "\t";
          results << NumberToChar((edgesB.at(threshold).at(iterator).RSquared + edgesA.at(threshold).at(iterator).RSquared)/2, 6) << "\t";
          results << NumberToChar((edgesB.at(threshold).at(iterator).FisherParameter + edgesA.at(threshold).at(iterator).FisherParameter)/2, 6) << "\t";
        } else
          results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
      } else
        results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
// Zeros are filled to fit the old type of the calibration which required a lot of fitting and therefore more parameters were included
      results << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << std::endl;
    } else {
      std::cerr << "Side of the scintillator read from the old file with constants was not one of the default (A or B)" << std::endl;
    }
  }
  results.close();
  
  results.open((fileWithABParametersToVelocity).c_str());
  
  results << "# Calibrated Veclocities for each strip" << std::endl;
  results << "# Velocity is calculated as EffecitveLength divided by HalfWidth of TimeDifferenceAB distribution" << std::endl;
  results << "# Effective Length is assumed constant and equal to " << effectiveLength;
  results << " cm, where the width of the TimeDifferenceAB distribution is" << std::endl;
  results << "# calculated as difference of middle of right edge and middle of left edge of TimeDifference AB distribution";
  results << "# Description of the parameters: layer | slot | side | threshold | velocity | velocity_uncertainity";
  results << "| Mean RSquared of the linear fit | Mean Fisher Parameter | blankToFit | blankToFit | blankToFit | blankToFit" << std::endl;
  
  iterator = 0;
  for (unsigned i=0; i<oldParams.size(); i++) {
    if (i == 0) {
      slot = (int)oldParams.at(i).at(1);
      previousSlot = slot;
    } else
      slot = (int)oldParams.at(i).at(1);
    
    if (previousSlot != slot) {
      iterator++;
      previousSlot = slot;
    }
    
    side = (char)oldParams.at(i).at(2);
    threshold = (int)oldParams.at(i).at(3) - 1;
    if (side == 'A' || side == 'B') {
      results << NumberToChar(oldParams.at(i).at(0), 0) << "\t" << NumberToChar(oldParams.at(i).at(1), 0) << "\t"
                                                                  << side << "\t" << NumberToChar(threshold+1, 0) << "\t";
      if (threshold<(int)edgesB.size()) {
        if (iterator<edgesB.at(threshold).size() && threshold >= 0) {
          results << NumberToChar(2*effectiveLength/(edgesB.at(threshold).at(iterator).Value - edgesA.at(threshold).at(iterator).Value), 6) << "\t";
          double Uncertainty = 2*effectiveLength/pow(edgesB.at(threshold).at(iterator).Value - edgesA.at(threshold).at(iterator).Value, 2);
          Uncertainty *= sqrt(pow(edgesB.at(threshold).at(iterator).Uncertainty, 2) + pow(edgesA.at(threshold).at(iterator).Uncertainty, 2));
          results << NumberToChar(Uncertainty, 6) << "\t";
          results << NumberToChar((edgesB.at(threshold).at(iterator).RSquared + edgesA.at(threshold).at(iterator).RSquared)/2, 6) << "\t";
          results << NumberToChar((edgesB.at(threshold).at(iterator).FisherParameter + edgesA.at(threshold).at(iterator).FisherParameter)/2, 6) << "\t";
        } else
          results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
      } else
        results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
// Zeros are filled to fit the old type of the calibration which required a lot of fitting and therefore more parameters were included
      results << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << std::endl;
// Zeros are filled to fit the old type of the calibration which required a lot of fitting and therefore more parameters were included
    } else {
      std::cerr << "Side of the scintillator read from the old file with constants was not one of the default (A or B)" << std::endl;
    }
  }
  results.close();
}

void SavePALSPeakResults(std::string oldFileWithConstants, std::string fileWithConstants,
                         std::vector<std::vector<Parameter>> peakAnni, std::vector<std::vector<Parameter>> peakDeex)
{
  std::vector<std::vector<double>> oldPALSParameters;
  std::ofstream results;
  
  std::ifstream oldCalib;
  oldCalib.open(oldFileWithConstants.c_str(), std::ifstream::in);
  std::string tempLine;
  std::vector<std::vector<double>> oldParams;
  if (oldCalib.is_open()) {
    while(std::getline(oldCalib, tempLine))
    {
      if(tempLine.at(0) == '#')
        continue;
      else
        oldParams.push_back(GetParamsFromLine(tempLine));
    }
  } else {
    std::cerr << "No file - " << oldFileWithConstants.c_str() << " - with old corrections" << std::endl;
    return;
  }
  oldCalib.close();
  
  int threshold, slot, previousSlot;
  unsigned iterator = 0;
  std::string side;
  struct stat buffer;
  if (stat (fileWithConstants.c_str(), &buffer) == 0) {
    std::cout << "Adding new parameters to the old PALS parameters file: " << fileWithConstants << std::endl;
    std::ifstream PALS_Corr(fileWithConstants.c_str());
    if (PALS_Corr.is_open()) {
      while(std::getline(PALS_Corr, tempLine))
      {
        if(tempLine.at(0) == '#')
          continue;
        else
          oldPALSParameters.push_back(GetParamsFromLine(tempLine));
      }
    } else {
      std::cout << "No file - " << fileWithConstants.c_str() << " - with corrections for calibration between modules" << std::endl;
    }
    results.open((fileWithConstants).c_str());
    
    results << "# Time calibration PALS constants. Correction is calculated as a difference between maximum for a Annihilation ID and Deexcitation ID distributions." << std::endl;
    results << "# Maximum is estimated as a zero of the first derivative of a time difference distribution." << std::endl;
    results << "# Description of the parameters: layer | slot | side | threshold | offset_value_leading | offset_uncertainty_leading ";
    results << "| Mean RSquared of the linear fit | Mean Fisher Parameter | blankToFit | blankToFit | blankToFit | blankToFit" << std::endl;	
    
    for (unsigned i=0; i<oldParams.size(); i++) {
      if (i == 0) {
        slot = (int)oldParams.at(i).at(1);
        previousSlot = slot;
      } else
        slot = (int)oldParams.at(i).at(1);
        
      if (previousSlot != slot ) {
        iterator++;
        previousSlot = slot;
      }
        
      side = (char)oldParams.at(i).at(2);
      threshold = (int)oldParams.at(i).at(3) - 1;
      if (side == 'A' || side == 'B') {
        results << NumberToChar(oldParams.at(i).at(0), 0) << "\t" << NumberToChar(oldParams.at(i).at(1), 0) << "\t"
                                                                  << side <<"\t" << NumberToChar(threshold+1, 0) << "\t";
        if (threshold<(int)peakAnni.size()) {
          if (iterator<peakAnni.at(threshold).size() && threshold >= 0) {
            if (isnan(peakAnni.at(threshold).at(iterator).Value) || isnan(peakDeex.at(threshold).at(iterator).Value) 
                                            || isnan(peakAnni.at(threshold).at(iterator).Uncertainty) || isnan(peakAnni.at(threshold).at(iterator).Uncertainty)) {
              results << NumberToChar(oldPALSParameters.at(i).at(4), 6) << "\t" << NumberToChar(oldPALSParameters.at(i).at(5), 6) << "\t";
              results << NumberToChar(oldPALSParameters.at(i).at(6), 6) << "\t" << NumberToChar(oldPALSParameters.at(i).at(7), 6) << "\t";
            } else {
              results << NumberToChar((peakAnni.at(threshold).at(iterator).Value - peakDeex.at(threshold).at(iterator).Value)/2 + oldPALSParameters.at(i).at(4), 6) << "\t";
              results << NumberToChar(sqrt(pow(peakAnni.at(threshold).at(iterator).Uncertainty, 2) + pow(peakDeex.at(threshold).at(iterator).Uncertainty, 2))/2, 6) << "\t";
              results << NumberToChar((peakAnni.at(threshold).at(iterator).RSquared + peakDeex.at(threshold).at(iterator).RSquared)/2, 6) << "\t";
              results << NumberToChar((peakAnni.at(threshold).at(iterator).FisherParameter + peakDeex.at(threshold).at(iterator).FisherParameter)/2, 6) << "\t";
            }
          } else
            results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
        } else
          results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
    // Zeros are filled to fit the old type of the calibration which required a lot of fitting and therefore more parameters were included
        results << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << std::endl;
      } else {
        std::cerr << "Side of the scintillator read from the old file with constants was not one of the default (A or B)" << std::endl;
      }
    }
  } else {
    std::cout << "Creating new file with PALS parameters: " << fileWithConstants << std::endl;
    results.open((fileWithConstants).c_str());
    
    results << "# Time calibration PALS constants. Correction is calculated as a difference between maximum for a Annihilation ID and Deexcitation ID distributions." << std::endl;
    results << "# Maximum is estimated as a zero of the first derivative of a time difference distribution." << std::endl;
    results << "# Description of the parameters: layer | slot | side | threshold | offset_value_leading | offset_uncertainty_leading ";
    results << "| Mean RSquared of the linear fit | Mean Fisher Parameter | blankToFit | blankToFit | blankToFit | blankToFit" << std::endl;	

    for (unsigned i=0; i<oldParams.size(); i++) {
      if (i == 0) {
        slot = (int)oldParams.at(i).at(1);
        previousSlot = slot;
      } else
        slot = (int)oldParams.at(i).at(1);
        
      if (previousSlot != slot ) {
        iterator++;
        previousSlot = slot;
      }
        
      side = (char)oldParams.at(i).at(2);
      threshold = (int)oldParams.at(i).at(3) - 1;
      if (side == 'A' || side == 'B') {
        results << NumberToChar(oldParams.at(i).at(0), 0) << "\t" << NumberToChar(oldParams.at(i).at(1), 0) << "\t"
                                                                  << side <<"\t" << NumberToChar(threshold+1, 0) << "\t";
        if (threshold<(int)peakAnni.size()) {
          if (iterator<peakAnni.at(threshold).size() && threshold >= 0) {
            if (isnan(peakAnni.at(threshold).at(iterator).Value) || isnan(peakDeex.at(threshold).at(iterator).Value) 
                                            || isnan(peakAnni.at(threshold).at(iterator).Uncertainty) || isnan(peakAnni.at(threshold).at(iterator).Uncertainty)) {
              results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
            } else {
              results << NumberToChar((peakAnni.at(threshold).at(iterator).Value - peakDeex.at(threshold).at(iterator).Value)/2, 6) << "\t";
              results << NumberToChar(sqrt(pow(peakAnni.at(threshold).at(iterator).Uncertainty, 2) + pow(peakDeex.at(threshold).at(iterator).Uncertainty, 2))/2, 6) << "\t";
              results << NumberToChar((peakAnni.at(threshold).at(iterator).RSquared + peakDeex.at(threshold).at(iterator).RSquared)/2, 6) << "\t";
              results << NumberToChar((peakAnni.at(threshold).at(iterator).FisherParameter + peakDeex.at(threshold).at(iterator).FisherParameter)/2, 6) << "\t";
            }
          } else
            results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
        } else
          results << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t" << NumberToChar(0, 6) << "\t";
    // Zeros are filled to fit the old type of the calibration which required a lot of fitting and therefore more parameters were included
        results << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << "\t" << NumberToChar(0, 0) << std::endl;
      } else {
        std::cerr << "Side of the scintillator read from the old file with constants was not one of the default (A or B)" << std::endl;
      }
    }
  }
  results.close();
}

std::string NumberToChar(double number, int precision)
{
  std::ostringstream conv;
  conv << std::fixed << std::setprecision(precision);
  conv << number;
  return conv.str();
}
