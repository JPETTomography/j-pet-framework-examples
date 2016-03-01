#include <iostream>
#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetWriter/JPetWriter.h>
#include <JPetReader/JPetReader.h>
#include <JPetManager/JPetManager.h>
#include "./JPetAnalysisModuleA.h"
#include "./JPetAnalysisModuleB.h"

#include "JPetHLDReader/JPetHLDReader.h"


using namespace std;
int main(int argc, char *argv[])
{
	DB::SERVICES::DBHandler::createInstance("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::GetManager();
  manager.AddTask(new JPetAnalysisModuleA("Module A", "Process Experimental data into JPetSignal structures."));
  manager.AddTask(new JPetAnalysisModuleB("Module B", "Merge JPetSignals into Events."));
  manager.Run();

}
