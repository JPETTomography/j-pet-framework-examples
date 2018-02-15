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
 *  @file SinogramCreatorTools.h
 */

#ifndef SINOGRAMCREATORTOOLS_H
#define SINOGRAMCREATORTOOLS_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include <JPetUserTask/JPetUserTask.h>
#include <vector>
#include <tuple>
#include <limits>

class SinogramCreatorTools
{
public:
  using LineResult = std::vector<std::tuple<int, int, int>>;
  using Point = std::pair<float, float>;

  /*
  * @brief Calculate integer line values between 2 hits without antialiasing
  *
  * \param X1 - x position of first hit
  * \param X2 - x postion of second hit
  * \param Y1 - y postion of first hit
  * \param Y2 - y postion of second hit
  *
  * @return vector with interpolated points and intensivity: (x, y, 255) where max intensivity is 255
  */
  static LineResult calculateLineWithoutAntialiasing(float X1, float Y1, float X2, float Y2);

  /*
   * @brief Calculate integer line values between 2 hits with antialiasing
   *
   * \param X1 - x position of first hit
   * \param X2 - x postion of second hit
   * \param Y1 - y postion of first hit
   * \param Y2 - y postion of second hit
   *
   * @return vector with interpolated points and intensivity: (x, y, c)
   */
  static LineResult calculateLineWithAntialiasing(float X1, float Y1, float X2, float Y2);

  static float calculateDistanceFromCenter(const float firstX, const float firstY, const float secondX, const float secondY, const float pointX = 0.f, const float pointY = 0.f);
  static float length2D(const float x, const float y);

  /*
  * @brief Calculating intersection point of 2 lines, if they are are not intersect, std::numeric_limits<float>::max() is returned
  * \param A std::pair<float, float> with starting point of first line
  * \param B std::pair<float, float> with ending point of first line
  * \param C std::pair<float, float> with starting point of second line
  * \param D std::pair<float, float> with ending point of second line
  *
  * @return point of intersection
  */
  static Point lineIntersection(const Point& A, const Point& B, const Point& C, const Point& D);

private :
  SinogramCreatorTools() = delete;
  ~SinogramCreatorTools() = delete;
  SinogramCreatorTools(const SinogramCreatorTools&) = delete;
  SinogramCreatorTools& operator=(const SinogramCreatorTools&) = delete;

  static float fpart(float x);
  static float rfpart(float x);

  static constexpr float EPSILON = 0.00001f;
};

#endif /*  !SINOGRAMCREATORTOOLS_H */