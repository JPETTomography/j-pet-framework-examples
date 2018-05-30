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
 *  @file main.cpp
 */

#include <JPetManager/JPetManager.h>
#include <JPetGeantParser/JPetGeantParser.h>

#include "TimeWindowCreator.h"
#include "TimeCalibLoader.h"
#include "SignalFinder.h"
#include "SignalTransformer.h"
#include "HitFinder.h"
#include "EventFinder.h"
#include "EventCategorizer.h"

#include "./JPetOptionsTools/JPetOptionsTools.h"
#include "./JPetCmdParser/JPetCmdParser.h" 
#include "./JPetOptionsGenerator/JPetOptionsGenerator.h"

using namespace std;

int main(int argc, const char* argv[])
{

    JPetOptionsGenerator optionsGenerator;
    JPetCmdParser parser;
    auto optionsFromCmdLine = parser.parseCmdLineArgs(argc, argv); 
    std::map<std::string, boost::any> fOptions = optionsGenerator.generateAndValidateOptions(optionsFromCmdLine); 
    auto fileType = jpet_options_tools::FileTypeChecker::getInputFileType(fOptions);


    JPetManager& manager = JPetManager::getManager();

    if (fileType == jpet_options_tools::FileTypeChecker::kMCGeant) {
        INFO("processing MC \n");

        manager.registerTask<JPetGeantParser>("JPetGeantParser");
        //manager.registerTask<EventFinder>("EventFinder");
        //manager.registerTask<EventCategorizer>("EventCategorizer");

        manager.useTask("JPetGeantParser", "mcGeant", "mc.hits"); 
        //manager.useTask("EventFinder", "mc.hits", "mc.unk.evt");
        //manager.useTask("EventCategorizer", "mc.unk.evt", "mc.cat.evt");

    } else {
        INFO("processing DATA \n");

        manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
        manager.registerTask<TimeCalibLoader>("TimeCalibLoader");
        manager.registerTask<SignalFinder>("SignalFinder");
        manager.registerTask<SignalTransformer>("SignalTransformer");
        manager.registerTask<HitFinder>("HitFinder");
        manager.registerTask<EventFinder>("EventFinder");
        manager.registerTask<EventCategorizer>("EventCategorizer");

        manager.useTask("TimeWindowCreator", "hld", "tslot.raw");
        manager.useTask("TimeCalibLoader", "tslot.raw", "tslot.calib");
        manager.useTask("SignalFinder", "tslot.calib", "raw.sig");
        manager.useTask("SignalTransformer", "raw.sig", "phys.sig");
        manager.useTask("HitFinder", "phys.sig", "hits");
        manager.useTask("EventFinder", "hits", "unk.evt");
        manager.useTask("EventCategorizer", "unk.evt", "cat.evt");

    }


    manager.run(argc, argv);
}
