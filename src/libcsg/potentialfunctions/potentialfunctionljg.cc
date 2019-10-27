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

#include <votca/csg/potentialfunctions/potentialfunctionljg.h>

using namespace std;

namespace votca {
namespace csg {

PotentialFunctionLJG::PotentialFunctionLJG(const string &name, double min,
                                           double max)
    : PotentialFunction(name, 5, min, max) {}

double PotentialFunctionLJG::CalculateF(double r) const {

  // lj 12-6 part + gaussian
  if (r >= _min && r <= _cut_off) {
    return _lam(0) / pow(r, 12) - _lam(1) / pow(r, 6) +
           _lam(2) * exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
  } else {
    return 0.0;
  }
}

// calculate first derivative w.r.t. ith parameter
double PotentialFunctionLJG::CalculateDF(long i, double r) const {

  if (r >= _min && r <= _cut_off) {

    switch (i) {
      case 0:
        return 1.0 / pow(r, 12);
      case 1:
        return -1.0 / pow(r, 6);
      case 2:
        return exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
      case 3:
        return -1.0 * _lam(2) * (r - _lam(4)) * (r - _lam(4)) *
               exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
      case 4:
        return 2.0 * _lam(2) * _lam(3) * (r - _lam(4)) *
               exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
    }
  }
  return 0.0;
}

// calculate second derivative w.r.t. ith parameter
double PotentialFunctionLJG::CalculateD2F(long i, long j, double r) const {

  if (r >= _min && r <= _cut_off) {

    switch (i) {
      case 0:
        // all second derivatives w.r.t. c12 are zero
        return 0.0;
      case 1:
        return 0.0;
      case 2:
        switch (j) {
          case 0:
            return 0.0;
          case 1:
            return 0.0;
          case 2:
            return 0.0;
          case 3:
            return -1.0 * (r - _lam(4)) * (r - _lam(4)) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          case 4:
            return 2.0 * _lam(3) * (r - _lam(4)) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          default:
            return 0.0;
        }
      case 3:
        switch (j) {
          case 0:
            return 0.0;
          case 1:
            return 0.0;
          case 2:
            return -1.0 * (r - _lam(4)) * (r - _lam(4)) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          case 3:
            return _lam(2) * pow((r - _lam(4)), 4) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          case 4:
            return 2.0 * _lam(2) * (r - _lam(4)) *
                   (1.0 - _lam(3) * pow((r - _lam(4)), 2)) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          default:
            return 0.0;
        }
      case 4:
        switch (j) {
          case 0:
            return 0.0;
          case 1:
            return 0.0;
          case 2:
            return 2.0 * _lam(3) * (r - _lam(4)) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          case 3:
            return 2.0 * _lam(2) * (r - _lam(4)) *
                   (1.0 - _lam(3) * pow((r - _lam(4)), 2)) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
          case 4:
            return 2.0 * _lam(2) * _lam(3) *
                   (2.0 * _lam(3) * pow((r - _lam(4)), 2) - 1.0) *
                   exp(-1.0 * _lam(3) * (r - _lam(4)) * (r - _lam(4)));
        }
    }
  }
  return 0.0;
}
}  // namespace csg
}  // namespace votca
