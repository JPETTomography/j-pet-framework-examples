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
 *  @file RedModuleEventCategorizer.h
 */

#ifndef REDMODULEEVENTCATEGORIZER_H
#define REDMODULEEVENTCATEGORIZER_H

#include "../ModularDetectorAnalysis/EventCategorizerTools.h"
#include "RedModuleEventCategorizer.h"
#include <JPetEvent/JPetEvent.h>
#include <JPetUserTask/JPetUserTask.h>
#include <TVector3.h>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <vector>

class JPetWriter;

/**
 * @brief User Task categorizing Events
 *
 * Task attempts to add types of events to each event. Each category/type
 * has separate method for checking, if current event fulfills set of conditions.
 * These methods are defined in tools class. More than one type can be added to an event.
 * Set of controll histograms are created, unless the user decides not to produce them.
 */
class RedModuleEventCategorizer : public JPetUserTask
{
public:
  RedModuleEventCategorizer(const char* name);
  virtual ~RedModuleEventCategorizer();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  const std::string kRefDetSlotIDParamKey = "RedModule_RefDetSlotID_int";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kEventTimeParamKey = "EventFinder_EventTime_double";

  boost::property_tree::ptree fConstansTree;
  double fEventTimeWindow = 5000.0;
  bool fSaveControlHistos = true;
  int fRefDetSlotID = -1;

  void saveEvents(const std::vector<JPetEvent>& event);
  void initialiseHistograms();
};
#endif /* !REDMODULEEVENTCATEGORIZER_H */
