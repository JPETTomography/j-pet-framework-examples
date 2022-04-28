/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file Downscaler.h
 */

#ifndef DOWNSCALER_H
#define DOWNSCALER_H

#include <JPetEvent/JPetEvent.h>
#include <JPetUserTask/JPetUserTask.h>
#include <random>

class JPetWriter;

/**
 * @brief User Task filtering JPetEvent-s according to downscaling rules
 *
 * This task acts as a simple filter on a steam of JPetEvent objects.
 * Its operation is controlled by user options speficying separate
 * "downscaling factors" for each multiplicity of hits in an event.
 * Each kind of events (1-hit, 2-hit, 3-hit events etc.) is either
 * passed on entirely (if factor=100%) or downscaled (factor < 100%)
 * in order to reduce the volume of the files occupied by events
 * for which the full statistics from the data is not needed.
 *
 */
class Downscaler : public JPetUserTask {
public:
  Downscaler(const char *name);
  virtual ~Downscaler();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  const std::string fDownscalingRatesKey =
      "Downscaler_DownscalingRates_std::vector<double>";
  std::vector<double> fDownscalingRates;
  std::mt19937 fRandomGenerator;
  std::uniform_real_distribution<double> fRandomDistribution =
      std::uniform_real_distribution<double>(0.0, 1.0);

  void saveEvent(const JPetEvent &event);
  double getNormalizedRandom();
};
#endif /* !DOWNSCALER_H */
