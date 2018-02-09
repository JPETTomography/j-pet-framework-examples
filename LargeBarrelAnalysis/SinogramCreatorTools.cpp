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
 *  @file SinogramCreator.cpp
 */

#include "SinogramCreatorTools.h"
#include <algorithm>

//#define PRINTDEBUG

#ifdef PRINTDEBUG
#include <iostream>
#endif

const float EPSILON = 0.00001f;
const int MAX_INTENSIVITY = 255;

//see: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
std::vector<std::tuple<int, int, int>> SinogramCreatorTools::calculateLineWithoutAntialiasing(float X1, float Y1, float X2, float Y2) //check is X1/X2 and Y1/Y2 is swapped if passed as lvalue
{
  std::vector<std::tuple<int, int, int>> result;
  if (X1 > X2) {
    std::swap(X1, X2);
    std::swap(Y1, Y2);
  }

  float xDiff = (X2 - X1);
  if (std::fabs(xDiff) < EPSILON) {
    return result;
  }
  float yDiff = (Y2 - Y1);
  float d = (2 * yDiff) - xDiff;
  int y = Y1;
  for (int x = X1; x <= X2; x++) {
#ifdef PRINTDEBUG
    std::cout << "x=" << x << ": plot(" << x << "," << y << "), ";
#endif
    result.push_back(std::make_tuple(x, y, MAX_INTENSIVITY)); // set intensivity always to 255
    if (d > 0) {
#ifdef PRINTDEBUG
      std::cout << "D<0";
#endif
      y += 1;
#ifdef PRINTDEBUG
      std::cout << " D=" << d;
#endif
      d -= 2 * xDiff;
#ifdef PRINTDEBUG
      std::cout << d;
#endif
    } else {
#ifdef PRINTDEBUG
      std::cout << "D>=0";
#endif
    }
#ifdef PRINTDEBUG
    std::cout << " D=" << d;
#endif
    d += 2 * yDiff;
#ifdef PRINTDEBUG
    std::cout << "+" << d << std::endl;
#endif
  }
  return result;
}

//see https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
std::vector<std::tuple<int, int, int>> SinogramCreatorTools::calculateLineWithAntialiasing(float X1, float Y1, float X2, float Y2)
{
#ifdef PRINTDEBUG
  std::cout << "X1: " << X1 << " Y1: " << Y1 << " X2: " << X2 << " Y2: " << Y2 << std::endl;
#endif
  std::vector<std::tuple<int, int, int>> result;
  bool steep = std::abs(Y2 - Y1) > std::abs(X2 - X1);
  if (steep) {
    std::swap(X1, Y1); // there should be swap(X,Y) it is not bug
    std::swap(X2, Y2);
  }
  if (X1 > X2) {
    std::swap(X1, X2);
    std::swap(Y1, Y2);
  }

  float dx = X2 - X1;
  float dy = Y2 - Y1;
  float gradient = 1.0f;
  if (std::fabs(dx) > EPSILON) {
    gradient = dy / dx;
  }
#ifdef PRINTDEBUG
  std::cout << "Gradient: " << gradient << " dx: " << dx << " dy: " << dy << std::endl;
#endif

  int xEnd = std::round(X1);
  float yEnd = Y1 + gradient * (xEnd - X1);
  float xGap = rfpart(X1 + 0.5);
  int xPxl1 = xEnd;
#ifdef PRINTDEBUG
  std::cout << "Start: " << xPxl1;
#endif
  int yPxl1 = std::floor(yEnd);
  if (steep) {
    result.push_back(std::make_tuple(yPxl1, xPxl1, std::round(rfpart(yEnd) * xGap * MAX_INTENSIVITY)));
    result.push_back(std::make_tuple(yPxl1 + 1, xPxl1, std::round(fpart(yEnd) * xGap * MAX_INTENSIVITY)));
  } else {
    result.push_back(std::make_tuple(xPxl1, yPxl1, std::round(rfpart(yEnd) * xGap * MAX_INTENSIVITY)));
    result.push_back(std::make_tuple(xPxl1, yPxl1 + 1, std::round(fpart(yEnd) * xGap * MAX_INTENSIVITY)));
  }
  float intery = yEnd + gradient;

  xEnd = std::round(X2);
  yEnd = Y2 + gradient * (xEnd - X2);
  xGap = fpart(X2 + 0.5);
  int xPxl2 = xEnd;
#ifdef PRINTDEBUG
  std::cout << " End: " << xPxl2 << std::endl;
#endif
  int yPxl2 = std::floor(yEnd);
  if (steep) {
    result.push_back(std::make_tuple(yPxl2, xPxl2, std::round(rfpart(yEnd) * xGap * MAX_INTENSIVITY)));
    result.push_back(std::make_tuple(yPxl2 + 1, xPxl2, std::round(fpart(yEnd) * xGap * MAX_INTENSIVITY)));
  } else {
    result.push_back(std::make_tuple(xPxl2, yPxl2, std::round(rfpart(yEnd) * xGap * MAX_INTENSIVITY)));
    result.push_back(std::make_tuple(xPxl2, yPxl2 + 1, std::round(fpart(yEnd) * xGap * MAX_INTENSIVITY)));
  }

  if (steep) {
    for (int x = xPxl1 + 1; x < xPxl2; x++) {
      result.push_back(std::make_tuple(std::floor(intery), x, std::round(rfpart(intery) * MAX_INTENSIVITY)));
      result.push_back(std::make_tuple(std::floor(intery) + 1, x, std::round(fpart(intery) * MAX_INTENSIVITY)));
      intery += gradient;
    }
  } else {
    for (int x = xPxl1 + 1; x < xPxl2; x++) {
      result.push_back(std::make_tuple(x, std::floor(intery), std::round(rfpart(intery) * MAX_INTENSIVITY)));
      result.push_back(std::make_tuple(x, std::floor(intery) + 1, std::round(fpart(intery) * MAX_INTENSIVITY)));
      intery += gradient;
    }
  }
  return result;
}

float SinogramCreatorTools::fpart(float x)
{
  return x - std::floor(x);
}

float SinogramCreatorTools::rfpart(float x)
{
  return 1 - fpart(x);
}

float SinogramCreatorTools::calculateDistanceFromCenter(const float firstX, const float firstY, const float secondX, const float secondY, const float pointX, const float pointY)
{
  float vecx = secondX - firstX;
  float vecy = secondY - firstY;
  float lineLen = length2D(vecx, vecy);
  if (lineLen == 0.0) {
    return length2D(pointX - firstX, pointY - firstY);
  }

  float normx, normy;
  normx = -vecy / lineLen;
  normy = vecx / lineLen;
  float dot = ((pointX - secondX) * normx + (pointY - secondY) * normy); // Compute dot product (P3 - P1) dot( norm ( P2 - P1 ))
  return std::abs(dot);
}

float SinogramCreatorTools::length2D(const float x, const float y)
{
  return std::sqrt(x * x + y * y);
}