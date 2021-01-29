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
/// @file    Circuit.cpp
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-12-15
///
// Representation of electric circuit of overhead wires
/// @note    based on work 2017 Ahmad Khaled, Ahmad Essam, Omnia Zakaria, Mary Nader
/****************************************************************************/
#include <cfloat>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <mutex>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#ifdef HAVE_EIGEN
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "Eigen/Geometry"
#endif
#include "Element.h"
#include "Circuit.h"

using namespace std;

std::mutex circuit_lock;

Node* Circuit::addNode(string name) {
    if (getNode(name) != nullptr) {
        //WRITE_ERROR("The node: '" + name + "' already exists.");
        std::cout << "The node '" + name + "' already exists." << std::endl;
        return nullptr;
    }

    if (nodes->size() == 0) {
        lastId = -1;
    }
    Node* tNode = new Node(name, this->lastId);
    if (lastId == -1) {
        tNode->setGround(true);
    }
    this->lastId++;
    circuit_lock.lock();
    this->nodes->push_back(tNode);
    circuit_lock.unlock();
    return tNode;
}

void Circuit::eraseNode(Node* node) {
    circuit_lock.lock();
    this->nodes->erase(std::remove(this->nodes->begin(), this->nodes->end(), node), this->nodes->end());
    circuit_lock.unlock();
}

double Circuit::getCurrent(string name) {
    Element* tElement = getElement(name);
    if (tElement == nullptr) {
        return DBL_MAX;
    }
    return tElement->getCurrent();
}

double Circuit::getVoltage(string name) {
    Element* tElement = getElement(name);
    if (tElement == nullptr) {
        Node* node = getNode(name);
        if (node != nullptr) {
            return node->getVoltage();
        } else {
            return DBL_MAX;
        }
    } else {
        return tElement->getVoltage();
    }
}

double Circuit::getResistance(string name) {
    Element* tElement = getElement(name);
    if (tElement == nullptr) {
        return -1;
    }
    return tElement->getResistance();
}

Node* Circuit::getNode(string name) {
    // for (vector<Node*>::iterator it = this->nodes->begin(); it != nodes->end(); it++) {
    for (auto&& it : *this->nodes) {
        if (it->getName() == name) {
            return it;
        }
    }
    return nullptr;
}

Node* Circuit::getNode(int id) {
    for (vector<Node*>::iterator it = this->nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->getId() == id) {
            return (*it);
        }
    }
    return nullptr;
}

Element* Circuit::getElement(string name) {
    for (vector<Element*>::iterator it = this->elements->begin(); it != elements->end(); it++) {
        if ((*it)->getName() == name) {
            return (*it);
        }
    }
    for (vector<Element*>::iterator it = this->voltageSources->begin(); it != voltageSources->end(); it++) {
        if ((*it)->getName() == name) {
            return (*it);
        }
    }
    return nullptr;
}

Element* Circuit::getElement(int id) {
    for (vector<Element*>::iterator it = this->elements->begin(); it != elements->end(); it++) {
        if ((*it)->getId() == id) {
            return (*it);
        }
    }
    for (vector<Element*>::iterator it = this->voltageSources->begin(); it != voltageSources->end(); it++) {
        if ((*it)->getId() == id) {
            return (*it);
        }
    }
    return nullptr;
}

Element* Circuit::getVoltageSource(int id) {
    for (vector<Element*>::iterator it = this->voltageSources->begin(); it != voltageSources->end(); it++) {
        if ((*it)->getId() == id) {
            return (*it);
        }
    }
    return nullptr;
}

vector<Element*>* Circuit::getCurrentSources() {
    vector<Element*>* vsources = new vector<Element*>(0);
    for (vector<Element*>::iterator it = this->elements->begin(); it != elements->end(); it++) {
        if ((*it)->getType() == Element::ElementType::CURRENT_SOURCE_traction_wire) {
            //if ((*it)->getType() == Element::ElementType::CURRENT_SOURCE_traction_wire && !isnan((*it)->getPowerWanted())) {
            vsources->push_back(*it);
        }
    }
    return vsources;
}

void Circuit::lock() {
    circuit_lock.lock();
}

void Circuit::unlock() {
    circuit_lock.unlock();
}

#ifdef HAVE_EIGEN
void Circuit::removeColumn(Eigen::MatrixXd& matrix, int colToRemove) {
    const int numRows = (int)matrix.rows();
    const int numCols = (int)matrix.cols() - 1;

    if (colToRemove < numCols) {
        matrix.block(0, colToRemove, numRows, numCols - colToRemove) = matrix.rightCols(numCols - colToRemove);
    }

    matrix.conservativeResize(numRows, numCols);
}

bool Circuit::solveEquationsNRmethod(double* eqn, double* vals, std::vector<int>* removable_ids) {
    // removable_ids includes nodes with voltage source already
    int numofcolumn = (int)voltageSources->size() + (int)nodes->size() - 1;
    int numofeqs = numofcolumn - (int)removable_ids->size();

    Eigen::MatrixXd A = Eigen::Map < Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> >(eqn, numofeqs, numofcolumn);

    int id;
    // removing removable columns
    for (std::vector<int>::reverse_iterator it = removable_ids->rbegin(); it != removable_ids->rend(); ++it) {
        id = (*it >= 0 ? *it : -(*it));
        removeColumn(A, id);
    }

    // detecting number of column for each node
    // in other words: detecting elements of x to certain node
    int j = 0;
    Element* tElem = nullptr;
    Node* tNode = nullptr;
    for (int i = 0; i < numofcolumn; i++) {
        tNode = getNode(i);
        if (tNode != nullptr)
            if (tNode->isRemovable()) {
                tNode->setNumMatrixCol(-1);
                continue;
            } else {
                // TODO: is the numofeqs (defined above) still the same as the part below?
                if (j > numofcolumn - (int) removable_ids->size()) {
                    WRITE_ERROR("Number of column deployment during circuit evaluation was unsuccessfull.");
                    break;
                }
                tNode->setNumMatrixCol(j);
                j++;
                continue;
            } else {
            tElem = getElement(i);
            if (tElem != nullptr) {
                // TODO: is the numofeqs (defined above) still the same as the part below?
                if (j > numofcolumn - (int) removable_ids->size()) {
                    WRITE_ERROR("Number of column deployment deployment during circuit evaluation was unsuccessfull.");
                    break;
                }
                continue;
            }
        }
        WRITE_ERROR("Number of column deployment during circuit evaluation was unsuccessfull.");
    }

    Eigen::Map<Eigen::VectorXd> b(vals, numofeqs);
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);
    // probably unused
    // double relative_error = (A*x - b).norm() / b.norm();

    Eigen::MatrixXd J = A;
    Eigen::VectorXd dx;
    int max_iter_of_NR = 10;
    int attemps = 0;
    double alpha = 1;
    alphaBest = 0;
    //TODORICE alphaBest private and function get and setAlphaBest
    std::vector<double> alpha_notSolution;
    double alpha_res = 1e-2;
    double* x_best = new double[numofeqs];
    //init x_best
    for (int i = 0; i < numofeqs; i++) {
        x_best[i] = x[i];
    }
    if (x.maxCoeff() > 10e6 || x.minCoeff() < -10e6) {

        WRITE_ERROR("Here is the matrix A:\n" + toString(A));
        WRITE_ERROR("Here is the vector b:\n" + toString(b));
        WRITE_ERROR("Here is the vector x:\n" + toString(x));
        for (int i = 0; i < numofeqs; i++) {
            x_best[i] = 600;
        }
    }
    //search alpha
    while (true) {

        ++attemps;
        int iterNR = 0;
        // run Newton-Raphson methods
        while (true) {

            for (int i = 0; i < numofeqs - (int) voltageSources->size(); i++) {
                vals[i] = 0;
            }
            J = A;

            int i = 0;
            for (auto& node : *nodes) {
                if (node->isGround() || node->isRemovable() || node->getNumMatrixRow() == -2) {
                    continue;
                }
                if (node->getNumMatrixRow() != i) {
                    WRITE_ERROR("wrongly assigned row of matrix A during solving the circuit");
                }
                // TODO: Range-based loop
                for (auto it_element = node->getElements()->begin(); it_element != node->getElements()->end(); it_element++) {
                    if ((*it_element)->getType() == Element::ElementType::CURRENT_SOURCE_traction_wire) {
                        if ((*it_element)->isEnabled()) {
                            double diff_voltage;
                            if ((*it_element)->getPosNode()->getNumMatrixCol() == -1) {
                                diff_voltage = -x[(*it_element)->getNegNode()->getNumMatrixCol()];
                            } else if ((*it_element)->getNegNode()->getNumMatrixCol() == -1) {
                                diff_voltage = x[(*it_element)->getPosNode()->getNumMatrixCol()];
                            } else {
                                diff_voltage = (x[(*it_element)->getPosNode()->getNumMatrixCol()] - x[(*it_element)->getNegNode()->getNumMatrixCol()]);
                            }

                            if ((*it_element)->getPosNode() == node) {
                                vals[i] -= alpha * (*it_element)->getPowerWanted() / diff_voltage;
                                (*it_element)->setCurrent(-alpha * (*it_element)->getPowerWanted() / diff_voltage);
                                if ((*it_element)->getPosNode()->getNumMatrixCol() != -1) {
                                    J(i, (*it_element)->getPosNode()->getNumMatrixCol()) -= alpha * (*it_element)->getPowerWanted() / diff_voltage / diff_voltage;
                                }
                                if ((*it_element)->getNegNode()->getNumMatrixCol() != -1) {
                                    J(i, (*it_element)->getNegNode()->getNumMatrixCol()) += alpha * (*it_element)->getPowerWanted() / diff_voltage / diff_voltage;
                                }
                            } else {
                                vals[i] += alpha * (*it_element)->getPowerWanted() / diff_voltage;
                                //sign before alpha - or + during setting current?
                                //(*it_element)->setCurrent(alpha * (*it_element)->getPowerWanted() / diff_voltage);
                                if ((*it_element)->getPosNode()->getNumMatrixCol() != -1) {
                                    J(i, (*it_element)->getPosNode()->getNumMatrixCol()) += alpha * (*it_element)->getPowerWanted() / diff_voltage / diff_voltage;
                                }
                                if ((*it_element)->getNegNode()->getNumMatrixCol() != -1) {
                                    J(i, (*it_element)->getNegNode()->getNumMatrixCol()) -= alpha * (*it_element)->getPowerWanted() / diff_voltage / diff_voltage;
                                }
                            }
                        }
                    }
                }
                i++;
            }

            // TODO: The variable below was declared as `b`, renamed to `bb`, check if the rename was consistent.
            Eigen::Map<Eigen::VectorXd> bb(vals, numofeqs);

            if ((A * x - bb).norm() < 1e-6) {
                alphaBest = alpha;
                for (int ii = 0; ii < numofeqs; ii++) {
                    x_best[ii] = x[ii];
                }
                break;
            } else if (iterNR == max_iter_of_NR) {
                alpha_notSolution.push_back(alpha);
                for (int ii = 0; ii < numofeqs; ii++) {
                    x[ii] = x_best[ii];
                }
                break;
            }

            dx = -J.colPivHouseholderQr().solve(A * x - bb);
            x = x + dx;
            ++iterNR;
        }

        if (alpha_notSolution.empty()) {
            break;
        }

        if ((alpha_notSolution.back() - alphaBest) < alpha_res) {
            max_iter_of_NR = 2 * max_iter_of_NR;
            alpha_res = alpha_res / 10;
            if (alpha_res < 5e-5) {
                break;
            }
            alpha = alpha_notSolution.back();
            alpha_notSolution.pop_back();
            continue;
        }

        alpha = alphaBest + 0.5 * (alpha_notSolution.back() - alphaBest);
    }

    for (int i = 0; i < numofeqs; i++) {
        vals[i] = x_best[i];
    }

    int i = 0;
    for (auto& node : *nodes) {
        if (node->isGround() || node->isRemovable() || node->getNumMatrixRow() == -2) {
            continue;
        }
        if (node->getNumMatrixRow() != i) {
            WRITE_ERROR("wrongly assigned row of matrix A during solving the circuit");
        }
        for (auto it_element = node->getElements()->begin(); it_element != node->getElements()->end(); it_element++) {
            if ((*it_element)->getType() == Element::ElementType::CURRENT_SOURCE_traction_wire) {
                if ((*it_element)->isEnabled()) {
                    double diff_voltage;
                    if ((*it_element)->getPosNode()->getNumMatrixCol() == -1) {
                        diff_voltage = -x_best[(*it_element)->getNegNode()->getNumMatrixCol()];
                    } else if ((*it_element)->getNegNode()->getNumMatrixCol() == -1) {
                        diff_voltage = x_best[(*it_element)->getPosNode()->getNumMatrixCol()];
                    } else {
                        diff_voltage = (x_best[(*it_element)->getPosNode()->getNumMatrixCol()] - x_best[(*it_element)->getNegNode()->getNumMatrixCol()]);
                    }

                    if ((*it_element)->getPosNode() == node) {
                        (*it_element)->setCurrent(-alphaBest * (*it_element)->getPowerWanted() / diff_voltage);
                    } else {
                        //sign before alpha - or + during setting current?
                        //(*it_element)->setCurrent(alpha * (*it_element)->getPowerWanted() / diff_voltage);
                    }
                }
            }
        }
        i++;
    }

    return true;
}
#endif

void Circuit::deployResults(double* vals, std::vector<int>* removable_ids) {
    int n = (int)(voltageSources->size() + nodes->size() - 1);
    int j = 0;
    Element* tElem = nullptr;
    Node* tNode = nullptr;
    for (int i = 0; i < n; i++) {
        tNode = getNode(i);
        if (tNode != nullptr)
            if (tNode->isRemovable()) {
                continue;
            } else {
                // TODO: Is the n - (int) removable_ids->size() constant?
                if (j > n - (int) removable_ids->size()) {
                    WRITE_ERROR("Results deployment during circuit evaluation was unsuccessfull.");
                    break;
                }
                tNode->setVoltage(vals[j]);
                j++;
                continue;
            } else {
            tElem = getElement(i);
            if (tElem != nullptr) {
                // TODO: Is the n - (int) removable_ids->size() constant?
                if (j > n - (int) removable_ids->size()) {
                    WRITE_ERROR("Results deployment during circuit evaluation was unsuccessfull.");
                    break;
                }
                //tElem->setCurrent(vals[j]);
                //TODORICE dodelat vypocet proudu na zdroji z Kirchhofova zakona: proudy dovnitr = proudy ven
                //tElem->setCurrent(-1000);
                //j++;
                continue;
            }
        }
        WRITE_ERROR("Results deployment during circuit evaluation was unsuccessfull.");
    }

    Element* el1 = nullptr;
    Element* el2 = nullptr;
    Node* nextNONremovableNode1 = nullptr;
    Node* nextNONremovableNode2 = nullptr;
    // interpolate result of voltage to removable nodes
    for (vector<Node*>::iterator it = nodes->begin(); it != nodes->end(); it++) {
        if (!(*it)->isRemovable()) {
            continue;
        }
        if ((*it)->getElements()->size() != 2) {
            continue;
        }

        el1 = (*it)->getElements()->front();
        el2 = (*it)->getElements()->back();
        nextNONremovableNode1 = el1->getTheOtherNode(*it);
        nextNONremovableNode2 = el2->getTheOtherNode(*it);
        double x = el1->getResistance();
        double y = el2->getResistance();

        while (nextNONremovableNode1->isRemovable()) {
            el1 = nextNONremovableNode1->getAnOtherElement(el1);
            x += el1->getResistance();
            nextNONremovableNode1 = el1->getTheOtherNode(nextNONremovableNode1);
        }

        while (nextNONremovableNode2->isRemovable()) {
            el2 = nextNONremovableNode2->getAnOtherElement(el2);
            y += el2->getResistance();
            nextNONremovableNode2 = el2->getTheOtherNode(nextNONremovableNode2);
        }

        x = x / (x + y);
        y = ((1 - x) * nextNONremovableNode1->getVoltage()) + (x * nextNONremovableNode2->getVoltage());
        (*it)->setVoltage(((1 - x)*nextNONremovableNode1->getVoltage()) + (x * nextNONremovableNode2->getVoltage()));
        (*it)->setRemovability(false);
    }

    /*
    for (int i = 0; i < n; i++) {
        Node* tNode = getNode(i);
        if (tNode != nullptr)
            tNode->setVoltage(vals[i]);
        else
        {
            Element* tElem = getElement(i);
            if (tElem != nullptr) {
                tElem->setCurrent(vals[i]);
            }
        }
    }
    */
}

Circuit::Circuit() {
    nodes = new vector<Node*>(0);
    elements = new vector<Element*>(0);
    voltageSources = new vector<Element*>(0);
    lastId = 0;
    iscleaned = true;
}

#ifdef HAVE_EIGEN
bool Circuit::_solveNRmethod() {
    double* eqn = nullptr;
    double* vals = nullptr;
    std::vector<int> removable_ids;

    detectRemovableNodes(&removable_ids);
    createEquationsNRmethod(eqn, vals, &removable_ids);
    if (!solveEquationsNRmethod(eqn, vals, &removable_ids)) {
        return false;
    }
    deployResults(vals, &removable_ids);

    return true;
}

bool Circuit::solve() {
    if (!iscleaned) {
        cleanUpSP();
    }
    return this->_solveNRmethod();
}

bool Circuit::createEquationsNRmethod(double*& eqs, double*& vals, std::vector<int>* removable_ids) {
    // removable_ids does not include nodes with voltage source
    int n = (int)(voltageSources->size() + nodes->size() - 1);
    int m = n - (int)(removable_ids->size() - voltageSources->size());
    //cout << endl << endl << n << endl << endl;
    eqs = new double[m * n];
    vals = new double[m];

    for (int i = 0; i < m; i++) {
        vals[i] = 0;
        for (int j = 0; j < n; j++) {
            eqs[i * n + j] = 0;
        }
    }

    int i = 0;
    for (vector<Node*>::iterator it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->isGround() || (*it)->isRemovable()) {
            (*it)->setNumMatrixRow(-1);
            continue;
        }
        bool noVoltageSource = createEquationNRmethod((*it), (eqs + n * i), vals[i], removable_ids);
        // if the node it has element of type "voltage source" we do not use the equation, because some value of current throw the voltage source can be always find
        if (noVoltageSource) {
            (*it)->setNumMatrixRow(i);
            i++;
        } else {
            (*it)->setNumMatrixRow(-2);
            vals[i] = 0;
            for (int j = 0; j < n; j++) {
                eqs[n * i + j] = 0;
            }
        }
    }
    // removable_ids includes nodes with voltage source already
    std::sort(removable_ids->begin(), removable_ids->end(), std::less<int>());

    for (vector<Element*>::iterator it = voltageSources->begin(); it != voltageSources->end(); it++) {
        createEquation((*it), (eqs + n * i), vals[i]);
        i++;
    }

    return true;
}

bool Circuit::createEquation(Element* vsource, double* eqn, double& val) {
    if (!vsource->getPosNode()->isGround()) {
        eqn[vsource->getPosNode()->getId()] = 1;
    }
    if (!vsource->getNegNode()->isGround()) {
        eqn[vsource->getNegNode()->getId()] = -1;
    }
    if (vsource->isEnabled()) {
        val = vsource->getVoltage();
    } else {
        val = 0;
    }
    return true;
}

bool Circuit::createEquationNRmethod(Node* node, double* eqn, double& val, std::vector<int>* removable_ids) {
    for (vector<Element*>::iterator it = node->getElements()->begin(); it != node->getElements()->end(); it++) {
        double x;
        switch ((*it)->getType()) {
            case Element::ElementType::RESISTOR_traction_wire:
                if ((*it)->isEnabled()) {
                    x = (*it)->getResistance();
                    Node* nextNONremovableNode = (*it)->getTheOtherNode(node);
                    Element* nextSerialResistor = *it;
                    while (nextNONremovableNode->isRemovable()) {
                        nextSerialResistor = nextNONremovableNode->getAnOtherElement(nextSerialResistor);
                        x += nextSerialResistor->getResistance();
                        nextNONremovableNode = nextSerialResistor->getTheOtherNode(nextNONremovableNode);
                    }
                    x = 1 / x;
                    eqn[node->getId()] += x;
                    if (!nextNONremovableNode->isGround()) {
                        eqn[nextNONremovableNode->getId()] -= x;
                    }
                    //if (!(*it)->getTheOtherNode(node)->isGround())
                    //	eqn[(*it)->getTheOtherNode(node)->getId()] -= x;
                }
                break;
            case Element::ElementType::CURRENT_SOURCE_traction_wire:
                if ((*it)->isEnabled()) {
                    if ((*it)->getPosNode() == node) {
                        x = (*it)->getCurrent();
                    } else {
                        x = -(*it)->getCurrent();
                    }
                } else {
                    x = 0;
                }
                val += x;
                break;
            case Element::ElementType::VOLTAGE_SOURCE_traction_wire:
                if ((*it)->getPosNode() == node) {
                    x = -1;
                } else {
                    x = 1;
                }
                eqn[(*it)->getId()] += x;
                // equations with voltage source can be igored, because some value of current throw the voltage source can be always find
                removable_ids->push_back((*it)->getId());
                return false;
                break;
            case Element::ElementType::ERROR_traction_wire:
                return false;
                break;
        }
    }
    return true;
}
#endif

void Circuit::detectRemovableNodes(std::vector<int>* removable_ids) {
    for (vector<Node*>::iterator it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->getElements()->size() == 2 && !(*it)->isGround()) {
            (*it)->setRemovability(true);
            for (vector<Element*>::iterator it2 = (*it)->getElements()->begin(); it2 != (*it)->getElements()->end(); it2++) {
                if ((*it2)->getType() != Element::ElementType::RESISTOR_traction_wire) {
                    (*it)->setRemovability(false);
                    break;
                }
            }
            if ((*it)->isRemovable()) {
                removable_ids->push_back((*it)->getId());
            }
        } else {
            (*it)->setRemovability(false);
        }
    }
    std::sort(removable_ids->begin(), removable_ids->end(), std::less<int>());
    return;
}

Element* Circuit::addElement(string name, double value, Node* pNode, Node* nNode, Element::ElementType et) {
    if ((et == Element::ElementType::RESISTOR_traction_wire && value <= 0) || et == Element::ElementType::ERROR_traction_wire) {
        return nullptr;
    }

    Element* e = getElement(name);

    if (e != nullptr) {
        //WRITE_ERROR("The element: '" + name + "' already exists.");
        std::cout << "The element: '" + name + "' already exists.";
        return nullptr;
    }

    e = new Element(name, et, value);
    if (e->getType() == Element::ElementType::VOLTAGE_SOURCE_traction_wire) {
        e->setId(lastId);
        lastId++;
        circuit_lock.lock();
        this->voltageSources->push_back(e);
        circuit_lock.unlock();
    } else {
        circuit_lock.lock();
        this->elements->push_back(e);
        circuit_lock.unlock();
    }

    e->setPosNode(pNode);
    e->setNegNode(nNode);

    pNode->addElement(e);
    nNode->addElement(e);
    return e;
}

void Circuit::eraseElement(Element* element) {
    //element->getPosNode()->eraseElement(element);
    //element->getNegNode()->eraseElement(element);
    circuit_lock.lock();
    this->elements->erase(std::remove(this->elements->begin(), this->elements->end(), element), this->elements->end());
    circuit_lock.unlock();
}

void Circuit::replaceAndDeleteNode(Node* unusedNode, Node* newNode) {
    //replace element node if it is unusedNode
    for (auto& voltageSource : *voltageSources) {
        if (voltageSource->getNegNode() == unusedNode) {
            voltageSource->setNegNode(newNode);
            newNode->eraseElement(voltageSource);
            newNode->addElement(voltageSource);
        }
        if (voltageSource->getPosNode() == unusedNode) {
            voltageSource->setPosNode(newNode);
            newNode->eraseElement(voltageSource);
            newNode->addElement(voltageSource);
        }
    }
    for (auto& element : *elements) {
        if (element->getNegNode() == unusedNode) {
            element->setNegNode(newNode);
            newNode->eraseElement(element);
            newNode->addElement(element);
        }
        if (element->getPosNode() == unusedNode) {
            element->setPosNode(newNode);
            newNode->eraseElement(element);
            newNode->addElement(element);
        }
    }

    //erase unusedNode from nodes vector
    this->eraseNode(unusedNode);

    //modify id of other elements and nodes
    int modLastId = this->getLastId() - 1;
    if (unusedNode->getId() != modLastId) {
        Node* node_last = this->getNode(modLastId);
        if (node_last != nullptr) {
            node_last->setId(unusedNode->getId());
        } else {
            Element* elem_last = this->getVoltageSource(modLastId);
            if (elem_last != nullptr) {
                elem_last->setId(unusedNode->getId());
            } else {
                WRITE_ERROR("The element or node with the last Id was not found in the circuit!");
            }
        }
    }

    this->descreaseLastId();
    delete unusedNode;
}

void Circuit::cleanUpSP() {
    for (vector<Element*>::iterator it = elements->begin(); it != elements->end(); it++) {
        if ((*it)->getType() != Element::ElementType::RESISTOR_traction_wire) {
            (*it)->setEnabled(true);
        }
    }

    for (vector<Element*>::iterator it = voltageSources->begin(); it != voltageSources->end(); it++) {
        (*it)->setEnabled(true);
    }
    this->iscleaned = true;
}

bool Circuit::checkCircuit(std::string substationId) {
    // check empty nodes
    for (vector<Node*>::iterator it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->getNumOfElements() < 2) {
            //cout << "WARNING: Node [" << (*it)->getName() << "] is connected to less than two elements, please enter other elements.\n";
            if ((*it)->getNumOfElements() < 1) {
                return false;
            }
        }
    }
    // check voltage sources
    for (vector<Element*>::iterator it = voltageSources->begin(); it != voltageSources->end(); it++) {
        if ((*it)->getPosNode() == nullptr || (*it)->getNegNode() == nullptr) {
            //cout << "ERROR: Voltage Source [" << (*it)->getName() << "] is connected to less than two nodes, please enter the other end.\n";
            WRITE_ERROR("Circuit Voltage Source '" + (*it)->getName() + "' is connected to less than two nodes, please adjust the definition of the section (with substation '" + substationId + "').");
            return false;
        }
    }
    // check other elements
    for (vector<Element*>::iterator it = elements->begin(); it != elements->end(); it++) {
        if ((*it)->getPosNode() == nullptr || (*it)->getNegNode() == nullptr) {
            //cout << "ERROR: Element [" << (*it)->getName() << "] is connected to less than two nodes, please enter the other end.\n";
            WRITE_ERROR("Circuit Element '" + (*it)->getName() + "' is connected to less than two nodes, please adjust the definition of the section (with substation '" + substationId + "').");
            return false;
        }
    }

    // check connectivity
    int num = (int)nodes->size() + getNumVoltageSources() - 1;
    bool* nodesVisited = new bool[num];
    for (int i = 0; i < num; i++) {
        nodesVisited[i] = false;
    }
    // TODO: Probably unused
    // int id = -1;
    if (!getNode(-1)->isGround()) {
        //cout << "ERROR: Node id -1 is not the ground \n";
        WRITE_ERROR("Circuit Node with id '-1' is not the grounded, please adjust the definition of the section (with substation '" + substationId + "').");
    }
    vector<Node*>* queue = new vector<Node*>(0);
    Node* node = nullptr;
    Node* neigboringNode = nullptr;
    //start with (voltageSources->front()->getPosNode())
    nodesVisited[voltageSources->front()->getId()] = 1;
    node = voltageSources->front()->getPosNode();
    queue->push_back(node);

    while (!queue->empty()) {
        node = queue->back();
        queue->pop_back();
        if (!nodesVisited[node->getId()]) {
            nodesVisited[node->getId()] = true;
            for (auto it = node->getElements()->begin(); it != node->getElements()->end(); it++) {
                neigboringNode = (*it)->getTheOtherNode(node);
                if (!neigboringNode->isGround()) {
                    queue->push_back(neigboringNode);
                } else if ((*it)->getType() == Element::ElementType::VOLTAGE_SOURCE_traction_wire) {
                    /// there used to be == 1 which was probably a typo ... check!
                    nodesVisited[(*it)->getId()] = 1;
                } else if ((*it)->getType() == Element::ElementType::RESISTOR_traction_wire) {
                    //cout << "ERROR: The resistor type connects the ground \n";
                    WRITE_ERROR("A Circuit Resistor Element connects the ground, please adjust the definition of the section (with substation '" + substationId + "').");
                }
            }
        }
    }

    for (int i = 0; i < num; i++) {
        if (nodesVisited[i] == 0) {
            //cout << "ERROR: Node or voltage source with id " << (i) << " has been not visited during checking of the circuit => Disconnectivity of the circuit. \n";
            WRITE_WARNING("Circuit Node or Voltage Source with internal id '" + toString(i) + "' has been not visited during checking of the circuit. The circuit is disconnected, please adjust the definition of the section (with substation '" + substationId + "').");
        }
    }

    return true;
}

int Circuit::getNumVoltageSources() {
    return (int) voltageSources->size();
}
