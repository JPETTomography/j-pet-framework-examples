/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file matrix_offests.C
 *
 *
 *
 *
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TRandom.h>
#include <TGraph.h>
#include <TLine.h>
#include <TFile.h>
#include <TMath.h>
#include <TH2D.h>
#include <TH1D.h>

#include <iostream>
#include <vector>

const int fMinPMID = 401;
const int fMaxPMID = 2896;

void matrix_offsets(std::string fileName, std::string calibJSONFileName = "calibration_constants.json") {

  TFile* fileMtxSynchro = new TFile(fileName.c_str(), "READ");
  boost::property_tree::ptree tree, pms;

  if(fileMtxSynchro->IsOpen()){
    for(int pmID = fMinPMID; pmID <= fMaxPMID; ++pmID){

      TH1F* sipm_offset = dynamic_cast<TH1F*>(fileMtxSynchro->Get(Form("offset_%d", pmID)));
      if(sipm_offset->GetEntries() < 100) { continue; }

      boost::property_tree::ptree pm_node;
      pm_node.put("matrix_offset", sipm_offset->GetMean());
      pms.push_back(std::make_pair(to_string(pmID), pm_node));
    }
  }

  tree.add_child("pm", pms);
  boost::property_tree::write_json(fOutputFileName, tree);

  // Add part of the code to read existing json file, if exists
  // and add prepared information, then overwrite
  // If option not provided or file does not exits, create a new file
}
