/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
 *  @file SinogramCreator.h
 */

#ifndef SINOGRAMCREATOR_H
#define SINOGRAMCREATOR_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetGeomMapping/JPetGeomMapping.h"
#include "JPetHit/JPetHit.h"
#include "JPetUserTask/JPetUserTask.h"
#include "SinogramCreatorTools.h"
#include <random>
#include <string>
#include <vector>

#include "JPetFilterNone.h"
#include "JPetFilterRamLak.h"
#include "JPetRecoImageTools.h"

/**
 * @brief Module creating sinogram from data
 *
 * Input: *.reco.unk.evt
 * Output: *.sino with root diagrams and sinogram *.ppm with calculated sinogram.
 *
 * Module creates 2D x/y sinogram as *.ppm files for further reconstruction, e.g. FBP.
 *
 * It is also possible to split Z coordinate into smaller sections, then for each section sinogram is created.
 * Split is done by dividing Z coordinate into equal sections starting from negative values to positive, assuming that center of Z axis is in 0.
 * It is saved as matrix(fi, r) with values corresponding to number of hits registered with that angle and distance from center,
 * with header needed by ppm P2 file:
 * P2
 * (number of horizontal elements) (number of vertical elements)
 * (maximal value in matrix).
 *
 * It defines 5 user options:
 * - "SinogramCreator_OutFileName_std::string": defines output file name where sinogram is saved
 * - "SinogramCreator_ReconstructionDistanceAccuracy_float": (in cm) defines maximal round value for distance, in cm, e.g. 0.1 means 1 bin in sinogram
 * corresponds to 0.1 cm in reality
 * - "SinogramCreator_SinogramZSplitNumber_int": defines number of splits around "z" coordinate
 * - "SinogramCreator_ScintillatorLenght_float": defines scintillator lenght in "z" coordinate
 */
class SinogramCreator : public JPetUserTask
{
public:
  explicit SinogramCreator(const char* name);
  virtual ~SinogramCreator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  bool analyzeHits(const JPetHit& firstHit, const JPetHit& secondHit);
  bool analyzeHits(const TVector3& firstHit, const float firstTOF, const TVector3& secondHit, const float secondTOF);
  bool analyzeHits(const float firstX, const float firstY, const float firstZ, const double firstTOF, const float secondX, const float secondY,
                   const float secondZ, const double secondTOF);
  /**
   * @brief Function returing value of TOF rescale, to match same annihilation point after projection from 3d to 2d
   * \param x_diff difference on x axis between hit ends
   * \param y_diff difference on y axis between hit ends
   * \param z_diff difference on z axis between hit ends
   */
  float getTOFRescaleFactor(const TVector3& posDiff) const;

  void readAndAnalyzeGojaFile();
  bool atenuation(const float value);

  const int kReconstructionMaxAngle = 180;
  int fZSplitNumber = 1;
  int fMaxDistanceNumber = 0;
  std::vector<std::pair<float, float>> fZSplitRange;

  float fMaxReconstructionLayerRadius = 0.f;    // in cm
  float fReconstructionDistanceAccuracy = 0.1f; // in cm, 1mm accuracy, maximal accuracy: 0.001f
  float fScintillatorLenght = 50.0f;            // in cm
  bool fEnableObliqueLORRemapping = false;      // enable remapping LORs to correct sinogram slices based on TOF value
  bool fEnableKDEReconstruction = false;        // enable saving extra TOF information

  const float kEPSILON = 0.0001f;

private:
  SinogramCreator(const SinogramCreator&) = delete;
  SinogramCreator& operator=(const SinogramCreator&) = delete;

  void setUpOptions();

  const std::string kOutFileNameKey = "SinogramCreator_OutFileName_std::string";

  const std::string kReconstructionDistanceAccuracy = "SinogramCreator_ReconstructionDistanceAccuracy_float";
  const std::string kZSplitNumber = "SinogramCreator_SinogramZSplitNumber_int";
  const std::string kScintillatorLenght = "SinogramCreator_ScintillatorLenght_float";
  const std::string kEnableObliqueLORRemapping = "SinogramCreator_EnableObliqueLORRemapping_bool";
  const std::string kEnableTOFReconstrution = "SinogramCreator_EnableKDEReconstruction_bool";
  const std::string kEnableNEMAAttenuation = "SinogramCreator_EnableNEMAAttenuation_bool";
  const std::string kTOFBinSliceSize = "SinogramCreator_TOFBinSliceSize_float";

  const std::string kGojaInputFilePath = "SinogramCreator_GojaInputFilesPaths_std::vector<std::string>";

  std::string fOutFileName = "sinogram.root";
  std::vector<std::string> fGojaInputFilePath;

  JPetSinogramType::WholeSinogram fSinogramData;
  float fTOFBinSliceSize = 100.f;

  bool fEnableNEMAAttenuation = false;

  unsigned int fTotalAnalyzedHits = 0;
  unsigned int fNumberOfCorrectHits = 0;
  unsigned int fTotalAttenuatedHits = 0;

  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;
};

#endif /*  !SINOGRAMCREATOR_H */
