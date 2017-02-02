/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file main.cpp
 */

#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetManager/JPetManager.h>
#include <JPetTaskLoader/JPetTaskLoader.h>
#include "TaskA.h"
#include "TimeCalibLoader.h"
#include "SignalFinder.h"
#include "HitFinder.h"
#include "TaskC2.h"
#include "TaskD.h"
#include "TaskE.h"

using namespace std;

int main(int argc, char* argv[])
{
  //DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::getManager();
  manager.parseCmdLine(argc, argv);

  // Here create all analysis modules to be used:

  manager.registerTask([]() {
    return new JPetTaskLoader("hld",
                              "tslot.raw",
                              new TaskA("TaskA: Unp to Timewindow",
                                        "Process unpacked HLD file into a tree of JPetTimeWindow objects")
                             );
  });

  manager.registerTask([]() {
    return new JPetTaskLoader("tslot.raw", "tslot_calib.raw",
                              new TimeCalibLoader("Apply time corrections from prepared calibrations",
                                  "Apply time corrections from prepared calibrations"));
  });
  manager.registerTask([]() {
    return new JPetTaskLoader("tslot_calib.raw",
                              "raw.sig",
                              new SignalFinder("SignalFinder: Create Raw Sigs",
                                  "Create Raw Signals, optional  draw TOTs per THR",
                                  true)
                             );
  });

  manager.registerTask([]() {
    return new JPetTaskLoader("raw.sig",
                              "phys.sig",
                              new TaskC2("SignalPhysTransform: Create Phys Sigs",
                                         "Create Phys Signals, ")
                             );
  });

  manager.registerTask([]() {
    return new JPetTaskLoader("phys.sig", "hits",
                              new HitFinder("Module: Pair signals",
                                            "Create hits from physical signals from both ends of scintilators"));
  });


  // manager.registerTask([](){
  //     return new JPetTaskLoader("phys.hit", "phys.hit.means",
  // 				new TaskD("Module D: Make histograms for hits",
  // 					  "Only make timeDiff histos and produce mean timeDiff value for each threshold and slot to be used by the next module"));
  //   });


  // manager.registerTask([](){
  //     return new JPetTaskLoader("phys.hit.means", "phys.hit.coincplots",
  // 				new TaskE("Module E: Filter hits",
  // 					  "Pass only hits with time diffrerence close to the peak"));
  //   });

  manager.run();
}
