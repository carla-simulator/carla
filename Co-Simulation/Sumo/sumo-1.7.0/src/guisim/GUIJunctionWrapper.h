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
/// @file    GUIJunctionWrapper.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 1 Jul 2003
///
// Holds geometrical values for a junction
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utility>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/globjects/GUIGlObject.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSJunction;
#ifdef HAVE_OSG
namespace osg {
class Geometry;
}
#endif


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIJunctionWrapper
 *
 * As MSJunctions do not have a graphical representation but a complex
 *  inheritance tree, this class is used to encapsulate the geometry of an
 *  abstract junction and to be used as a gl-object.
 *
 * In the case the represented junction's shape is empty, the boundary
 *  is computed using the junction's position to which an offset of 1m to each
 *  side is added.
 */
class GUIJunctionWrapper : public GUIGlObject {
public:
    /** @brief Constructor
     * @param[in] junction The represented junction
     */
    GUIJunctionWrapper(MSJunction& junction, const std::string& tllID);


    /// @brief Destructor
    virtual ~GUIJunctionWrapper();



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
    GUIParameterTableWindow* getParameterWindow(
        GUIMainWindow& app, GUISUMOAbstractView& parent);


    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;

    /// @brief Returns the value for generic parameter 'name' or ''
    const std::string getOptionalName() const;

    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    //@}

    /** @brief Returns the boundary of the junction
     * @return This junction's boundary
     */
    Boundary getBoundary() const {
        return myBoundary;
    }

    /// @brief whether this is an inner junction (a waiting spot for crossing a "real" junction)
    bool isInternal() const {
        return myIsInternal;
    }

    /** @brief Returns the represented junction
     * @return The junction itself
     */
    const MSJunction& getJunction() const {
        return myJunction;
    }


#ifdef HAVE_OSG
    void setGeometry(osg::Geometry* geom) {
        myGeom = geom;
    }

    void updateColor(const GUIVisualizationSettings& s);
#endif

private:
    double getColorValue(const GUIVisualizationSettings& s, int activeScheme) const;

private:
    /// @brief A reference to the represented junction
    MSJunction& myJunction;

    /// @brief The maximum size (in either x-, or y-dimension) for determining whether to draw or not
    double myMaxSize;

    /// @brief The represented junction's boundary
    Boundary myBoundary;

    /// @brief whether this wraps an instance of MSInternalJunction
    bool myIsInternal;

    /// @brief whether this junction has only waterways as incoming and outgoing edges
    bool myAmWaterway;
    /// @brief whether this junction has only railways as incoming and outgoing edges
    bool myAmRailway;

    /// @brief the associated traffic light or ""
    const std::string myTLLID;

#ifdef HAVE_OSG
    osg::Geometry* myGeom;
#endif


private:
    /// @brief Invalidated copy constructor.
    GUIJunctionWrapper(const GUIJunctionWrapper&);

    /// @brief Invalidated assignment operator.
    GUIJunctionWrapper& operator=(const GUIJunctionWrapper&);

};
