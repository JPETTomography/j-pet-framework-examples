#include <JPetManager/JPetManager.h>

#include <TString.h>

using namespace std;
int main(int argc, char* argv[])
{
  JPetManager& manager = JPetManager::GetManager();
  manager.ParseCmdLine(argc, argv);
  manager.Run();
}
