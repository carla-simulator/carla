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
/// @file    GNEAttributeCarrier.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// Abstract Base class for gui objects which carry attributes
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/frames/common/GNESelectorFrame.h>
#include <utils/common/StringTokenizer.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/options/OptionsCont.h>

#include "GNEAttributeCarrier.h"


// ===========================================================================
// static members
// ===========================================================================

std::map<SumoXMLTag, GNETagProperties> GNEAttributeCarrier::myTagProperties;
GNETagProperties GNEAttributeCarrier::dummyTagProperty;
const std::string GNEAttributeCarrier::FEATURE_LOADED = "loaded";
const std::string GNEAttributeCarrier::FEATURE_GUESSED = "guessed";
const std::string GNEAttributeCarrier::FEATURE_MODIFIED = "modified";
const std::string GNEAttributeCarrier::FEATURE_APPROVED = "approved";
const size_t GNEAttributeCarrier::MAXNUMBEROFATTRIBUTES = 128;
const double GNEAttributeCarrier::INVALID_POSITION(-1000000);


// ===========================================================================
// method definitions
// ===========================================================================

GNEAttributeCarrier::GNEAttributeCarrier(const SumoXMLTag tag, GNENet* net) :
    myTagProperty(getTagProperties(tag)),
    myNet(net),
    mySelected(false) {
}


GNEAttributeCarrier::~GNEAttributeCarrier() {}


GNENet*
GNEAttributeCarrier::getNet() const {
    return myNet;
}


void
GNEAttributeCarrier::selectAttributeCarrier(const bool changeFlag) {
    if (getGUIGlObject() && myTagProperty.isSelectable()) {
        gSelected.select(getGUIGlObject()->getGlID());
        // add object into list of selected objects
        myNet->getViewNet()->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->addedLockedObject(getGUIGlObject()->getType());
        if (changeFlag) {
            mySelected = true;
        }
    }
}


void
GNEAttributeCarrier::unselectAttributeCarrier(const bool changeFlag) {
    if (getGUIGlObject() && myTagProperty.isSelectable()) {
        gSelected.deselect(getGUIGlObject()->getGlID());
        // remove object of list of selected objects
        myNet->getViewNet()->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->removeLockedObject(getGUIGlObject()->getType());
        if (changeFlag) {
            mySelected = false;
        }
    }
}


bool
GNEAttributeCarrier::isAttributeCarrierSelected() const {
    return mySelected;
}


bool
GNEAttributeCarrier::drawUsingSelectColor() const {
    // get flag for network element
    const bool networkElement = myTagProperty.isNetworkElement() || myTagProperty.isAdditionalElement() || myTagProperty.isShape() || myTagProperty.isTAZElement();
    // check supermode network
    if ((networkElement && myNet->getViewNet()->getEditModes().isCurrentSupermodeNetwork()) ||
            (myTagProperty.isDemandElement() && myNet->getViewNet()->getEditModes().isCurrentSupermodeDemand()) ||
            (myTagProperty.isGenericData() && myNet->getViewNet()->getEditModes().isCurrentSupermodeData())) {
        return mySelected;
    } else {
        return false;
    }
}


template<> int
GNEAttributeCarrier::parse(const std::string& string) {
    return StringUtils::toInt(string);
}


template<> double
GNEAttributeCarrier::parse(const std::string& string) {
    return StringUtils::toDouble(string);
}


template<> SUMOTime
GNEAttributeCarrier::parse(const std::string& string) {
    SUMOTime time = string2time(string);
    if (time < 0) {
        throw TimeFormatException("SUMOTIME cannot be negative");
    } else {
        return time;
    }
}


template<> bool
GNEAttributeCarrier::parse(const std::string& string) {
    return StringUtils::toBool(string);
}


template<> std::string
GNEAttributeCarrier::parse(const std::string& string) {
    return string;
}


template<> SUMOVehicleClass
GNEAttributeCarrier::parse(const std::string& string) {
    if (string.size() == 0) {
        throw EmptyData();
    } else if (!SumoVehicleClassStrings.hasString(string)) {
        return SVC_IGNORING;
    } else {
        return SumoVehicleClassStrings.get(string);
    }
}


template<> RGBColor
GNEAttributeCarrier::parse(const std::string& string) {
    return RGBColor::parseColor(string);
}


template<> Position
GNEAttributeCarrier::parse(const std::string& string) {
    if (string.size() == 0) {
        throw EmptyData();
    } else {
        bool ok = true;
        PositionVector pos = GeomConvHelper::parseShapeReporting(string, "user-supplied position", 0, ok, false, false);
        if (!ok || (pos.size() != 1)) {
            throw NumberFormatException("(Position) " + string);
        } else {
            return pos[0];
        }
    }
}


template<> PositionVector
GNEAttributeCarrier::parse(const std::string& string) {
    PositionVector posVector;
    // empty string are allowed (It means empty position vector)
    if (string.empty()) {
        return posVector;
    } else {
        bool ok = true;
        posVector = GeomConvHelper::parseShapeReporting(string, "user-supplied shape", 0, ok, false, true);
        if (!ok) {
            throw NumberFormatException("(Position List) " + string);
        } else {
            return posVector;
        }
    }
}


template<> SUMOVehicleShape
GNEAttributeCarrier::parse(const std::string& string) {
    if ((string == "unknown") || (!SumoVehicleShapeStrings.hasString(string))) {
        return SVS_UNKNOWN;
    } else {
        return SumoVehicleShapeStrings.get(string);
    }
}


template<> std::vector<std::string>
GNEAttributeCarrier::parse(const std::string& string) {
    return StringTokenizer(string).getVector();
}


template<> std::set<std::string>
GNEAttributeCarrier::parse(const std::string& string) {
    std::vector<std::string> vectorString = StringTokenizer(string).getVector();
    std::set<std::string> solution;
    for (const auto& i : vectorString) {
        solution.insert(i);
    }
    return solution;
}


template<> std::vector<int>
GNEAttributeCarrier::parse(const std::string& string) {
    std::vector<std::string> parsedValues = parse<std::vector<std::string> >(string);
    std::vector<int> parsedIntValues;
    for (const auto& i : parsedValues) {
        parsedIntValues.push_back(parse<int>(i));
    }
    return parsedIntValues;
}


template<> std::vector<double>
GNEAttributeCarrier::parse(const std::string& string) {
    std::vector<std::string> parsedValues = parse<std::vector<std::string> >(string);
    std::vector<double> parsedDoubleValues;
    for (const auto& i : parsedValues) {
        parsedDoubleValues.push_back(parse<double>(i));
    }
    return parsedDoubleValues;
}


template<> std::vector<bool>
GNEAttributeCarrier::parse(const std::string& string) {
    std::vector<std::string> parsedValues = parse<std::vector<std::string> >(string);
    std::vector<bool> parsedBoolValues;
    for (const auto& i : parsedValues) {
        parsedBoolValues.push_back(parse<bool>(i));
    }
    return parsedBoolValues;
}


template<> std::vector<GNEEdge*>
GNEAttributeCarrier::parse(GNENet* net, const std::string& value) {
    // Declare string vector
    std::vector<std::string> edgeIds = GNEAttributeCarrier::parse<std::vector<std::string> > (value);
    std::vector<GNEEdge*> parsedEdges;
    // Iterate over edges IDs, retrieve Edges and add it into parsedEdges
    for (const auto& i : edgeIds) {
        GNEEdge* retrievedEdge = net->retrieveEdge(i, false);
        if (retrievedEdge) {
            parsedEdges.push_back(net->retrieveEdge(i));
        } else {
            throw FormatException("Error parsing parameter " + toString(SUMO_ATTR_EDGES) + ". " + toString(SUMO_TAG_EDGE) + " '" + i + "' doesn't exist");
        }
    }
    return parsedEdges;
}


template<> std::vector<GNELane*>
GNEAttributeCarrier::parse(GNENet* net, const std::string& value) {
    // Declare string vector
    std::vector<std::string> laneIds = GNEAttributeCarrier::parse<std::vector<std::string> > (value);
    std::vector<GNELane*> parsedLanes;
    // Iterate over lanes IDs, retrieve Lanes and add it into parsedLanes
    for (const auto& i : laneIds) {
        GNELane* retrievedLane = net->retrieveLane(i, false);
        if (retrievedLane) {
            parsedLanes.push_back(net->retrieveLane(i));
        } else {
            throw FormatException("Error parsing parameter " + toString(SUMO_ATTR_LANES) + ". " + toString(SUMO_TAG_LANE) + " '" + i + "'  doesn't exist");
        }
    }
    return parsedLanes;
}


template<> std::string
GNEAttributeCarrier::parseIDs(const std::vector<GNEEdge*>& ACs) {
    // obtain ID's of edges and return their join
    std::vector<std::string> edgeIDs;
    for (const auto& i : ACs) {
        edgeIDs.push_back(i->getID());
    }
    return joinToString(edgeIDs, " ");
}


template<> std::string
GNEAttributeCarrier::parseIDs(const std::vector<GNELane*>& ACs) {
    // obtain ID's of lanes and return their join
    std::vector<std::string> laneIDs;
    for (const auto& i : ACs) {
        laneIDs.push_back(i->getID());
    }
    return joinToString(laneIDs, " ");
}


bool
GNEAttributeCarrier::lanesConsecutives(const std::vector<GNELane*>& lanes) {
    // we need at least two lanes
    if (lanes.size() > 1) {
        // now check that lanes are consecutives (not neccesary connected)
        int currentLane = 0;
        while (currentLane < ((int)lanes.size() - 1)) {
            int nextLane = -1;
            // iterate over outgoing edges of destiny juntion of edge's lane
            for (int i = 0; (i < (int)lanes.at(currentLane)->getParentEdge()->getParentJunctions().back()->getGNEOutgoingEdges().size()) && (nextLane == -1); i++) {
                // iterate over lanes of outgoing edges of destiny juntion of edge's lane
                for (int j = 0; (j < (int)lanes.at(currentLane)->getParentEdge()->getParentJunctions().back()->getGNEOutgoingEdges().at(i)->getLanes().size()) && (nextLane == -1); j++) {
                    // check if lane correspond to the next lane of "lanes"
                    if (lanes.at(currentLane)->getParentEdge()->getParentJunctions().back()->getGNEOutgoingEdges().at(i)->getLanes().at(j) == lanes.at(currentLane + 1)) {
                        nextLane = currentLane;
                    }
                }
            }
            if (nextLane == -1) {
                return false;
            } else {
                currentLane++;
            }
        }
        return true;
    } else {
        return false;
    }
}


std::string
GNEAttributeCarrier::getAlternativeValueForDisabledAttributes(SumoXMLAttr key) const {
    switch (key) {
        // Crossings
        case SUMO_ATTR_TLLINKINDEX:
        case SUMO_ATTR_TLLINKINDEX2:
            return "No TLS";
        // connections
        case SUMO_ATTR_DIR: {
            // special case for connection directions
            std::string direction = getAttribute(key);
            if (direction == "s") {
                return "Straight (s)";
            } else if (direction ==  "t") {
                return "Turn (t))";
            } else if (direction ==  "l") {
                return "Left (l)";
            } else if (direction ==  "r") {
                return "Right (r)";
            } else if (direction ==  "L") {
                return "Partially left (L)";
            } else if (direction ==  "R") {
                return "Partially right (R)";
            } else if (direction ==  "invalid") {
                return "No direction (Invalid))";
            } else {
                return "undefined";
            }
        }
        case SUMO_ATTR_STATE: {
            // special case for connection states
            std::string state = getAttribute(key);
            if (state == "-") {
                return "Dead end (-)";
            } else if (state == "=") {
                return "equal (=)";
            } else if (state == "m") {
                return "Minor link (m)";
            } else if (state == "M") {
                return "Major link (M)";
            } else if (state == "O") {
                return "TLS controller off (O)";
            } else if (state == "o") {
                return "TLS yellow flashing (o)";
            } else if (state == "y") {
                return "TLS yellow minor link (y)";
            } else if (state == "Y") {
                return "TLS yellow major link (Y)";
            } else if (state == "r") {
                return "TLS red (r)";
            } else if (state == "g") {
                return "TLS green minor (g)";
            } else if (state == "G") {
                return "TLS green major (G)";
            } else {
                return "undefined";
            }
        }
        // flows
        case SUMO_ATTR_VEHSPERHOUR:
        case SUMO_ATTR_PERSONSPERHOUR:
        case SUMO_ATTR_PERIOD:
        case SUMO_ATTR_PROB:
        case SUMO_ATTR_END:
        case SUMO_ATTR_NUMBER:
            if (myTagProperty.hasAttribute(key) && myTagProperty.getAttributeProperties(key).isFlowDefinition()) {
                if (myTagProperty.hasAttribute(SUMO_ATTR_VEHSPERHOUR) && isAttributeEnabled(SUMO_ATTR_VEHSPERHOUR)) {
                    if (isAttributeEnabled(SUMO_ATTR_END)) {
                        return "not together with number and period or probability";
                    } else {
                        return "not together with end and period or probability";
                    }
                } else if (myTagProperty.hasAttribute(SUMO_ATTR_PERSONSPERHOUR) && isAttributeEnabled(SUMO_ATTR_PERSONSPERHOUR)) {
                    if (isAttributeEnabled(SUMO_ATTR_END)) {
                        return "not together with number and period or probability";
                    } else {
                        return "not together with end and period or probability";
                    }
                } else if (isAttributeEnabled(SUMO_ATTR_PERIOD)) {
                    if (isAttributeEnabled(SUMO_ATTR_END)) {
                        return "not together with number and vehsPerHour or probability";
                    } else {
                        return "not together with end and vehsPerHour or probability";
                    }
                } else if (isAttributeEnabled(SUMO_ATTR_PROB)) {
                    if (isAttributeEnabled(SUMO_ATTR_END)) {
                        return "not together with number and vehsPerHour or period";
                    } else {
                        return "not together with end and vehsPerHour or period";
                    }
                } else if (isAttributeEnabled(SUMO_ATTR_END) && (isAttributeEnabled(SUMO_ATTR_NUMBER))) {
                    return "not together with end and number";
                }
            }
            FALLTHROUGH;
        default:
            return getAttribute(key);
    }
}


std::string
GNEAttributeCarrier::getAttributeForSelection(SumoXMLAttr key) const {
    return getAttribute(key);
}


const std::string&
GNEAttributeCarrier::getTagStr() const {
    return myTagProperty.getTagStr();
}


const GNETagProperties&
GNEAttributeCarrier::getTagProperty() const {
    return myTagProperty;
}


FXIcon*
GNEAttributeCarrier::getIcon() const {
    // define on first access
    if (myTagProperties.size() == 0) {
        fillAttributeCarriers();
    }
    return GUIIconSubSys::getIcon(myTagProperty.getGUIIcon());
}

// ===========================================================================
// static methods
// ===========================================================================

const GNETagProperties&
GNEAttributeCarrier::getTagProperties(SumoXMLTag tag) {
    if (tag == SUMO_TAG_NOTHING) {
        return dummyTagProperty;
    }
    // define on first access
    if (myTagProperties.size() == 0) {
        fillAttributeCarriers();
    }
    // check that tag is defined
    if (myTagProperties.count(tag) == 0) {
        throw ProcessError("Attributes for tag '" + toString(tag) + "' not defined");
    } else {
        return myTagProperties.at(tag);
    }
}


std::vector<SumoXMLTag>
GNEAttributeCarrier::allowedTags(const bool onlyDrawables) {
    std::vector<SumoXMLTag> allTags;
    // define on first access
    if (myTagProperties.size() == 0) {
        fillAttributeCarriers();
    }
    // fill all tags
    for (const auto& i : myTagProperties) {
        if (!onlyDrawables || i.second.isDrawable()) {
            allTags.push_back(i.first);
        }
    }
    return allTags;
}


std::vector<std::pair<SumoXMLTag, const std::string> >
GNEAttributeCarrier::getAllowedTagsByCategory(const int tagPropertyCategory, const bool onlyDrawables) {
    std::vector<std::pair<SumoXMLTag, const std::string> > allowedTags;
    // define on first access
    if (myTagProperties.size() == 0) {
        fillAttributeCarriers();
    }
    if (tagPropertyCategory & GNETagProperties::NETWORKELEMENT) {
        // fill networkElements tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isNetworkElement() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::ADDITIONALELEMENT) {
        // fill additional tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isAdditionalElement() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::SHAPE) {
        // fill shape tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isShape() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::TAZELEMENT) {
        // fill taz tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isTAZElement() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::DEMANDELEMENT) {
        // fill demand tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isDemandElement() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::ROUTE) {
        // fill route tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isRoute() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::VEHICLE) {
        // fill vehicle tags (Special case)
        allowedTags.push_back(std::make_pair(SUMO_TAG_TRIP,             "trip"));
        allowedTags.push_back(std::make_pair(SUMO_TAG_VEHICLE,          "vehicle (over route)"));
        allowedTags.push_back(std::make_pair(GNE_TAG_VEHICLE_WITHROUTE, "vehicle (embedded route)"));
        allowedTags.push_back(std::make_pair(SUMO_TAG_FLOW,             "flow (from-to)"));
        allowedTags.push_back(std::make_pair(GNE_TAG_FLOW_ROUTE,        "flow (over route)"));
        allowedTags.push_back(std::make_pair(GNE_TAG_FLOW_WITHROUTE,    "flow (embedded route)"));

    }
    if (tagPropertyCategory & GNETagProperties::STOP) {
        // fill stop tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isStop() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::PERSON) {
        // fill person tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isPerson() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::PERSONPLAN) {
        // fill person plan tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isPersonPlan() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::PERSONTRIP) {
        // fill demand tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isPersonTrip() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::WALK) {
        // fill demand tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isWalk() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::RIDE) {
        // fill demand tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isRide() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::PERSONSTOP) {
        // fill demand tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isPersonStop() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    if (tagPropertyCategory & GNETagProperties::GENERICDATA) {
        // fill generic data tags
        for (const auto& tagProperty : myTagProperties) {
            if (tagProperty.second.isGenericData() && (!onlyDrawables || tagProperty.second.isDrawable())) {
                allowedTags.push_back(std::make_pair(tagProperty.first, toString(tagProperty.first)));
            }
        }
    }
    return allowedTags;
}

// ===========================================================================
// private
// ===========================================================================

void
GNEAttributeCarrier::fillAttributeCarriers() {
    // fill all groups of ACs
    fillNetworkElements();
    fillAdditionals();
    fillShapes();
    fillTAZElements();
    fillDemandElements();
    fillVehicleElements();
    fillStopElements();
    fillPersonElements();
    fillPersonPlanTrips();
    fillPersonPlanWalks();
    fillPersonPlanRides();
    fillPersonStopElements();
    fillPersonStopElements();
    fillDataElements();
    // check integrity of all Tags (function checkTagIntegrity() throw an exception if there is an inconsistency)
    for (const auto& tagProperty : myTagProperties) {
        tagProperty.second.checkTagIntegrity();
    }
}


void
GNEAttributeCarrier::fillNetworkElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // obtain Node Types except SumoXMLNodeType::DEAD_END_DEPRECATED
    const OptionsCont& oc = OptionsCont::getOptions();
    std::vector<std::string> nodeTypes = SUMOXMLDefinitions::NodeTypes.getStrings();
    nodeTypes.erase(std::find(nodeTypes.begin(), nodeTypes.end(), toString(SumoXMLNodeType::DEAD_END_DEPRECATED)));
    nodeTypes.erase(std::find(nodeTypes.begin(), nodeTypes.end(), toString(SumoXMLNodeType::DEAD_END)));
    nodeTypes.erase(std::find(nodeTypes.begin(), nodeTypes.end(), toString(SumoXMLNodeType::INTERNAL)));
    // fill networkElement ACs
    SumoXMLTag currentTag = SUMO_TAG_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::NETWORKELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE,
                                      GUIIcon::EDGE);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the edge");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of a node within the nodes-file the edge shall start at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of a node within the nodes-file the edge shall end at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEED,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The maximum speed allowed on the edge in m/s",
                                              toString(oc.getFloat("default.speed")));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PRIORITY,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The priority of the edge",
                                              toString(oc.getInt("default.priority")));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NUMLANES,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The number of lanes of the edge",
                                              toString(oc.getInt("default.lanenumber")));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The name of a type within the SUMO edge type file");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES,
                                              "Explicitly allows the given vehicle classes (not given will be not allowed)",
                                              "all");
        attrProperty.setDiscreteValues(SumoVehicleClassStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DISALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES,
                                              "Explicitly disallows the given vehicle classes (not given will be allowed)");
        attrProperty.setDiscreteValues(SumoVehicleClassStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "If the shape is given it should start and end with the positions of the from-node and to-node");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The length of the edge in meter");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPREADTYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Lane width for all lanes of this edge in meters (used for visualization)",
                                              "right");
        attrProperty.setDiscreteValues(SUMOXMLDefinitions::LaneSpreadFunctions.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "street name (need not be unique, used for visualization)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Lane width for all lanes of this edge in meters (used for visualization)",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDOFFSET,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Move the stop line back from the intersection by the given amount",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(GNE_ATTR_SHAPE_START,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY, // virtual attribute used to define an endPoint
                                              "Custom position in which shape start (by default position of junction from)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(GNE_ATTR_SHAPE_END,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY, // virtual attribute from to define an endPoint
                                              "Custom position in which shape end (by default position of junction from)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(GNE_ATTR_BIDIR,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC, // virtual attribute to check of this edge is part of a bidirectional railway (cannot be edited)
                                              "Show if edge is bidireccional",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DISTANCE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UNIQUE,
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_JUNCTION;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::NETWORKELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE,
                                      GUIIcon::JUNCTION);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the node");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::POSITION | GNEAttributeProperties::UPDATEGEOMETRY, // virtual attribute from the combination of the actually attributes SUMO_ATTR_X, SUMO_ATTR_Y
                                              "The x-y-z position of the node on the plane in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "An optional type for the node");
        attrProperty.setDiscreteValues(nodeTypes);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "A custom shape for that node");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_RADIUS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Optional turning radius (for all corners) for that node in meters",
                                              "1.5");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_KEEP_CLEAR,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Whether the junction-blocking-heuristic should be activated at this node",
                                              "1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_RIGHT_OF_WAY,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "How to compute right of way rules at this node",
                                              SUMOXMLDefinitions::RightOfWayValues.getString(RightOfWay::DEFAULT));
        attrProperty.setDiscreteValues(SUMOXMLDefinitions::RightOfWayValues.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRINGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Whether this junction is at the fringe of the network",
                                              SUMOXMLDefinitions::FringeTypeValues.getString(FringeType::DEFAULT));
        attrProperty.setDiscreteValues(SUMOXMLDefinitions::FringeTypeValues.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Optional name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLTYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "An optional type for the traffic light algorithm");
        attrProperty.setDiscreteValues({toString(TrafficLightType::STATIC), toString(TrafficLightType::ACTUATED), toString(TrafficLightType::DELAYBASED)});
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "An optional id for the traffic light program");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_LANE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::NETWORKELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::LANE);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "ID of lane (Automatic, non editable)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_INDEX,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The enumeration index of the lane (0 is the rightmost lane, <NUMBER_LANES>-1 is the leftmost one)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEED,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Speed in meters per second",
                                              "13.89");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES,
                                              "Explicitly allows the given vehicle classes (not given will be not allowed)",
                                              "all");
        attrProperty.setDiscreteValues(SumoVehicleClassStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DISALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES,
                                              "Explicitly disallows the given vehicle classes (not given will be allowed)");
        attrProperty.setDiscreteValues(SumoVehicleClassStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Width in meters (used for visualization)",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDOFFSET,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Move the stop line back from the intersection by the given amount",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ACCELERATION,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Enable or disable lane as acceleration lane",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CUSTOMSHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "If the shape is given it overrides the computation based on edge shape");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_CROSSING;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::NETWORKELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::CROSSING);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The ID of Crossing");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The (road) edges which are crossed");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PRIORITY,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Whether the pedestrians have priority over the vehicles (automatically set to true at tls-controlled intersections)",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The width of the crossings",
                                              toString(OptionsCont::getOptions().getFloat("default.crossing-width")));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLLINKINDEX,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "sets the tls-index for this crossing",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLLINKINDEX2,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "sets the opposite-direction tls-index for this crossing",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CUSTOMSHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Overrids default shape of pedestrian crossing");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_CONNECTION;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::NETWORKELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::CONNECTION);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the vehicles leave");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the vehicles may reach when leaving 'from'");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM_LANE,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "the lane index of the incoming lane (numbers starting with 0)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO_LANE,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "the lane index of the outgoing lane (numbers starting with 0)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PASS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "if set, vehicles which pass this (lane-2-lane) connection) will not wait",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_KEEP_CLEAR,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "if set to false, vehicles which pass this (lane-2-lane) connection) will not worry about blocking the intersection",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CONTPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "If set to a more than 0 value, an internal junction will be built at this position (in m) from the start of the internal lane for this connection",
                                              toString(NBEdge::UNSPECIFIED_CONTPOS));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_UNCONTROLLED,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "If set to true, This connection will not be TLS-controlled despite its node being controlled",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VISIBILITY_DISTANCE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Vision distance between vehicles",
                                              toString(NBEdge::UNSPECIFIED_VISIBILITY_DISTANCE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLLINKINDEX,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "sets index of this connection within the controlling trafficlight",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLLINKINDEX2,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "sets index for the internal junction of this connection within the controlling trafficlight",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES,
                                              "Explicitly allows the given vehicle classes (not given will be not allowed)",
                                              "all");
        attrProperty.setDiscreteValues(SumoVehicleClassStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DISALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES,
                                              "Explicitly disallows the given vehicle classes (not given will be allowed)");
        attrProperty.setDiscreteValues(SumoVehicleClassStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEED,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "sets custom speed limit for the connection",
                                              toString(NBEdge::UNSPECIFIED_SPEED));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "sets custom length for the connection",
                                              toString(NBEdge::UNSPECIFIED_LOADED_LENGTH));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CUSTOMSHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "sets custom shape for the connection");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DIR,
                                              GNEAttributeProperties::STRING,
                                              "turning direction for this connection (computed)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_STATE,
                                              GNEAttributeProperties::STRING,
                                              "link state for this connection (computed)");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_INTERNAL_LANE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::NETWORKELEMENT,
                                      GNETagProperties::DRAWABLE,
                                      GUIIcon::JUNCTION);
        //  internal lanes don't have attributes
    }
}


void
GNEAttributeCarrier::fillAdditionals() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill additional elements
    SumoXMLTag currentTag = SUMO_TAG_BUS_STOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::STOPPINGPLACE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::MASKSTARTENDPOS,
                                      GUIIcon::BUSSTOP);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of bus stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the lane the bus stop shall be located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_STARTPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The begin position on the lane (the lower position on the lane) in meters");

        myTagProperties[currentTag].addAttribute(attrProperty);
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The end position on the lane (the higher position on the lane) in meters, must be larger than startPos by more than 0.1m");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LINES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Meant to be the names of the bus lines that stop at this bus stop. This is only used for visualization purposes");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PERSON_CAPACITY,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Meant to be the names of the bus lines that stop at this bus stop. This is only used for visualization purposes",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PARKING_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Optional space definition for vehicles that park at this stop",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_ACCESS;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::ACCESS, {SUMO_TAG_BUS_STOP});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the lane the stop access shall be located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane (the lower position on the lane) in meters",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The walking length of the access in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_CONTAINER_STOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::STOPPINGPLACE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::MASKSTARTENDPOS,
                                      GUIIcon::CONTAINERSTOP);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of container stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the lane the container stop shall be located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_STARTPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The begin position on the lane (the lower position on the lane) in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The end position on the lane (the higher position on the lane) in meters, must be larger than startPos by more than 0.1m");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LINES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "meant to be the names of the bus lines that stop at this container stop. This is only used for visualization purposes");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_CHARGING_STATION;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::STOPPINGPLACE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::MASKSTARTENDPOS,
                                      GUIIcon::CHARGINGSTATION);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of charging station");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Lane of the charging station location");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_STARTPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Begin position in the specified lane");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "End position in the specified lane");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CHARGINGPOWER,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Charging power in W",
                                              "22000.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EFFICIENCY,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::RANGE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Charging efficiency [0,1]",
                                              "0.95");
        attrProperty.setRange(0, 1);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CHARGEINTRANSIT,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Enable or disable charge in transit, i.e. vehicle must or must not to stop for charging",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CHARGEDELAY,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Time delay after the vehicles has reached / stopped on the charging station, before the energy transfer (charging) begins",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_PARKING_AREA;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::STOPPINGPLACE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::MASKSTARTENDPOS,
                                      GUIIcon::PARKINGAREA);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of ParkingArea");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the lane the Parking Area shall be located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_STARTPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The begin position on the lane (the lower position on the lane) in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The end position on the lane (the higher position on the lane) in meters, must be larger than startPos by more than 0.1m");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROADSIDE_CAPACITY,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              " The number of parking spaces for road-side parking",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ONROAD,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, vehicles will park on the road lane and thereby reducing capacity",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The width of the road-side parking spaces",
                                              "3.20");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The length of the road-side parking spaces. By default (endPos - startPos) / roadsideCapacity");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ANGLE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::ANGLE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The angle of the road-side parking spaces relative to the lane angle, positive means clockwise",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_PARKING_SPACE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::MASKXYZPOSITION | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE | GNETagProperties::REPARENT | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::PARKINGSPACE, {SUMO_TAG_PARKING_AREA});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::POSITION | GNEAttributeProperties::UPDATEGEOMETRY, // virtual attribute from the combination of the actually attributes SUMO_ATTR_X, SUMO_ATTR_Y
                                              "The x-y-z position of the parking vehicle on the plane");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The width of the road-side parking spaces",
                                              "3.20");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The length of the road-side parking spaces",
                                              "5.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ANGLE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::ANGLE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The angle of the road-side parking spaces relative to the lane angle, positive means clockwise",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_E1DETECTOR;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::E1);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of E1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the lane the detector shall be laid on. The lane must be a part of the network used");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane the detector shall be laid on in meters. The position must be a value between -1*lane's length and the lane's length");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The aggregation period the values the detector collects shall be summed up",
                                              "900.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The path to the output file");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Space separated list of vehicle type ids to consider");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_E2DETECTOR;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::E2);
        // set "file" as deprecated attribute
        myTagProperties[currentTag].addDeprecatedAttribute(SUMO_ATTR_CONT);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of E2");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the lane the detector shall be laid on. The lane must be a part of the network used");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane the detector shall be laid on in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The length of the detector in meters",
                                              "10.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The aggregation period the values the detector collects shall be summed up");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The traffic light that triggers aggregation when switching");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The path to the output file");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Space separated list of vehicle type ids to consider");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_TIME_THRESHOLD,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting)",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_SPEED_THRESHOLD,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting) in m/s",
                                              "1.39");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_JAM_DIST_THRESHOLD,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The minimum distance to the next standing vehicle in order to make this vehicle count as a participant to the jam) in m",
                                              "10.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_E2DETECTOR_MULTILANE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SYNONYM | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::E2, {}, SUMO_TAG_E2DETECTOR);
        // set "file" as deprecated attribute
        myTagProperties[currentTag].addDeprecatedAttribute(SUMO_ATTR_CONT);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of Multilane E2");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::SECUENCIAL | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The list of secuencial lane ids in which the detector shall be laid on");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane the detector shall be laid on in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The end position on the lane the detector shall be laid on in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The aggregation period the values the detector collects shall be summed up");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TLID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The traffic light that triggers aggregation when switching");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The path to the output file");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Space separated list of vehicle type ids to consider");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_TIME_THRESHOLD,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting)",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_SPEED_THRESHOLD,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting) in m/s",
                                              "1.39");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_JAM_DIST_THRESHOLD,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The minimum distance to the next standing vehicle in order to make this vehicle count as a participant to the jam) in m",
                                              "10.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_E3DETECTOR;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::MINIMUMCHILDREN,
                                      GUIIcon::E3);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of E3");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "X-Y position of detector in editor (Only used in NETEDIT)",
                                              "0,0"); // virtual attribute from the combination of the actually attributes SUMO_ATTR_X, SUMO_ATTR_Y
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The aggregation period the values the detector collects shall be summed up",
                                              "900.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The path to the output file");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Space separated list of vehicle type ids to consider");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_TIME_THRESHOLD,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting) in s",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_SPEED_THRESHOLD,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting) in m/s",
                                              "1.39");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_DET_ENTRY;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE | GNETagProperties::REPARENT | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::E3ENTRY, {SUMO_TAG_E3DETECTOR});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the lane the detector shall be laid on. The lane must be a part of the network used");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane the detector shall be laid on in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_DET_EXIT;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE | GNETagProperties::REPARENT | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::E3EXIT, {SUMO_TAG_E3DETECTOR});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the lane the detector shall be laid on. The lane must be a part of the network used");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane the detector shall be laid on in meters");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_INSTANT_INDUCTION_LOOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::DETECTOR,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::E1INSTANT);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of Instant Induction Loop (E1Instant)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the lane the detector shall be laid on. The lane must be a part of the network used");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the lane the detector shall be laid on in meters. The position must be a value between -1*lane's length and the lane's length");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The path to the output file");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Space separated list of vehicle type ids to consider");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_VSS;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::DIALOG,
                                      GUIIcon::VARIABLESPEEDSIGN);
        // set "file" as deprecated attribute
        myTagProperties[currentTag].addDeprecatedAttribute(SUMO_ATTR_FILE);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The id of Variable Speed Signal");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "X-Y position of detector in editor (Only used in NETEDIT)",
                                              "0,0"); // virtual attribute from the combination of the actually attributes SUMO_ATTR_X, SUMO_ATTR_Y
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "list of lanes of Variable Speed Sign");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_VSS_SYMBOL;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::SYMBOL,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE,
                                      GUIIcon::VARIABLESPEEDSIGN, {SUMO_TAG_VSS});
    }
    currentTag = SUMO_TAG_STEP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::VSSSTEP, {SUMO_TAG_VSS});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_TIME,
                                              GNEAttributeProperties::SUMOTIME,
                                              "Time");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEED,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Speed",
                                              "13.89");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_CALIBRATOR;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::DIALOG | GNETagProperties::CENTERAFTERCREATION,
                                      GUIIcon::CALIBRATOR);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of Calibrator");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of edge in the simulation network");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position of the calibrator on the specified lane",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The aggregation interval in which to calibrate the flows. Default is step-length",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTEPROBE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The id of the routeProbe element from which to determine the route distribution for generated vehicles");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_OUTPUT,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The output file for writing calibrator information or NULL");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_LANECALIBRATOR;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SYNONYM | GNETagProperties::DIALOG | GNETagProperties::CENTERAFTERCREATION,
                                      GUIIcon::CALIBRATOR, {}, SUMO_TAG_CALIBRATOR);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of Calibrator");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of lane in the simulation network");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position of the calibrator on the specified lane",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The aggregation interval in which to calibrate the flows. Default is step-length",
                                              "100.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTEPROBE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The id of the routeProbe element from which to determine the route distribution for generated vehicles");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_OUTPUT,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The output file for writing calibrator information or NULL");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_FLOW_CALIBRATOR;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::FLOW, {SUMO_TAG_CALIBRATOR});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the vehicle type to use for this " + toString(currentTag),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the route the vehicle shall drive along");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common vehicle attributes
        fillCommonVehicleAttributes(currentTag);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_BEGIN,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "First " + toString(currentTag) + " departure time",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_END,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "End of departure interval",
                                              "3600.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VEHSPERHOUR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::ACTIVATABLE,
                                              "Number of " + toString(currentTag) + "s per hour, equally spaced");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEED,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::ACTIVATABLE,
                                              "Speed of " + toString(currentTag) + "s");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_REROUTER;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::DIALOG | GNETagProperties::WRITECHILDRENSEPARATE,
                                      GUIIcon::REROUTER);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of Rerouter");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "An edge id or a list of edge ids where vehicles shall be rerouted");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "X,Y position in editor (Only used in NETEDIT)",
                                              "0,0"); // virtual attribute from the combination of the actually attributes SUMO_ATTR_X, SUMO_ATTR_Y
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The path to the definition file (alternatively, the intervals may defined as children of the rerouter)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PROB,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::PROBABILITY | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The probability for vehicle rerouting (0-1)",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HALTING_TIME_THRESHOLD,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The waiting time threshold (in s) that must be reached to activate rerouting (default -1 which disables the threshold)",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The list of vehicle types that shall be affected by this rerouter (empty to affect all types)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_OFF,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Whether the router should be inactive initially (and switched on in the gui)",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_REROUTER_SYMBOL;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT | GNETagProperties::SYMBOL,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE,
                                      GUIIcon::REROUTER, {GNE_TAG_REROUTER_SYMBOL});
    }
    currentTag = SUMO_TAG_INTERVAL;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::REROUTERINTERVAL, {SUMO_TAG_REROUTER});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_BEGIN,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Begin",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_END,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "End",
                                              "3600.00");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_CLOSING_REROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::CLOSINGREROUTE, {SUMO_TAG_INTERVAL});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Edge ID");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES | GNEAttributeProperties::XMLOPTIONAL,
                                              "allowed vehicles");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DISALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES | GNEAttributeProperties::XMLOPTIONAL,
                                              "disallowed vehicles");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_CLOSING_LANE_REROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::CLOSINGLANEREROUTE, {SUMO_TAG_INTERVAL});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Lane ID");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES | GNEAttributeProperties::XMLOPTIONAL,
                                              "allowed vehicles");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DISALLOW,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::VCLASSES | GNEAttributeProperties::XMLOPTIONAL,
                                              "disallowed vehicles");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_DEST_PROB_REROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::DESTPROBREROUTE, {SUMO_TAG_INTERVAL});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Edge ID");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PROB,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "SUMO Probability",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_PARKING_ZONE_REROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::PARKINGZONEREROUTE, {SUMO_TAG_INTERVAL});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_PARKING,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM,
                                              "ParkingArea ID");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PROB,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "SUMO Probability",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VISIBLE,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Enable or disable visibility for parking area reroutes",
                                              "1");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_ROUTE_PROB_REROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::ROUTEPROBREROUTE, {SUMO_TAG_INTERVAL});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Route");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PROB,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "SUMO Probability",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_ROUTEPROBE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::CENTERAFTERCREATION,
                                      GUIIcon::ROUTEPROBE);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of RouteProbe");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of an edge in the simulation network");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FREQUENCY,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The frequency in which to report the distribution",
                                              "3600");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The file for generated output");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_BEGIN,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The time at which to start generating output",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_VAPORIZER;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::ADDITIONALELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::CENTERAFTERCREATION,
                                      GUIIcon::VAPORIZER);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "Edge in which vaporizer is placed");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_BEGIN,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Start Time",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_END,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "End Time",
                                              "3600.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_NAME,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Name of " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
}


void
GNEAttributeCarrier::fillShapes() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill shape ACs
    SumoXMLTag currentTag = SUMO_TAG_POLY;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::SHAPE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::BLOCKSHAPE | GNETagProperties::CLOSESHAPE | GNETagProperties::GEOSHAPE,
                                      GUIIcon::LOCATEPOLY /* temporal */);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the polygon");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE,
                                              "The shape of the polygon");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The RGBA color with which the polygon shall be displayed",
                                              "red");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILL,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "An information whether the polygon shall be filled",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LINEWIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The default line width for drawing an unfilled polygon",
                                              "1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LAYER,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The layer in which the polygon lies",
                                              toString(Shape::DEFAULT_LAYER));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "A typename for the polygon",
                                              toString(Shape::DEFAULT_TYPE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_IMGFILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "A bitmap to use for rendering this polygon",
                                              toString(Shape::DEFAULT_IMG_FILE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_RELATIVEPATH,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Enable or disable use image file as a relative path",
                                              toString(Shape::DEFAULT_RELATIVEPATH));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ANGLE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::ANGLE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Angle of rendered image in degree",
                                              toString(Shape::DEFAULT_ANGLE));
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_POI;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::SHAPE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::MASKXYZPOSITION | GNETagProperties::GEOPOSITION,
                                      GUIIcon::LOCATEPOI /* temporal */);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY, // virtual attribute from the combination of the actually attributes SUMO_ATTR_X, SUMO_ATTR_Y
                                              "The position in view");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The color with which the " + toString(currentTag) + " shall be displayed",
                                              "red");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "A typename for the " + toString(currentTag),
                                              toString(Shape::DEFAULT_TYPE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LAYER,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The layer of the " + toString(currentTag) + " for drawing and selecting",
                                              toString(Shape::DEFAULT_LAYER_POI));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Width of rendered image in meters",
                                              toString(Shape::DEFAULT_IMG_WIDTH));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HEIGHT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Height of rendered image in meters",
                                              toString(Shape::DEFAULT_IMG_HEIGHT));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_IMGFILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "A bitmap to use for rendering this " + toString(currentTag),
                                              toString(Shape::DEFAULT_IMG_FILE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_RELATIVEPATH,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Enable or disable use image file as a relative path",
                                              toString(Shape::DEFAULT_RELATIVEPATH));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ANGLE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::ANGLE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Angle of rendered image in degree",
                                              toString(Shape::DEFAULT_ANGLE));
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_POILANE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::SHAPE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT,
                                      GUIIcon::LOCATEPOI /* temporal */);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of the lane at which the " + toString(currentTag) + " is located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The position on the named lane or in the net in meters at which the " + toString(currentTag) + " is located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_POSITION_LAT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The lateral offset on the named lane at which the " + toString(currentTag) + " is located at",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The color with which the " + toString(currentTag) + " shall be displayed",
                                              "red");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "A typename for the " + toString(currentTag),
                                              toString(Shape::DEFAULT_TYPE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LAYER,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The layer of the " + toString(currentTag) + " for drawing and selecting",
                                              toString(Shape::DEFAULT_LAYER_POI));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Width of rendered image in meters",
                                              toString(Shape::DEFAULT_IMG_WIDTH));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HEIGHT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Height of rendered image in meters",
                                              toString(Shape::DEFAULT_IMG_HEIGHT));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_IMGFILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "A bitmap to use for rendering this " + toString(currentTag),
                                              toString(Shape::DEFAULT_IMG_FILE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_RELATIVEPATH,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Enable or disable use image file as a relative path",
                                              toString(Shape::DEFAULT_RELATIVEPATH));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ANGLE,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::ANGLE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Angle of rendered image in degree",
                                              toString(Shape::DEFAULT_ANGLE));
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
}


void
GNEAttributeCarrier::fillTAZElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill TAZ ACs
    SumoXMLTag currentTag = SUMO_TAG_TAZ;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::TAZELEMENT,
                                      GNETagProperties::DRAWABLE | GNETagProperties::RTREE | GNETagProperties::SELECTABLE | GNETagProperties::BLOCKMOVEMENT | GNETagProperties::BLOCKSHAPE,
                                      GUIIcon::TAZ);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of the TAZ");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::POSITION | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The shape of the TAZ");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "The RGBA color with which the TAZ shall be displayed",
                                              "red");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FILL,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "An information whether the TAZ shall be filled (Only in NETEDIT)",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_TAZSOURCE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::TAZELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::TAZEDGE, {SUMO_TAG_TAZ});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of edge in the simulation network");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WEIGHT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Depart weight associated to this Edge",
                                              "1");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_TAZSINK;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::TAZELEMENT,
                                      GNETagProperties::SLAVE,
                                      GUIIcon::TAZEDGE, {SUMO_TAG_TAZ});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::SYNONYM | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of edge in the simulation network");
        attrProperty.setSynonym(SUMO_ATTR_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WEIGHT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "Arrival weight associated to this Edget",
                                              "1");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
}


void
GNEAttributeCarrier::fillDemandElements() {
    // first VClass separate between vehicles and persons
    std::vector<std::string> vClassesVehicles, vClassesPersons;
    auto vClasses = SumoVehicleClassStrings.getStrings();
    for (const auto& i : vClasses) {
        if (i == SumoVehicleClassStrings.getString(SVC_PEDESTRIAN)) {
            vClassesPersons.push_back(i);
        } else {
            vClassesVehicles.push_back(i);
        }
    }
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;

    // fill demand elements
    SumoXMLTag currentTag = SUMO_TAG_ROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::ROUTE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::ROUTE);

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of Route");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The edges the vehicle shall drive along, given as their ids, separated using spaces");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "This route's color",
                                              "yellow");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_ROUTE_EMBEDDED;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::ROUTE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE,
                                      GUIIcon::ROUTE, {GNE_TAG_VEHICLE_WITHROUTE, GNE_TAG_FLOW_WITHROUTE});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The edges the vehicle shall drive along, given as their ids, separated using spaces");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "This route's color",
                                              "yellow");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_VTYPE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VTYPE, 0,
                                      GUIIcon::VTYPE);

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of VehicleType");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VCLASS,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "An abstract vehicle class",
                                              "passenger");
        attrProperty.setDiscreteValues(vClassesVehicles);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "This vehicle type's color",
                                              "");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "The vehicle's netto-length (length) [m]");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_MINGAP,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "Empty space after leader [m]");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_MAXSPEED,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "The vehicle's maximum velocity [m/s]");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEEDFACTOR,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The vehicles expected multiplicator for lane speed limits");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_SPEEDDEV,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The deviation of the speedFactor");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_EMISSIONCLASS,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "An abstract emission class");
        attrProperty.setDiscreteValues(PollutantsInterface::getAllClassesStr());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_GUISHAPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "How this vehicle is rendered");
        attrProperty.setDiscreteValues(SumoVehicleShapeStrings.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The vehicle's width [m] (only used for drawing)",
                                              "1.8");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_HEIGHT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The vehicle's height [m] (only used for drawing)",
                                              "1.5");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_IMGFILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "Image file for rendering vehicles of this type (should be grayscale to allow functional coloring)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE_CHANGE_MODEL,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The model used for changing lanes",
                                              "default");
        attrProperty.setDiscreteValues(SUMOXMLDefinitions::LaneChangeModels.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CAR_FOLLOW_MODEL,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The model used for car following",
                                              "Krauss");
        attrProperty.setDiscreteValues(SUMOXMLDefinitions::CarFollowModels.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PERSON_CAPACITY,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The number of persons (excluding an autonomous driver) the vehicle can transport");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CONTAINER_CAPACITY,
                                              GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The number of containers the vehicle can transport");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_BOARDING_DURATION,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The time required by a person to board the vehicle",
                                              "0.50");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LOADING_DURATION,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The time required to load a container onto the vehicle",
                                              "90.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LATALIGNMENT,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The preferred lateral alignment when using the sublane-model",
                                              "center");
        attrProperty.setDiscreteValues(SUMOXMLDefinitions::LateralAlignments.getStrings());
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_MINGAP_LAT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The minimum lateral gap at a speed difference of 50km/h when using the sublane-model",
                                              "0.12");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_MAXSPEED_LAT,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The maximum lateral speed when using the sublane-model",
                                              "1.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ACTIONSTEPLENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The interval length for which vehicle performs its decision logic (acceleration and lane-changing)",
                                              toString(OptionsCont::getOptions().getFloat("default.action-step-length")));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_PROB,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "The probability when being added to a distribution without an explicit probability",
                                              toString(DEFAULT_VEH_PROB));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_OSGFILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "3D model file for this class",
                                              "");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CARRIAGE_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "Carriage lengths");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LOCOMOTIVE_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "Locomotive lengths");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_CARRIAGE_GAP,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                              "GAP between carriages",
                                              "1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill VType Car Following Model Values (implemented in a separated function to improve code legibility)
        fillCarFollowingModelAttributes(currentTag);

        // fill VType Junction Model Parameters (implemented in a separated function to improve code legibility)
        fillJunctionModelAttributes(currentTag);

        // fill VType Lane Change Model Parameters (implemented in a separated function to improve code legibility)
        fillLaneChangingModelAttributes(currentTag);
    }
    currentTag = SUMO_TAG_PTYPE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VTYPE,
                                      GNETagProperties::SYNONYM,
                                      GUIIcon::PTYPE, {}, SUMO_TAG_VTYPE);

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The id of PersonType");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VCLASS,
                                              GNEAttributeProperties::VCLASS | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "An abstract person class",
                                              "pedestrian");
        attrProperty.setDiscreteValues(vClassesPersons);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "This person type's color",
                                              "");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_WIDTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "The person's width [m] (only used for drawing)");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_LENGTH,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "The person's netto-length (length) [m]");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_MINGAP,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "Empty space after leader [m]");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_MAXSPEED,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUEMUTABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "The person's maximum velocity [m/s]");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "This value causes persons to violate a red light if the duration of the red phase is lower than the given threshold.",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_IMGFILE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::FILENAME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Image file for rendering persons of this type (should be grayscale to allow functional coloring)");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
}


void
GNEAttributeCarrier::fillVehicleElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill vehicle ACs
    SumoXMLTag currentTag = SUMO_TAG_VEHICLE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::VEHICLE);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the vehicle type to use for this " + toString(currentTag),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the route the " + toString(currentTag) + " shall drive along");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // add common attributes
        fillCommonVehicleAttributes(currentTag);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPART,
                                              GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The time step at which the " + toString(currentTag) + " shall enter the network",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_VEHICLE_WITHROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SYNONYM | GNETagProperties::EMBEDDED_ROUTE,
                                      GUIIcon::VEHICLE, {}, SUMO_TAG_VEHICLE);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of the " + toString(SUMO_TAG_VEHICLE));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the vehicle type to use for this " + toString(SUMO_TAG_VEHICLE),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        // add common attributes
        fillCommonVehicleAttributes(currentTag);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPART,
                                              GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The time step at which the " + toString(SUMO_TAG_VEHICLE) + " shall enter the network",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_FLOW_ROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SYNONYM,
                                      GUIIcon::ROUTEFLOW, {}, SUMO_TAG_FLOW);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the " + toString(currentTag) + " type to use for this " + toString(currentTag),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the route the " + toString(currentTag) + " shall drive along");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // add common attributes
        fillCommonVehicleAttributes(currentTag);

        // add flow attributes
        fillCommonFlowAttributes(currentTag, true);
    }
    currentTag = GNE_TAG_FLOW_WITHROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::SYNONYM | GNETagProperties::EMBEDDED_ROUTE,
                                      GUIIcon::ROUTEFLOW, {}, SUMO_TAG_FLOW);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of the " + toString(SUMO_TAG_FLOW));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the " + toString(currentTag) + " type to use for this " + toString(SUMO_TAG_FLOW),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        // add common attributes
        fillCommonVehicleAttributes(currentTag);

        // add flow attributes
        fillCommonFlowAttributes(currentTag, true);
    }
    currentTag = SUMO_TAG_TRIP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::TRIP);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of " + toString(currentTag) + "s that will be generated using this trip definition");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the " + toString(currentTag) + " type to use for this " + toString(currentTag),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the " + toString(currentTag) + " starts at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the " + toString(currentTag) + " ends at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VIA,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "List of intermediate edge ids which shall be part of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        // add common attributes
        fillCommonVehicleAttributes(currentTag);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPART,
                                              GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The departure time of the (first) " + toString(currentTag) + " which is generated using this " + toString(currentTag) + " definition",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_FLOW;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::FLOW);
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "The name of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the " + toString(currentTag) + " type to use for this " + toString(currentTag),
                                              DEFAULT_VTYPE_ID);
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the " + toString(currentTag) + " starts at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the " + toString(currentTag) + " ends at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_VIA,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "List of intermediate edge ids which shall be part of the " + toString(currentTag));
        myTagProperties[currentTag].addAttribute(attrProperty);

        // add common attributes
        fillCommonVehicleAttributes(currentTag);

        // add flow attributes
        fillCommonFlowAttributes(currentTag, true);
    }
    /* currently disabled. See #5259
    currentTag = SUMO_TAG_TRIP_TAZ;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
            GNETagProperties::DEMANDELEMENT | GNETagProperties::VEHICLE,
            GNETagProperties::DRAWABLE,
            GUIIcon::TRIP);
    }
    */
}


void
GNEAttributeCarrier::fillStopElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill stops ACs
    SumoXMLTag currentTag = SUMO_TAG_STOP_LANE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::STOP,
                                      GNETagProperties::SLAVE | GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::MASKSTARTENDPOS | GNETagProperties::NOPARAMETERS,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_ROUTE, SUMO_TAG_TRIP, SUMO_TAG_FLOW});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_LANE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the lane the stop shall be located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_STARTPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::ACTIVATABLE |  GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The begin position on the lane (the lower position on the lane) in meters",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_ENDPOS,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::ACTIVATABLE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The end position on the lane (the higher position on the lane) in meters, must be larger than startPos by more than 0.1m",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_FRIENDLY_POS,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "If set, no error will be reported if element is placed behind the lane. Instead,it will be placed 0.1 meters from the lanes end or at position 0.1, if the position was negative and larger than the lanes length after multiplication with - 1",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes
        fillCommonStopAttributes(currentTag, true);
    }
    currentTag = SUMO_TAG_STOP_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::STOP,
                                      GNETagProperties::SLAVE | GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::NOPARAMETERS,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_ROUTE, SUMO_TAG_TRIP, SUMO_TAG_FLOW});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_BUS_STOP,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "BusStop associated with this stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes
        fillCommonStopAttributes(currentTag, true);
    }
    currentTag = SUMO_TAG_STOP_CONTAINERSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::STOP,
                                      GNETagProperties::SLAVE | GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::NOPARAMETERS,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_ROUTE, SUMO_TAG_TRIP, SUMO_TAG_FLOW});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_CONTAINER_STOP,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "ContainerStop associated with this stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes
        fillCommonStopAttributes(currentTag, true);
    }
    currentTag = SUMO_TAG_STOP_CHARGINGSTATION;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::STOP,
                                      GNETagProperties::SLAVE | GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::NOPARAMETERS,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_ROUTE, SUMO_TAG_TRIP, SUMO_TAG_FLOW});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_CHARGING_STATION,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "ChargingStation associated with this stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes
        fillCommonStopAttributes(currentTag, true);
    }
    currentTag = SUMO_TAG_STOP_PARKINGAREA;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::STOP,
                                      GNETagProperties::SLAVE | GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE | GNETagProperties::NOPARAMETERS,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_ROUTE, SUMO_TAG_TRIP, SUMO_TAG_FLOW});
        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_PARKING_AREA,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "ParkingArea associated with this stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes (no parking)
        fillCommonStopAttributes(currentTag, false);
    }
}


void
GNEAttributeCarrier::fillPersonElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill vehicle ACs
    SumoXMLTag currentTag = SUMO_TAG_PERSON;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSON,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::PERSON);

        // add flow attributes
        fillCommonPersonAttributes(currentTag);

        // set specific attribute depart (note: Persons doesn't support triggered and containerTriggered values)
        attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPART,
                                              GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "The time step at which the " + toString(currentTag) + " shall enter the network",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    currentTag = SUMO_TAG_PERSONFLOW;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSON,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SELECTABLE,
                                      GUIIcon::PERSONFLOW);

        // add flow attributes
        fillCommonPersonAttributes(currentTag);

        // add flow attributes
        fillCommonFlowAttributes(currentTag, false);
    }
}


void
GNEAttributeCarrier::fillPersonPlanTrips() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill person trips
    SumoXMLTag currentTag = GNE_TAG_PERSONTRIP_EDGE_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::PERSONTRIP_FROMTO, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeEdge(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_EDGE_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeBusStop(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_EDGE_STOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_STOP,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeStop(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_BUSSTOP_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopEdge(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopBusStop(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_BUSSTOP_STOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_STOP,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopStop(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_STOP_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_STOP | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanStopEdge(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_STOP_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_STOP | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanStopBusStop(currentTag);
        fillPersonTripAttributes(currentTag);
    }
    currentTag = GNE_TAG_PERSONTRIP_STOP_STOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::PERSONTRIP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_STOP | GNETagProperties::PERSONPLAN_END_STOP,
                                      GUIIcon::PERSONTRIP_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanStopStop(currentTag);
        fillPersonTripAttributes(currentTag);
    }
}


void
GNEAttributeCarrier::fillPersonPlanWalks() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill walks
    SumoXMLTag currentTag = GNE_TAG_WALK_EDGES;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::WALK,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::WALK_EDGES, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // edges
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGES,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "id of the edges to walk");
        myTagProperties[currentTag].addAttribute(attrProperty);
        // arrival pos
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Arrival position on the destination edge",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_WALK_ROUTE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::WALK,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::WALK_ROUTE, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // route
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ROUTE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The id of the route to walk");
        myTagProperties[currentTag].addAttribute(attrProperty);
        // arrival pos
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                              "Arrival position on the destination edge",
                                              "-1");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = GNE_TAG_WALK_EDGE_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::WALK,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::WALK_FROMTO, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeEdge(currentTag);
    }
    currentTag = GNE_TAG_WALK_EDGE_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::WALK,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::WALK_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeBusStop(currentTag);
    }
    currentTag = GNE_TAG_WALK_BUSSTOP_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::WALK,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::WALK_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopEdge(currentTag);
    }
    currentTag = GNE_TAG_WALK_BUSSTOP_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::WALK,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::WALK_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopBusStop(currentTag);
    }
}


void
GNEAttributeCarrier::fillPersonPlanRides() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill rides
    SumoXMLTag currentTag = GNE_TAG_RIDE_EDGE_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::RIDE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::RIDE_FROMTO, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeEdge(currentTag);
        fillRideAttributes(currentTag);
    }
    currentTag = GNE_TAG_RIDE_EDGE_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::RIDE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::RIDE_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanEdgeBusStop(currentTag);
        fillRideAttributes(currentTag);
    }
    currentTag = GNE_TAG_RIDE_BUSSTOP_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::RIDE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::RIDE_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopEdge(currentTag);
        fillRideAttributes(currentTag);
    }
    currentTag = GNE_TAG_RIDE_BUSSTOP_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONPLAN | GNETagProperties::RIDE,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::RIDE_BUSSTOP, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});
        // fill attributes
        fillPersonPlanBusStopBusStop(currentTag);
        fillRideAttributes(currentTag);
    }
}


void
GNEAttributeCarrier::fillPersonPlanEdgeEdge(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from edge
    attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The name of the edge the " + toString(currentTag) + " starts at");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to edge
    attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The name of the edge the " + toString(currentTag) + " ends at");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // arrival position
    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "arrival position on the destination edge",
                                          "-1");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanEdgeBusStop(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from edge
    attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The name of the edge the " + toString(currentTag) + " starts at");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_TO_BUSSTOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Id of the destination " + toString(SUMO_TAG_BUS_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanEdgeStop(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from edge
    attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The name of the edge the " + toString(currentTag) + " starts at");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to stop
    attrProperty = GNEAttributeProperties(GNE_ATTR_TO_STOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Destination " + toString(SUMO_TAG_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanBusStopEdge(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_FROM_BUSSTOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Id of the origin " + toString(SUMO_TAG_BUS_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to edge
    attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The name of the edge the " + toString(currentTag) + " ends at");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // arrival position
    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "arrival position on the destination edge",
                                          "-1");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanBusStopBusStop(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_FROM_BUSSTOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Id of the origin " + toString(SUMO_TAG_BUS_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_TO_BUSSTOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Id of the destination " + toString(SUMO_TAG_BUS_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanBusStopStop(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_FROM_BUSSTOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Id of the origin " + toString(SUMO_TAG_BUS_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to stop
    attrProperty = GNEAttributeProperties(GNE_ATTR_TO_STOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Destination " + toString(SUMO_TAG_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanStopEdge(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_FROM_STOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Origin " + toString(SUMO_TAG_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to edge
    attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The name of the edge the " + toString(currentTag) + " ends at");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // arrival position
    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "arrival position on the destination edge",
                                          "-1");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanStopBusStop(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_FROM_STOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Origin " + toString(SUMO_TAG_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_TO_BUSSTOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Id of the destination " + toString(SUMO_TAG_BUS_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonPlanStopStop(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // from busStop
    attrProperty = GNEAttributeProperties(GNE_ATTR_FROM_STOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Origin " + toString(SUMO_TAG_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
    // to stop
    attrProperty = GNEAttributeProperties(GNE_ATTR_TO_STOP,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "Destination " + toString(SUMO_TAG_STOP));
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonTripAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // vTypes
    attrProperty = GNEAttributeProperties(SUMO_ATTR_VTYPES,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "List of possible vehicle types to take");
    myTagProperties[currentTag].addAttribute(attrProperty);
    // modes
    attrProperty = GNEAttributeProperties(SUMO_ATTR_MODES,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "List of possible traffic modes. Walking is always possible regardless of this value");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillRideAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // lines
    attrProperty = GNEAttributeProperties(SUMO_ATTR_LINES,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "list of vehicle alternatives to take for the " + toString(currentTag),
                                          "ANY");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillPersonStopElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill vehicle ACs
    SumoXMLTag currentTag = GNE_TAG_PERSONSTOP_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONSTOP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::NOPARAMETERS |
                                      GNETagProperties::PERSONPLAN_START_EDGE | GNETagProperties::PERSONPLAN_END_EDGE,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_EDGE,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the stop shall be located at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes
        fillCommonStopAttributes(currentTag, true);
    }
    currentTag = GNE_TAG_PERSONSTOP_BUSSTOP;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DEMANDELEMENT | GNETagProperties::PERSONSTOP,
                                      GNETagProperties::DRAWABLE | GNETagProperties::SLAVE | GNETagProperties::SELECTABLE | GNETagProperties::NOPARAMETERS |
                                      GNETagProperties::PERSONPLAN_START_BUSSTOP | GNETagProperties::PERSONPLAN_END_BUSSTOP,
                                      GUIIcon::STOPELEMENT, {SUMO_TAG_PERSON, SUMO_TAG_PERSONFLOW});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_BUS_STOP,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "BusStop associated with this stop");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // fill common stop attributes
        fillCommonStopAttributes(currentTag, true);
    }
}


void
GNEAttributeCarrier::fillCommonVehicleAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "This " + toString(currentTag) + "'s color",
                                          "yellow");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPARTLANE,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The lane on which the " + toString(currentTag) + " shall be inserted",
                                          "first");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPARTPOS,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY /* GNEAttributeProperties::MULTIDISCRETE (Currently disabled) */,
                                          "The position at which the " + toString(currentTag) + " shall enter the net",
                                          "base");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPARTSPEED,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL /* GNEAttributeProperties::MULTIDISCRETE (Currently disabled) */,
                                          "The speed with which the " + toString(currentTag) + " shall enter the network",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALLANE,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY /* GNEAttributeProperties::MULTIDISCRETE (Currently disabled) */,
                                          "The lane at which the " + toString(currentTag) + " shall leave the network",
                                          "current");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY /* GNEAttributeProperties::MULTIDISCRETE (Currently disabled) */,
                                          "The position at which the " + toString(currentTag) + " shall leave the network",
                                          "max");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALSPEED,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL /* GNEAttributeProperties::MULTIDISCRETE (Currently disabled) */,
                                          "The speed with which the " + toString(currentTag) + " shall leave the network",
                                          "current");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LINE,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "A string specifying the id of a public transport line which can be used when specifying person rides");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_PERSON_NUMBER,
                                          GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "The number of occupied seats when the " + toString(currentTag) + " is inserted",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CONTAINER_NUMBER,
                                          GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "The number of occupied container places when the " + toString(currentTag) + " is inserted",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPARTPOS_LAT,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The lateral position on the departure lane at which the " + toString(currentTag) + " shall enter the net",
                                          "center");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ARRIVALPOS_LAT,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The lateral position on the arrival lane at which the " + toString(currentTag) + " shall arrive",
                                          "center");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillCommonFlowAttributes(SumoXMLTag currentTag, const bool forVehicles) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;

    attrProperty = GNEAttributeProperties(SUMO_ATTR_BEGIN,
                                          GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "First " + toString(currentTag) + " departure time",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_END,
                                          GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::FLOWDEFINITION,
                                          "End of departure interval",
                                          "3600.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_NUMBER,
                                          GNEAttributeProperties::INT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::FLOWDEFINITION,
                                          "probability for emitting a " + toString(currentTag) + " each second (not together with vehsPerHour or period)",
                                          "1800");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(forVehicles ? SUMO_ATTR_VEHSPERHOUR : SUMO_ATTR_PERSONSPERHOUR,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::FLOWDEFINITION,
                                          "Number of " + toString(currentTag) + "s per hour, equally spaced (not together with period or probability)",
                                          "1800");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_PERIOD,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::FLOWDEFINITION,
                                          "Insert equally spaced " + toString(currentTag) + "s at that period (not together with vehsPerHour or probability)",
                                          "2");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_PROB,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::FLOWDEFINITION,
                                          "probability for emitting a " + toString(currentTag) + " each second (not together with vehsPerHour or period)",
                                          "0.5");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillCarFollowingModelAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ACCEL,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "The acceleration ability of vehicles of this type [m/s^2]",
                                          "2.60");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DECEL,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "The deceleration ability of vehicles of this type [m/s^2]",
                                          "4.50");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_APPARENTDECEL,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The apparent deceleration of the vehicle as used by the standard model [m/s^2]",
                                          "4.50");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_EMERGENCYDECEL,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The maximal physically possible deceleration for the vehicle [m/s^2]",
                                          "4.50");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_SIGMA,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::RANGE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "Car-following model parameter",
                                          "0.50");
    attrProperty.setRange(0, 1);
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TAU,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "Car-following model parameter",
                                          "1.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TMP1,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "SKRAUSSX parameter 1",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TMP2,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "SKRAUSSX parameter 2",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TMP3,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "SKRAUSSX parameter 3",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TMP4,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "SKRAUSSX parameter 4",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TMP5,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "SKRAUSSX parameter 5",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_PWAGNER2009_TAULAST,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Peter Wagner 2009 parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_PWAGNER2009_APPROB,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Peter Wagner 2009 parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_IDMM_ADAPT_FACTOR,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "IDMM parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_IDMM_ADAPT_TIME,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "IDMM parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_WIEDEMANN_SECURITY,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Wiedemann parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_WIEDEMANN_ESTIMATION,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Wiedemann parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_COLLISION_MINGAP_FACTOR,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "MinGap factor parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_K,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "K parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);


    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_KERNER_PHI,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Kerner Phi parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_IDM_DELTA,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "IDM Delta parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CF_IDM_STEPPING,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "IDM Stepping parameter",
                                          "");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TRAIN_TYPE,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DISCRETE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Train Types",
                                          "NGT400");
    attrProperty.setDiscreteValues(SUMOXMLDefinitions::TrainTypes.getStrings());
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillJunctionModelAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_CROSSING_GAP,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Minimum distance to pedestrians that are walking towards the conflict point with the ego vehicle.",
                                          "10");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_IGNORE_KEEPCLEAR_TIME,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The accumulated waiting time after which a vehicle will drive onto an intersection even though this might cause jamming.",
                                          "-1");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_DRIVE_AFTER_YELLOW_TIME,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value causes vehicles to violate a yellow light if the duration of the yellow phase is lower than the given threshold.",
                                          "-1");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value causes vehicles to violate a red light if the duration of the red phase is lower than the given threshold.",
                                          "-1");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_DRIVE_RED_SPEED,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value causes vehicles affected by jmDriveAfterRedTime to slow down when violating a red light.",
                                          "0.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_IGNORE_FOE_PROB,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value causes vehicles to ignore foe vehicles that have right-of-way with the given probability.",
                                          "0.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_IGNORE_FOE_SPEED,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value is used in conjunction with jmIgnoreFoeProb. Only vehicles with a speed below or equal to the given value may be ignored.",
                                          "0.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_SIGMA_MINOR,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value configures driving imperfection (dawdling) while passing a minor link.",
                                          "0.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_JM_TIMEGAP_MINOR,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "This value defines the minimum time gap when passing ahead of a prioritized vehicle. ",
                                          "1");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_IMPATIENCE,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Willingess of drivers to impede vehicles with higher priority",
                                          "0.0");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillLaneChangingModelAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_STRATEGIC_PARAM,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The eagerness for performing strategic lane changing. Higher values result in earlier lane-changing.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_COOPERATIVE_PARAM,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The willingness for performing cooperative lane changing. Lower values result in reduced cooperation.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_SPEEDGAIN_PARAM,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The eagerness for performing lane changing to gain speed. Higher values result in more lane-changing.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_KEEPRIGHT_PARAM,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The eagerness for following the obligation to keep right. Higher values result in earlier lane-changing.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_SUBLANE_PARAM,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The eagerness for using the configured lateral alignment within the lane. Higher values result in increased willingness to sacrifice speed for alignment.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_OPPOSITE_PARAM,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The eagerness for overtaking through the opposite-direction lane. Higher values result in more lane-changing.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_PUSHY,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Willingness to encroach laterally on other drivers.",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_PUSHYGAP,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Minimum lateral gap when encroaching laterally on other drives (alternative way to define lcPushy)",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_ASSERTIVE,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Willingness to accept lower front and rear gaps on the target lane.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_IMPATIENCE,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Dynamic factor for modifying lcAssertive and lcPushy.",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_TIME_TO_IMPATIENCE,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Time to reach maximum impatience (of 1). Impatience grows whenever a lane-change manoeuvre is blocked.",
                                          "infinity");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_ACCEL_LAT,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Maximum lateral acceleration per second.",
                                          "1.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_LOOKAHEADLEFT,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Factor for configuring the strategic lookahead distance when a change to the left is necessary (relative to right lookahead).",
                                          "2.0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_SPEEDGAINRIGHT,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Factor for configuring the treshold asymmetry when changing to the left or to the right for speed gain.",
                                          "0.1");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_MAXSPEEDLATSTANDING,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Upper bound on lateral speed when standing.",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_MAXSPEEDLATFACTOR,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Upper bound on lateral speed while moving computed as lcMaxSpeedLatStanding + lcMaxSpeedLatFactor * getSpeed()",
                                          "1.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "Distance to an upcoming turn on the vehicles route, below which the alignment should be dynamically adapted to match the turn direction.",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_OVERTAKE_RIGHT,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
                                          "The probability for violating rules gainst overtaking on the right.",
                                          "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);

    /*
    attrProperty = GNEAttributeProperties(SUMO_ATTR_LCA_EXPERIMENTAL1,
        GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL | GNEAttributeProperties::EXTENDED,
        "XXXXX",
        "0.00");
    myTagProperties[currentTag].addAttribute(attrProperty);
    */
}


void
GNEAttributeCarrier::fillCommonPersonAttributes(SumoXMLTag currentTag) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                          "The name of the " + toString(currentTag));
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TYPE,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The id of the " + toString(currentTag) + " type to use for this " + toString(currentTag) +
                                          DEFAULT_VTYPE_ID);
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_COLOR,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::COLOR | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "This " + toString(currentTag) + "'s color",
                                          "yellow");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DEPARTPOS,
                                          GNEAttributeProperties::COMPLEX | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL |  GNEAttributeProperties::UPDATEGEOMETRY,
                                          "The position at which the " + toString(currentTag) + " shall enter the net",
                                          "base");
    myTagProperties[currentTag].addAttribute(attrProperty);
}


void
GNEAttributeCarrier::fillCommonStopAttributes(SumoXMLTag currentTag, const bool parking) {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;

    attrProperty = GNEAttributeProperties(SUMO_ATTR_DURATION,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::ACTIVATABLE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "Minimum duration for stopping",
                                          "60");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_UNTIL,
                                          GNEAttributeProperties::FLOAT | GNEAttributeProperties::POSITIVE | GNEAttributeProperties::ACTIVATABLE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "The time step at which the route continues",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_EXTENSION,
                                          GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::ACTIVATABLE | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                          "If set to a non-negative time value, then the stop duration can be extended at most by the extension value in seconds",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_TRIGGERED,
                                          GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "Whether a person may end the stop",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_EXPECTED,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "List of persons that must board the vehicle before it may continue");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_CONTAINER_TRIGGERED,
                                          GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "Whether a container may end the stop",
                                          "0");
    myTagProperties[currentTag].addAttribute(attrProperty);

    attrProperty = GNEAttributeProperties(SUMO_ATTR_EXPECTED_CONTAINERS,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::LIST | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "List of containers that must be loaded onto the vehicle before it may continue");
    myTagProperties[currentTag].addAttribute(attrProperty);

    if (parking) {
        attrProperty = GNEAttributeProperties(SUMO_ATTR_PARKING,
                                              GNEAttributeProperties::BOOL | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::ACTIVATABLE | GNEAttributeProperties::XMLOPTIONAL,
                                              "whether the vehicle stops on the road or beside ",
                                              "0");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }

    attrProperty = GNEAttributeProperties(SUMO_ATTR_ACTTYPE,
                                          GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
                                          "Activity displayed for stopped person in GUI and output files ",
                                          "waiting");
    myTagProperties[currentTag].addAttribute(attrProperty);

    /** Attribute currently disabled. It will be implemented in #6011
    attrProperty = GNEAttributeProperties(SUMO_ATTR_TRIP_ID,
        GNEAttributeProperties::STRING | GNEAttributeProperties::DEFAULTVALUESTATIC | GNEAttributeProperties::XMLOPTIONAL,
        "Value used for trips that uses this stop");
    myTagProperties[currentTag].addAttribute(attrProperty);
    */
}


void
GNEAttributeCarrier::fillDataElements() {
    // declare empty GNEAttributeProperties
    GNEAttributeProperties attrProperty;
    // fill data set element
    SumoXMLTag currentTag = SUMO_TAG_DATASET;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DATAELEMENT,
                                      GNETagProperties::NOPARAMETERS,
                                      GUIIcon::DATASET);

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "Data set ID");
        myTagProperties[currentTag].addAttribute(attrProperty);

    }
    // fill data interval element
    currentTag = SUMO_TAG_DATAINTERVAL;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DATAELEMENT,
                                      GNETagProperties::SLAVE | GNETagProperties::NOPARAMETERS | GNETagProperties::SLAVE,
                                      GUIIcon::DATAINTERVAL, {SUMO_TAG_DATASET});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE,
                                              "Interval ID");
        myTagProperties[currentTag].addAttribute(attrProperty);

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_BEGIN,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "First " + toString(currentTag) + " departure time",
                                              "0.00");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_END,
                                              GNEAttributeProperties::SUMOTIME | GNEAttributeProperties::DEFAULTVALUESTATIC,
                                              "End of departure interval",
                                              "3600.00");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    // fill edge data element
    currentTag = SUMO_TAG_MEANDATA_EDGE;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DATAELEMENT | GNETagProperties::GENERICDATA,
                                      GNETagProperties::DRAWABLE | GNETagProperties::PARAMETERSDOUBLE | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE,
                                      GUIIcon::EDGEDATA, {SUMO_TAG_DATAINTERVAL});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_ID,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "edge ID");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_EDGEREL;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DATAELEMENT | GNETagProperties::GENERICDATA,
                                      GNETagProperties::DRAWABLE | GNETagProperties::PARAMETERSDOUBLE | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE,
                                      GUIIcon::EDGERELDATA, {SUMO_TAG_DATAINTERVAL});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the " + toString(currentTag) + " starts at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the edge the " + toString(currentTag) + " ends at");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
    currentTag = SUMO_TAG_TAZREL;
    {
        // set values of tag
        myTagProperties[currentTag] = GNETagProperties(currentTag,
                                      GNETagProperties::DATAELEMENT | GNETagProperties::GENERICDATA,
                                      GNETagProperties::DRAWABLE | GNETagProperties::PARAMETERSDOUBLE | GNETagProperties::SELECTABLE | GNETagProperties::SLAVE,
                                      GUIIcon::TAZRELDATA, {SUMO_TAG_DATAINTERVAL});

        // set values of attributes
        attrProperty = GNEAttributeProperties(SUMO_ATTR_FROM,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the TAZ the " + toString(currentTag) + " starts at");
        myTagProperties[currentTag].addAttribute(attrProperty);

        attrProperty = GNEAttributeProperties(SUMO_ATTR_TO,
                                              GNEAttributeProperties::STRING | GNEAttributeProperties::UNIQUE | GNEAttributeProperties::UPDATEGEOMETRY,
                                              "The name of the TAZ the " + toString(currentTag) + " ends at");
        myTagProperties[currentTag].addAttribute(attrProperty);
    }
}


bool
GNEAttributeCarrier::checkParsedAttribute(const GNETagProperties& GNETagProperties,
        const GNEAttributeProperties& attrProperties, const SumoXMLAttr attribute,
        std::string& defaultValue, std::string& parsedAttribute, std::string& warningMessage) {
    // declare a string for details about error formats
    std::string errorFormat;
    // set extra check for ID Values
    if (attribute == SUMO_ATTR_ID) {
        if (parsedAttribute.empty()) {
            errorFormat = "ID cannot be empty; ";
        } else if (GNETagProperties.isDetector()) {
            // special case for detectors (because in this case empty spaces are allowed)
            if (SUMOXMLDefinitions::isValidDetectorID(parsedAttribute) == false) {
                errorFormat = "Detector ID contains invalid characters; ";
            }
        } else if (GNETagProperties.isDemandElement()) {
            // special case for detectors (because in this case empty spaces are allowed)
            if (SUMOXMLDefinitions::isValidVehicleID(parsedAttribute) == false) {
                errorFormat = "Demand Element ID contains invalid characters; ";
            }
        } else if (SUMOXMLDefinitions::isValidAdditionalID(parsedAttribute) == false) {
            errorFormat = "ID contains invalid characters; ";
        }
    }
    // Set extra checks for int values
    if (attrProperties.isInt()) {
        if (canParse<int>(parsedAttribute)) {
            // obtain int value
            int parsedIntAttribute = parse<int>(parsedAttribute);
            // check if attribute can be negative or zero
            if (attrProperties.isPositive() && (parsedIntAttribute < 0)) {
                errorFormat = "Cannot be negative; ";
            }
        } else if (canParse<double>(parsedAttribute)) {
            errorFormat = "Float cannot be reinterpreted as int; ";
        } else {
            errorFormat = "Cannot be parsed to int; ";
        }
    }
    // Set extra checks for float(double) values
    if (attrProperties.isFloat()) {
        if (canParse<double>(parsedAttribute)) {
            // obtain double value
            double parsedDoubleAttribute = parse<double>(parsedAttribute);
            //check if can be negative and Zero
            if (attrProperties.isPositive() && (parsedDoubleAttribute < 0)) {
                errorFormat = "Cannot be negative; ";
            }
        } else {
            errorFormat = "Cannot be parsed to float; ";
        }
    }
    // Set extra checks for bool values
    if (attrProperties.isBool()) {
        if (!canParse<bool>(parsedAttribute)) {
            errorFormat = "Cannot be parsed to boolean; ";
        }
    }
    // Set extra checks for position values
    if (attrProperties.isposition()) {
        // check if we're parsing a single position or an entire shape
        if (attrProperties.isList()) {
            // check if parsed attribute can be parsed to Position Vector
            if (!canParse<PositionVector>(parsedAttribute)) {
                errorFormat = "List of Positions aren't neither x,y nor x,y,z; ";
            }
        } else if (!canParse<Position>(parsedAttribute)) {
            errorFormat = "Position is neither x,y nor x,y,z; ";
        }
    }
    // set extra check for time(double) values
    if (attrProperties.isSUMOTime()) {
        if (!canParse<SUMOTime>(parsedAttribute)) {
            errorFormat = "Cannot be parsed to SUMOTime; ";
        }
    }
    // set extra check for probability values
    if (attrProperties.isProbability()) {
        if (canParse<double>(parsedAttribute)) {
            // parse to double and check if is between [0,1]
            double probability = parse<double>(parsedAttribute);
            if (probability < 0) {
                errorFormat = "Probability cannot be smaller than 0; ";
            } else if (probability > 1) {
                errorFormat = "Probability cannot be greather than 1; ";
            }
        } else {
            errorFormat = "Cannot be parsed to probability; ";
        }
    }
    // set extra check for range values
    if (attrProperties.hasAttrRange()) {
        if (canParse<double>(parsedAttribute)) {
            // parse to double and check if is in range
            double range = parse<double>(parsedAttribute);
            if (range < attrProperties.getMinimumRange()) {
                errorFormat = "Float cannot be smaller than " + toString(attrProperties.getMinimumRange()) + "; ";
            } else if (range > attrProperties.getMaximumRange()) {
                errorFormat = "Float cannot be greather than " + toString(attrProperties.getMaximumRange()) + "; ";
            }
        } else {
            errorFormat = "Cannot be parsed to float; ";
        }
    }
    // set extra check for discrete values
    if (attrProperties.isDiscrete()) {
        // search value in the list of discretes values of attribute properties
        auto finder = std::find(attrProperties.getDiscreteValues().begin(), attrProperties.getDiscreteValues().end(), parsedAttribute);
        // check if attribute is valid
        if (finder == attrProperties.getDiscreteValues().end()) {
            errorFormat = "value is not within the set of allowed values for attribute '" + toString(attribute) + "'";
        }
    }
    // set extra check for color values
    if (attrProperties.isColor() && !canParse<RGBColor>(parsedAttribute)) {
        errorFormat = "Invalid RGB format or named color; ";
    }
    // set extra check for filename values
    if (attrProperties.isFilename()) {
        if (SUMOXMLDefinitions::isValidFilename(parsedAttribute) == false) {
            errorFormat = "Filename contains invalid characters; ";
        } else if (parsedAttribute.empty() && !attrProperties.isOptional()) {
            errorFormat = "Filename cannot be empty; ";
        }
    }
    // set extra check for name values
    if ((attribute == SUMO_ATTR_NAME) && !SUMOXMLDefinitions::isValidAttribute(parsedAttribute)) {
        errorFormat = "name contains invalid characters; ";
    }
    // set extra check for SVCPermissions values
    if (attrProperties.isVClass()) {
        if (!canParseVehicleClasses(parsedAttribute)) {
            errorFormat = "List of VClasses isn't valid; ";
            parsedAttribute = defaultValue;
        }
    }
    // set extra check for RouteProbes
    if ((attribute == SUMO_ATTR_ROUTEPROBE) && !SUMOXMLDefinitions::isValidAdditionalID(parsedAttribute)) {
        errorFormat = "RouteProbe ID contains invalid characters; ";
    }
    // set extra check for list of edges
    if ((attribute == SUMO_ATTR_EDGES) && parsedAttribute.empty()) {
        errorFormat = "List of edges cannot be empty; ";
    }
    // set extra check for list of lanes
    if ((attribute == SUMO_ATTR_LANES) && parsedAttribute.empty()) {
        errorFormat = "List of lanes cannot be empty; ";
    }
    // set extra check for list of VTypes
    if ((attribute == SUMO_ATTR_VTYPES) && !parsedAttribute.empty() && !SUMOXMLDefinitions::isValidListOfTypeID(parsedAttribute)) {
        errorFormat = "List of vTypes contains invalid characters; ";
    }
    // set extra check for list of RouteProbe
    if ((attribute == SUMO_ATTR_ROUTEPROBE) && !parsedAttribute.empty() && !SUMOXMLDefinitions::isValidAdditionalID(parsedAttribute)) {
        errorFormat = "RouteProbe ID contains invalid characters; ";
    }
    // If attribute has an invalid format
    if (errorFormat.size() > 0) {
        // if attribute is optional and has a default value, obtain it as string. In other case, abort.
        if (attrProperties.isOptional()) {
            WRITE_DEBUG("Format of optional " + attrProperties.getDescription() + " attribute '" + toString(attribute) + "' of " +
                        warningMessage +  " is invalid; " + errorFormat + "Default value will be used.");
            // set default value defined in AttrProperties
            parsedAttribute = attrProperties.getDefaultValue();
        } else {
            WRITE_WARNING("Format of essential " + attrProperties.getDescription() + " attribute '" + toString(attribute) + "' of " +
                          warningMessage +  " is invalid; " + errorFormat + GNETagProperties.getTagStr() + " cannot be created");
            // set default value (To avoid errors in parse<T>(parsedAttribute))
            parsedAttribute = defaultValue;
            // return false to abort creation of element
            return false;
        }
    }
    // return true to continue creation of element
    return true;
}


bool
GNEAttributeCarrier::parseMaskedPositionAttribute(const SUMOSAXAttributes& attrs, const std::string& objectID, const GNETagProperties& GNETagProperties,
        const GNEAttributeProperties& attrProperties, std::string& parsedAttribute, std::string& warningMessage) {
    // if element can mask their XYPosition, then must be extracted X Y coordiantes separeted
    std::string x, y, z;
    bool parsedOk = true;
    // give a default value to parsedAttribute to avoid problem parsing invalid positions
    parsedAttribute = "0,0";
    if (attrs.hasAttribute(SUMO_ATTR_X)) {
        x = attrs.get<std::string>(SUMO_ATTR_X, objectID.c_str(), parsedOk, false);
        // check that X attribute is valid
        if (!canParse<double>(x)) {
            WRITE_WARNING("Format of essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_X) + "' of " +
                          warningMessage +  " is invalid; Cannot be parsed to float; " + GNETagProperties.getTagStr() + " cannot be created");
            // abort parsing (and creation) of element
            return false;
        }
    } else {
        WRITE_WARNING("Essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_X) + "' of " +
                      warningMessage +  " is missing; " + GNETagProperties.getTagStr() + " cannot be created");
        // abort parsing (and creation) of element
        return false;
    }
    if (attrs.hasAttribute(SUMO_ATTR_Y)) {
        y = attrs.get<std::string>(SUMO_ATTR_Y, objectID.c_str(), parsedOk, false);
        // check that X attribute is valid
        if (!canParse<double>(y)) {
            WRITE_WARNING("Format of essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_Y) + "' of " +
                          warningMessage + " is invalid; Cannot be parsed to float; " + GNETagProperties.getTagStr() + " cannot be created");
            // abort parsing (and creation) of element
            return false;
        }
    } else {
        WRITE_WARNING("Essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_Y) + "' of " +
                      warningMessage +  " is missing; " + GNETagProperties.getTagStr() + " cannot be created");
        // abort parsing (and creation) of element
        return false;
    }
    // Z attribute is optional
    if (attrs.hasAttribute(SUMO_ATTR_Z)) {
        z = attrs.get<std::string>(SUMO_ATTR_Z, objectID.c_str(), parsedOk, false);
        // check that Z attribute is valid
        if (!canParse<double>(z)) {
            WRITE_WARNING("Format of optional " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_Z) + "' of " +
                          warningMessage + " is invalid; Cannot be parsed to float; " + GNETagProperties.getTagStr() + " cannot be created");
            // leave Z attribute empty
            z.clear();
        }
    }
    // create Position attribute using parsed coordinates X, Y and, optionally, Z
    if (z.empty()) {
        parsedAttribute = x + "," + y;
    } else {
        parsedAttribute = x + "," + y + "," + z;
    }
    // continue creation of element
    return true;
}


/****************************************************************************/
