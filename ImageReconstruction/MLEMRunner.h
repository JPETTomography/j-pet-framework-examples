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
#include "JPetLoggerInclude.h"
#include "JPetUserTask/JPetUserTask.h"
#include <fstream>

#include "util/png_writer.h"
#include "util/progress.h"
#include "util/random.h"

#include "2d/barrel/generic_scanner.h"
#include "2d/barrel/geometry.h"
#include "2d/barrel/matrix_pixel_major.h"
#include "2d/barrel/monte_carlo.h"
#include "2d/barrel/options.h"
#include "2d/barrel/scanner_builder.h"
#include "2d/barrel/square_detector.h"
#include "2d/strip/gaussian_kernel.h"

#include "2d/geometry/pixel.h"

#include "3d/hybrid/reconstruction.h"
#include "3d/hybrid/scanner.h"
#include "common/model.h"

/**
 * @brief Module translating data to format accepted by A.Strzelecki MLEM package and running reconstruction.
 *  It automatically generate system matrix if needed or load it from file.
 *  Reconstructed image can be viewed by: MRIcroGL (http://www.mccauslandcenter.sc.edu/mricrogl/)
 *
 * Input: *.reco.unk.evt
 * Output: none
 *
 *
 * It defines 5 user options:
 *  "MLEMRunner_OutFileName_std::string" : filename of ascii file where translated data will be saved
 *  "MLEMRunner_NumberOfPixelsInOneDimension_int" : number of pixels in reconstructed image
 *  "MLEMRunner_PixelSize_double" : size of single pixel (in meters: e.g: 0.004 means 1 px corresponds 4mm)
 *  "MLEMRunner_StartPixelForPartialMatrix_int" : start pixel for system matrix
 *  "MLEMRunner_NumberOfEmissionsPerPixel_int" : number of emmisions per pixel for system matrix
 *  "MLEMRunner_TOFStepSize_double" : TOF step size
 *  "MLEMRunner_DisplayInfo_bool" : if true prints extra information about reconstruction on terminal
 *  "MLEMRunner_SystemMatrixOutputPath_std::string" : path where system matrix will be/is saved
 *  "MLEMRunner_SystemMatrixSaveFull_bool" : if true full system matrix will be saved
 *  "MLEMRunner_ReconstructionOutputPath_std::string" : path where reconstructed image will be saved
 *  "MLEMRunner_ReconstuctionIterations_int" : number of iterations in reconstruction
 *  "MLEMRunner_TOFSigmaAlongZAxis_float" : TOF sigma along Z axis
 *  "MLEMRunner_TOFSigmaAxis_float" : TOF sigma axis
 */

class MLEMRunner : public JPetUserTask
{
public:
  MLEMRunner(const char* name);
  virtual ~MLEMRunner();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
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
  using Geometry = PET2D::Barrel::Geometry<F, S>;

  using Detector = PET2D::Barrel::SquareDetector<F>;
  using Scanner2D = PET2D::Barrel::GenericScanner<Detector, S>;
  using ScannerReconstruction = PET3D::Hybrid::Scanner<Scanner2D>;
  using Kernel = PET2D::Strip::GaussianKernel<F>;
  using Reconstruction = PET3D::Hybrid::Reconstruction<ScannerReconstruction, Kernel>;

  MLEMRunner(const MLEMRunner&) = delete;
  MLEMRunner& operator=(const MLEMRunner&) = delete;

  void setUpOptions();
  bool parseEvent(const JPetEvent& event);
  SquareMatrix runGenerateSystemMatrix();
  void setSystemMatrix();
  bool setUpRunReconstructionWithMatrix();
  void runReconstruction();

  const std::string kOutFileNameKey = "MLEMRunner_OutFileName_std::string";
  const std::string kNumberOfPixelsInOneDimensionKey = "MLEMRunner_NumberOfPixelsInOneDimension_int";
  const std::string kPixelSizeKey = "MLEMRunner_PixelSize_double";
  const std::string kStartPixelForPartialMatrixKey = "MLEMRunner_StartPixelForPartialMatrix_int";
  const std::string kNumberOfEmissionsPerPixelKey = "MLEMRunner_NumberOfEmissionsPerPixel_int";
  const std::string kTOFStepKey = "MLEMRunner_TOFStepSize_double";
  const std::string kTOFSigmaAxisKey = "MLEMRunner_TOFSigmaAxis_float";
  const std::string kTOFSigmaAlongZAxisKey = "MLEMRunner_TOFSigmaAlongZAxis_float";
  const std::string kVerboseKey = "MLEMRunner_DisplayInfo_bool";
  const std::string kSystemMatrixOutputPathKey = "MLEMRunner_SystemMatrixOutputPath_std::string";
  const std::string kSystemMatrixSaveFullKey = "MLEMRunner_SystemMatrixSaveFull_bool";

  const std::string kReconstructionOutputPathKey = "MLEMRunner_ReconstructionOutputPath_std::string";
  const std::string kReconstructionIterationsKey = "MLEMRunner_ReconstuctionIterations_int";

  const double kSpeedOfLightMetersPerPs = 299792458.0e-12;
  const double kCentimetersToMeters = 0.01;
  const float EPSILON = 0.0001;

  unsigned int fMissedEvents = 0u;
  unsigned int fReconstructedEvents = 0u;

  std::string fOutFileName = "mlem_reconstruction_output.txt";
  float fHalfStripLenght = 0.f;

  Scanner2D fScanner;

  std::ofstream fOutputStream;                   // outputs data in format accepted by 3d_hybrid_reconstruction
  std::stringstream fReconstructionStringStream; // connection between parsing events and reconstruction

  int fNumberOfPixelsInOneDimension = 160;             // Dimension of 1 axis in 3d reconstructed image(n-pixels)
  double fPixelSize = 0.004;                           // Size of 1 pixel in mm(s-pixel)
  unsigned int fStartPixelForPartialMatrix = 0u;       // Starting position of matrix
  unsigned int fNumberOfEmissionsPerPixel = 100000000; // Number of emissions per pixel for system matrix
  double fTOFStep = 0.;
  float fTOFSigmaAxis = 0.06f;
  float fTOFSigmaAlongZAxis = 0.015f;

  bool fVerbose = true;               // if true prints extra information about reconstruction to cerr
  bool fSystemMatrixSaveFull = true;  // if true generate and save full system matrix, if not only 1/8 of it and then converts to full
  int fReconstructionIterations = 10; // number of iterations in reconstruction

  std::string fSystemMatrixOutputPath = "system_matix.bin";
  std::string fReconstructionOutputPath = "reconstuction";

  SquareMatrix* fMatrix = nullptr;
  Reconstruction* fReconstruction = nullptr;
  Geometry::Grid* fGrid2d = nullptr;
  Reconstruction::Grid* fGrid = nullptr;
  ScannerReconstruction* fScannerReconstruction = nullptr;
  Reconstruction::Geometry* fGeometrySOA = nullptr;
  Scanner2D fSymmetryDescriptor;
};

#endif /*  !MLEMRUNNER_H */
