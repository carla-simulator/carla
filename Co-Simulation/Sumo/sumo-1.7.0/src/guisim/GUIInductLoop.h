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
/// @file    GUIInductLoop.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Aug 2003
///
// The gui-version of the MSInductLoop, together with the according
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <microsim/output/MSInductLoop.h>
#include <utils/geom/Position.h>
#include "GUIDetectorWrapper.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIInductLoop
 * @brief The gui-version of the MSInductLoop.
 *
 * Allows the building of a wrapper (also declared herein) which draws the
 *  detector on the gl-canvas. Uses a mutex to avoid parallel read/write operations.
 *  The mutex is only set within methods that change MSInductLoop-internal state
 *  and within "collectVehiclesOnDet". All other reading operations should be performed
 *  via the simulation loop only.
 */
class GUIInductLoop : public MSInductLoop {
public:
    /**
     * @brief Constructor.
     * @param[in] id Unique id
     * @param[in] lane Lane where detector woks on
     * @param[in] position Position of the detector within the lane
     * @param[in] vTypes which vehicle types are considered
     */
    GUIInductLoop(const std::string& id, MSLane* const lane, double position, const std::string& vTypes, bool show);


    /// @brief Destructor
    ~GUIInductLoop();


    /** @brief Returns this detector's visualisation-wrapper
     * @return The wrapper representing the detector
     */
    virtual GUIDetectorWrapper* buildDetectorGUIRepresentation();

    /// @brief sets special caller for myWrapper
    void setSpecialColor(const RGBColor* color);

    /// @brief whether the induction loop shall be visible
    bool isVisible() const {
        return myShow;
    }

    /// @brief toggle visibility
    void setVisible(bool show) {
        myShow = show;
    }

public:
    /**
     * @class GUIInductLoop::MyWrapper
     * @brief A MSInductLoop-visualiser
     */
    class MyWrapper : public GUIDetectorWrapper {
    public:
        /// @brief Constructor
        MyWrapper(GUIInductLoop& detector, double pos);

        /// @brief Destructor
        ~MyWrapper();


        /// @name inherited from GUIGlObject
        //@{

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


        /** @brief Draws the object
         * @param[in] s The settings for the current view (may influence drawing)
         * @see GUIGlObject::drawGL
         */
        void drawGL(const GUIVisualizationSettings& s) const;
        //@}


        /// @brief set (outline) color for extra visualiaztion
        void setSpecialColor(const RGBColor* color) {
            mySpecialColor = color;
        }

    private:
        /// @brief The wrapped detector
        GUIInductLoop& myDetector;

        /// @brief The detector's boundary
        Boundary myBoundary;

        /// @brief The position in full-geometry mode
        Position myFGPosition;

        /// @brief The rotation in full-geometry mode
        double myFGRotation;

        /// @brief The position on the lane
        double myPosition;

        /// @brief color for extra visualization
        const RGBColor* mySpecialColor;

    private:
        /// @brief Invalidated copy constructor.
        MyWrapper(const MyWrapper&);

        /// @brief Invalidated assignment operator.
        MyWrapper& operator=(const MyWrapper&);

    };

private:

    /// @brief the glObject wrapper for this induction loop
    MyWrapper* myWrapper;

    /// @brief whether this induction loop shall be visible in the gui
    bool myShow;

};
