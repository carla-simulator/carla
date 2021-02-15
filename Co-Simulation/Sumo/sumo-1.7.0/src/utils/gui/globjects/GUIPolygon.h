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
/// @file    GUIPolygon.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    June 2006
///
// The GUI-version of a polygon
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/shapes/SUMOPolygon.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>


// ===========================================================================
// class definitions
// ===========================================================================
/*
 * @class GUIPolygon
 * @brief The GUI-version of a polygon
 */
class GUIPolygon : public SUMOPolygon, public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
     * @param[in] id The name of the polygon
     * @param[in] type The (abstract) type of the polygon
     * @param[in] color The color of the polygon
     * @param[in] layer The layer of the polygon
     * @param[in] angle The rotation of the polygon
     * @param[in] imgFile The raster image of the polygon
     * @param[in] relativePath set image file as relative path
     * @param[in] shape The shape of the polygon
     * @param[in] geo specifiy if shape was loaded as GEO
     * @param[in] fill Whether the polygon shall be filled
     * @param[in] lineWidth Line width when drawing unfilled polygon
     */
    GUIPolygon(const std::string& id, const std::string& type,
               const RGBColor& color, const PositionVector& shape, bool geo, bool fill, double lineWidth,
               double layer = 0, double angle = 0, const std::string& imgFile = "", bool relativePath = false);

    /// @brief Destructor
    ~GUIPolygon();


    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app,
                                       GUISUMOAbstractView& parent);


    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app,
            GUISUMOAbstractView& parent);


    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;


    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    virtual void drawGL(const GUIVisualizationSettings& s) const;
    //@}


    /// @brief set a new shape and update the tesselation
    virtual void setShape(const PositionVector& shape);

    /** @brief Sets a new angle in navigational degrees
     * @param[in] layer The new angle to use
     */
    virtual void setShapeNaviDegree(const double angle) {
        SUMOPolygon::setShapeNaviDegree(angle);
        if (angle != 0.) {
            setShape(myShape);
        }
    }

protected:
    /// @brief set color
    void setColor(const GUIVisualizationSettings& s, bool disableSelectionColor) const;

    /// @brief check if Polygon can be drawn
    bool checkDraw(const GUIVisualizationSettings& s) const;

    /// @brief draw inner Polygon (before pushName() )
    void drawInnerPolygon(const GUIVisualizationSettings& s, const PositionVector& shape, double layer, bool disableSelectionColor) const;

private:
    /// The mutex used to avoid concurrent updates of the shape
    mutable FXMutex myLock;

    /// @brief id of the display list for the cached tesselation
    mutable GLuint myDisplayList;

    /// @brief shape rotated on the centroid, if rotation is needed, nullptr otherwise
    PositionVector* myRotatedShape;

    /// @brief store the drawing commands in a display list
    void storeTesselation(const PositionVector& shape, double lineWidth) const;

    // @brief perform the tesselation / drawing
    void performTesselation(const PositionVector& shape, double lineWidth) const;

};
