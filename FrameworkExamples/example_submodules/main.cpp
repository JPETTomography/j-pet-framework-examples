//
//  main.cpp
//  
//
//  Created by Karol Stola on 22.04.2014.
//
//
#include <DBHandler/HeaderFiles/DBHandler.h>
#include "JPetAnalysisModuleA.h"

int main(){
	DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
	JPetAnalysisModuleA main_module;
	main_module.exec();
}