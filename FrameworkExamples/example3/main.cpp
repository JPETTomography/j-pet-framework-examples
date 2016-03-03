#include <iostream>
#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetManager/JPetManager.h>
#include "./JPetAnalysisModuleContainers.h"


int main(int argc, char *argv[])
{
	DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::GetManager();
  manager.ParseCmdLine(argc, argv);
  manager.AddTask(new JPetAnalysisModuleContainers("Module Containers", "Test Containers"));
  manager.Run();
  
  return 0;
}
