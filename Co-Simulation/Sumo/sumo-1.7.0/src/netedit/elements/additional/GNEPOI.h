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
/// @file    GNEPOI.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2017
///
// A class for visualizing and editing POIS in netedit (adapted from
// GUIPointOfInterest and NLHandler)
/****************************************************************************/
#pragma once
#include <utils/gui/globjects/GUIPointOfInterest.h>
#include <netedit/GNEMoveShape.h>

#include "GNEShape.h"

// ===========================================================================
// class declarations
// ===========================================================================
class GNELane;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEPOI
 *
 * In the case the represented junction's shape is empty, the boundary
 *  is computed using the junction's position to which an offset of 1m to each
 *  side is added.
 */
class GNEPOI : public GUIPointOfInterest, public GNEShape {

public:
    /// @brief needed to avoid diamond Problem between GUIPointOfInterest and GNEShape
    using GNEShape::getID;

    /** @brief Constructor
     * @param[in] net net in which this polygon is placed
     * @param[in] id The name of the POI
     * @param[in] type The (abstract) type of the POI
     * @param[in] color The color of the POI
     * @param[in] pos The position of the POI
     * @param[in[ geo use GEO coordinates (lon/lat)
     * @param[in] layer The layer of the POI
     * @param[in] angle The rotation of the POI
     * @param[in] imgFile The raster image of the shape
     * @param[in] relativePath set image file as relative path
     * @param[in] width The width of the POI image
     * @param[in] height The height of the POI image
     * @param[in] movementBlocked if movement of POI is blocked
     */
    GNEPOI(GNENet* net, const std::string& id, const std::string& type, const RGBColor& color, const Position& pos, bool geo,
           double layer, double angle, const std::string& imgFile, bool relativePath, double width, double height, bool movementBlocked);

    /** @brief Constructor
    * @param[in] net net in which this polygon is placed
    * @param[in] id The name of the POI
    * @param[in] type The (abstract) type of the POI
    * @param[in] color The color of the POI
    * @param[in] layer The layer of the POI
    * @param[in] angle The rotation of the POI
    * @param[in] imgFile The raster image of the shape
    * @param[in] relativePath set image file as relative path
    * @param[in] lane lane in which this POILane is placed
    * @param[in] posOverLane Position over lane in which this POILane is placed
    * @param[in] posLat Lateral position over lane
    * @param[in] width The width of the POI image
    * @param[in] height The height of the POI image
    * @param[in] movementBlocked if movement of POI is blocked
    */
    GNEPOI(GNENet* net, const std::string& id, const std::string& type, const RGBColor& color,
           double layer, double angle, const std::string& imgFile, bool relativePath, GNELane* lane, double posOverLane,
           double posLat, double width, double height, bool movementBlocked);

    /// @brief Destructor
    ~GNEPOI();

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
    void startPOIGeometryMoving();

    /// @brief begin movement (used when user click over edge to start a movement, to avoid problems with GL Tree)
    void endPOIGeometryMoving();

    /**@brief change the position of the element geometry without saving in undoList
    * @param[in] newPosition new position of geometry
    * @note should't be called in drawGL(...) functions to avoid smoothness issues
    */
    void movePOIGeometry(const Position& offset);

    /**@brief commit geometry changes in the attributes of an element after use of moveGeometry(...)
    * @param[in] undoList The undoList on which to register changes
    */
    void commitPOIGeometryMoving(GNEUndoList* undoList);
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
    /**@brief method for getting the Attribute of an XML key
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

    /**@brief method for checking if the key and their correspond attribute are valids
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
    /// @brief Position of POI in GEO coordinates (Only used by POIs that aren't placed over lanes)
    Position myGEOPosition;

private:
    /// @brief position used for move Lanes
    Position myPositionBeforeMoving;

    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief Invalidated copy constructor.
    GNEPOI(const GNEPOI&) = delete;

    /// @brief Invalidated assignment operator.
    GNEPOI& operator=(const GNEPOI&) = delete;
};
