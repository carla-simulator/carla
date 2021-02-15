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
/// @file    GNEAttributeCarrier.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// Abstract Base class for gui objects which carry attributes
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <netedit/GNEReferenceCounter.h>

#include "GNETagProperties.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GNENet;
class GNEUndoList;
class GUIGlObject;
class GNEHierarchicalElement;
class GNELane;
class GNEEdge;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEAttributeCarrier
 *
 * Abstract Base class for gui objects which carry attributes
 * inherits from GNEReferenceCounter for convenience
 */
class GNEAttributeCarrier : public GNEReferenceCounter {

    /// @brief declare friend class
    friend class GNEChange_Attribute;
    friend class GNEChange_EnableAttribute;

public:

    /**@brief Constructor
     * @param[in] tag SUMO Tag assigned to this type of object
     * @param[in] net GNENet in which this AttributeCarrier is stored
     */
    GNEAttributeCarrier(const SumoXMLTag tag, GNENet* net);

    /// @brief Destructor
    virtual ~GNEAttributeCarrier();

    /// @brief get pointer to net
    GNENet* getNet() const;

    /// @brief select attribute carrier using GUIGlobalSelection
    void selectAttributeCarrier(const bool changeFlag = true);

    /// @brief unselect attribute carrier using GUIGlobalSelection
    void unselectAttributeCarrier(const bool changeFlag = true);

    /// @brief check if attribute carrier is selected
    bool isAttributeCarrierSelected() const;

    /// @brief check if attribute carrier must be drawn using selecting color.
    bool drawUsingSelectColor() const;

    /// @name Function related with graphics (must be implemented in all childs)
    /// @{
    /// @brief get ID (all Attribute Carriers have one)
    virtual const std::string& getID() const = 0;

    /// @brief get GUIGlObject associated with this AttributeCarrier
    virtual GUIGlObject* getGUIGlObject() = 0;

    /// @brief update pre-computed geometry information
    virtual void updateGeometry() = 0;

    /// @}

    /// @name Function related with get/set attributes (must be implemented in all childs)
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    virtual std::string getAttribute(SumoXMLAttr key) const = 0;

    /* @brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    virtual void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) = 0;

    /* @brief method for check if new value for certain attribute is valid
     * @param[in] key The attribute key
     * @param[in] value The new value
     */
    virtual bool isValid(SumoXMLAttr key, const std::string& value) = 0;

    /* @brief method for enable attribute
     * @param[in] key The attribute key
     * @param[in] undoList The undoList on which to register changes
     * @note certain attributes can be only enabled, and can produce the disabling of other attributes
     */
    virtual void enableAttribute(SumoXMLAttr key, GNEUndoList* undoList) = 0;

    /* @brief method for disable attribute
     * @param[in] key The attribute key
     * @param[in] undoList The undoList on which to register changes
     * @note certain attributes can be only enabled, and can produce the disabling of other attributes
     */
    virtual void disableAttribute(SumoXMLAttr key, GNEUndoList* undoList) = 0;

    /* @brief method for check if the value for certain attribute is set
     * @param[in] key The attribute key
     */
    virtual bool isAttributeEnabled(SumoXMLAttr key) const = 0;

    /// @brief get PopPup ID (Used in AC Hierarchy)
    virtual std::string getPopUpID() const = 0;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    virtual std::string getHierarchyName() const = 0;
    /// @}

    /* @brief method for return an alternative value for disabled attributes. Used only in GNEFrames
     * @param[in] key The attribute key
     */
    std::string getAlternativeValueForDisabledAttributes(SumoXMLAttr key) const;

    /// @name Certain attributes and ACs (for example, connections) can be either loaded or guessed. The following static variables are used to remark it.
    /// @{
    /// @brief feature is still unchanged after being loaded (implies approval)
    static const std::string FEATURE_LOADED;

    /// @brief feature has been reguessed (may still be unchanged be we can't tell (yet)
    static const std::string FEATURE_GUESSED;

    /// @brief feature has been manually modified (implies approval)
    static const std::string FEATURE_MODIFIED;

    /// @brief feature has been approved but not changed (i.e. after being reguessed)
    static const std::string FEATURE_APPROVED;
    /// @}

    /// @brief max number of attributes allowed for every tag
    static const size_t MAXNUMBEROFATTRIBUTES;

    /// @brief invalid double position
    static const double INVALID_POSITION;

    /// @brief method for getting the attribute in the context of object selection
    virtual std::string getAttributeForSelection(SumoXMLAttr key) const;

    /// @brief get tag assigned to this object in string format
    const std::string& getTagStr() const;

    /// @brief get Tag Property assigned to this object
    const GNETagProperties& getTagProperty() const;

    /// @brief get FXIcon associated to this AC
    FXIcon* getIcon() const;

    /// @brief get Tag Properties
    static const GNETagProperties& getTagProperties(SumoXMLTag tag);

    /// @brief get tags of all editable element types
    static std::vector<SumoXMLTag> allowedTags(const bool onlyDrawables);

    /// @brief get tags of all editable element types using TagProperty Type (NetworkEditMode::NETWORKELEMENT, ADDITIONALELEMENT, etc.)
    static std::vector<std::pair<SumoXMLTag, const std::string> > getAllowedTagsByCategory(const int tagPropertyCategory, const bool onlyDrawables);

    /// @brief true if a value of type T can be parsed from string
    template<typename T>
    static bool canParse(const std::string& string) {
        try {
            GNEAttributeCarrier::parse<T>(string);
        } catch (NumberFormatException&) {
            return false;
        } catch (TimeFormatException&) {
            return false;
        } catch (EmptyData&) {
            return false;
        } catch (BoolFormatException&) {
            return false;
        }
        return true;
    }

    /// @brief parses a value of type T from string (used for basic types: int, double, bool, etc.)
    template<typename T>
    static T parse(const std::string& string);

    /// @brief true if a value of type T can be parsed from string
    template<typename T>
    static bool canParse(GNENet* net, const std::string& value, bool report) {
        try {
            parse<T>(net, value);
        } catch (FormatException& exception) {
            if (report) {
                WRITE_WARNING(exception.what())
            }
            return false;
        }
        return true;
    }

    /// @brief parses a complex value of type T from string (use for list of edges, list of lanes, etc.)
    template<typename T>
    static T parse(GNENet* net, const std::string& value);

    /// @brief parses a list of specific Attribute Carriers into a string of IDs
    template<typename T>
    static std::string parseIDs(const std::vector<T>& ACs);

    /// @brief check if lanes are consecutives
    static bool lanesConsecutives(const std::vector<GNELane*>& lanes);

    /// @brief Parse attribute from XML and show warnings if there are problems parsing it
    template <typename T>
    static T parseAttributeFromXML(const SUMOSAXAttributes& attrs, const std::string& objectID, const SumoXMLTag tag, const SumoXMLAttr attribute, bool& abort) {
        bool parsedOk = true;
        // declare string values
        std::string defaultValue, parsedAttribute, warningMessage;
        // obtain tag properties
        const auto& tagProperties = getTagProperties(tag);
        // first check if attribute is deprecated
        if (tagProperties.isAttributeDeprecated(attribute)) {
            // show warning if deprecateda ttribute is in the SUMOSAXAttributes
            if (attrs.hasAttribute(attribute)) {
                WRITE_WARNING("Attribute " + toString(attribute) + "' of " + tagProperties.getTagStr() + " is deprecated and will not be loaded.");
            }
            // return a dummy value
            return parse<T>("");
        }
        // now check if we're obtaining attribute of an object with an already parsed ID
        if (objectID != "") {
            warningMessage = tagProperties.getTagStr() + " with ID '" + objectID + "'";
        } else {
            warningMessage = tagProperties.getTagStr();
        }
        // check if we're parsing block movement
        if (attribute == GNE_ATTR_BLOCK_MOVEMENT) {
            // first check if we can parse
            if (tagProperties.canBlockMovement()) {
                // First check if attribute can be parsed to bool
                parsedAttribute = attrs.get<std::string>(attribute, objectID.c_str(), parsedOk, false);
                // check that sucesfully parsed attribute can be converted to type double
                if (!canParse<bool>(parsedAttribute)) {
                    abort = true;
                    // return default value
                    return parse<T>("0");
                } else {
                    // return readed value
                    return parse<T>(parsedAttribute);
                }
            } else {
                throw ProcessError("Trying to parsing block movement attribute in an AC that cannot be moved");
            }
        }
        // now check if we're parsing a GEO Attribute
        if (tagProperties.hasGEOPosition() && ((attribute == SUMO_ATTR_LON) || (attribute == SUMO_ATTR_LAT))) {
            // first check if GEO Attribute is defined
            if (attrs.hasAttribute(attribute)) {
                // First check if attribute can be parsed to string
                parsedAttribute = attrs.get<std::string>(attribute, objectID.c_str(), parsedOk, false);
                // check that sucesfully parsed attribute can be converted to type double
                if (!canParse<double>(parsedAttribute)) {
                    WRITE_WARNING("Format of GEO attribute '" + toString(attribute) + "' of " +
                                  warningMessage + " is invalid; Cannot be parsed to float; " + tagProperties.getTagStr() + " cannot be created");
                    // return default value
                    return parse<T>("0");
                } else {
                    // return readed value
                    return parse<T>(parsedAttribute);
                }
            }
            parsedOk = false;
            // return default value
            return parse<T>("0");
        }
        // obtain attribute properties (Only for improving efficiency)
        const auto& attrProperties = tagProperties.getAttributeProperties(attribute);
        // set a special default value for numerical and boolean attributes (To avoid errors parsing)
        if (attrProperties.isNumerical() || attrProperties.isBool()) {
            defaultValue = "0";
        } else if (attrProperties.isColor()) {
            defaultValue = "black";
        } else if (attrProperties.isposition()) {
            defaultValue = "0,0";
        }
        // first check that attribute exists in XML
        if (attrs.hasAttribute(attribute)) {
            // First check if attribute can be parsed to string
            parsedAttribute = attrs.get<std::string>(attribute, objectID.c_str(), parsedOk, false);
            // check parsed attribute
            if (!checkParsedAttribute(tagProperties, attrProperties, attribute, defaultValue, parsedAttribute, warningMessage)) {
                abort = true;
            }
        } else if (tagProperties.canMaskXYZPositions() && (attribute == SUMO_ATTR_POSITION)) {
            // obtain masked position attribute
            if (!parseMaskedPositionAttribute(attrs, objectID, tagProperties, attrProperties, parsedAttribute, warningMessage)) {
                abort = true;
            }
        } else {
            // if attribute is optional and has a default value, obtain it. In other case, abort.
            if (attrProperties.isOptional()) {
                parsedAttribute = attrProperties.getDefaultValue();
            } else {
                WRITE_WARNING("Essential " + attrProperties.getDescription() + " attribute '" + toString(attribute) + "' of " +
                              warningMessage +  " is missing; " + tagProperties.getTagStr() + " cannot be created");
                // abort parsing (and creation) of element
                abort = true;
                // set default value (To avoid errors in parse<T>(parsedAttribute))
                parsedAttribute = defaultValue;
            }
        }
        // return parsed attribute
        return parse<T>(parsedAttribute);
    }

protected:
    /// @brief the xml tag to which this attribute carrier corresponds
    const GNETagProperties& myTagProperty;

    /// @brief pointer to net
    GNENet* myNet;

    /// @brief boolean to check if this AC is selected (instead of GUIGlObjectStorage)
    bool mySelected;

    /// @brief dummy TagProperty used for reference some elements (for Example, dummyEdge)
    static GNETagProperties dummyTagProperty;

private:
    /// @brief method for setting the attribute and nothing else (used in GNEChange_Attribute)
    virtual void setAttribute(SumoXMLAttr key, const std::string& value) = 0;

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    virtual void setEnabledAttribute(const int enabledAttributes) = 0;

    /// @brief fill Attribute Carriers
    static void fillAttributeCarriers();

    /// @brief fill network elements
    static void fillNetworkElements();

    /// @brief fill additional elements
    static void fillAdditionals();

    /// @brief fill shape elements
    static void fillShapes();

    /// @brief fill TAZ elements
    static void fillTAZElements();

    /// @brief fill demand elements
    static void fillDemandElements();

    /// @brief fill vehicle elements
    static void fillVehicleElements();

    /// @brief fill stop elements
    static void fillStopElements();

    /// @brief fill person elements
    static void fillPersonElements();

    /// @brief fill person plan trips
    static void fillPersonPlanTrips();

    /// @brief fill person plan walks
    static void fillPersonPlanWalks();

    /// @brief fill person plan rides
    static void fillPersonPlanRides();

    /// @brief fill person plan edge -> edge
    static void fillPersonPlanEdgeEdge(SumoXMLTag currentTag);

    /// @brief fill person plan edge -> edge
    static void fillPersonPlanEdgeBusStop(SumoXMLTag currentTag);

    /// @brief fill person plan edge -> stop
    static void fillPersonPlanEdgeStop(SumoXMLTag currentTag);

    /// @brief fill person plan busStop -> edge
    static void fillPersonPlanBusStopEdge(SumoXMLTag currentTag);

    /// @brief fill person plan busStop -> edge
    static void fillPersonPlanBusStopBusStop(SumoXMLTag currentTag);

    /// @brief fill person plan busStop -> stop
    static void fillPersonPlanBusStopStop(SumoXMLTag currentTag);

    /// @brief fill person plan stop -> edge
    static void fillPersonPlanStopEdge(SumoXMLTag currentTag);

    /// @brief fill person plan stop -> edge
    static void fillPersonPlanStopBusStop(SumoXMLTag currentTag);

    /// @brief fill person plan stop -> stop
    static void fillPersonPlanStopStop(SumoXMLTag currentTag);

    /// @brief fill specific person trip attributes
    static void fillPersonTripAttributes(SumoXMLTag currentTag);

    /// @brief fill specific ride attributes
    static void fillRideAttributes(SumoXMLTag currentTag);

    /// @brief fill personStop elements
    static void fillPersonStopElements();

    /// @brief fill common vehicle attributes (used by vehicles, trips, routeFlows and flows)
    static void fillCommonVehicleAttributes(SumoXMLTag currentTag);

    /// @brief fill common flow attributes (used by flows, routeFlows and personFlows)
    static void fillCommonFlowAttributes(SumoXMLTag currentTag, const bool forVehicles);

    /// @brief fill Car Following Model of Vehicle/Person Types
    static void fillCarFollowingModelAttributes(SumoXMLTag currentTag);

    /// @brief fill Junction Model Attributes of Vehicle/Person Types
    static void fillJunctionModelAttributes(SumoXMLTag currentTag);

    /// @brief fill Junction Model Attributes of Vehicle/Person Types
    static void fillLaneChangingModelAttributes(SumoXMLTag currentTag);

    /// @brief fill common person attributes (used by person and personFlows)
    static void fillCommonPersonAttributes(SumoXMLTag currentTag);

    /// @brief fill stop person attributes (used by stops and personStps)
    static void fillCommonStopAttributes(SumoXMLTag currentTag, const bool parking);

    /// @brief fill Data elements
    static void fillDataElements();

    /// @brief parse and check attribute (note: This function is only to improve legilibility)
    static bool checkParsedAttribute(const GNETagProperties& tagProperties, const GNEAttributeProperties& attrProperties, const SumoXMLAttr attribute,
                                     std::string& defaultValue, std::string& parsedAttribute, std::string& warningMessage);

    /// @brief parse and check masked  (note: This function is only to improve legilibility)
    static bool parseMaskedPositionAttribute(const SUMOSAXAttributes& attrs, const std::string& objectID, const GNETagProperties& tagProperties,
            const GNEAttributeProperties& attrProperties, std::string& parsedAttribute, std::string& warningMessage);

    /// @brief map with the tags properties
    static std::map<SumoXMLTag, GNETagProperties> myTagProperties;

    /// @brief Invalidated copy constructor.
    GNEAttributeCarrier(const GNEAttributeCarrier&) = delete;

    /// @brief Invalidated assignment operator
    GNEAttributeCarrier& operator=(const GNEAttributeCarrier& src) = delete;
};
