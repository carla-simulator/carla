/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    Circuit.h
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-12-15
///
// Representation of electric circuit of overhead wires
/// @note    based on work 2017 Ahmad Khaled, Ahmad Essam, Omnia Zakaria, Mary Nader
/****************************************************************************/
#pragma once

#include <vector>
#ifdef HAVE_EIGEN
#include "Eigen/Dense"
#include "Eigen/Geometry"
#endif
#include "Node.h"
#include "Element.h"

/**
 * All interactions will be through this class, the user will know nothing about the other classes,
 * and will interact only through the names of the elements/nodes.
 */


class Circuit {

private:

    vector<Node*>* nodes;
    vector<Element*>* elements;
    vector<Element*>* voltageSources;

    int lastId;
    bool iscleaned;

public:
    Node* getNode(string name);
    Element* getElement(string name);
    Node* getNode(int id);
    Element* getVoltageSource(int id);
    vector<Element*>* getCurrentSources();

    void lock();
    void unlock();

    /**
     * @brief Best alpha scaling value.
     *
     * This parameter is used to scale down the power demands of current sources (vehicles
     * that draw power from the circuit) so that a solution of the system can be found.
     * Note: the system is nonlinear (quadratic), hence in some cases (typically too high
     * power demands) a solution cannot be found. In that moment we decrease all power
     * requirements by `alpha` and try to solve again, until we find alpha that ensures
     * stable solution. This is then reported as alphaBest.
     */
    double alphaBest;

private:
    Element* getElement(int id);

    /*
    *    detects removable nodes = sets node variable "isremovable" to true if node is removable and adds id of such node to "removable_ids" vector
    *    node is denoted as removable if it is connected just to 2 elements and both of them are resistor
    *    the reason is that in such case there are two serial resistor and we can only sum their resistance value
    *
    *    "removable_ids" vector is sort from the least to the greatest
    */
    void detectRemovableNodes(std::vector<int>* removable_ids);

    void deployResults(double* vals, std::vector<int>* removable_ids);

#ifdef HAVE_EIGEN
    /*
    *    creates all of the equations that represent the circuit
    *    in the form Ax = B(1/x) where A and B are matricies
    *    @param eqn : A
    *    @param vals : B
    */
    bool createEquationsNRmethod(double*& eqs, double*& vals, std::vector<int>* removable_ids);


    /*
    *    creates the nodal equation of the node 'node' GV = I
    *    in the form Ax = B(1/x) where A is a matrix with one row
    *    @param node : the node to be analyzed
    *    @param eqn : A
    *    @param val : B
    */
    bool createEquationNRmethod(Node* node, double* eqn, double& val, std::vector<int>* removable_ids);

    /**
     * @brief Create the equation of the voltage source.
     * Create the equation V2 - V1 = E of the voltage source in the form Ax = B,
     * where A is a matrix with one row, B a value
     * @param[in] vsource The voltage source
     * @param[in] eqn : A
     * @param[in] val : B
     * @return ???
    */
    bool createEquation(Element* vsource, double* eqn, double& val);

    /*
     *    removes the "colToRemove"-th column from matrix "matrix"
     */
    void removeColumn(Eigen::MatrixXd& matrix, const int colToRemove);

    /*
     * solves the system of nonlinear equations Ax = B(1/x)
     * @param eqn : A
     * @param vals : B
     */
    bool solveEquationsNRmethod(double* eqn, double* vals, std::vector<int>*);

    bool _solveNRmethod();

#endif
public:

    // a Constructor, same functionality as "init" functions
    Circuit();

    // adds an element with name "name", type "type" and value "value" to positive node "pNode" and negative node "nNode""
    Element* addElement(string name, double value, Node* pNode, Node* nNode, Element::ElementType et);

    void eraseElement(Element* element);

    // adds a node with name "name"
    Node* addNode(string name);

    // erases a node with name "name"
    void eraseNode(Node* node);

    // gets current through element "name"
    double getCurrent(string name);

    // gets voltage across element or node "name"
    double getVoltage(string name);

    // gets the resistance of an element.
    double getResistance(string name);

    // gets the number of voltage sources in the circuit.
    int getNumVoltageSources();

    // checks if the circuit's connections are correct.
    bool checkCircuit(std::string substationId = "");

#ifdef HAVE_EIGEN
    // solves the circuit and deploys the results
    bool solve();
#endif

    // cleans up after superposition.
    void cleanUpSP();

    //replaces unusedNode with newNode everywhere in the circuit, modifies the ids of other nodes and elements, descreases the id by one and deletes unusedNode
    void replaceAndDeleteNode(Node* unusedNode, Node* newNode);

    // returns lastId
    int getLastId() {
        return lastId;
    };

    // decreases lastId by one
    void descreaseLastId() {
        lastId--;
    };
};
