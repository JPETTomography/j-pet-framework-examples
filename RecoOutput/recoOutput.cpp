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
 *  @file recoOutput.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "recoOutput.h"

using namespace std;

RecoOutput::RecoOutput(const char * name, const char * description):JPetTask(name, description){}

void RecoOutput::init(const JPetTaskInterface::Options&){

	INFO("Reconstruction output production started.");
	INFO("Selecting only two hit events");
   
    fOutput.open("output.txt");
}

void RecoOutput::exec(){

	if(auto event = dynamic_cast<const JPetEvent*const>(getEvent())){

		  if(event->getHits().size() == 2){
                    saveEventToOutput(*event);
              }
     }       
}

void RecoOutput::terminate(){

	INFO("More than one hit Events done. Writing conrtrol histograms.");
    fOutput.close();
}

void RecoOutput::setWriter(JPetWriter* writer) { fWriter = writer; }

void RecoOutput::saveEventToOutput(const JPetEvent& event)
{
    fOutput << event.getHits().at(0).getPosX() << "\t" << event.getHits().at(0).getPosY() 
    << "\t" << event.getHits().at(0).getPosZ() << "\t" << event.getHits().at(0).getTime()*1000 << "\t"
    << event.getHits().at(1).getPosX() << "\t" << event.getHits().at(1).getPosY() 
    << "\t" << event.getHits().at(1).getPosZ() << "\t" << event.getHits().at(1).getTime()*1000 << endl;
}
