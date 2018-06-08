/*
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
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

#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE cubicspline_test
#include <boost/test/unit_test.hpp>
#include <votca/tools/cubicspline.h>
#include <boost/numeric/ublas/io.hpp>
#include <iostream>

using namespace votca::tools;

BOOST_AUTO_TEST_SUITE(cubicspline_test)

BOOST_AUTO_TEST_CASE(cubicspline_fit_test) {
  
  int size=80;
  ub::vector<double> x=ub::zero_vector<double>(size);
  ub::vector<double> y=ub::zero_vector<double>(size);
  for (int i=0;i<size;++i){
    x(i)=0.25*i;
    y(i)=std::sin(x(i));
  }
  CubicSpline cspline;   
  cspline.setBCInt(0);
  cspline.GenerateGrid(0.4,0.6,0.1);
  cspline.Fit(x,y);
  
  ub::vector<double> Fref=ub::zero_vector<double>(3);
  ub::vector<double> F2ref=ub::zero_vector<double>(3);
  Fref(0)=0.313364;
  Fref(1)=0.309062;
  Fref(2)=0.304759;
  F2ref(0)=0;
  F2ref(1)=-4.10698e-05;
  F2ref(2)=-7.3746e-17;
  
  ub::vector<double> F=cspline.getSplineF();
  ub::vector<double> F2=cspline.getSplineF2();
  
  bool equal1=true;
  for( unsigned i=0;i<F.size();i++){
    if(std::abs(Fref(i)-F(i))>1e-5){
      equal1=false;
      break;
    }
  }
  if(!equal1){
    std::cout<<"result F"<<std::endl;
  std::cout<<F<<std::endl;
   std::cout<<"ref F"<<std::endl;
  std::cout<<Fref<<std::endl;
  }
  BOOST_CHECK_EQUAL(equal1, true);
  
  bool equal2=true;
  for( unsigned i=0;i<F2.size();i++){
    if(std::abs(F2ref(i)-F2(i))>1e-7){
      equal2=false;
      break;
    }
  }
  if(!equal2){
    std::cout<<"result F2"<<std::endl;
  std::cout<<F2<<std::endl;
   std::cout<<"ref F2"<<std::endl;
  std::cout<<F2ref<<std::endl;
  }
  BOOST_CHECK_EQUAL(equal2, true);
 
  
}



BOOST_AUTO_TEST_SUITE_END()
