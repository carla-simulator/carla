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
/// @file    GUIPointOfInterest.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    June 2006
///
// missing_desc
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/shapes/PointOfInterest.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>


// ===========================================================================
// class declarations
// ===========================================================================


// ===========================================================================
// class definitions
// ===========================================================================
/*
 * @class GUIPointOfInterest
 * @brief The GUI-version of a point of interest
 */
class GUIPointOfInterest : public PointOfInterest, public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
     * @param[in] id The name of the POI
     * @param[in] type The (abstract) type of the POI
     * @param[in] color The color of the POI
     * @param[in] pos The position of the POI
     * @param[in[ geo use GEO coordinates (lon/lat)
     * @param[in] lane The Lane in which this POI is placed
     * @param[in] posOverLane The position over Lane
     * @param[in] posLat The position lateral over Lane
     * @param[in] layer The layer of the POI
     * @param[in] angle The rotation of the POI
     * @param[in] imgFile The raster image of the shape
     * @param[in] relativePath set image file as relative path
     * @param[in] width The width of the POI image
     * @param[in] height The height of the POI image
     */
    GUIPointOfInterest(const std::string& id, const std::string& type,
                       const RGBColor& color, const Position& pos, bool geo,
                       const std::string& lane, double posOverLane, double posLat,
                       double layer, double angle, const std::string& imgFile,
                       bool relativePath, double width, double height);

    /// @brief Destructor
    virtual ~GUIPointOfInterest();



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
    void drawGL(const GUIVisualizationSettings& s) const;
    //@}

protected:
    /// @brief set color
    void setColor(const GUIVisualizationSettings& s, bool forceSelectionColor) const;

    /// @brief check if POI can be drawn
    bool checkDraw(const GUIVisualizationSettings& s) const;

    /// @brief draw inner POI (before pushName() )
    void drawInnerPOI(const GUIVisualizationSettings& s, const bool disableSelectionColor, const double layer) const;
};
