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
/// @file    GNENetworkElement.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2016
///
// A abstract class for network elements
/****************************************************************************/
#pragma once
#include <config.h>

#include <netedit/elements/GNEHierarchicalElement.h>
#include <netedit/GNEGeometry.h>
#include <netedit/GNEMoveShape.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/geom/PositionVector.h>


// ===========================================================================
// class declarations
// ===========================================================================

class GNEAdditional;
class GNEDemandElement;

// ===========================================================================
// class definitions
// ===========================================================================

class GNENetworkElement : public GUIGlObject, public GNEHierarchicalElement, protected GNEMoveShape {

public:
    /**@brief Constructor.
     * @param[in] net The net to inform about gui updates
     * @param[in] id of the element
     * @param[in] type type of GL object
     * @param[in] tag sumo xml tag of the element
     * @param[in] junctionParents vector of junction parents
     * @param[in] edgeParents vector of edge parents
     * @param[in] laneParents vector of lane parents
     * @param[in] additionalParents vector of additional parents
     * @param[in] shapeParents vector of shape parents
     * @param[in] TAZElementParents vector of TAZElement parents
     * @param[in] demandElementParents vector of demand element parents
     * @param[in] genericDataParents vector of generic data parents
     */
    GNENetworkElement(GNENet* net, const std::string& id, GUIGlObjectType type, SumoXMLTag tag,
                      const std::vector<GNEJunction*>& junctionParents,
                      const std::vector<GNEEdge*>& edgeParents,
                      const std::vector<GNELane*>& laneParents,
                      const std::vector<GNEAdditional*>& additionalParents,
                      const std::vector<GNEShape*>& shapeParents,
                      const std::vector<GNETAZElement*>& TAZElementParents,
                      const std::vector<GNEDemandElement*>& demandElementParents,
                      const std::vector<GNEGenericData*>& genericDataParents);

    /// @brief Destructor
    virtual ~GNENetworkElement();

    /// @brief get ID
    const std::string& getID() const;

    /// @brief get GUIGlObject associated with this AttributeCarrier
    GUIGlObject* getGUIGlObject();

    /// @brief set shape edited
    void setShapeEdited(const bool value);

    /// @brief check if shape is being edited
    bool isShapeEdited() const;

    /// @name Functions related with geometry of element
    /// @{
    /// @brief update pre-computed geometry information
    virtual void updateGeometry() = 0;

    /// @brief Returns position of hierarchical element in view
    virtual Position getPositionInView() const = 0;
    /// @}

    /// @name inherited from GUIGlObject
    /// @{

    /**@brief Returns an own parameter window
    *
    * @param[in] app The application needed to build the parameter window
    * @param[in] parent The parent window needed to build the parameter window
    * @return The built parameter window
    * @see GUIGlObject::getParameterWindow
    */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /**@brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    virtual GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) = 0;

    /// @brief Returns the boundary to which the view shall be centered in order to show the object
    virtual Boundary getCenteringBoundary() const = 0;

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

    /* @brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    virtual void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) = 0;

    /* @brief method for checking if the key and their conrrespond attribute are valids
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
    std::string getPopUpID() const;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    std::string getHierarchyName() const;
    /// @}

protected:
    /// @brief boundary used during moving of elements
    Boundary myMovingGeometryBoundary;

    /// @brief position used during moving
    Position myMovingPosition;

    /// @brief flag to check if element shape is being edited
    bool myShapeEdited;

private:
    /// @brief set attribute after validation
    virtual void setAttribute(SumoXMLAttr key, const std::string& value) = 0;

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    void setEnabledAttribute(const int enabledAttributes);

    /// @brief Invalidated copy constructor.
    GNENetworkElement(const GNENetworkElement&) = delete;

    /// @brief Invalidated assignment operator.
    GNENetworkElement& operator=(const GNENetworkElement&) = delete;
};
