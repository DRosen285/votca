#!/usr/bin/env python3
#
# Copyright 2009-2017 The VOTCA Development Team (http://www.votca.org)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import numpy as np
import sys


def readin_table(filename):
    table_dtype = {'names': ('x', 'y', 'y_flag'),
                   'formats': ('f', 'f', 'S1')}
    x, y, y_flag = np.loadtxt(filename, dtype=table_dtype, comments=['#', '@'], unpack=True)
    return x, y, y_flag


def saveto_table(filename, x, y, y_flag, comment=""):
    data = np.column_stack((x.T, y.T, y_flag.T))
    np.savetxt(filename, data, header=comment, fmt='%s')


def compare_grids(grid_a, grid_b):
    if np.any(grid_a - grid_b > 0.0001):
        print("Different grids!")
        sys.exit(1)


def fourier(r, y, omega):
    Delta_r = r[1] - r[0]
    y_hat = np.zeros_like(omega)
    np.seterr(divide='ignore', invalid='ignore', under='ignore')
    for i, omega_i in enumerate(omega):
        y_hat[i] = 2 / omega_i * Delta_r * np.sum(r * y * np.sin(2 * np.pi * omega_i * r))
    np.seterr(all='raise')
    return y_hat


def calc_dpot_ihnc_core(r, rdf_current_g, rdf_target_g, kBT, density, dump_steps=False):
    """calculates dU for all r (full width)"""

    # density 'ρ0'
    rho = density

    # difference of rdf to target 'f'
    f = rdf_target_g - rdf_current_g

    # pair correlation function 'h'
    h = rdf_current_g - 1

    # special Fourier of h
    omega = np.arange(1, len(r)) / (2 * max(r))
    h_hat = fourier(r, h, omega)

    # special Fourier of f
    f_hat = fourier(r, f, omega)

    # special Fourier of φ
    phi_hat = (2 + rho * h_hat) / (1 + rho * h_hat)**2 * rho * h_hat * f_hat

    # φ
    phi = fourier(omega, phi_hat, r)

    # zero order dU (similar IBI)
    np.seterr(divide='ignore', invalid='ignore')
    dU_order_0 = kBT * np.log(rdf_current_g / rdf_target_g)
    np.seterr(all='raise')

    # first order dU
    dU_order_1 = kBT * phi

    # total dU
    dU = dU_order_0 + dU_order_1

    # dump files
    if dump_steps:
        np.savetxt("ihnc_f_hat.xvg", (omega, f_hat), header="omega, f_hat")
        np.savetxt("ihnc_h_hat.xvg", (omega, h_hat), header="omega, h_hat")
        np.savetxt("ihnc_phi_hat.xvg", (omega, phi_hat), header="omega, phi_hat")

    return dU


def calc_dpot_ihnc(r, rdf_target_g, rdf_target_flag,
                   rdf_current_g, rdf_current_flag,
                   pot_current_U, pot_current_flag,
                   kBT, density, cut_off, g_min):
    # allways raise an error
    np.seterr(all='raise')

    # prepare dpot
    dpot_dU = np.zeros_like(pot_current_U)
    dpot_flag = np.array([''] * len(dpot_dU))

    # full range dU
    dU_full = calc_dpot_ihnc_core(r, rdf_current_g, rdf_target_g, kBT, density)

    # calculate dpot
    for i in range(len(r)):
        if rdf_target_g[i] > g_min and rdf_current_g[i] > g_min:
            dpot_dU[i] = dU_full[i]
            dpot_flag[i] = 'i'
        else:
            dpot_dU[i] = np.nan
            dpot_flag[i] = 'o'
        # check for unset value in current potential
        if 'u' in str(pot_current_flag[i]):
            dpot_dU[i] = np.nan
            dpot_flag[i] = 'o'

    # find first valid dU value
    first_dU_index = np.where(dpot_flag == 'i')[0][0]
    first_dU = dpot_dU[first_dU_index]

    # replace out of range dU values
    dpot_dU = np.where(dpot_flag == 'i', dpot_dU, first_dU)

    # shift dU to be zero at cut_off and beyond
    index_cut_off = np.searchsorted(r, cut_off)
    U_cut_off = pot_current_U[index_cut_off] + dpot_dU[index_cut_off]
    dpot_dU -= U_cut_off
    dpot_dU[index_cut_off:] = - pot_current_U[index_cut_off:]

    return dpot_dU, dpot_flag


description = """\
This script calculatess dU with the IHNC scheme.
It uses some magic tricks:
- beyond the cut_off dU is set to -U such that U becomes zero.
"""

parser = argparse.ArgumentParser(description=description)
parser.add_argument('rdf_target', type=argparse.FileType('r'))
parser.add_argument('rdf_current', type=argparse.FileType('r'))
parser.add_argument('pot_current', type=argparse.FileType('r'))
parser.add_argument('dpot', type=argparse.FileType('wb'))
parser.add_argument('kBT', type=float)
parser.add_argument('density', type=float)
parser.add_argument('cut_off', type=float)

if __name__ == '__main__':
    args = parser.parse_args()

    # load rdf and potential
    rdf_target_r, rdf_target_g, rdf_target_flag = readin_table(args.rdf_target)
    rdf_current_r, rdf_current_g, rdf_current_flag = readin_table(args.rdf_current)
    pot_current_r, pot_current_U, pot_current_flag = readin_table(args.pot_current)

    # sanity checks on grid
    compare_grids(rdf_target_r, rdf_current_r)
    r = rdf_target_r

    # calculate dpot
    dpot_dU, dpot_flag = calc_dpot_ihnc(r, rdf_target_g, rdf_target_flag,
                                        rdf_current_g, rdf_current_flag,
                                        pot_current_U, pot_current_flag,
                                        args.kBT, args.density, args.cut_off, 1e-10)

    # save dpot
    comment = "created by: {}".format(" ".join(sys.argv))
    saveto_table(args.dpot, r, dpot_dU, dpot_flag, comment)
