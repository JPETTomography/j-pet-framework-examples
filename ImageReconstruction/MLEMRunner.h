/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file MLEMRunner.h
 */

#ifndef MLEMRUNNER_H
#define MLEMRUNNER_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetEvent/JPetEvent.h"
#include "JPetUserTask/JPetUserTask.h"
#include <fstream>

#include "util/png_writer.h"
#include "util/progress.h"
#include "util/random.h"

#include "2d/barrel/generic_scanner.h"
#include "2d/barrel/matrix_pixel_major.h"
#include "2d/barrel/monte_carlo.h"
#include "2d/barrel/options.h"
#include "2d/barrel/scanner_builder.h"
#include "2d/barrel/square_detector.h"

#include "2d/geometry/pixel.h"

#include "common/model.h"

using F = float;
using S = short;
using Hit = int;

using Point = PET2D::Point<F>;
using Pixel = PET2D::Pixel<S>;
using LOR = PET2D::Barrel::LOR<S>;

template <class DetectorClass> using Scanner = PET2D::Barrel::GenericScanner<DetectorClass, S>;
template <class DetectorClass> using ScannerBuilder = PET2D::Barrel::ScannerBuilder<DetectorClass>;

using SquareScanner = Scanner<PET2D::Barrel::SquareDetector<F>>;

using SquareMatrix = PET2D::Barrel::SparseMatrix<Pixel, LOR, Hit>;
using ComputeMatrix = PET2D::Barrel::MatrixPixelMajor<Pixel, LOR, Hit>;

class MLEMRunner : public JPetUserTask {
public:
  MLEMRunner(const char* name);
  virtual ~MLEMRunner();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  MLEMRunner(const MLEMRunner&) = delete;
  MLEMRunner& operator=(const MLEMRunner&) = delete;

  void setUpOptions();
  bool parseEvent(const JPetEvent& event);
  void runGenerateSystemMatrix();

  const std::string kOutFileNameKey = "MLEMRunner_OutFileName_std::string";

  const double speed_of_light_m_per_ps = 299792458.0e-12;
  const double cm = 0.01;
  const float EPSILON = 0.0001;

  std::string fOutFileName = "mlem_reconstruction_output.txt";
  std::string fOutFileNameTest = "mlem_reconstruction_output_test.txt";
  float fHalfStripLenght = 0.f;

  SquareScanner fScanner;

  std::ofstream fOutputStream;
  std::ofstream fOutputStreamTest;

  // SYSTEM MATRIX

  const std::string kNumberOfPixelsInOneDimensionKey = "MLEMRunner_NumberOfPixelsInOneDimension_int";
  const std::string kPixelSizeKey = "MLEMRunner_PixelSize_double";
  const std::string kStartPixelForPartialMatrixKey = "MLEMRunner_StartPixelForPartialMatrix_int";
  const std::string kNumberOfEmissionsPerPixelKey = "MLEMRunner_NumberOfEmissionsPerPixel_int";
  const std::string kTOFStepKey = "MLEMRunner_TOFStepSize_double";
  const std::string kVerboseKey = "MLEMRunner_DisplayInfo_bool";
  const std::string kSystemMatrixOutputPathKey = "MLEMRunner_SystemMatrixOutputPath_std::string";
  const std::string kSystemMatrixSaveFullKey = "MLEMRunner_SystemMatrixSaveFull_bool";

  int fNumberOfPixelsInOneDimension = 160;
  double fPixelSize = 0.004;
  unsigned int fStartPixelForPartialMatrix = 0u;
  unsigned int fNumberOfEmissionsPerPixel = 1000000;
  double fTOFStep = 0.;

  bool fVerbose = true;
  bool fSystemMatrixSaveFull = true;

  std::string fSystemMatrixOutputPath = "system_matix.bin";
};

#endif /*  !MLEMRUNNER_H */
