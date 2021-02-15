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
/// @file    GNETAZElement.h
/// @author  Pablo Alvarez Lopez
/// @date    April 2020
///
// Abstract class for TAZElements uses in netedit
/****************************************************************************/
#pragma once
#include <config.h>

#include <netedit/elements/GNEHierarchicalElement.h>
#include <netedit/GNEGeometry.h>
#include <utils/common/Parameterised.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGlObject.h>

// ===========================================================================
// class declarations
// ===========================================================================

class GNEViewNet;
class GNENetworkElement;
class GUIGLObjectPopupMenu;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNETAZElement
 * @brief An Element which don't belongs to GNENet but has influency in the simulation
 */
class GNETAZElement : public GUIGlObject, public Parameterised, public GNEHierarchicalElement {

public:
    /**@brief Constructor
     * @param[in] id Gl-id of the TAZElement element (Must be unique)
     * @param[in] net pointer to GNENet of this TAZElement element belongs
     * @param[in] type GUIGlObjectType of TAZElement
     * @param[in] tag Type of xml tag that define the TAZElement element (SUMO_TAG_BUS_STOP, SUMO_TAG_REROUTER, etc...)
     * @param[in] block movement enable or disable TAZElement movement
     * @param[in] junctionParents vector of junction parents
     * @param[in] edgeParents vector of edge parents
     * @param[in] laneParents vector of lane parents
     * @param[in] additionalParents vector of additional parents
     * @param[in] shapeParents vector of shape parents
     * @param[in] TAZElementParents vector of TAZElement parents
     * @param[in] demandElementParents vector of demand element parents
     * @param[in] genericDataParents vector of generic data parents
     */
    GNETAZElement(const std::string& id, GNENet* net, GUIGlObjectType type, SumoXMLTag tag, bool blockMovement,
                  const std::vector<GNEJunction*>& junctionParents,
                  const std::vector<GNEEdge*>& edgeParents,
                  const std::vector<GNELane*>& laneParents,
                  const std::vector<GNEAdditional*>& additionalParents,
                  const std::vector<GNEShape*>& shapeParents,
                  const std::vector<GNETAZElement*>& TAZElementParents,
                  const std::vector<GNEDemandElement*>& demandElementParents,
                  const std::vector<GNEGenericData*>& genericDataParents);

    /**@brief Constructor used by TAZElements that have two TAZElements as parent
     * @param[in] TAZElementParent pointer to parent TAZElement pointer (used to generate an ID)
     * @param[in] net pointer to GNENet of this TAZElement element belongs
     * @param[in] type GUIGlObjectType of TAZElement
     * @param[in] tag Type of xml tag that define the TAZElement element (SUMO_TAG_BUS_STOP, SUMO_TAG_REROUTER, etc...)
     * @param[in] block movement enable or disable TAZElement movement
     * @param[in] junctionParents vector of junction parents
     * @param[in] edgeParents vector of edge parents
     * @param[in] laneParents vector of lane parents
     * @param[in] additionalParents vector of additional parents
     * @param[in] shapeParents vector of shape parents
     * @param[in] TAZElementParents vector of TAZElement parents
     * @param[in] demandElementParents vector of demand element parents
     * @param[in] genericDataParents vector of generic data parents
    */
    GNETAZElement(GNETAZElement* TAZElementParent, GNENet* net, GUIGlObjectType type, SumoXMLTag tag, bool blockMovement,
                  const std::vector<GNEJunction*>& junctionParents,
                  const std::vector<GNEEdge*>& edgeParents,
                  const std::vector<GNELane*>& laneParents,
                  const std::vector<GNEAdditional*>& additionalParents,
                  const std::vector<GNEShape*>& shapeParents,
                  const std::vector<GNETAZElement*>& TAZElementParents,
                  const std::vector<GNEDemandElement*>& demandElementParents,
                  const std::vector<GNEGenericData*>& genericDataParents);

    /// @brief Destructor
    virtual ~GNETAZElement();

    /// @brief get ID
    const std::string& getID() const;

    /// @brief get GUIGlObject associated with this AttributeCarrier
    GUIGlObject* getGUIGlObject();

    /// @brief gererate a new ID for an element child
    std::string generateChildID(SumoXMLTag childTag);

    /// @brief get TAZ Shape
    virtual const PositionVector& getTAZElementShape() const = 0;

    /**@brief writte TAZElement element into a xml file
     * @param[in] device device in which write parameters of TAZElement element
     */
    virtual void writeTAZElement(OutputDevice& device) const = 0;

    /// @name Functions related with geometry of element
    /// @{

    /// @brief update pre-computed geometry information
    virtual void updateGeometry() = 0;

    /// @brief Returns position of TAZElement in view
    virtual Position getPositionInView() const = 0;

    /// @brief Returns the boundary to which the view shall be centered in order to show the object
    virtual Boundary getCenteringBoundary() const = 0;
    /// @}

    /// @brief Check if TAZElement item is currently blocked (i.e. cannot be moved with mouse)
    bool isTAZElementBlocked() const;

    /// @name inherited from GUIGlObject
    /// @{

    /**@brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /**@brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    virtual void drawGL(const GUIVisualizationSettings& s) const = 0;
    /// @}

    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    virtual std::string getAttribute(SumoXMLAttr key) const = 0;

    /* @brief method for getting the Attribute of an XML key in double format (to avoid unnecessary parse<double>(...) for certain attributes)
     * @param[in] key The attribute key
     * @return double with the value associated to key
     */
    virtual double getAttributeDouble(SumoXMLAttr key) const = 0;

    /**@brief method for setting the attribute and letting the object perform TAZElement changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    virtual void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) = 0;

    /**@brief method for checking if the key and their conrrespond attribute are valids
     * @param[in] key The attribute key
     * @param[in] value The value asociated to key key
     * @return true if the value is valid, false in other case
     */
    virtual bool isValid(SumoXMLAttr key, const std::string& value) = 0;

    /* @brief method for enable attribute
     * @param[in] key The attribute key
     * @param[in] undoList The undoList on which to register changes
     * @note certain attributes can be only enabled, and can produce the disabling of other attributes
     */
    void enableAttribute(SumoXMLAttr key, GNEUndoList* undoList);

    /* @brief method for disable attribute
     * @param[in] key The attribute key
     * @param[in] undoList The undoList on which to register changes
     * @note certain attributes can be only enabled, and can produce the disabling of other attributes
     */
    void disableAttribute(SumoXMLAttr key, GNEUndoList* undoList);

    /* @brief method for check if the value for certain attribute is set
     * @param[in] key The attribute key
     */
    virtual bool isAttributeEnabled(SumoXMLAttr key) const = 0;

    /// @brief get PopPup ID (Used in AC Hierarchy)
    virtual std::string getPopUpID() const = 0;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    virtual std::string getHierarchyName() const = 0;
    /// @}

protected:
    /// @brief boolean to check if TAZElement element is blocked (i.e. cannot be moved with mouse)
    bool myBlockMovement;

    /// @brief check if a new TAZElement ID is valid
    bool isValidTAZElementID(const std::string& newID) const;

private:
    /**@brief check restriction with the number of children
     * @throw ProcessError if itis called without be reimplemented in child class
     */
    virtual bool checkChildTAZElementRestriction() const;

    /// @brief method for setting the attribute and nothing else (used in GNEChange_Attribute)
    virtual void setAttribute(SumoXMLAttr key, const std::string& value) = 0;

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    void setEnabledAttribute(const int enabledAttributes);

    /// @brief Invalidated copy constructor.
    GNETAZElement(const GNETAZElement&) = delete;

    /// @brief Invalidated assignment operator.
    GNETAZElement& operator=(const GNETAZElement&) = delete;
};
