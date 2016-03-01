#include <TString.h>
#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetManager/JPetManager.h>

using namespace std;
int main(int argc, char* argv[])
{
	DB::SERVICES::DBHandler::createInstance("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::getManager();
  manager.parseCmdLine(argc, argv);
  manager.run();
}
