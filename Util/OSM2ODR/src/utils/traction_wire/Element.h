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
/// @file    Element.h
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-12-15
///
// Representation of electric circuit elements: resistors, voltage and current sources
/// @note    based on work 2017 Ahmad Khaled, Ahmad Essam, Omnia Zakaria, Mary Nader
/****************************************************************************/
#pragma once

#include <string>
#include <iostream>
using namespace std;

class Node;

/**
 * An element is any component in the circuit (resistor, current source, voltage source)
 * Every element has two terminals pNode (positive terminal) and nNode (negative terminal)
 * value is the resistance in case of a resistor, current in case of a current source
 * and voltage in case of voltage source.
 *
 * Conventions used:
 *
 *   1 - in case of a current source, "value" represents the current going from nNode to pNode,
 *   2 - in case of a voltage source, "value" represents the voltage difference of pNode - nNode.
 */

class Element {

public:
    enum ElementType {
        RESISTOR_traction_wire,
        CURRENT_SOURCE_traction_wire,
        VOLTAGE_SOURCE_traction_wire,
        ERROR_traction_wire
    };

private:
    Node* pNode;
    Node* nNode;
    double voltage;
    double current;
    double resistance;
    double powerWanted;
    ElementType type;
    string name;  // unique property, each object has distinctive and unique name
    int id;  // a sequential ID number, might be useful when making the equation
    bool isenabled;

public:
    // a constructor. same functionality as init functions in the last project
    Element(string name, ElementType type, double value);

    //getters and setters
    double getVoltage();        // get the voltage across the element
    double getCurrent();        // get the current running through the element
    double getResistance();
    double getPowerWanted();
    double getPower();
    int getId();
    Node* getPosNode();
    Node* getNegNode();
    ElementType getType();
    string getName();
    bool isEnabled();

    void setPosNode(Node* node);
    void setNegNode(Node* node);
    void setId(int id);
    void setVoltage(double voltage);
    void setCurrent(double current);
    void setResistance(double resistance);
    void setPowerWanted(double powerWanted);
    void setEnabled(bool isenabled);

    // if node == pNode, return nNode, else if node == nNode return pNode, else return NULL
    Node* getTheOtherNode(Node* node);
    // sets the type of elements
    void setType(ElementType ET);

};
