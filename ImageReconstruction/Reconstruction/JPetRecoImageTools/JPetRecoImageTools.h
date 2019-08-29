/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file JPetRecoImageTools.h
 */

#ifndef _JPetRecoImageTools_H_
#define _JPetRecoImageTools_H_

#include <cmath>
#include <functional>
#include <unordered_map>
#include <vector>

#include <cassert>
#include <fftw3.h>
#include <memory>
#include <utility>

#include "JPetSinogramType.h"

#include "JPetFilterInterface.h"

template <class T, typename U>
struct PairHash
{
  size_t operator()(const std::pair<T, U>& key) const { return std::hash<T>()(key.first) ^ std::hash<U>()(key.second); }
};

struct Point
{
  Point(int x, int y)
  {
    this->x = x;
    this->y = y;
  }
  Point(const std::pair<int, int>& p)
  {
    this->x = p.first;
    this->y = p.second;
  }
  int x = 0;
  int y = 0;
};

struct SinogramPoints
{
  SinogramPoints(double value, double time)
  {
    this->value = value;
    timeVector.push_back(time);
  }

  void operator+=(const std::pair<double, double>& other)
  {
    value += other.first;
    timeVector.push_back(other.second);
  }
  double value = 0.;
  std::vector<double> timeVector;
};

class JPetRecoImageTools
{
public:
  using Matrix2DTOF = std::unordered_map<std::pair<int, int>, std::vector<float>, PairHash<int, int>>;
  using InterpolationFunc = std::function<double(int i, double y, std::function<double(int, int)>&)>;
  using RescaleFunc = std::function<void(JPetSinogramType::SparseMatrix& v, double minCutoff, double rescaleFactor)>;
  using FourierTransformFunction =
      std::function<JPetSinogramType::SparseMatrix(const JPetSinogramType::SparseMatrix& sinogram, JPetFilterInterface& filterFunction)>;
  using FilteredBackProjectionWeightingFunction = std::function<double(double, double, double)>;

  /// Returns a matrixGetter, that can be used to return matrix elements in the
  /// following way:
  /// if isTransposed is set to false, matrixGetter returns matrix[i][j]
  /// else matrixGetter returns matrix[j][i].
  /// In addition if the indices goes outside of the matrix range 0 is retuned.
  /// It is assumed that the input matrix is quadratic.
  /// The produced functions can be used as an input to interpolation functions.
  static std::function<double(int, int)> matrixGetterFactory(const JPetSinogramType::SparseMatrix& emissionMatrix, bool isTransposed = false);

  /*! \brief function returning func(i,j) where j is the nearest neighbour
   * index with respect to y.
   *  \param i discrete index being the first parameter of the function func.
   *  \param y the double value for which the nearste neighoubring discrete
   * index is calculated.
   *  \param func function that returns double value based on two discrete i,j.
   */
  static double nearestNeighbour(int i, double y, std::function<double(int, int)>& func);
  /*! \brief Linear interpolation function returning (1-t)*func(i,j) + t*
   * func(i,j+1).
   *  \param i discrete index being the first parameter of the function func.
   *  \param y the double value for which the j index  and t parameters are
   * calculated.
   *  \param func function that returns double value based on two discrete i,j.
   */
  static double linear(int i, double y, std::function<double(int, int)>& func);

  /// Rescale the Matrix in the following way:
  /// 1. All the values less than minCutoff are set to minCutoff
  /// 2. Removes the common backgroud term. So the values start at zero
  /// 3. Rescales all values by rescaleFactor/maxElement
  /// The final value range is from 0 to rescaleFactor
  static void rescale(JPetSinogramType::SparseMatrix& v, double minCutoff, double rescaleFactor);
  /// PseudoRescale which does nothing
  static void nonRescale(JPetSinogramType::SparseMatrix&, double, double) { return; }

  /*! \brief Function returning sinogram matrix.
   *  \param emissionMatrix matrix,  needs to be NxN
   *  \param nViews number of views on object, degree step is calculated as
   * (angleEnd - angleBeg) / nViews
   *  \param nScans number of scans on object, step is calculated as
   * emissionMatrix[0].size() / nScans
   *  \param interpolationFunction function to interpolate values (Optional,
   * default linear)
   *  \param angleBeg start angle for projection in deg (Optional, default 0)
   *  \param angleEnd end angle for projection in deg(Optional, default 180)
   *  \param rescaleFunc function that rescales the final result (Optional,
   * default no rescaling)
   */
  static JPetSinogramType::SparseMatrix createSinogramWithSingleInterpolation(JPetSinogramType::SparseMatrix& emissionMatrix, int nViews, int nScans,
                                                                              double angleBeg = 0, double angleEnd = 180,
                                                                              InterpolationFunc interpolationFunction = linear,
                                                                              RescaleFunc rescaleFunc = nonRescale, int rescaleMinCutoff = 0,
                                                                              int rescaleFactor = 255);

  static double calculateProjection(const JPetSinogramType::SparseMatrix& emissionMatrix, double phi, int scanNumber, int nScans,
                                    InterpolationFunc& interpolationFunction);

  /*! \brief Function returning sinogram matrix with both variables interpolated
   *  \param emissionMatrix matrix, needs to be NxN
   *  \param nAngles angle step is calculated as PI / nAngles
   *  \param rescaleFunc function that rescales the final result (Optional,
   * default no rescaling)
   *  \param rescaleMinCutoff min value to set in rescale (Optional)
   *  \param rescaleFactor max value to set in rescale (Optional)
   */
  static JPetSinogramType::SparseMatrix createSinogramWithDoubleInterpolation(JPetSinogramType::SparseMatrix& emissionMatrix, int nAngles,
                                                                              RescaleFunc rescaleFunc = nonRescale, int rescaleMinCutoff = 0,
                                                                              int rescaleFactor = 255);

  static double calculateProjection2(int step, double cos, double sin, int imageSize, double center, double length,
                                     std::function<double(int, int)> matrixGet);

  /*! \brief Function image from sinogram matrix
   *  \param sinogram matrix containing sinogram to backProject
   *  \param tof vector with information about TOF for every hit
   *  \param nAngles angle step is calculated as PI / nAngles
   *  \param rescaleFunc function that rescales the final result (Optional,
   * default no rescaling)
   *  \param rescaleMinCutoff min value to set in rescale (Optional)
   *  \param rescaleFactor max value to set in rescale (Optional)
   */
  static JPetSinogramType::SparseMatrix backProjectWithKDE(const JPetSinogramType::SparseMatrix& sinogram, Matrix2DTOF& tof, int angles,
                                                           RescaleFunc rescaleFunc, int rescaleMinCutoff, int rescaleFactor);

  static double normalDistributionProbability(float x, float mean, float stddev);

  /*! \brief Function image from sinogram matrix
   *  \param sinogram matrix containing sinogram to backProject
   *  \param sinogramAccuracy
   *  \param tofWindow
   *  \param lorTOFSigma
   *  \param fbpwf
   *  \param rescaleFunc function that rescales the final result (Optional,
   * default no rescaling)
   *  \param rescaleMinCutoff min value to set in rescale (Optional)
   *  \param rescaleFactor max value to set in rescale (Optional)
   */
  static JPetSinogramType::SparseMatrix backProject(const JPetSinogramType::Matrix3D& sinogram, float sinogramAccuracy, float tofWindow,
                                                    float lorTOFSigma, FilteredBackProjectionWeightingFunction fbpwf, RescaleFunc rescaleFunc,
                                                    int rescaleMinCutoff, int rescaleFactor);

  /*! \brief Function filtering given sinogram using fouriner implementation and
   filter
   *  \param ftf function filtering sinogram with given filter
   *  \param filter type of filter
   *  \param sinogram data to filter
  */
  static JPetSinogramType::SparseMatrix FilterSinogram(FourierTransformFunction& ftf, JPetFilterInterface& filter,
                                                       const JPetSinogramType::SparseMatrix& sinogram);

  /*! \brief Function filtering given sinogram using fouriner implementation and
 filter
 *  \param ftf function filtering sinogram with given filter
 *  \param filter type of filter
 *  \param matrix data to filter
 *  \param TOFSliceSize size of TOF slice in matrix
*/
  static JPetSinogramType::Matrix3D FilterSinograms(FourierTransformFunction& ftf, JPetFilterInterface& filter,
                                                    const JPetSinogramType::Matrix3D& matrix, float TOFSliceSize);

  /*! \brief Fourier transform implementation using FFTW library
   *  \param sinogram data to filter
   *  \param filter type of filter
   */
  static JPetSinogramType::SparseMatrix doFFTW1D(const JPetSinogramType::SparseMatrix& sinogram, JPetFilterInterface& filter);

  /*! \brief Fourier transform implementation using FFTW library
   *  \param sinogram data to filter
   *  \param filter type of filter
   */
  static JPetSinogramType::SparseMatrix doFFTW2D(const JPetSinogramType::SparseMatrix& sinogram, JPetFilterInterface& filter);

  /*! \brief Fourier transform implementation
   *  \param sinogram data to filter
   *  \param filter type of filter
   */
  static JPetSinogramType::SparseMatrix doFFTSLOW(const JPetSinogramType::SparseMatrix& sinogram, JPetFilterInterface& filter);

  /*! \brief Returns max value in given matrix
   *  \param result matrix to calculate max value
   */
  static int getMaxValue(const JPetSinogramType::SparseMatrix& result);

  /*! \brief Weighting in FBP, always returns 1;
   */
  static double FBPWeight(double, double, double);

  /*! \brief Weighting in TOF-FBP, returns value based on normal distribution in the center of tof annihilation point
   *  \param lor_tof_center center position of the LOR based on TOF value
   *  \param lor_postion distance from the center(current calculated x)
   *  \param sigma TOFsigma
   */
  static double FBPTOFWeight(double lor_tof_center, double lor_position, double sigma);

private:
  JPetRecoImageTools();
  ~JPetRecoImageTools();
  JPetRecoImageTools(const JPetRecoImageTools&) = delete;
  JPetRecoImageTools& operator=(const JPetRecoImageTools&) = delete;

  static void doFFTSLOWT(std::vector<double>& Re, std::vector<double>& Im, int size, int shift);

  static void doFFTSLOWI(std::vector<double>& Re, std::vector<double>& Im, int size, int shift);

  static inline double setToZeroIfSmall(double value, double epsilon)
  {
    if (std::abs(value) < epsilon)
      return 0;
    else
      return value;
  }
};

#endif
