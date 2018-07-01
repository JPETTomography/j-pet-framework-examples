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
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetUserTask/JPetUserTask.h"
#include "JPetEvent/JPetEvent.h"
#include <fstream>

#include "2d/barrel/options.h"
#include "2d/barrel/scanner_builder.h"
#include "2d/barrel/generic_scanner.h"
#include "2d/barrel/square_detector.h"

template <class DetectorClass>
using Scanner = PET2D::Barrel::GenericScanner<DetectorClass, short>;
template <class DetectorClass>
using ScannerBuilder = PET2D::Barrel::ScannerBuilder<DetectorClass>;

using SquareScanner = Scanner<PET2D::Barrel::SquareDetector<float>>;

class MLEMRunner : public JPetUserTask
{
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

  const std::string kOutFileNameKey = "MLEMRunner_OutFileName_std::string";
  const double speed_of_light_m_per_ps = 299792458.0e-12;
  const double cm = 0.01;

  std::string fOutFileName = "mlem_reconstruction_output.txt";
  std::string fOutFileNameTest = "mlem_reconstruction_output_test.txt";

  SquareScanner kScanner =
    ScannerBuilder<SquareScanner>::build_multiple_rings(__PET2D_BARREL(
          std::vector<float> {0.4155f, 0.458f, 0.5655f}, //radius
          std::vector<float> {0.f, 0.5f, 0.5f},         //rotation
          std::vector<int> {48, 48, 96},                //n-detectors
          0.007f,                                       //w-detector
          0.019f,                                       //h-detector
          0.f,                                          //should be d-detector
          0.40f                                         //fow radius
        ));

  std::ofstream fOutputStream;
  std::ofstream fOutputStreamTest;
};

#endif /*  !MLEMRUNNER_H */
