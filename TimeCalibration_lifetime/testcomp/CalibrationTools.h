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
 *  @file CalibrationTools.h
 */

#ifndef CALIBRATIONTOOLS_H
#define CALIBRATIONTOOLS_H

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
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
#include <TH2F.h>
#include <TH1F.h>
#include <TF12.h>
#include <TF1.h>
#include <TF2.h>

#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iomanip> 
#include <vector>
#include <string>
#include <cmath>

struct Parameter {
  double Value;
  double Uncertainty;
};

enum Side {
  Right,
  Left,
  MaxAnni,
  MaxDeex
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
  void GenerateCalibrationFile();
  
  Parameter FindMiddle(TFile* output, TH1D* histo, double firstBinCenter, double lastBinCenter, 
                       unsigned iterator, Side side, std::string titleOfHistogram);
  unsigned FindExtremum(std::vector<double> Vector, int filterHalf, unsigned shiftFromFilterHalf, Side side);
  Parameter FindPeak(std::vector<double> Arguments, std::vector<double> Values, unsigned firstPoint, unsigned lastPoint);
  
private:
  const std::string kHistoFileSingleKey = "File_with_histos_single_std::string";
  const std::string kHistoFileMultiKey = "File_with_histos_multi_std::string";
  const std::string kHistoOutputFileSingleKey = "File_with_histos_from calibration_single_std::string";
  const std::string kHistoOutputFileMultiKey = "File_with_histos_from calibration_multi_std::string";
  const std::string kConstantsFileSingleKey = "File_with_constants_single_std::string";
  const std::string kConstantsFileSingleToVelocityKey = "File_with_constants_single_for_velocity_std::string";
  const std::string kConstantsFileMultiKey = "File_with_constants_multi_std::string";
  const std::string kConstantsFileFinalKey = "File_with_constants_final_std::string";
  const std::string kVelocitiesFileFinalKey = "File_with_velocities_final_std::string";
  const std::string kConstantsOldFileKey = "File_with_old_constants_final_std::string";
  const std::string kHistoNameSingleKey = "Histo_name_pattern_single_std::string";
  const std::string kAnniHistoNameMultiKey = "Annihilation_histo_name_pattern_multi_std::string";
  const std::string kDeexHistoNameMultiKey = "Deexcitation_histo_name_pattern_multi_std::string";
  const std::string kSaveDerivativesKey = "Save_derivatives_bool";
  const std::string kNumberOfThresholdsKey = "Number_of_thresholds_int";
  const std::string kNumberOfScintillatorsKey = "Number_of_scintillators_int";
  const std::string kNumberOfPointsToFilterKey = "Number_of_points_to_filter_int";
  const std::string kThresholdForDerivativeKey = "Threshold_for_derivative_int";
  const std::string kEffectiveLengthKey = "Effective_Length_int";
    
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
  bool fSaveDerivatives = true;
  int fNumberOfThresholds = 4;
  int fNumberOfScintillators = 192;
  int fNumberOfPointsToFilter = 6;
  int fThresholdForDerivative = 100;
  int fHalfRangeForExtremumEstimation = 2;
  int fEffectiveLength = 48; // cm
  std::vector<std::vector<Parameter>> fEdgesA;
  std::vector<std::vector<Parameter>> fEdgesB;
  std::vector<std::vector<Parameter>> fMaxAnnihilation;
  std::vector<std::vector<Parameter>> fMaxDeexcitation;
};

unsigned FindMaximum(std::vector<double> Values, std::vector<double> Arguments, double minimalArgument, double maximalArgument);
std::string NumberToChar(double number, int precision);
std::vector<double> GetParamsFromLine(std::string line);
#endif /* !CALIBRATIONTOOLS_H */
