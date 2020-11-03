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
      
      temp = loadPtreeRoot.get_child(kCalibrationOptionKey);
      fCalibrationOption = temp.get_value<std::string>();
      if (fCalibrationOption == "single") {
        temp = loadPtreeRoot.get_child(kHistoFileSingleKey);
        fFileWithHistos = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kHistoOutputFileSingleKey);
        fFileWithHistosOut = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleKey);
        fFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kHistoNameSingleKey);
        fHistoNameSingle = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kNumberOfThresholdsKey);
        fNumberOfThresholds = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfScintillatorsKey);
        fNumberOfScintillators = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfPointsToFilterKey);
        fNumberOfPointsToFilter = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kThresholdForDerivative);
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
        temp = loadPtreeRoot.get_child(kNumberOfThresholdsKey);
        fNumberOfThresholds = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfScintillatorsKey);
        fNumberOfScintillators = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kNumberOfPointsToFilterKey);
        fNumberOfPointsToFilter = temp.get_value<int>();
        temp = loadPtreeRoot.get_child(kThresholdForDerivative);
        fThresholdForDerivative = temp.get_value<int>();
      } else if (fCalibrationOption == "final") {
        fFileWithHistos = "";
        temp = loadPtreeRoot.get_child(kConstantsFileFinalKey);
        fFileWithConstants = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileSingleKey);
        fFileWithABParameters = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileMultiKey);
        fFileWithPALSParameters = temp.get_value<std::string>();
        temp = loadPtreeRoot.get_child(kConstantsFileMultiKey);
        fOldFileWithConstants = temp.get_value<std::string>();
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
  }

  Parameter middleLeft, middleRight;
  double meanTemp;
  unsigned iterator = 0;
  std::string titleOfHistogram;
  std::vector<Parameter> tempContainerA, tempContainerB;
  
  TLatex lat3, lat4;
  TFile* fileOut = new TFile(fFileWithHistosOut.c_str(), "RECREATE" );
  TCanvas *c1 = new TCanvas( "c1", "", 710, 500 );
  c1 -> SetHighLightColor( 2 );
  c1 -> SetFillColor( 0 );
  c1 -> SetFrameBorderMode( 0 );
  c1 -> SetBorderSize( 2 );
  c1 -> SetFrameLineWidth( 2 );
  
  for (unsigned i=0; i<Histos.size(); i++) {
    tempContainerA.clear();
    tempContainerB.clear();
    for (int j=2; j<2+fNumberOfScintillators; j++) {
      iterator++;
      projection_copy = Histos[i]->ProjectionX("_px", j,j );
      meanTemp = projection_copy->GetMean(1);
      
      middleLeft = FindMiddle(projection_copy, meanTemp - 15, meanTemp - 1, iterator, Side::Left);
      middleRight = FindMiddle(projection_copy, meanTemp + 1, meanTemp + 15, iterator, Side::Right);

      tempContainerA.push_back(middleLeft);
      tempContainerB.push_back(middleRight);
        
      projection_copy -> Draw();
      projection_copy -> SetLineColor(kBlack);
      projection_copy ->GetXaxis()->SetTitle("TDiff B-A [ns]"); 
      projection_copy ->GetYaxis()->SetTitle("Counts");
        
      lat3.SetTextSize(0.020);
      lat3.SetTextAngle(270.);
      lat3.DrawLatex(middleLeft.Value, 1, Form("( %g )", middleLeft.Value));
      lat4.SetTextSize(0.020);
      lat4.SetTextAngle(270.);
      lat4.DrawLatex(middleRight.Value, 1, Form("( %g )", middleRight.Value));
            
      titleOfHistogram = "thr" + std::to_string(i+1) + "_ID_nr" + std::to_string(j-1);
      c1 -> Write( titleOfHistogram.c_str() );
    }
    fEdgesA.push_back(tempContainerA);
    fEdgesB.push_back(tempContainerB);
  }
  delete c1;
  fileOut->Close();
  fileIn->Close();
  
  std::ofstream results;
  for (unsigned i=0; i<Histos.size(); i++) {
    results.open((fFileWithConstants + "_thr" + std::to_string(i+1) + ".dat").c_str());
    for (unsigned j=0; j<fEdgesA.size()-2; j++) {
      results << NumberToChar(fEdgesB[i][j].Value + fEdgesA[i][j].Value, 5) << "\t" << NumberToChar(fEdgesA[i][j].Uncertainty + fEdgesB[i][j].Uncertainty, 5) << std::endl;
    }
    results.close();
  }
}


void CalibrationTools::CalibrateBetweenModules()
{
    
}


void CalibrationTools::GenerateCalibrationFile()
{
    
}

Parameter CalibrationTools::FindMiddle(TH1D* histo, double firstBinCenter, double lastBinCenter, unsigned iterator, Side side)
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
  firstEstimationForExtremumBin = FindExtremum(FirstDerivative, filterHalf, fNumberOfPointsToFilter, side);

  SecondDerivative.push_back(FirstDerivative[0]);
  for( unsigned i=0; i<FirstDerivative.size()-1; i++ ) {
    SecondDerivative.push_back(FirstDerivative[i+1] - FirstDerivative[i]);
  }
  Parameter finalEstimatioOfExtremum = FindPeak(Arguments, SecondDerivative, 
                                             firstEstimationForExtremumBin-5, firstEstimationForExtremumBin+5);
  finalEstimatioOfExtremum.Value = finalEstimatioOfExtremum.Value - 
                                    2*(Arguments[firstEstimationForExtremumBin+1] - Arguments[firstEstimationForExtremumBin]);
  return finalEstimatioOfExtremum;
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
  int iterator = firstPoint + sideParameter*fHalfRangeForExtremumEstimation;
  while ( ( (mean < previousMean && sideParameter == 1) || (mean > previousMean && sideParameter == -1))
                                        && iterator+fHalfRangeForExtremumEstimation < (int)Vector.size() 
                                                && iterator > fHalfRangeForExtremumEstimation) {
    previousMean = mean;
    for (int k=1; k<=fHalfRangeForExtremumEstimation; k++) {
      mean += -1*sideParameter*Vector[iterator-k];
      mean += sideParameter*Vector[iterator+k];
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

std::string NumberToChar(double number, int precision)		//Converting Numbers to char with given preicision
{
  std::ostringstream conv;
  conv << std::fixed << std::setprecision(precision);
  conv << number;
  return conv.str();
}
