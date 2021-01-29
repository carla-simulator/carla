/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    GNEConnection.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2016
///
// A class for represent connections between Lanes
/****************************************************************************/
#pragma once
#include "GNENetworkElement.h"

#include <netbuild/NBEdge.h>

// ===========================================================================
// class declarations
// ===========================================================================
class GNEEdge;


// ===========================================================================
// class definitions
// ===========================================================================

class GNEConnection : public GNENetworkElement {
public:
    /** Constructor
     * @param[in] from The edge the vehicles leave
     * @param[in] connection NBEdge::Connection in which the rest of parameters are defined
     * @param[in] uncontrolled if set to true, This connection will not be TLS-controlled despite its node being controlled.
    **/
    GNEConnection(GNELane* from, GNELane* to);

    /// @brief Destructor
    ~GNEConnection();

    /// @name Functions related with geometry of element
    /// @{
    /// @brief get connection shape
    const PositionVector& getConnectionShape() const;

    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns position of hierarchical element in view
    Position getPositionInView() const;
    /// @}

    /// @name functions for edit shape
    /// @{
    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void startConnectionShapeGeometryMoving(const double shapeOffset);

    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void endConnectionShapeGeometryMoving();

    /**@brief return index of geometry point placed in given position, or -1 if no exist
    * @param pos position of new/existent vertex
    * @param snapToGrid enable or disable snapToActiveGrid
    * @return index of position vector
    */
    int getConnectionShapeVertexIndex(Position pos, const bool snapToGrid) const;

    /**@brief move shape
    * @param[in] offset the offset of movement
    */
    void moveConnectionShape(const Position& offset);

    /**@brief commit geometry changes in the attributes of an element after use of changeShapeGeometry(...)
    * @param[in] undoList The undoList on which to register changes
    */
    void commitConnectionShapeChange(GNEUndoList* undoList);

    /// @brief delete geometry point
    void deleteConnectionShapeGeometryPoint(const Position& mousePosition, GNEUndoList* undoList);
    /// @}

    /// Returns the street's geometry
    Boundary getBoundary() const;

    /// @brief get the name of the edge the vehicles leave
    GNEEdge* getEdgeFrom() const;

    /// @brief get the name of the edge the vehicles may reach when leaving "from"
    GNEEdge* getEdgeTo() const;

    /// @briefthe get lane of the incoming lane
    GNELane* getLaneFrom() const;

    /// @briefthe get lane of the outgoing lane
    GNELane* getLaneTo() const;

    /// @briefthe get lane index of the incoming lane
    int getFromLaneIndex() const;

    /// @briefthe get lane index of the outgoing lane
    int getToLaneIndex() const;

    /// @brief get Edge::Connection
    NBEdge::Connection& getNBEdgeConnection() const;

    /// @brief get NBConnection
    NBConnection getNBConnection() const;

    /// @brief get LinkState
    LinkState getLinkState() const;

    /// @brief check that connection's Geometry has to be updated
    void markConnectionGeometryDeprecated();

    /// @brief update internal ID of Connection
    void updateID();

    /// @brief recompute cached myLinkState
    void updateLinkState();

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

    /**@brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     */
    Boundary getCenteringBoundary() const;

    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    /// @}

    /* @brief method for setting the special color of the connection
    * @param[in] color Pointer to new special color
    */
    void setSpecialColor(const RGBColor* Color2);

    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    std::string getAttribute(SumoXMLAttr key) const;

    /* @brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /* @brief method for checking if the key and their conrrespond attribute are valids
     * @param[in] key The attribute key
     * @param[in] value The value asociated to key key
     * @return true if the value is valid, false in other case
     */
    bool isValid(SumoXMLAttr key, const std::string& value);

    /* @brief method for check if the value for certain attribute is set
     * @param[in] key The attribute key
     */
    bool isAttributeEnabled(SumoXMLAttr key) const;
    /// @}

protected:
    /// @brief incoming lane of this connection
    GNELane* myFromLane;

    /// @brief outgoing lane of this connection
    GNELane* myToLane;

    /// @brief Linkstate. @note cached because after 'undo' the connection needs to be drawn while the node logic (NBRequest) has not been recomputed
    LinkState myLinkState;

    /// @brief optional special color
    const RGBColor* mySpecialColor;

    /// @brief connection geometry
    GNEGeometry::Geometry myConnectionGeometry;

    /// @brief flag to indicate that connection's shape has to be updated
    bool myShapeDeprecated;

    /// @brief waiting position for internal junction
    PositionVector myInternalJunctionMarker;

private:
    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief manage change of tlLinkindices
    void changeTLIndex(SumoXMLAttr key, int tlIndex, int tlIndex2, GNEUndoList* undoList);

    /// @brief Invalidated copy constructor.
    GNEConnection(const GNEConnection&) = delete;

    /// @brief Invalidated assignment operator.
    GNEConnection& operator=(const GNEConnection&) = delete;
};
