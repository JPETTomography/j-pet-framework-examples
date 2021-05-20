/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 */
#ifndef INTERFILERECONSTRUCTIONTOOLS_H
#define INTERFILERECONSTRUCTIONTOOLS_H

#include <vector>
#include <TH3D.h>

class InterfileReconstructionTools
{

public:
  static void saveToInterfile(TH3D* image, const std::string& outFileName);
};

#endif /* !INTERFILERECONSTRUCTIONTOOLS_H */
