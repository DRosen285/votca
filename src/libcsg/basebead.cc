/*
 * Copyright 2009-2019 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <votca/csg/basebead.h>
#include <votca/csg/beadtype.h>

using namespace std;

namespace votca {
namespace csg {

std::string BaseBead::getBeadTypeName() {
  if (std::shared_ptr<BeadType> shared_type = type_.lock()) {
    return shared_type->getName();
  }
  assert(!"Cannot get bead type name because bead type is not accessible.");
  return "";
}

int BaseBead::getBeadTypeId() {
  if (std::shared_ptr<BeadType> shared_type = type_.lock()) {
    return shared_type->getId();
  }
  assert(!"Cannot get bead type id because bead type is not accessible.");
  return -1;
}

}  // namespace csg
}  // namespace votca
