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

#define BOOST_TEST_MODULE graph_test
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <exception>
#include <iostream>
#include <iostream>
#include <votca/tools/edge.h>
#include <votca/tools/graph.h>
#include <votca/tools/graphnode.h>
using namespace std;
using namespace votca::tools;

// used for rounding doubles so we can compare them
double round_(double v, int p) {
  v *= pow(10, p);
  v = round(v);
  v /= pow(10, p);
  return v;
}

BOOST_AUTO_TEST_SUITE(graph_test)

BOOST_AUTO_TEST_CASE(constructors_test) { Graph g; }

/** 
 * \brief Test on isolated nodes method
 *
 * The isolated nodes method is meant to grab any nodes that have no edges, as
 * in they exist as islands within the context of the graph. 
 */
BOOST_AUTO_TEST_CASE(isolatednodes_test) {

  {

    /// Here gn is a single node as is thus isolated
    vector<Edge> vec_ed;
    GraphNode gn;
    unordered_map<int, GraphNode> m_gn;
    m_gn[0] = gn;

    Graph g(vec_ed, m_gn);
    auto iso_gn = g.getIsolatedNodes();
    BOOST_CHECK_EQUAL(iso_gn.at(0).first, 0);
  }

  {

    /// In this test case gn, gn1 and gn2 are all islands no edges have been
    /// specified to connect them. Calling getIsolatedNodes() thus returns all
    /// three of them. 
    vector<Edge> vec_ed;
    GraphNode gn;
    GraphNode gn1;
    GraphNode gn2;

    unordered_map<int, GraphNode> m_gn;
    m_gn[0] = gn;
    m_gn[1] = gn1;
    m_gn[2] = gn2;

    Graph g(vec_ed, m_gn);
    auto iso_gn = g.getIsolatedNodes();
    bool node0 = false;
    bool node1 = false;
    bool node2 = false;

    for (auto n_pr : iso_gn) {
      if (n_pr.first == 0) node0 = true;
      if (n_pr.first == 1) node1 = true;
      if (n_pr.first == 2) node2 = true;
    }

    BOOST_CHECK(node0);
    BOOST_CHECK(node1);
    BOOST_CHECK(node2);
  }

  {

    /// In this test both node 0 and 1 share an edge and are no longer isolated
    /// however node 2 is isolated, a call getIsolatedNodes() only returns node
    /// 2
    vector<Edge> vec_ed;
    Edge ed(0, 1);
    vec_ed.push_back(ed);

    GraphNode gn;
    GraphNode gn1;
    GraphNode gn2;

    unordered_map<int, GraphNode> m_gn;
    m_gn[0] = gn;
    m_gn[1] = gn1;
    m_gn[2] = gn2;

    Graph g(vec_ed, m_gn);
    auto iso_gn = g.getIsolatedNodes();
    bool node0 = false;
    bool node1 = false;
    bool node2 = false;

    for (auto n_pr : iso_gn) {
      if (n_pr.first == 0) node0 = true;
      if (n_pr.first == 1) node1 = true;
      if (n_pr.first == 2) node2 = true;
    }

    BOOST_CHECK(!node0);
    BOOST_CHECK(!node1);
    BOOST_CHECK(node2);
  }
}

/** 
 * \brief Determine which vertices are missing a ndoe object
 *
 * The graph class used here is composed of vertices, edges and nodes. The node
 * objects contain information about the vertex. When the graph is created both
 * the edges and nodes are passed as arguments. It may happen that an edge 
 * refers to a vertex that does not contain a corresponding graph object, this 
 * method will determine which vertices have no graph object associated with 
 * them.
 */
BOOST_AUTO_TEST_CASE(verticesmissingnodes_test) {
  {

    /// Here we have created an edge that corresponds to vertex 0 and 1
    vector<Edge> vec_ed;
    Edge ed(0, 1);
    vec_ed.push_back(ed);

    GraphNode gn;
    GraphNode gn2;
    GraphNode gn3;

    /// Notice there is no node with id 1 though there is an
    /// edge that refers to vertex 1
    unordered_map<int, GraphNode> m_gn;
    m_gn[0] = gn;
    m_gn[2] = gn2;
    m_gn[3] = gn3;

    Graph g(vec_ed, m_gn);
    /// A call to getVerticesMissingNodes should return vertex 1
    auto missing_nd = g.getVerticesMissingNodes();
    BOOST_CHECK_EQUAL(missing_nd.at(0), 1);
  }
}

BOOST_AUTO_TEST_CASE(compare_test) {
  {

    unordered_map<string, int> int_vals0 = {{"a", 0}};
    unordered_map<string, int> int_vals1 = {{"b", 1}};
    unordered_map<string, int> int_vals2 = {{"c", 2}};
    unordered_map<string, int> int_vals3 = {{"d", 3}};
    unordered_map<string, int> int_vals4 = {{"e", 4}};

    unordered_map<string, double> double_vals;
    unordered_map<string, string> str_vals;

    vector<Edge> vec_ed;
    Edge ed(0, 1);
    Edge ed1(1, 2);
    Edge ed2(2, 3);
    Edge ed3(2, 4);

    vec_ed.push_back(ed);
    vec_ed.push_back(ed1);
    vec_ed.push_back(ed2);
    vec_ed.push_back(ed3);

    GraphNode gn(int_vals0, double_vals, str_vals);
    GraphNode gn1(int_vals1, double_vals, str_vals);
    GraphNode gn2(int_vals2, double_vals, str_vals);
    GraphNode gn3(int_vals3, double_vals, str_vals);
    GraphNode gn4(int_vals4, double_vals, str_vals);

    unordered_map<int, GraphNode> m_gn;
    m_gn[0] = gn;
    m_gn[1] = gn1;
    m_gn[2] = gn2;
    m_gn[3] = gn3;
    m_gn[4] = gn4;

    Graph g(vec_ed, m_gn);

    auto vec_pr = g.getNodes();

    sort(vec_pr.begin(), vec_pr.end(), cmpVertNodePair);
    BOOST_CHECK_EQUAL(vec_pr.at(0).first, 0);
    BOOST_CHECK_EQUAL(vec_pr.at(1).first, 1);
    BOOST_CHECK_EQUAL(vec_pr.at(2).first, 2);
    BOOST_CHECK_EQUAL(vec_pr.at(3).first, 3);
    BOOST_CHECK_EQUAL(vec_pr.at(4).first, 4);
  }

  {
    unordered_map<string, int> int_vals0 = {{"a", 0}};
    unordered_map<string, int> int_vals1 = {{"b", 1}};
    unordered_map<string, int> int_vals2 = {{"c", 2}};
    unordered_map<string, int> int_vals3 = {{"d", 3}};
    unordered_map<string, int> int_vals4 = {{"e", 4}};

    unordered_map<string, double> double_vals;
    unordered_map<string, string> str_vals;

    vector<Edge> vec_ed;
    Edge ed(0, 1);
    Edge ed1(1, 2);
    Edge ed2(2, 3);
    Edge ed3(2, 4);

    vec_ed.push_back(ed);
    vec_ed.push_back(ed1);
    vec_ed.push_back(ed2);
    vec_ed.push_back(ed3);

    GraphNode gn(int_vals0, double_vals, str_vals);
    GraphNode gn1(int_vals1, double_vals, str_vals);
    GraphNode gn2(int_vals2, double_vals, str_vals);
    GraphNode gn3(int_vals3, double_vals, str_vals);
    GraphNode gn4(int_vals4, double_vals, str_vals);

    /// Only difference is here where we have rearanged the nodes
    unordered_map<int, GraphNode> m_gn;
    m_gn[4] = gn;
    m_gn[1] = gn1;
    m_gn[3] = gn2;
    m_gn[2] = gn3;
    m_gn[0] = gn4;

    Graph g(vec_ed, m_gn);

    auto vec_pr = g.getNodes();

    sort(vec_pr.begin(), vec_pr.end(), cmpVertNodePair);
    BOOST_CHECK_EQUAL(vec_pr.at(0).first, 4);
    BOOST_CHECK_EQUAL(vec_pr.at(1).first, 1);
    BOOST_CHECK_EQUAL(vec_pr.at(2).first, 3);
    BOOST_CHECK_EQUAL(vec_pr.at(3).first, 2);
    BOOST_CHECK_EQUAL(vec_pr.at(4).first, 0);
  }
}

/**
 * \brief Equivalence test 
 *
 * Here we demonstrate how the equivalence test works it is purely dependendent
 * on whether the contents of the graphnodes in the graph contain the same 
 * information. 
 */
BOOST_AUTO_TEST_CASE(id_test) {
  {
    unordered_map<string, int> int_vals0 = {{"a", 0}};
    unordered_map<string, int> int_vals1 = {{"b", 1}};
    unordered_map<string, int> int_vals2 = {{"c", 2}};
    unordered_map<string, int> int_vals3 = {{"d", 3}};
    unordered_map<string, int> int_vals4 = {{"e", 4}};

    unordered_map<string, double> double_vals;

    unordered_map<string, string> str_vals;

    vector<Edge> vec_ed;
    Edge ed(0, 1);
    Edge ed1(1, 2);
    Edge ed2(2, 3);
    Edge ed3(2, 4);

    vec_ed.push_back(ed);
    vec_ed.push_back(ed1);
    vec_ed.push_back(ed2);
    vec_ed.push_back(ed3);

    GraphNode gn(int_vals0, double_vals, str_vals);
    GraphNode gn1(int_vals1, double_vals, str_vals);
    GraphNode gn2(int_vals2, double_vals, str_vals);
    GraphNode gn3(int_vals3, double_vals, str_vals);
    GraphNode gn4(int_vals4, double_vals, str_vals);

    unordered_map<int, GraphNode> m_gn;
    /// Here the graph nodes are assigne to different vertices
    m_gn[4] = gn;
    m_gn[1] = gn1;
    m_gn[3] = gn2;
    m_gn[2] = gn3;
    m_gn[0] = gn4;

    Graph g(vec_ed, m_gn);

    /// Here is what the string id of the graph should look like
    string str = "a0b1c2d3e4";
    string s_id = g.getId();
    BOOST_CHECK_EQUAL(s_id, str);

    Graph g2(vec_ed, m_gn);
    BOOST_CHECK(g == g2);

    /// Here we switch up which vertices contain which graphnodes and show that
    /// the graph id is the same. This is because the vertex ids are not used to
    /// create the id and neither the edges. Only the contens in the graphnodes
    m_gn[1] = gn3;
    m_gn[2] = gn1;
    Graph g3(vec_ed, m_gn);
    BOOST_CHECK(g == g3);

    GraphNode gn5(int_vals3, double_vals, str_vals);
    m_gn[5] = gn5;
    Graph g4(vec_ed, m_gn);
    BOOST_CHECK(g != g4);
  }
}

BOOST_AUTO_TEST_SUITE_END()
