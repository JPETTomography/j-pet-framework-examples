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
 *  @file CalibrationTools.h
 */

#ifndef CALIBRATIONTOOLS_H
#define CALIBRATIONTOOLS_H

#include <TH2D.h>
#include <TFile.h>
#include <TH1F.h>
#include <TF12.h>
#include <TF1.h>
#include <TF2.h>

#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>
#include <fstream>
#include <sstream>
#include <iomanip> 
#include <vector>
#include <string>
#include <cmath>

struct Parameter {
  Parameter() {Value = 0; Uncertainty = 0; RSquared = 0, FisherParameter = 0;}
  double Value;
  double Uncertainty;
  double RSquared;
  double FisherParameter;
};

enum Side {
  Right,
  Left,
  MaxAnni,
  MaxDeex,
  EdgeAnni,
  EdgeDeex
};

class CalibrationTools {
public:
  CalibrationTools();
  CalibrationTools(std::string fileWithParameters, std::string calibrationOption);
  virtual ~CalibrationTools();
  void LoadCalibrationParameters();
  void Calibrate();
  void CalibrateSingleModule();
  void CalibrateBetweenModules();
  void CalibrateTOTs();
  void GenerateCalibrationFile();
  
  void FindEdges(std::vector<TH2D*> Histos);
  void FindPeaks(std::vector<TH2D*> Histos);
  void FindTOTEdges(std::vector<TH2D*> Histos);
  Parameter FindMiddle(TH1D* histo, double firstBinCenter, double lastBinCenter, 
                                                            Side side, std::string titleOfHistogram);
  unsigned EstimateExtremumBin(const std::vector<double> vector, int filterHalf, unsigned shiftFromFilterHalf, Side side);
  Parameter FindPeak(const std::vector<double> arguments, const std::vector<double> values, unsigned firstPoint, unsigned lastPoint);
  
private:
  const std::string kHistoFileSingleKey = "File_with_histos_single_std::string";
  const std::string kHistoFileMultiKey = "File_with_histos_multi_std::string";
  const std::string kHistoOutputFileSingleKey = "File_with_histos_from calibration_single_std::string";
  const std::string kHistoOutputFileMultiKey = "File_with_histos_from calibration_multi_std::string";
  const std::string kConstantsFileSingleKey = "File_with_constants_single_std::string";
  const std::string kConstantsFileSingleToVelocityKey = "File_with_constants_single_for_velocity_std::string";
  const std::string kConstantsFileMultiKey = "File_with_constants_multi_std::string";
  const std::string kConstantsFileFinalKey = "File_with_constants_final_std::string";
  const std::string kConstantsOldFileKey = "File_with_old_constants_final_std::string";
  const std::string kHistoNameSingleKey = "Histo_name_pattern_single_std::string";
  const std::string kAnniHistoNameMultiKey = "Annihilation_histo_name_pattern_multi_std::string";
  const std::string kDeexHistoNameMultiKey = "Deexcitation_histo_name_pattern_multi_std::string";
  const std::string kSaveDerivativesKey = "Save_derivatives_bool";
  const std::string kNumberOfThresholdsKey = "Number_of_thresholds_int";
  const std::string kMinScintillatorIDKey = "Minimal_Scintillator_ID_int";
  const std::string kMaxScintillatorIDKey = "Maximal_Scintillator_ID_int";
  const std::string kNumberOfPointsToFilterKey = "Number_of_points_to_filter_int";
  const std::string kThresholdForDerivativeKey = "Threshold_for_derivative_int";
  const std::string kEffectiveLengthKey = "Effective_Length_float";
    
  std::string fFileWithParameters = "";
  std::string fCalibrationOption = "";
  std::string fFileWithHistos = "";
  std::string fFileWithHistosOut = "Out.root";
  std::string fFileWithConstants = "";
  std::string fFileWithVelocities = "";
  std::string fFileWithABParameters = "";
  std::string fFileWithABParametersToVelocity = "";
  std::string fFileWithPALSParameters = "";
  std::string fOldFileWithConstants = "";
  std::string fHistoNameSingle = "";
  std::string fAnniHistoNameMulti = "";
  std::string fDeexHistoNameMulti = "";
  bool fSaveDerivatives = false;
  int fNumberOfThresholds = 4;
  int fMinScintillatorID = 1;
  int fMaxScintillatorID = 192;
  int fNumberOfPointsToFilter = 6;
  int fThresholdForDerivative = 100;
  int fHalfRangeForExtremumEstimation = 2;
  float fEffectiveLength = 48; // cm
  unsigned binRangeForLinearFitting = 5;
  Parameter tempForSigma;
  std::vector<std::vector<Parameter>> fEdgesA;
  std::vector<std::vector<Parameter>> fEdgesB;
  std::vector<std::vector<Parameter>> fMaxAnnihilation;
  std::vector<std::vector<Parameter>> fMaxDeexcitation;
  std::vector<std::vector<Parameter>> fTOTsAnni;
  std::vector<std::vector<Parameter>> fTOTsDeex;
};

class EffLengthTools {
public:
  EffLengthTools();
  EffLengthTools(std::string fileWithParameters);
  virtual ~EffLengthTools();
  void LoadCalibrationParameters();
  void CalculateEffectiveLength();
  
private:
  const std::string kFileNumberKey = "Number_of_files_int";
  const std::string kReferenceFileIDKey = "Reference_file_ID_int";
  const std::string kOutputRootFileKey = "Output_root_file_std::string";
  const char* kFileNamePattern1Key = "File__std::string";
  const char* kLengthPattern1Key = "Length__float";
    
  std::string fFileWithParameters = "";
  int fNmbOfFiles = 0;
  int fReferenceFileID = 5;
  int fIteratorToNumberInFileName = 5;
  int fIteratorToNumberInLength = 7;
  std::string fOutputRootFile = "EffectiveLengths.root";
  std::vector<std::string> fFileNames;
  std::vector<double> fLengths;
  std::vector<std::vector<std::vector<double>>> fCorrDiff_vs_Length;
  unsigned fReferenceLengthID;  
};

std::vector<TH2D*> GetHistosFromFile(TFile* fileIn, int numberOfThresholds, std::string calibrationOption, std::string histoName, std::string histoName2);
void DrawEdgesOnHistogram(TH1D *projection_copy, Parameter middleLeft, Parameter middleRight, std::string titleOfHistogram);
void DrawPeaksOnHistogram(TH1D *projection_copy1, TH1D *projection_copy2, Parameter middleAnni, Parameter middleDeex, std::string titleOfHistogram);
void DrawDerivatives(std::string titleOfHistogram, Side side, int filterHalf, std::vector<double> arguments, 
                     std::vector<double> firstDerivativeVector, std::vector<double> secondDerivativeVector);
void PlotCorrectionHistos(std::vector<std::vector<Parameter>> vector1, std::vector<std::vector<Parameter>> vector2, double meanUnc);
unsigned FindMaximum(std::vector<double> Values, std::vector<double> Arguments, double minimalArgument, double maximalArgument);
std::pair<int, int> FindRangeForMinimum(const std::vector<double> arguments, const std::vector<double> values, double peakValue);
Parameter getSigmaFromFit(const std::vector<double> arguments, const std::vector<double> values, std::pair<int, int> range, std::string title);
std::vector<double> GetParamsFromLine(std::string line);
std::vector<std::vector<std::vector<double>>> LoadParametersFromFiles(std::vector<std::string> filenames);
std::vector<double> CalculateDerivative(std::vector<double> values);
std::vector<double> SmoothByLinearFilter(std::vector<double> values, int filterHalf);
std::vector<double> ReduceBoundaryEffect(std::vector<double> values, int filterHalf);
Parameter CalculateMeanCorrection(std::vector<std::vector<Parameter>> vector1, std::vector<std::vector<Parameter>> vector2);
double CalcMeanDiff(std::vector<double> vector1, std::vector<double> vector2);
void SaveABEdgesResults(std::string oldFileWithConstants, std::string fileWithConstants, std::string fileWithABParametersToVelocity, 
                        float fEffectiveLength, std::vector<std::vector<Parameter>> edgesA, std::vector<std::vector<Parameter>> edgesB);
void SavePALSPeakResults(std::string oldFileWithConstants, std::string fileWithConstants, 
                         std::vector<std::vector<Parameter>> peakAnni, std::vector<std::vector<Parameter>> peakDeex);
std::string NumberToChar(double number, int precision);
#endif /* !CALIBRATIONTOOLS_H */
