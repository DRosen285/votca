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

#pragma once
#ifndef VOTCA_CSG_BEADLIST_H
#define VOTCA_CSG_BEADLIST_H

#include <string>
#include <vector>
#include <votca/tools/eigen.h>
#include <votca/tools/types.h>

namespace votca {
namespace csg {
/**
    \brief Generate lists of beads

    This class generates a list of beads based on some criteria, currently
    only the bead type.

*/

class Topology;
class Bead;

class BeadList {
 public:
  /// \brief Select all beads of type <select>
  Index Generate(Topology &top, const std::string &select);
  /// \brief Select all beads of type <select> withn a radius <radius> of
  /// reference vector <ref>
  Index GenerateInSphericalSubvolume(Topology &top, const std::string &select,
                                     Eigen::Vector3d ref, double radius);

  Index size() const { return _beads.size(); }

  bool empty() const { return _beads.empty(); }

  void push_back(Bead *bead) { _beads.push_back(bead); }

  using iterator = typename std::vector<Bead *>::iterator;

  iterator begin() { return _beads.begin(); }
  iterator end() { return _beads.end(); }

  const Topology & getTopology() const { return *_topology; }

 private:
  std::vector<Bead *> _beads;
  Topology *_topology = nullptr;
};

}  // namespace csg
}  // namespace votca

#endif  // VOTCA_CSG_BEADLIST_H
