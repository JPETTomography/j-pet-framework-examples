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
 *  @file JPetRecoImageTools.cpp
 */

#include "JPetRecoImageTools.h"
#include "JPetLoggerInclude.h"

JPetRecoImageTools::JPetRecoImageTools() {}

JPetRecoImageTools::~JPetRecoImageTools() {}

std::function<double(int, int)> JPetRecoImageTools::matrixGetterFactory(const JPetSinogramType::SparseMatrix& emissionMatrix, bool isTransposed)
{
  if (!isTransposed)
  {
    return [&emissionMatrix](int i, int j) {
      if (i >= 0 && i < (int)emissionMatrix.size1() && j >= 0 && j < (int)emissionMatrix.size2()) { return emissionMatrix(i, j); }
      else
      {
        return 0.;
      }
    };
  }
  else
  {
    return [&emissionMatrix](int i, int j) {
      if (i >= 0 && i < (int)emissionMatrix.size1() && j >= 0 && j < (int)emissionMatrix.size2()) { return emissionMatrix(j, i); }
      else
      {
        return 0.;
      }
    };
  }
}

double JPetRecoImageTools::nearestNeighbour(int i, double y, std::function<double(int, int)>& func)
{
  int j = std::round(y);
  return func(i, j);
}

double JPetRecoImageTools::linear(int i, double y, std::function<double(int, int)>& func)
{
  int j = std::floor(y);
  double weight = std::abs(y - std::floor(y));
  return (1 - weight) * func(i, j) + weight * func(i, j + 1);
}

void JPetRecoImageTools::rescale(JPetSinogramType::SparseMatrix& matrix, double minCutoff, double rescaleFactor)
{

  double datamax, datamin = matrix(0, 0);
  for (unsigned int k = 0; k < matrix.size1(); k++)
  {
    for (unsigned int j = 0; j < matrix.size2(); j++)
    {
      /// Applying min Cutoff
      if (matrix(k, j) < minCutoff) matrix(k, j) = minCutoff;
      /// Finding the largest and the smallest element in the matrix.
      if (matrix(k, j) > datamax) datamax = matrix(k, j);
      if (matrix(k, j) < datamin) datamin = matrix(k, j);
    }
  }

  /// datamin represents the constant background factor.
  if ((datamax - datamin) == 0.)
  {
    WARNING("Maximum value in the matrix to rescale is 0. No rescaling performed.");
    return;
  }

  for (unsigned int k = 0; k < matrix.size1(); k++)
  {
    for (unsigned int j = 0; j < matrix.size2(); j++) { matrix(k, j) = (matrix(k, j) - datamin) * rescaleFactor / (datamax - datamin); }
  }
}

int JPetRecoImageTools::getMaxValue(const JPetSinogramType::SparseMatrix& result)
{
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size1(); i++)
  {
    for (unsigned int j = 0; j < result.size2(); j++)
    {
      if (static_cast<int>(result(i, j)) > maxValue) maxValue = static_cast<int>(result(i, j));
    }
  }
  return maxValue;
}


JPetSinogramType::SparseMatrix JPetRecoImageTools::backProject(const JPetSinogramType::Matrix3D& sinogram, float sinogramAccuracy, float tofWindow,
                                                               float lorTOFSigma, FilteredBackProjectionWeightingFunction fbpwf,
                                                               RescaleFunc rescaleFunc, int rescaleMinCutoff, int rescaleFactor)
{
  if (sinogram.size() == 0)
    return JPetSinogramType::SparseMatrix(0, 0);
  const auto sinogramBegin = sinogram.cbegin();
  int imageSize = sinogramBegin->second.size1();
  double center = (double)(imageSize - 1) / 2.0;
  double center2 = center * center;
  double angleStep = M_PI / (double)sinogramBegin->second.size2();

  JPetSinogramType::SparseMatrix reconstructedProjection(imageSize, imageSize);
  const double speed_of_light = 2.99792458 * sinogramAccuracy; // in reconstruction space, accuracy * ps/cm

  const int max_sigma_multi = 3;

  for (int angle = 0; angle < sinogramBegin->second.size2(); angle++)
  {
    double cos = std::cos((double)angle * angleStep);
    double sin = std::sin((double)angle * angleStep);

    for (const auto& tofBin : sinogram)
    {
      const double lor_tof_center = tofBin.first * tofWindow * speed_of_light;
      for (int x = 0; x < imageSize; x++)
      {
        double xMinusCenter = (double)x - center;
        double xMinusCenter2 = xMinusCenter * xMinusCenter;
        double ttemp = xMinusCenter * cos + center;

        for (int y = 0; y < imageSize; y++)
        {
          double yMinusCenter = (double)y - center;
          double yMinusCenter2 = yMinusCenter * yMinusCenter;

          if (yMinusCenter2 + xMinusCenter2 < center2)
          {
            double t = ttemp - yMinusCenter * sin;
            int n = std::round(t);

            float lor_center_x = center + cos * (n - center);
            float lor_center_y = center + sin * (n - center);

            double diffBetweenLORCenterYandY = lor_center_y - y;
            double diffBetweenLORCenterXandX = lor_center_x - x;
            double distanceToCenterOfLOR =
                std::sqrt((diffBetweenLORCenterXandX * diffBetweenLORCenterXandX) + (diffBetweenLORCenterYandY * diffBetweenLORCenterYandY));
            if (distanceToCenterOfLOR > max_sigma_multi * lorTOFSigma * speed_of_light)
              continue;
            if (x < lor_center_x) distanceToCenterOfLOR = -distanceToCenterOfLOR;
            reconstructedProjection(y, x) += tofBin.second(n, angle) * fbpwf(lor_tof_center, distanceToCenterOfLOR, lorTOFSigma);
          }
        }
      }
    }
  }

  for (int x = 0; x < imageSize; x++)
  {
    for (int y = 0; y < imageSize; y++) { reconstructedProjection(y, x) *= M_PI / 360.; }
  }
  rescaleFunc(reconstructedProjection, rescaleMinCutoff, rescaleFactor);
  return reconstructedProjection;
}

JPetSinogramType::SparseMatrix JPetRecoImageTools::backProjectMatlab(const JPetSinogramType::Matrix3D& sinogram, float sinogramAccuracy, float tofWindow,
                                                    float lorTOFSigma, FilteredBackProjectionWeightingFunction fbpwf, RescaleFunc rescaleFunc,
                                                    int rescaleMinCutoff, int rescaleFactor)
{
  if (sinogram.size() == 0)
    return JPetSinogramType::SparseMatrix(0, 0);
  const auto sinogramBegin = sinogram.cbegin();
  const int projectionLenght = sinogramBegin->second.size1();
  const int projectionAngles = sinogramBegin->second.size2();
  const double angleStep = M_PI / (double)projectionAngles;

  const int N = 2 * std::floor((double)projectionLenght / (2. * std::sqrt(2)));
  const int center =  std::floor((double)(N + 1) / 2.);

  const int xLeft = -center + 1;

  std::vector<std::vector<int>> x(N);
  std::vector<int> xTmp(N);
  std::iota(xTmp.begin(), xTmp.end(), xLeft);
  std::fill(x.begin(), x.end(), xTmp);

  const int yTop = center - 1;
  std::vector<std::vector<int>> y(N);
  { //limit visibility of `i` variable
    int i = 0;
    for(auto it = y.begin(); it != y.end(); it++) {
      *it = std::vector<int>(N, yTop - i);
      i++;
    }
  }

  const int ctrIdx = std::ceil((double)projectionLenght / 2.);

  const int imgDiag = 2 * std::ceil((double)N / std::sqrt(2)) + 1;

  if(projectionLenght < imgDiag) {
    const int rz = imgDiag - projectionLenght;
    std::cout << "Implement This!!" << std::endl;
  }

  JPetSinogramType::SparseMatrix reconstructedProjection(N, N);
  for (const auto& tofBin : sinogram) {
    for (int angle = 0; angle < projectionAngles; angle++)
    {
      const double costheta = std::cos((double)angle * angleStep);
      const double sintheta = std::sin((double)angle * angleStep);
      //std::cout << "cos: " << costheta << " sin: " << sintheta << " angle: " << angle * angleStep << " angle: " << angle << " anglestep: " << angleStep << std::endl;
      for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
          //std::cout << "x[" << i << "][" << j << "]: " << x[i][j] << " y[" << i << "][" << j << "]" << y[i][j] << std::endl;
          //const int t = std::round(x[i][j] * costheta + y[i][j] * sintheta); nearest neighbor
          //reconstructedProjection(i, j) += tofBin.second(ctrIdx + t, angle);
          const double t = (double)x[i][j] * costheta + (double)y[i][j] * sintheta;
          const int a = std::floor(t);
          if(ctrIdx + a + 1 < N)
            reconstructedProjection(i, j) += (t - (double)a) * tofBin.second(ctrIdx + a + 1, angle) + ((double)(a + 1) - t) * tofBin.second(ctrIdx + a, angle);
          
        }
      }
    }
  }

  for (int x = 0; x < N; x++){
    for (int y = 0; y < N; y++) {
      reconstructedProjection(y, x) *= M_PI / (2. * (double)projectionAngles);
    }
  }

  return reconstructedProjection;
}

double JPetRecoImageTools::FBPTOFWeight(double lor_tof_center, double lor_position, double sigma)
{
  double x = lor_position - lor_tof_center;
  x *= x;
  double y = 2 * (sigma * sigma);
  return std::exp(-x / y);
}

double JPetRecoImageTools::FBPWeight(double, double, double) { return 1.; }

JPetSinogramType::SparseMatrix JPetRecoImageTools::backProjectWithKDE(const JPetSinogramType::SparseMatrix& sinogram, Matrix2DTOF& tof, int nAngles,
                                                                      RescaleFunc rescaleFunc, int rescaleMinCutoff, int rescaleFactor)
{
  int imageSize = sinogram.size1();
  double center = (double)(imageSize - 1) / 2.0;
  double center2 = center * center;
  double angleStep = M_PI / (double)nAngles;

  JPetSinogramType::SparseMatrix reconstructedProjection(imageSize, imageSize);

  for (int angle = 0; angle < nAngles; angle++)
  {
    double cos = std::cos((double)angle * (double)angleStep);
    double sin = std::sin((double)angle * (double)angleStep);

    for (int x = 0; x < imageSize; x++)
    {
      double xMinusCenter = (double)x - center;
      double xMinusCenter2 = xMinusCenter * xMinusCenter;
      double ttemp = xMinusCenter * cos + center;

      for (int y = 0; y < imageSize; y++)
      {
        double yMinusCenter = (double)y - center;
        double yMinusCenter2 = yMinusCenter * yMinusCenter;
        if (yMinusCenter2 + xMinusCenter2 < center2)
        {
          double t = ttemp - yMinusCenter * sin;
          int n = std::floor(t + 0.5F);
          auto tofInfo = tof.find(std::make_pair(n, angle));
          std::vector<float> tofVector;
          if (tofInfo != tof.end()) { tofVector = tofInfo->second; }
          else
          {
            continue;
          }
          assert(sinogram(n, angle) == tofVector.size());
          float lor_center_x = center + cos * (n - center);
          float lor_center_y = center + sin * (n - center);
          double diffBetweenLORCenterYandY = lor_center_y - y;
          double diffBetweenLORCenterXandX = lor_center_x - x;
          double distanceToCenterOfLOR =
              std::sqrt((diffBetweenLORCenterXandX * diffBetweenLORCenterXandX) + (diffBetweenLORCenterYandY * diffBetweenLORCenterYandY));
          for (unsigned int i = 0; i < tofVector.size(); i++)
          {
            float delta = tofVector[i] * 0.299792458;
            double distributionProbability = normalDistributionProbability(distanceToCenterOfLOR, delta, 150.);
            reconstructedProjection(y, x) += distributionProbability * 1000;
          }
        }
      }
    }
  }

  for (int x = 0; x < imageSize; x++)
  {
    for (int y = 0; y < imageSize; y++) { reconstructedProjection(y, x) *= angleStep; }
  }
  rescaleFunc(reconstructedProjection, rescaleMinCutoff, rescaleFactor);
  return reconstructedProjection;
}

double JPetRecoImageTools::normalDistributionProbability(float x, float mean, float stddev)
{
  double diff = x - mean;
  double stddev2 = stddev * stddev;
  return 1. / std::sqrt(2. * M_PI * stddev2) * std::exp(-(diff * diff) / (2. * stddev2));
}

int JPetRecoImageTools::nextPowerOf2(int n)   
{ 
    n--; 
    n |= n >> 1; 
    n |= n >> 2; 
    n |= n >> 4; 
    n |= n >> 8; 
    n |= n >> 16; 
    n++; 
    return n; 
}  

JPetSinogramType::SparseMatrix JPetRecoImageTools::FilterSinogram(JPetRecoImageTools::FourierTransformFunction& ftf,
                                                                  JPetFilterInterface& filterFunction, const JPetSinogramType::SparseMatrix& sinogram)
{
  return ftf(sinogram, filterFunction);
}

JPetSinogramType::SparseMatrix JPetRecoImageTools::doFFTW1D(const JPetSinogramType::SparseMatrix& sinogram, JPetFilterInterface& filter)
{
  assert(sinogram.size1() > 1);
  int N = sinogram.size1();
  int M = nextPowerOf2((2 * N));
  int padlen = M - N;
  int nAngles = sinogram.size2();
  JPetSinogramType::SparseMatrix result(N, nAngles);
  int inFTLength = ((double)M / 2.) + 1;
  double* in = (double*)malloc(M * sizeof(double));
  double* outDouble = (double*)malloc(M * sizeof(double));
  fftw_complex* out = (fftw_complex*)fftw_malloc(inFTLength * sizeof(fftw_complex));
  fftw_plan plan = fftw_plan_dft_r2c_1d(M, in, out, FFTW_MEASURE);
  fftw_plan invPlan = fftw_plan_dft_c2r_1d(M, out, outDouble, FFTW_MEASURE);

  double* inFilter = (double*)malloc(M * sizeof(double));
  inFilter[0] = 0.25;
  for(int i = 1; i < inFTLength; i++) {
    if(i % 2 == 0) inFilter[i] = inFilter[M - i] = 0;
    else inFilter[i] = inFilter[M - i] = -1./((M_PI*(double)(i)) * (M_PI*(double)(i)));
  }

  fftw_complex* outFilter = (fftw_complex*)fftw_malloc(M * sizeof(fftw_complex));
  fftw_plan planFilter = fftw_plan_dft_r2c_1d(M, inFilter, outFilter, FFTW_MEASURE);
  fftw_execute(planFilter);
  for (int x = 0; x < nAngles; x++)
  {
    for (int y = 0; y < N; y++) { in[y] = sinogram(y ,x); }
    for (int y = N; y < M; y++) { in[y] = 0; }
    fftw_execute(plan);
    for (int y = 0; y < inFTLength; y++)
    {
      if(y == 0) {
        out[y][0] *= 2 * outFilter[y][0];
        out[y][1] *= 2 * outFilter[y][0];
      }
      out[y][0] *= 2 * outFilter[y][0] * filter((double)(y + 1) / M);
      out[y][1] *= 2 * outFilter[y][0] * filter((double)(y + 1) / M);
    }
    fftw_execute(invPlan);
    for (int y = 0; y < N ; y++) { result(y, x) = outDouble[y] / N; }
  }

  fftw_free(out);
  fftw_destroy_plan(plan);
  fftw_destroy_plan(invPlan);
  fftw_cleanup();
  return result;
}
