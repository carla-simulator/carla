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
/// @file    GNEPoly.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2017
///
// A class for visualizing and editing POIS in netedit (adapted from
// GUIPolygon and NLHandler)
/****************************************************************************/
#pragma once
#include <netedit/GNEMoveShape.h>
#include <utils/gui/globjects/GUIPolygon.h>

#include "GNEShape.h"

// ===========================================================================
// class declarations
// ===========================================================================
class GeoConvHelper;
class GNENetworkElement;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEPoly
 *
 * In the case the represented junction's shape is empty, the boundary
 *  is computed using the junction's position to which an offset of 1m to each
 *  side is added.
 */
class GNEPoly : public GUIPolygon, public GNEShape, protected GNEMoveShape {

public:
    /// @brief needed to avoid diamond Problem between GUIPolygon and GNEShape
    using GNEShape::getID;

    /** @brief Constructor
     * @param[in] net net in which this polygon is placed
     * @param[in] id The name of the polygon
     * @param[in] type The (abstract) type of the polygon
     * @param[in] shape The shape of the polygon
     * @param[in] geo specifiy if shape was loaded as GEO
     * @param[in] color The color of the polygon
     * @param[in] layer The layer of the polygon
     * @param[in] angle The rotation of the polygon
     * @param[in] imgFile The raster image of the polygon
     * @param[in] relativePath set image file as relative path
     * @param[in] fill Whether the polygon shall be filled
     * @param[in] lineWidth Line width when drawing unfilled polygon
     * @param[in] movementBlocked if movement of POI is blocked
     * @param[in] shapeBlocked if shape of POI is blocked
     */
    GNEPoly(GNENet* net, const std::string& id, const std::string& type, const PositionVector& shape, bool geo, bool fill, double lineWidth,
            const RGBColor& color, double layer, double angle, const std::string& imgFile, bool relativePath, bool movementBlocked, bool shapeBlocked);

    /// @brief Destructor
    ~GNEPoly();

    /// @brief get ID
    const std::string& getID() const;

    /// @brief get GUIGlObject associated with this AttributeCarrier
    GUIGlObject* getGUIGlObject();

    /// @brief gererate a new ID for an element child
    std::string generateChildID(SumoXMLTag childTag);

    /**@brief Sets a parameter
     * @param[in] key The parameter's name
     * @param[in] value The parameter's value
     */
    void setParameter(const std::string& key, const std::string& value);

    /// @name functions for edit geometry
    /// @{
    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void startPolyShapeGeometryMoving(const double shapeOffset);

    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void endPolyShapeGeometryMoving();

    /**@brief return index of geometry point placed in given position, or -1 if no exist
    * @param pos position of new/existent vertex
    * @param snapToGrid enable or disable snapToActiveGrid
    * @return index of position vector
    */
    int getPolyVertexIndex(Position pos, const bool snapToGrid) const;

    /**@brief move shape
    * @param[in] offset the offset of movement
    */
    void movePolyShape(const Position& offset);

    /**@brief commit geometry changes in the attributes of an element after use of changeShapeGeometry(...)
    * @param[in] undoList The undoList on which to register changes
    */
    void commitPolyShapeChange(GNEUndoList* undoList);
    /// @}

    /// @name inherited from GNEShape
    /// @{
    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns the boundary to which the view shall be centered in order to show the object
    Boundary getCenteringBoundary() const;

    /**@brief writte shape element into a xml file
    * @param[in] device device in which write parameters of additional element
    */
    void writeShape(OutputDevice& device);

    /// @brief Returns position of additional in view
    Position getPositionInView() const;

    /// @brief Returns the numerical id of the object
    GUIGlID getGlID() const;

    /// @}

    /// @name inherited from GUIGlObject
    /// @{
    /**@brief Returns the name of the parent object
     * @return This object's parent id
     */
    std::string getParentName() const;

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
    void drawGL(const GUIVisualizationSettings& s) const;
    /// @}

    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    std::string getAttribute(SumoXMLAttr key) const;

    /**@brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /**@brief method for checking if the key and their conrrespond attribute are valids
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

    /**@brief return index of a vertex of shape, or of a new vertex if position is over an shape's edge
     * @param pos position of new/existent vertex
     * @param snapToGrid enable or disable snapToActiveGrid
     * @return index of position vector
     */
    int getVertexIndex(Position pos, bool snapToGrid);

    /// @brief delete the geometry point closest to the given pos
    void deleteGeometryPoint(const Position& pos, bool allowUndo = true);

    /// @brief return true if polygon is blocked
    bool isPolygonBlocked() const;

    /// @brief check if polygon is closed
    bool isPolygonClosed() const;

    /// @brief open polygon
    void openPolygon(bool allowUndo = true);

    /// @brief close polygon
    void closePolygon(bool allowUndo = true);

    /// @brief change first geometry point
    void changeFirstGeometryPoint(int oldIndex, bool allowUndo = true);

    /// @brief replace the current shape with a rectangle
    void simplifyShape(bool allowUndo = true);

protected:
    /// @brief Latitude of Polygon
    PositionVector myGeoShape;

    /// @brief flag for block shape
    bool myBlockShape;

    /// @brief flag to indicate if polygon is simplified
    bool mySimplifiedShape;

    /// @brief geometry for lenghts/rotations
    GNEGeometry::Geometry myPolygonGeometry;

private:
    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief Invalidated copy constructor.
    GNEPoly(const GNEPoly&) = delete;

    /// @brief Invalidated assignment operator.
    GNEPoly& operator=(const GNEPoly&) = delete;
};
