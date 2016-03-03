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
