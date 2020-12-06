/*
 *            Copyright 2009-2020 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once
#ifndef VOTCA_XTP_OPENMP_CUDA_H
#define VOTCA_XTP_OPENMP_CUDA_H

// Local VOTCA includes
#include "eigen.h"

#ifdef USE_CUDA
#include "cudapipeline.h"
#endif

/**
 * \brief Supports operations on Matrices using OPENMP and
 * CUDA. It works in two steps a) set the variables for contraction
 * b) write the normal OPENMP for-loop but use the multiplyright on the
 * individual matrix. If an OpenMP_CUDA is constructed inside an active OpenMP
 * region, it only creates a single gpu job, if the thread id is smaller than
 * number of gpus.
 *
 */

namespace votca {
namespace xtp {

class OpenMP_CUDA {
 public:
  OpenMP_CUDA();
  static Index UsingGPUs() {
#ifdef USE_CUDA
    return count_available_gpus();
#else
    return 0;
#endif
  }
  void setOperators(const std::vector<Eigen::MatrixXd>& tensor,
                    const Eigen::MatrixXd& rightoperator);
  void MultiplyRight(Eigen::MatrixXd& matrix);

  void setOperators(const Eigen::MatrixXd& leftoperator,
                    const Eigen::MatrixXd& rightoperator);
  void MultiplyLeftRight(Eigen::MatrixXd& matrix);

  void createTemporaries(Index rows, Index cols);
  void A_TDA(const Eigen::MatrixXd& matrix, const Eigen::VectorXd& vec);
  Eigen::MatrixXd A_TDA_result();

 private:
  const Eigen::MatrixXd* rightoperator_ = nullptr;
  const Eigen::MatrixXd* leftoperator_ = nullptr;

  std::vector<Eigen::MatrixXd> reduction_;
  bool inside_Parallel_region_;
  Index threadID_parent_;

#ifdef USE_CUDA

  std::vector<Index> gpuIDs_;
  std::vector<std::unique_ptr<CudaPipeline>> cuda_pips_;

  struct temporaries {
    std::unique_ptr<CudaMatrix> A = nullptr;
    std::unique_ptr<CudaMatrix> B = nullptr;
    std::unique_ptr<CudaMatrix> C = nullptr;
    std::unique_ptr<CudaMatrix> D = nullptr;
    std::unique_ptr<CudaMatrix> E = nullptr;
  };

  std::vector<temporaries> temp_;
#endif
};

}  // namespace xtp
}  // namespace votca

#endif  // VOTCA_XTP_OPENMP_CUDA_H
