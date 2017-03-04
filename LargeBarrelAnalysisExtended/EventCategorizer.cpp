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
 *  @file EventCategorizer.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "EventCategorizer.h"

using namespace std;

EventCategorizer::EventCategorizer(const char * name, const char * description):JPetTask(name, description){}

void EventCategorizer::init(const JPetTaskInterface::Options&){

	INFO("Event categorization started.");
	INFO("Looking at two hit Events.");

}

void EventCategorizer::exec(){

	//if(auto event = dynamic_cast<const JPetEvent*const>(getEvent())){
	//}

}

void EventCategorizer::terminate(){

	INFO("Two hit Events done. Writing to tree");

}

void EventCategorizer::setWriter(JPetWriter* writer) { fWriter = writer; }

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
	assert(fWriter);
	for (const auto & event : events) {
		fWriter->write(event);
	}
}
