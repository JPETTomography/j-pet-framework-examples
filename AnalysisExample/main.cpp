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
#include "TaskB.h"
#include "TaskC1.h"
#include "TaskC2.h"
#include "TaskC3.h"
#include "TaskD.h"
#include "TaskE.h"
using namespace std;
int main(int argc, char* argv[]) {
	DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::getManager();
  manager.parseCmdLine(argc, argv);
  
  // Here create all analysis modules to be used:
    manager.registerTask([](){
        return new JPetTaskLoader("hld", "tslot.raw",
				  new TaskA("Module A: Unp to TSlot Raw",
					    "Process unpacked HLD file into a tree of JPetTSlot objects"));
      });
    
    manager.registerTask([](){
        return new JPetTaskLoader("tslot.raw", "tslot.cal",
				  new TaskB("Module B: TSlot Raw to TSlot Cal",
					    "Calibrate Raw TSlot and write to TSlot Cal"));
    });
    manager.registerTask([](){
        return new JPetTaskLoader("tslot.cal", "raw.sig",
				  new TaskC1("Module C1: TSlot Cal to Raw Signal",
					     "Build Raw Signals from Calibrated TSlots"));
      });
    manager.registerTask([](){
        return new JPetTaskLoader("raw.sig", "reco.sig",
				  new TaskC2("Module C2: Raw Signals to Reco Signals",
					     "Build Reco Signals from Raw Signals"));
    });
    manager.registerTask([](){
        return new JPetTaskLoader("reco.sig", "phys.sig",
				  new TaskC3("Module C3: Reco Signals to Phys Signals",
					     "Build Physical Signals from Reco Signals"));
      });
    manager.registerTask([](){
        return new JPetTaskLoader("phys.sig", "phys.hit",
				  new TaskD("Module D: Phys Sig to Phys Hit",
					    "Build Hits from paired Phys Signals"));
      });
    manager.registerTask([](){
        return new JPetTaskLoader("phys.hit", "phys.eve",
				  new TaskE("Module E: Phys Hit to Phys Eve",
					    "Build LOR-s from paired Phys Hits"));
      });
    
    manager.run();
}
