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
/// @file    Node.cpp
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-12-15
///
// Representation of electric circuit nodes, i.e. wire junctions and connection points.
/// @note    based on work 2017 Ahmad Khaled, Ahmad Essam, Omnia Zakaria, Mary Nader
/****************************************************************************/
#include <string>
#include <algorithm>
#include "Node.h"
#include "Element.h"

using namespace std;

// A constructor, same functionality as "init" functions
Node::Node(string name, int id) {
    isground = false;
    this->name = name;		// unique property, each object has distinctive and unique name
    this->id = id;				// a sequential ID number, might be useful when making the equation
    this->num_matrixRow = -1;
    this->num_matrixCol = -1;
    this->voltage = 0;
    this->elements = new vector<Element*>(0);
    isremovable = false;
}

// connects an element to the node
void Node::addElement(Element* element) {
    elements->push_back(element);
}

void Node::eraseElement(Element* element) {
    elements->erase(std::remove(elements->begin(), elements->end(), element), elements->end());
}

// getters and setters
double Node::getVoltage() {
    return this->voltage;
}

void  Node::setVoltage(double voltage) {
    this->voltage = voltage;
}

int Node::getNumOfElements() {
    return (int) elements->size();
}

string& Node::getName() {
    return this->name;
}

bool Node::isGround() {
    return this->isground;
}

void Node::setGround(bool newIsGround) {
    this->isground = newIsGround;
}

int Node::getId() {
    return this->id;
}

void Node::setId(int newId) {
    this->id = newId;
}

void Node::setNumMatrixRow(int num) {
    this->num_matrixRow = num;
}

int Node::getNumMatrixRow() {
    return this->num_matrixRow;
}

void Node::setNumMatrixCol(int num) {
    this->num_matrixCol = num;
}

int Node::getNumMatrixCol() {
    return this->num_matrixCol;
}

vector<Element*>* Node::getElements() {
    return elements;
}

void Node::setRemovability(bool newIsRemovable) {
    this->isremovable = newIsRemovable;
}

Element* Node::getAnOtherElement(Element* element) {
    // for (vector<Element*>::iterator it = this->getElements()->begin(); it != this->getElements()->end(); it++) {
    for (Element* it : *this->getElements()) {
        if (it != element) {
            return it;
        }
    }
    return nullptr;
}
