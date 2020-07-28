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
/// @file    Element.cpp
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-12-15
///
// Representation of electric circuit elements: resistors, voltage and current sources
/// @note    based on work 2017 Ahmad Khaled, Ahmad Essam, Omnia Zakaria, Mary Nader
/****************************************************************************/
#include <cfloat>
#include <cmath>
#include "Element.h"
#include "Node.h"

Element::Element(string name, ElementType type, double value) {
    this->id = -2;
    this->name = name;
    this->type = type;
    this->isenabled = true;
    this->resistance = 0;
    this->current = 0;
    this->voltage = 0;
    this->powerWanted = NAN;
    switch (type) {
        case CURRENT_SOURCE_traction_wire:
            this->current = value;
            break;
        case VOLTAGE_SOURCE_traction_wire:
            this->voltage = value;
            break;
        case RESISTOR_traction_wire:
            this->resistance = value;
            break;
        default:
            cout << "ERROR: TYPE UNDEFINED.\n";
            break;
    }
    this->pNode = nullptr;
    this->nNode = nullptr;
}

void Element::setVoltage(double voltageIn) {
    this->voltage = voltageIn;
}
void Element::setCurrent(double currentIn) {
    this->current = currentIn;
}
void Element::setResistance(double resistanceIn) {
    if (resistanceIn <= 1e-6) {
        this->resistance = 1e-6;
    } else {
        this->resistance = resistanceIn;
    }
}
void Element::setPowerWanted(double powerWantedIn) {
    this->powerWanted = powerWantedIn;
}
double Element::getVoltage() {
    if (this->isenabled == false) {
        return DBL_MAX;
    }
    if (getType() == Element::ElementType::VOLTAGE_SOURCE_traction_wire) {
        return voltage;
    }
    return this->pNode->getVoltage() - this->nNode->getVoltage();
}
double Element::getCurrent() {
    if (this->isenabled == false) {
        return DBL_MAX;
    }
    switch (this->type) {
        case Element::ElementType::RESISTOR_traction_wire:
            return -1 * getVoltage() / resistance;
        case Element::ElementType::CURRENT_SOURCE_traction_wire:
        case Element::ElementType::VOLTAGE_SOURCE_traction_wire:
            return current;
        default:
            return 0;
    }
}
double Element::getResistance() {
    return this->resistance;
}
double Element::getPowerWanted() {
    return 	this->powerWanted;
}
double Element::getPower() {
    return 	-1 * getCurrent() * getVoltage();
}
int Element::getId() {

    return this->id;
}
Node* Element::getPosNode() {
    return this->pNode;
}
Node* Element::getNegNode() {
    return this->nNode;
}

Element::ElementType Element::getType() {
    return this->type;
}
string Element::getName() {
    return this->name;
}

void Element::setPosNode(Node* node) {
    this->pNode = node;

}
void Element::setNegNode(Node* node) {
    this->nNode = node;
}
void Element::setId(int newId) {
    this->id = newId;
}

// if node == pNode, return nNode, else if node == nNode return pNode, else return nullptr
Node* Element::getTheOtherNode(Node* node) {
    if (node == pNode) {
        return nNode;
    } else if (node == nNode) {
        return pNode;
    } else {
        return nullptr;
    }
}

bool Element::isEnabled() {
    return isenabled;
}

void Element::setEnabled(bool newIsEnabled) {
    this->isenabled = newIsEnabled;
}

void Element::setType(ElementType ET) {
    this->type = ET;
}
