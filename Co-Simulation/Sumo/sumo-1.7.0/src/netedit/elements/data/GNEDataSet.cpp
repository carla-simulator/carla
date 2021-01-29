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
/// @file    GNEDataSet.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A abstract class for data sets
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/GNEUndoList.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/frames/common/GNEInspectorFrame.h>

#include "GNEDataSet.h"
#include "GNEDataInterval.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEDataSet::AttributeColors - methods
// ---------------------------------------------------------------------------

GNEDataSet::AttributeColors::AttributeColors() {
}


void
GNEDataSet::AttributeColors::updateValues(const std::string& attribute, const double value) {
    // check if exist
    if (myMinMaxValue.count(attribute) == 0) {
        myMinMaxValue[attribute] = std::make_pair(value, value);
    } else {
        // update min value
        if (value < myMinMaxValue.at(attribute).first) {
            myMinMaxValue.at(attribute).first = value;
        }
        // update max value
        if (value > myMinMaxValue.at(attribute).second) {
            myMinMaxValue.at(attribute).second = value;
        }
    }
}


void
GNEDataSet::AttributeColors::updateAllValues(const AttributeColors& attributeColors) {
    // iterate over map
    for (const auto& attributeColor : attributeColors.myMinMaxValue) {
        if (myMinMaxValue.count(attributeColor.first) == 0) {
            myMinMaxValue[attributeColor.first] = attributeColor.second;
        } else {
            // update min value
            if (attributeColor.second.first < myMinMaxValue.at(attributeColor.first).first) {
                myMinMaxValue.at(attributeColor.first).first = attributeColor.second.first;
            }
            // update max value
            if (attributeColor.second.second > myMinMaxValue.at(attributeColor.first).second) {
                myMinMaxValue.at(attributeColor.first).second = attributeColor.second.second;
            }
        }
    }
}


double
GNEDataSet::AttributeColors::getMinValue(const std::string& attribute) const {
    return myMinMaxValue.at(attribute).first;
}


double
GNEDataSet::AttributeColors::getMaxValue(const std::string& attribute) const {
    return myMinMaxValue.at(attribute).second;
}


void
GNEDataSet::AttributeColors::clear() {
    myMinMaxValue.clear();
}

// ---------------------------------------------------------------------------
// GNEDataSet - methods
// ---------------------------------------------------------------------------

GNEDataSet::GNEDataSet(GNENet* net, const std::string dataSetID) :
    GNEAttributeCarrier(SUMO_TAG_DATASET, net),
    myDataSetID(dataSetID) {
}


GNEDataSet::~GNEDataSet() {}


const std::string&
GNEDataSet::getID() const {
    return myDataSetID;
}


GUIGlObject*
GNEDataSet::getGUIGlObject() {
    return nullptr;
}


void
GNEDataSet::setDataSetID(const std::string& newID) {
    // update ID
    myDataSetID = newID;
    // iterate over all intervals
    for (const auto& interval : myDataIntervalChildren) {
        interval.second->updateGenericDataIDs();
    }
}


void
GNEDataSet::updateAttributeColors() {
    // first update attribute colors in data interval childrens
    for (const auto& interval : myDataIntervalChildren) {
        interval.second->updateAttributeColors();
    }
    // continue with data sets containers
    myAllAttributeColors.clear();
    mySpecificAttributeColors.clear();
    // iterate over all data interval children
    for (const auto& interval : myDataIntervalChildren) {
        myAllAttributeColors.updateAllValues(interval.second->getAllAttributeColors());
    }
    // iterate over specificdata interval children
    for (const auto& interval : myDataIntervalChildren) {
        for (const auto& specificAttributeColor : interval.second->getSpecificAttributeColors()) {
            mySpecificAttributeColors[specificAttributeColor.first].updateAllValues(specificAttributeColor.second);
        }
    }
}


const GNEDataSet::AttributeColors&
GNEDataSet::getAllAttributeColors() const {
    return myAllAttributeColors;
}


const std::map<SumoXMLTag, GNEDataSet::AttributeColors>&
GNEDataSet::getSpecificAttributeColors() const {
    return mySpecificAttributeColors;
}


void
GNEDataSet::updateGeometry() {
    // nothing to update
}


Position
GNEDataSet::getPositionInView() const {
    return Position(0, 0);
}


void
GNEDataSet::writeDataSet(OutputDevice& device) const {
    // iterate over intervals
    for (const auto& interval : myDataIntervalChildren) {
        // open device
        device.openTag(SUMO_TAG_INTERVAL);
        // write ID
        device.writeAttr(SUMO_ATTR_ID, getID());
        // write begin
        device.writeAttr(SUMO_ATTR_BEGIN, interval.second->getAttribute(SUMO_ATTR_BEGIN));
        // write end
        device.writeAttr(SUMO_ATTR_END, interval.second->getAttribute(SUMO_ATTR_END));
        // iterate over interval generic datas
        for (const auto& genericData : interval.second->getGenericDataChildren()) {
            // write generic data
            genericData->writeGenericData(device);
        }
        // close device
        device.closeTag();
    }
}


void
GNEDataSet::addDataIntervalChild(GNEDataInterval* dataInterval) {
    // check that dataInterval wasn't previously inserted
    if (myDataIntervalChildren.count(dataInterval->getAttributeDouble(SUMO_ATTR_BEGIN)) == 0) {
        // add data interval child
        myDataIntervalChildren[dataInterval->getAttributeDouble(SUMO_ATTR_BEGIN)] = dataInterval;
    } else {
        throw ProcessError("DataInterval was already inserted");
    }
}


void
GNEDataSet::removeDataIntervalChild(GNEDataInterval* dataInterval) {
    // check that dataInterval was previously inserted
    if (myDataIntervalChildren.count(dataInterval->getAttributeDouble(SUMO_ATTR_BEGIN)) == 1) {
        // remove data interval child
        myDataIntervalChildren.erase(dataInterval->getAttributeDouble(SUMO_ATTR_BEGIN));
        // remove it from Inspector Frame and HierarchicalElementTree
        myNet->getViewNet()->getViewParent()->getInspectorFrame()->getAttributesEditor()->removeEditedAC(dataInterval);
        myNet->getViewNet()->getViewParent()->getInspectorFrame()->getHierarchicalElementTree()->removeCurrentEditedAttributeCarrier(dataInterval);
    } else {
        throw ProcessError("DataInterval wasn't previously inserted");
    }
}


bool
GNEDataSet::dataIntervalChildrenExist(GNEDataInterval* dataInterval) const {
    for (const auto& interval : myDataIntervalChildren) {
        if (interval.second == dataInterval) {
            return true;
        }
    }
    return false;
}

void
GNEDataSet::updateDataIntervalBegin(const double oldBegin) {
    // check that dataInterval was previously inserted
    if (myDataIntervalChildren.count(oldBegin) == 1) {
        // get data interval
        GNEDataInterval* dataInterval = myDataIntervalChildren.at(oldBegin);
        // insert again using new begin
        myDataIntervalChildren[dataInterval->getAttributeDouble(SUMO_ATTR_BEGIN)] = dataInterval;
    } else {
        throw ProcessError("DataInterval wasn't previously inserted");
    }
}


bool
GNEDataSet::checkNewInterval(const double newBegin, const double newEnd) {
    return checkNewInterval(myDataIntervalChildren, newBegin, newEnd);
}


bool
GNEDataSet::checkNewBeginEnd(const GNEDataInterval* dataInterval, const double newBegin, const double newEnd) {
    // make a copy of myDataIntervalChildren without dataInterval, and check checkNewInterval
    std::map<const double, GNEDataInterval*> copyOfDataIntervalMap;
    for (const auto& element : myDataIntervalChildren) {
        if (element.second != dataInterval) {
            copyOfDataIntervalMap.insert(element);
        }
    }
    return checkNewInterval(copyOfDataIntervalMap, newBegin, newEnd);
}


GNEDataInterval*
GNEDataSet::retrieveInterval(const double begin, const double end) const {
    if (myDataIntervalChildren.count(begin) == 0) {
        return nullptr;
    } else if (myDataIntervalChildren.at(begin)->getAttributeDouble(SUMO_ATTR_END) != end) {
        return nullptr;
    } else {
        return myDataIntervalChildren.at(begin);
    }
}


const std::map<const double, GNEDataInterval*>&
GNEDataSet::getDataIntervalChildren() const {
    return myDataIntervalChildren;
}


std::string
GNEDataSet::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return myDataSetID;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEDataSet::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEDataSet::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    switch (key) {
        case SUMO_ATTR_ID:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEDataSet::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            if (SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveDataSet(value, false) == nullptr)) {
                return true;
            } else {
                return false;
            }
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEDataSet::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // nothing to enable
}


void
GNEDataSet::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // nothing to disable
}


bool
GNEDataSet::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNEDataSet::getPopUpID() const {
    return getTagStr();
}


std::string
GNEDataSet::getHierarchyName() const {
    return getTagStr() + ": " + myDataSetID;
}


void
GNEDataSet::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEDataSet::setEnabledAttribute(const int /*enabledAttributes*/) {
    throw InvalidArgument("Nothing to enable");
}


bool
GNEDataSet::checkNewInterval(const std::map<const double, GNEDataInterval*>& dataIntervalMap, const double newBegin, const double newEnd) {
    if (dataIntervalMap.empty()) {
        return true;
    } else {
        // declare first and last element
        const auto itFirstElement = dataIntervalMap.begin();
        const auto itLastElement = dataIntervalMap.rbegin();
        if (newBegin > newEnd) {
            return false;
        } else if (dataIntervalMap.count(newBegin) == 1) {
            return false;
        } else if (newBegin < itFirstElement->first) {
            return (newEnd <= itFirstElement->first);
        } else if (newBegin > itLastElement->first) {
            return (newBegin >= itLastElement->second->getAttributeDouble(SUMO_ATTR_END));
        } else {
            // iterate over myDataIntervalChildren
            for (auto it = itFirstElement; it != dataIntervalMap.end(); it++) {
                if (newBegin < it->first) {
                    // obtain previous edge
                    auto itPrevious = it;
                    itPrevious--;
                    // check overlapping with end
                    if (itPrevious->second->getAttributeDouble(SUMO_ATTR_END) < newBegin) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
}

/****************************************************************************/
