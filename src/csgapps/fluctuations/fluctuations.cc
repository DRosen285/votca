/*
 * Copyright 2009 The VOTCA Development Team (http://www.votca.org)
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

// Third party includes
#include <boost/program_options.hpp>

// VOTCA includes
#include <votca/tools/histogramnew.h>
#include <votca/tools/tokenizer.h>

// Local VOTCA includes
#include <votca/csg/cgengine.h>
#include <votca/csg/csgapplication.h>

using namespace std;
using namespace votca::csg;

class CsgFluctuations : public CsgApplication {
  string ProgramName() override { return "fluctuations"; }
  void HelpText(ostream &out) override {
    out << "calculate density fluctuations in subvolumes of the simulation "
           "box.";
    out << "Subolumes can be either cubic slabs in dimensions (x|y|z) or "
           "spherical";
    out << "slabs with respect to either the center of box or a reference "
           "molecule";
  }

  // some program options are added here

  void Initialize() override {
    CsgApplication::Initialize();
    // add program option to pick molecule
    AddProgramOptions("Fluctuation options")(
        "filter",
        boost::program_options::value<string>(&_filter)->default_value("*"),
        "filter molecule names")("rmax",
                                 boost::program_options::value<double>(),
                                 "maximal distance to be considered")(
        "rmin",
        boost::program_options::value<double>(&_rmin)->default_value(0.0),
        "minimal distance to be considered")(
        "refmol",
        boost::program_options::value<string>(&_refmol)->default_value(""),
        "Reference molecule")(
        "nbin",
        boost::program_options::value<votca::Index>(&_nbins)->default_value(
            100),
        "Number of bins")(
        "geometry", boost::program_options::value<string>(),
        "(sphere|x|y|z) Take radial or x, y, z slabs from rmin to rmax")(
        "outfile",
        boost::program_options::value<string>(&_outfilename)
            ->default_value("fluctuations.dat"),
        "Output file");
  }
  bool EvaluateOptions() override {
    CsgApplication::EvaluateOptions();
    CheckRequired("rmax");
    CheckRequired("geometry");
    return true;
  }

  // we want to process a trajectory
  bool DoTrajectory() override { return true; }
  bool DoMapping() override { return true; }

  void BeginEvaluate(Topology *top, Topology *) override {
    _filter = OptionsMap()["filter"].as<string>();
    _refmol = OptionsMap()["refmol"].as<string>();
    _rmin = OptionsMap()["rmin"].as<double>();
    _rmax = OptionsMap()["rmax"].as<double>();
    _nbins = OptionsMap()["nbin"].as<votca::Index>();
    _outfilename = OptionsMap()["outfile"].as<string>();
    _geometryinput = OptionsMap()["geometry"].as<string>();
    _nframes = 0;

    _do_spherical = false;

    if (_geometryinput == "sphere") {
      cout << "Doing spherical slabs" << endl;
      _do_spherical = true;
    } else if (_geometryinput == "x") {
      cout << "Doing slabs along x-axis" << endl;
      _dim = 0;
    } else if (_geometryinput == "y") {
      cout << "Doing slabs along  y-axis" << endl;
      _dim = 1;
    } else if (_geometryinput == "z") {
      cout << "Doing slabs along  z-axis" << endl;
      _dim = 2;
    } else {
      throw std::runtime_error("Unrecognized geometry option. (sphere|x|y|z)");
    }

    _N_avg = Eigen::VectorXd::Zero(_nbins);
    _N_sq_avg = Eigen::VectorXd::Zero(_nbins);

    if (_do_spherical) {
      cout << "Calculating fluctions for " << _rmin << "<r<" << _rmax;
      cout << "using " << _nbins << " bins" << endl;
    } else {
      cout << "Calculating fluctions for " << _rmin << "<" << _geometryinput
           << "<" << _rmax;
      cout << "using " << _nbins << " bins" << endl;
    }

    if (_refmol == "" && _do_spherical) {
      Eigen::Matrix3d box = top->getBox();
      _ref = box.rowwise().sum() / 2;

      cout << "Reference is center of box " << _ref << endl;
    }

    _outfile.open(_outfilename);
    if (!_outfile) {
      throw runtime_error("cannot open" + _outfilename + " for output");
    }
  }

  // write out results in EndEvaluate
  void EndEvaluate() override;
  // do calculation in this function
  void EvalConfiguration(Topology *conf, Topology *top_ref) override;

 protected:
  // number of particles in dV
  votca::Index _nbins;
  Eigen::VectorXd _N_avg;
  // sqare
  Eigen::VectorXd _N_sq_avg;
  string _filter;
  string _refmol;
  double _rmax;
  double _rmin;
  Eigen::Vector3d _ref;
  votca::Index _nframes;
  string _outfilename;
  ofstream _outfile;
  string _geometryinput;
  bool _do_spherical;
  votca::Index _dim;
};

int main(int argc, char **argv) {
  CsgFluctuations app;

  return app.Exec(argc, argv);
}

void CsgFluctuations::EvalConfiguration(Topology *conf, Topology *) {

  if (_refmol != "") {
    for (Bead *bead : conf->Beads()) {
      if (votca::tools::wildcmp(_refmol, bead->getName())) {
        _ref = bead->getPos();
        cout << " Solute pos " << _ref << endl;
      }
    }
  }

  votca::tools::HistogramNew hist;
  hist.Initialize(_rmin, _rmax, _nbins);

  /* check how many molecules are in each bin*/
  for (Bead *bead : conf->Beads()) {
    if (!votca::tools::wildcmp(_filter, bead->getName())) {
      continue;
    }
    double r = 0;
    if (_do_spherical) {
      Eigen::Vector3d eR = bead->getPos() - _ref;
      r = eR.norm();
    } else {
      Eigen::Vector3d eR = bead->getPos();
      if (_dim == 0) {
        r = eR.x();
      } else if (_dim == 1) {
        r = eR.y();
      } else if (_dim == 2) {
        r = eR.z();
      }
    }
    hist.Process(r);
  }

  /* update averages*/
  _N_avg += hist.data().y();
  _N_sq_avg += hist.data().y().cwiseAbs2();

  _nframes++;
}

// output everything when processing frames is done
void CsgFluctuations::EndEvaluate() {
  cout << "Writing results to " << _outfilename << endl;
  _outfile << "# radius number_fluct avg_number" << endl;

  for (votca::Index i = 0; i < _nbins; i++) {
    _N_avg[i] /= (double)_nframes;
    _N_sq_avg[i] /= (double)_nframes;
  }
  for (votca::Index i = 0; i < _nbins; i++) {
    _outfile << _rmin + (double)i * (_rmax - _rmin) / (double)_nbins << " ";
    _outfile << (_N_sq_avg[i] - _N_avg[i] * _N_avg[i]) / _N_avg[i]
             << " ";  // fluctuation
    _outfile << _N_avg[i] << endl;
  }
}

// add our user program options
