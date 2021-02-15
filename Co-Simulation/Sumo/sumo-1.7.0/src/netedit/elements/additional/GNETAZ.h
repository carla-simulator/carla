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
/// @file    GNETAZ.h
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2018
///
//
/****************************************************************************/
#pragma once
#include <netedit/GNEMoveShape.h>
#include <utils/gui/globjects/GUIPolygon.h>

#include "GNETAZElement.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNETAZ
 * Class for Traffic Assign Zones (TAZs)
 */
class GNETAZ : public GNETAZElement, private GUIPolygon, protected GNEMoveShape {

public:
    /// @brief needed to avoid diamond Problem between GUIPolygon and GNETAZElement
    using GNETAZElement::getID;

    /**@brief GNETAZ Constructor
     * @param[in] id The storage of gl-ids to get the one for this lane representation from
     * @param[in] net pointer to GNENet of this additional element belongs
     * @param[in] block movement enable or disable additional movement
     */
    GNETAZ(const std::string& id, GNENet* net, PositionVector shape, RGBColor color, bool blockMovement);

    /// @brief GNETAZ Destructor
    ~GNETAZ();

    /// @brief get TAZ Shape
    const PositionVector& getTAZElementShape() const;

    /**@brief writte TAZElement element into a xml file
     * @param[in] device device in which write parameters of additional element
     */
    void writeTAZElement(OutputDevice& device) const;

    /// @name Functions related with geometry of element
    /// @{
    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns position of additional in view
    Position getPositionInView() const;

    /// @brief Returns the boundary to which the view shall be centered in order to show the object
    Boundary getCenteringBoundary() const;

    /// @brief split geometry
    void splitEdgeGeometry(const double splitPosition, const GNENetworkElement* originalElement, const GNENetworkElement* newElement, GNEUndoList* undoList);
    /// @}

    /// @name functions for edit geometry
    /// @{
    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void startTAZShapeGeometryMoving(const double shapeOffset);

    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void endTAZShapeGeometryMoving();

    /**@brief return index of geometry point placed in given position, or -1 if no exist
    * @param pos position of new/existent vertex
    * @param snapToGrid enable or disable snapToActiveGrid
    * @return index of position vector
    */
    int getTAZVertexIndex(Position pos, const bool snapToGrid) const;

    /**@brief move shape
    * @param[in] offset the offset of movement
    */
    void moveTAZShape(const Position& offset);

    /**@brief commit geometry changes in the attributes of an element after use of changeShapeGeometry(...)
    * @param[in] undoList The undoList on which to register changes
    */
    void commitTAZShapeChange(GNEUndoList* undoList);

    /// @brief return true if Shape TAZ is blocked
    bool isShapeBlocked() const;
    /// @}

    /// @name inherited from GUIGlObject
    /// @{
    /// @brief Returns the name of the parent object
    /// @return This object's parent id
    std::string getParentName() const;

    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    /// @}

    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    std::string getAttribute(SumoXMLAttr key) const;

    /* @brief method for getting the Attribute of an XML key in double format (to avoid unnecessary parse<double>(...) for certain attributes)
     * @param[in] key The attribute key
     * @return double with the value associated to key
     */
    double getAttributeDouble(SumoXMLAttr key) const;

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

    /// @brief get PopPup ID (Used in AC Hierarchy)
    std::string getPopUpID() const;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    std::string getHierarchyName() const;
    /// @}

    /// @brief update TAZ after add or remove a Source/sink, or change their weight
    void updateParentAdditional();

protected:
    /// @brief boundary used during moving of elements
    Boundary myMovingGeometryBoundary;

    /// @brief flag for block shape
    bool myBlockShape;

    /// @brief geometry for lenghts/rotations
    GNEGeometry::Geometry myTAZGeometry;

private:
    /// @brief hint size of vertex
    static const double myHintSize;

    /// @brief squaredhint size of vertex
    static const double myHintSizeSquared;

    /// @brief Max source weight
    double myMaxWeightSource;

    /// @brief Min source weight
    double myMinWeightSource;

    /// @brief Average source weight
    double myAverageWeightSource;

    /// @brief Max Sink weight
    double myMaxWeightSink;

    /// @brief Min Sink weight
    double myMinWeightSink;

    /// @brief Average Sink weight
    double myAverageWeightSink;

    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief Invalidated copy constructor.
    GNETAZ(const GNETAZ&) = delete;

    /// @brief Invalidated assignment operator.
    GNETAZ& operator=(const GNETAZ&) = delete;
};
