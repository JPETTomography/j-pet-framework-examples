/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
 *  @file DeltaTFinder.h
 */

#ifndef DELTATFINDER_H 
#define DELTATFINDER_H 

#include <vector>
#include <map>
#include <boost/lexical_cast.hpp>
#include "TF1.h"
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetWriter/JPetWriter.h>
#include <JPetReader/JPetReader.h>
#include <JPetTreeHeader/JPetTreeHeader.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <JPetCommonTools/JPetCommonTools.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class DeltaTFinder : public JPetUserTask{
public:
	DeltaTFinder(const char * name);
	virtual ~DeltaTFinder(){};
	virtual bool init()override;
	virtual bool exec()override;
	virtual bool terminate()override;
protected:
	static std::vector<std::string> split(const std::string inString);
	static std::pair<int, std::string> retrievePositionAndFileName(const std::string inString);
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillHistosForHit(const JPetHit& hit);
	JPetGeomMapping* fBarrelMap = nullptr;
  	bool fSaveControlHistos = true;
	const std::string fInput_file_key = "inputFile_std::string";
	const std::string fPosition = "DeltaTFinder_Position_";
	const std::string fNumberOfPositionsKey = "DeltaTFinder_numberOfPositions_std::string";
	const std::string fOutputPath_key = "DeltaTFinder_outputPath_std::string";
	const std::string fVelocityCalibFile_key = "DeltaTFinder_velocityCalibFile_std::string";
	std::string fOutputPath = "";
	std::string fOutputVelocityCalibName = "";
	double fPos = 999;
 	const int fRangeAroundMaximumBin = 2;
};
#endif /*  !DELTATFINDER_H */
