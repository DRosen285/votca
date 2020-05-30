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

#ifndef VOTCA_CSG_XMLTOPOLOGYREADER_PRIVATE_H
#define VOTCA_CSG_XMLTOPOLOGYREADER_PRIVATE_H

// Standard includes
#include <stack>
#include <string>

// Third party includes
#include <boost/unordered_map.hpp>

// Local VOTCA includes
#include "votca/csg/topologyreader.h"

namespace votca {
namespace csg {

namespace TOOLS = votca::tools;

class BondBead {
 public:
  BondBead(std::string &line) {
    TOOLS::Tokenizer tok(line, ":");
    std::vector<std::string> tmp_vec = tok.ToVector();
    if (tmp_vec.size() != 2) {
      throw std::runtime_error("Wrong number of elements in bead: " + line);
    }
    molname = tmp_vec[0];
    atname = tmp_vec[1];
    molname.erase(molname.find_last_not_of(" \n\r\t") + 1);
    atname.erase(atname.find_last_not_of(" \n\r\t") + 1);
  }

  std::string molname;
  std::string atname;
};

class XMLBead {
 public:
  XMLBead(std::string _name, std::string _type, double _mass = 1.0,
          double _q = 0.0)
      : name(_name), type(_type), mass(_mass), q(_q){};
  XMLBead() = default;

  Index pid;
  std::string name;
  std::string type;
  double mass;
  double q;
};

class XMLMolecule {
 public:
  XMLMolecule(std::string _name, Index _nmols) : name(_name), nmols(_nmols) {}
  std::string name;
  Index nmols;
  Index pid;
  std::vector<XMLBead *> beads;
  std::map<std::string, XMLBead *> name2beads;
  Molecule *mi;
};

/**
 *  Reads in an xml topology
 *
 * \todo this is a sloppy implementation using expat, is just reads attributes
 * \todo should be extended to also read beads, ...
 *
 */
class XMLTopologyReader : public TopologyReader {
 public:
  /// read a topology file
  bool ReadTopology(std::string filename, Topology &top) override;
  ~XMLTopologyReader() override;

 private:
  typedef boost::unordered_multimap<std::string, XMLMolecule *> MoleculesMap;

  void ReadTopolFile(std::string file);

  void ParseRoot(tools::Property &property);
  void ParseMolecules(tools::Property &p);
  void ParseBeadTypes(tools::Property &el);
  void ParseBonded(tools::Property &el);
  void ParseBox(tools::Property &p);
  void ParseMolecule(tools::Property &p, std::string molname, Index nmols);
  void ParseBond(tools::Property &p);
  void ParseAngle(tools::Property &p);
  void ParseDihedral(tools::Property &p);

 private:
  Topology *_top;
  MoleculesMap _molecules;
  Index _mol_index;
  Index _bead_index;

  bool _has_base_topology;
};

}  // namespace csg
}  // namespace votca

#endif  // VOTCA_CSG_XMLTOPOLOGYREADER_PRIVATE_H
