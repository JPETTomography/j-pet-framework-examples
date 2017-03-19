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
 *  @file EventCategorizer.h
 */

#ifndef EVENTCATEGORIZER_H
#define EVENTCATEGORIZER_H

#include <vector>
#include <map>
#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventCategorizer : public JPetTask{
    public:
    	EventCategorizer(const char * name, const char * description);
    	virtual ~EventCategorizer(){}
    	virtual void init(const JPetTaskInterface::Options& opts)override;
    	virtual void exec()override;
    	virtual void terminate()override;
    	virtual void setWriter(JPetWriter* writer)override;
    protected:
    	JPetWriter* fWriter;
    	void saveEvents(const std::vector<JPetEvent>& event);
    	bool fSaveControlHistos = true;
    };
#endif /*  !EVENTCATEGORIZER_H */
