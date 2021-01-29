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
/// @file    GNEDataHandler.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// Builds data objects for netedit
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <netedit/changes/GNEChange_DataSet.h>
#include <netedit/changes/GNEChange_DataInterval.h>
#include <netedit/changes/GNEChange_GenericData.h>
#include <netedit/elements/data/GNEEdgeRelData.h>
#include <netedit/elements/data/GNEEdgeData.h>
#include <netedit/elements/data/GNETAZRelData.h>
#include <netedit/elements/data/GNEDataInterval.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>

#include "GNEDataHandler.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEAdditionalHandler::HierarchyInsertedDatas method definitions
// ---------------------------------------------------------------------------

void
GNEDataHandler::HierarchyInsertedDatas::insertElement(SumoXMLTag tag) {
    myInsertedElements.push_back(std::make_pair(tag, std::make_pair(nullptr, nullptr)));
}



void
GNEDataHandler::HierarchyInsertedDatas::commitDataIntervalInsertion(GNEDataInterval* dataIntervalCreated) {
    myInsertedElements.back().second.first = dataIntervalCreated;
}


void
GNEDataHandler::HierarchyInsertedDatas::commitGenericDataInsertion(GNEGenericData* genericDataCreated) {
    myInsertedElements.back().second.second = genericDataCreated;
}


void
GNEDataHandler::HierarchyInsertedDatas::popElement() {
    if (!myInsertedElements.empty()) {
        myInsertedElements.pop_back();
    }
}


GNEDataInterval*
GNEDataHandler::HierarchyInsertedDatas::retrieveParentDataInterval(SumoXMLTag expectedTag) const {
    if (myInsertedElements.size() < 2) {
        // currently we're finding parent additional in the additional XML root
        WRITE_WARNING("A " + toString(myInsertedElements.back().first) + " must be declared within the definition of a " + toString(expectedTag) + ".");
        return nullptr;
    } else {
        if (myInsertedElements.size() < 2) {
            // additional was hierarchically bad loaded, then return nullptr
            return nullptr;
        } else if ((myInsertedElements.end() - 2)->second.first == nullptr) {
            WRITE_WARNING(toString(expectedTag) + " parent of " + toString((myInsertedElements.end() - 1)->first) + " was not loaded sucesfully.");
            // parent additional wasn't sucesfully loaded, then return nullptr
            return nullptr;
        }
        return nullptr;
    }
}


GNEDataInterval*
GNEDataHandler::HierarchyInsertedDatas::getLastInsertedDataInterval() const {
    // ierate in reverse mode over myInsertedElements to obtain last inserted additional
    for (std::vector<std::pair<SumoXMLTag, std::pair<GNEDataInterval*, GNEGenericData* > > >::const_reverse_iterator i = myInsertedElements.rbegin(); i != myInsertedElements.rend(); i++) {
        // we need to avoid Tag Param because isn't a data element
        if ((i->first != SUMO_TAG_PARAM) && (i->first == SUMO_TAG_INTERVAL)) {
            return i->second.first;
        }
    }
    return nullptr;
}


GNEGenericData*
GNEDataHandler::HierarchyInsertedDatas::retrieveParentGenericData(SumoXMLTag expectedTag) const {
    if (myInsertedElements.size() < 2) {
        // currently we're finding parent additional in the additional XML root
        WRITE_WARNING("A " + toString(myInsertedElements.back().first) + " must be declared within the definition of a " + toString(expectedTag) + ".");
        return nullptr;
    } else {
        if (myInsertedElements.size() < 2) {
            // additional was hierarchically bad loaded, then return nullptr
            return nullptr;
        } else if ((myInsertedElements.end() - 2)->second.second == nullptr) {
            WRITE_WARNING(toString(expectedTag) + " parent of " + toString((myInsertedElements.end() - 1)->first) + " was not loaded sucesfully.");
            // parent additional wasn't sucesfully loaded, then return nullptr
            return nullptr;
        }
        return nullptr;
    }
}


GNEGenericData*
GNEDataHandler::HierarchyInsertedDatas::getLastInsertedGenericData() const {
    // ierate in reverse mode over myInsertedElements to obtain last inserted additional
    for (std::vector<std::pair<SumoXMLTag, std::pair<GNEDataInterval*, GNEGenericData* > > >::const_reverse_iterator i = myInsertedElements.rbegin(); i != myInsertedElements.rend(); i++) {
        // we need to avoid Tag Param because isn't a data element
        if ((i->first != SUMO_TAG_PARAM) && (i->first != SUMO_TAG_INTERVAL)) {
            return i->second.second;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// GNEAdditionalHandler::HierarchyInsertedDatas method definitions
// ---------------------------------------------------------------------------

GNEDataHandler::GNEDataHandler(const std::string& file, GNENet* net) :
    SUMOSAXHandler(file),
    myNet(net) {
}


GNEDataHandler::~GNEDataHandler() {}


void
GNEDataHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    // Obtain tag of element
    SumoXMLTag tag = static_cast<SumoXMLTag>(element);
    // check if we're parsing a parameter
    if (tag == SUMO_TAG_PARAM) {
        // push element int stack
        myHierarchyInsertedGenericDatas.insertElement(tag);
        // parse parameter
        parseParameter(attrs);
    } else if (tag != SUMO_TAG_NOTHING) {
        // push element int stack
        myHierarchyInsertedGenericDatas.insertElement(tag);
        // build data
        buildData(myNet, true, tag, attrs, &myHierarchyInsertedGenericDatas);
    }
}


void
GNEDataHandler::myEndElement(int /*element*/) {
    // pop last inserted element
    myHierarchyInsertedGenericDatas.popElement();
}


bool
GNEDataHandler::buildData(GNENet* net, bool allowUndoRedo, SumoXMLTag tag, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas) {
    // Call parse and build depending of tag
    switch (tag) {
        case SUMO_TAG_INTERVAL:
            return parseAndBuildInterval(net, allowUndoRedo, attrs, insertedDatas);
        case SUMO_TAG_EDGE:
            return parseAndBuildEdgeData(net, allowUndoRedo, attrs, insertedDatas);
        case SUMO_TAG_EDGEREL:
            return parseAndBuildEdgeRelationData(net, allowUndoRedo, attrs, insertedDatas);
        case SUMO_TAG_TAZREL:
            return parseAndBuildTAZRelationData(net, allowUndoRedo, attrs, insertedDatas);
        default:
            return false;
    }
}


GNEDataSet*
GNEDataHandler::buildDataSet(GNENet* net, bool allowUndoRedo, const std::string& dataSetID) {
    GNEDataSet* dataSet = new GNEDataSet(net, dataSetID);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_DATASET));
        net->getViewNet()->getUndoList()->add(new GNEChange_DataSet(dataSet, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        dataSet->incRef("buildDataSet");
    }
    return dataSet;
}


GNEDataInterval*
GNEDataHandler::buildDataInterval(GNENet* net, bool allowUndoRedo, GNEDataSet* dataSetParent, const double begin, const double end) {
    GNEDataInterval* dataInterval = new GNEDataInterval(dataSetParent, begin, end);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_DATAINTERVAL));
        net->getViewNet()->getUndoList()->add(new GNEChange_DataInterval(dataInterval, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        dataSetParent->addDataIntervalChild(dataInterval);
        dataInterval->incRef("buildDataInterval");
    }
    return dataInterval;
}


GNEGenericData*
GNEDataHandler::buildEdgeData(GNENet* net, bool allowUndoRedo, GNEDataInterval* dataIntervalParent, GNEEdge* edge,
                              const std::map<std::string, std::string>& parameters) {
    GNEGenericData* edgeData = new GNEEdgeData(dataIntervalParent, edge, parameters);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_MEANDATA_EDGE));
        net->getViewNet()->getUndoList()->add(new GNEChange_GenericData(edgeData, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        dataIntervalParent->addGenericDataChild(edgeData);
        edge->addChildElement(edgeData);
        edgeData->incRef("buildEdgeData");
    }
    return edgeData;
}


GNEGenericData*
GNEDataHandler::buildEdgeRelationData(GNENet* net, bool allowUndoRedo, GNEDataInterval* dataIntervalParent,
                                      GNEEdge* fromEdge, GNEEdge* toEdge, const std::map<std::string, std::string>& parameters) {
    GNEGenericData* edgeRelationData = new GNEEdgeRelData(dataIntervalParent, fromEdge, toEdge, parameters);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_EDGEREL));
        net->getViewNet()->getUndoList()->add(new GNEChange_GenericData(edgeRelationData, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        dataIntervalParent->addGenericDataChild(edgeRelationData);
        fromEdge->addChildElement(edgeRelationData);
        toEdge->addChildElement(edgeRelationData);
        edgeRelationData->incRef("buildEdgeRelationData");
    }
    return edgeRelationData;
}


GNEGenericData*
GNEDataHandler::buildTAZRelationData(GNENet* net, bool allowUndoRedo, GNEDataInterval* dataIntervalParent,
                                     GNETAZElement* fromTAZ, GNETAZElement* toTAZ, const std::map<std::string, std::string>& parameters) {
    GNEGenericData* TAZRelationData = new GNETAZRelData(dataIntervalParent, fromTAZ, toTAZ, parameters);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_TAZREL));
        net->getViewNet()->getUndoList()->add(new GNEChange_GenericData(TAZRelationData, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        dataIntervalParent->addGenericDataChild(TAZRelationData);
        fromTAZ->addChildElement(TAZRelationData);
        toTAZ->addChildElement(TAZRelationData);
        TAZRelationData->incRef("buildTAZRelationData");
    }
    return TAZRelationData;
}


bool
GNEDataHandler::parseAndBuildInterval(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas) {
    bool abort = false;
    // parse edgeData attributes
    const std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_DATAINTERVAL, SUMO_ATTR_ID, abort);
    const double begin = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_DATAINTERVAL, SUMO_ATTR_BEGIN, abort);
    const double end = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_DATAINTERVAL, SUMO_ATTR_END, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // retrieve data set parent
        GNEDataSet* dataSet = net->retrieveDataSet(id, false);
        // check if we need to create a new data set
        if (dataSet == nullptr) {
            dataSet = buildDataSet(net, true, id);
        }
        // retrieve data interval
        GNEDataInterval* dataInterval = dataSet->retrieveInterval(begin, end);
        // check if data interval exist
        if (dataInterval) {
            // check if insertion has to be commited
            if (insertedDatas) {
                insertedDatas->commitDataIntervalInsertion(dataInterval);
            }
        } else {
            // create data interval
            dataInterval = buildDataInterval(net, allowUndoRedo, dataSet, begin, end);
            // check if insertion has to be commited
            if (insertedDatas) {
                insertedDatas->commitDataIntervalInsertion(dataInterval);
            }
        }
        return true;
    }
    return false;
}


bool
GNEDataHandler::parseAndBuildEdgeData(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas) {
    bool abort = false;
    // parse edgeData attributes
    std::string edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_MEANDATA_EDGE, SUMO_ATTR_ID, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to edge
        GNEEdge* edge = net->retrieveEdge(edgeID, false);
        // check that edge is valid
        if (edge == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_MEANDATA_EDGE) + " '" + edgeID + "' is not known.");
        } else if (insertedDatas->getLastInsertedDataInterval() == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING(toString(SUMO_TAG_MEANDATA_EDGE) + " '" + edgeID + "' must be created within a data interval.");
        } else {
            // check if there is already a edge data for the given edge in the given interval
            for (const auto& genericData : insertedDatas->getLastInsertedDataInterval()->getGenericDataChildren()) {
                if ((genericData->getTagProperty().getTag() == SUMO_TAG_MEANDATA_EDGE) &&
                        (genericData->getParentEdges().front() == edge)) {
                    WRITE_WARNING("There is already a " + genericData->getTagStr() + " in edge '" +
                                  edge->getID() + "' in interval " +
                                  insertedDatas->getLastInsertedDataInterval()->getID() + " [" +
                                  insertedDatas->getLastInsertedDataInterval()->getAttribute(SUMO_ATTR_BEGIN) + ", " +
                                  insertedDatas->getLastInsertedDataInterval()->getAttribute(SUMO_ATTR_END) + "]");
                    return false;
                }
            }
            // declare parameter map
            std::map<std::string, std::string> parameters;
            // obtain all attribute
            const std::vector<std::string> attributes = attrs.getAttributeNames();
            // iterate over attributes and fill parameters map
            for (const auto& attribute : attributes) {
                if (attribute != toString(SUMO_ATTR_ID)) {
                    parameters[attribute] = attrs.getStringSecure(attribute, "");
                }
            }
            // save ID of last created element
            GNEGenericData* dataCreated = buildEdgeData(net, allowUndoRedo, insertedDatas->getLastInsertedDataInterval(), edge, parameters);
            // check if insertion has to be commited
            if (insertedDatas) {
                insertedDatas->commitGenericDataInsertion(dataCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEDataHandler::parseAndBuildEdgeRelationData(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas) {
    bool abort = false;
    // parse edgeRelationData attributes
    std::string fromEdgeStr = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_EDGEREL, SUMO_ATTR_FROM, abort);
    std::string toEdgeStr = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_EDGEREL, SUMO_ATTR_TO, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointers to edges
        GNEEdge* fromEdge = net->retrieveEdge(fromEdgeStr, false);
        GNEEdge* toEdge = net->retrieveEdge(toEdgeStr, false);
        // check that edge is valid
        if (fromEdge == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The from edge '" + fromEdgeStr + "' to use within " + toString(SUMO_TAG_EDGEREL) + " is not known.");
        } else if (toEdge == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The to edge '" + toEdgeStr + "' to use within " + toString(SUMO_TAG_EDGEREL) + " is not known.");
        } else if (insertedDatas->getLastInsertedDataInterval() == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING(toString(SUMO_TAG_EDGEREL) + " must be created within a data interval.");
        } else {
            // check if there is already a edge data for the given edge in the interval
            for (const auto& genericData : insertedDatas->getLastInsertedDataInterval()->getGenericDataChildren()) {
                if ((genericData->getTagProperty().getTag() == SUMO_TAG_EDGEREL) &&
                        (genericData->getParentEdges().front() == fromEdge) &&
                        (genericData->getParentEdges().back() == toEdge)) {
                    WRITE_WARNING("There is already a " + genericData->getTagStr() + " for edges '" +
                                  fromEdge->getID() + "'->'" + toEdge->getID() + "' in interval " +
                                  insertedDatas->getLastInsertedDataInterval()->getID() + " [" +
                                  insertedDatas->getLastInsertedDataInterval()->getAttribute(SUMO_ATTR_BEGIN) + ", " +
                                  insertedDatas->getLastInsertedDataInterval()->getAttribute(SUMO_ATTR_END) + "]");
                    return false;
                }
            }
            // declare parameter map
            std::map<std::string, std::string> parameters;
            // obtain all attribute
            const std::vector<std::string> attributes = attrs.getAttributeNames();
            // iterate over attributes and fill parameters map
            for (const auto& attribute : attributes) {
                if ((attribute != toString(SUMO_ATTR_ID)) && (attribute != toString(SUMO_ATTR_FROM)) && (attribute != toString(SUMO_ATTR_TO))) {
                    parameters[attribute] = attrs.getStringSecure(attribute, "");
                }
            }
            // save ID of last created element
            GNEGenericData* dataCreated = buildEdgeRelationData(net, allowUndoRedo, insertedDatas->getLastInsertedDataInterval(),
                                          fromEdge, toEdge, parameters);
            // check if insertion has to be commited
            if (insertedDatas) {
                insertedDatas->commitGenericDataInsertion(dataCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEDataHandler::parseAndBuildTAZRelationData(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas) {
    bool abort = false;
    // parse TAZRelationData attributes
    std::string fromTAZStr = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_TAZREL, SUMO_ATTR_FROM, abort);
    std::string toTAZStr = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_TAZREL, SUMO_ATTR_TO, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointers to TAZs
        GNETAZElement* fromTAZ = net->retrieveTAZElement(SUMO_TAG_TAZ, fromTAZStr, false);
        GNETAZElement* toTAZ = net->retrieveTAZElement(SUMO_TAG_TAZ, toTAZStr, false);
        // check that TAZ is valid
        if (fromTAZ == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The from TAZ '" + fromTAZStr + "' to use within " + toString(SUMO_TAG_TAZREL) + " is not known.");
        } else if (toTAZ == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The to TAZ '" + toTAZStr + "' to use within " + toString(SUMO_TAG_TAZREL) + " is not known.");
        } else if (insertedDatas->getLastInsertedDataInterval() == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING(toString(SUMO_TAG_TAZREL) + " must be created within a data interval.");
        } else {
            // declare parameter map
            std::map<std::string, std::string> parameters;
            // obtain all attribute
            const std::vector<std::string> attributes = attrs.getAttributeNames();
            // iterate over attributes and fill parameters map
            for (const auto& attribute : attributes) {
                if ((attribute != toString(SUMO_ATTR_ID)) && (attribute != toString(SUMO_ATTR_FROM)) && (attribute != toString(SUMO_ATTR_TO))) {
                    parameters[attribute] = attrs.getStringSecure(attribute, "");
                }
            }
            // save ID of last created element
            GNEGenericData* dataCreated = buildTAZRelationData(net, allowUndoRedo, insertedDatas->getLastInsertedDataInterval(),
                                          fromTAZ, toTAZ, parameters);
            // check if insertion has to be commited
            if (insertedDatas) {
                insertedDatas->commitGenericDataInsertion(dataCreated);
            }
            return true;
        }
    }
    return false;
}

// ===========================================================================
// private method definitions
// ===========================================================================

void
GNEDataHandler::parseParameter(const SUMOSAXAttributes& attrs) {
    if (myHierarchyInsertedGenericDatas.getLastInsertedGenericData()) {
        // first check if given data supports parameters
        if (myHierarchyInsertedGenericDatas.getLastInsertedGenericData()->getTagProperty().hasParameters()) {
            bool ok = true;
            std::string key;
            if (attrs.hasAttribute(SUMO_ATTR_KEY)) {
                // obtain key
                key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                if (key.empty()) {
                    WRITE_WARNING("Error parsing key from data parameter. Key cannot be empty");
                    ok = false;
                }
                if (!SUMOXMLDefinitions::isValidTypeID(key)) {
                    WRITE_WARNING("Error parsing key from data parameter. Key contains invalid characters");
                    ok = false;
                }
            } else {
                WRITE_WARNING("Error parsing key from data parameter. Key doesn't exist");
                ok = false;
            }
            // circumventing empty string test
            const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
            if (!SUMOXMLDefinitions::isValidAttribute(val)) {
                WRITE_WARNING("Error parsing value from data parameter. Value contains invalid characters");
                ok = false;
            }
            // check double values
            if (myHierarchyInsertedGenericDatas.getLastInsertedGenericData()->getTagProperty().hasParameters() && !GNEAttributeCarrier::canParse<double>(val)) {
                WRITE_WARNING("Error parsing value from data float parameter. Value cannot be parsed to float");
                ok = false;
            }
            // set parameter in last inserted data
            if (ok) {
                WRITE_DEBUG("Inserting parameter '" + key + "|" + val + "' into data " + myHierarchyInsertedGenericDatas.getLastInsertedGenericData()->getTagStr() + ".");
                myHierarchyInsertedGenericDatas.getLastInsertedGenericData()->setParameter(key, val);
            }
        } else {
            WRITE_WARNING("Datas of type '" + myHierarchyInsertedGenericDatas.getLastInsertedGenericData()->getTagStr() + "' doesn't support parameters");
        }
    } else {
        WRITE_WARNING("Parameters has to be declared within the definition of an data");
    }
}

/****************************************************************************/
