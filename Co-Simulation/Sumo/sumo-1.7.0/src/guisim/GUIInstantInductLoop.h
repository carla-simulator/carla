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
/// @file    GUIInstantInductLoop.h
/// @author  Daniel Krajzewicz
/// @date    Aug 2003
///
// The gui-version of the MSInstantInductLoop
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <microsim/output/MSInstantInductLoop.h>
#include <utils/geom/Position.h>
#include "GUIDetectorWrapper.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIInstantInductLoop
 * @brief The gui-version of the MSInstantInductLoop
 */
class GUIInstantInductLoop : public MSInstantInductLoop {
public:
    /**
     * @brief Constructor.
     * @param[in] id Unique id
     * @param[in] od The device to write to
     * @param[in] lane Lane where detector woks on.
     * @param[in] position Position of the detector within the lane.
     */
    GUIInstantInductLoop(const std::string& id, OutputDevice& od,
                         MSLane* const lane, double positionInMeters, const std::string& vTypes);


    /// @brief Destructor
    ~GUIInstantInductLoop();



    /** @brief Returns this detector's visualisation-wrapper
     * @return The wrapper representing the detector
     */
    virtual GUIDetectorWrapper* buildDetectorGUIRepresentation();


public:
    /**
     * @class GUIInstantInductLoop::MyWrapper
     * @brief A MSInductLoop-visualiser
     */
    class MyWrapper : public GUIDetectorWrapper {
    public:
        /// @brief Constructor
        MyWrapper(GUIInstantInductLoop& detector, double pos);

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


        /// @brief Returns the detector itself
        GUIInstantInductLoop& getLoop();


    private:
        /// @brief The wrapped detector
        GUIInstantInductLoop& myDetector;

        /// @brief The detector's boundary
        Boundary myBoundary;

        /// @brief The position in full-geometry mode
        Position myFGPosition;

        /// @brief The rotation in full-geometry mode
        double myFGRotation;

        /// @brief The position on the lane
        double myPosition;

    private:
        /// @brief Invalidated copy constructor.
        MyWrapper(const MyWrapper&);

        /// @brief Invalidated assignment operator.
        MyWrapper& operator=(const MyWrapper&);

    };


    /// @brief Mutex preventing parallel read/write access to internal MSInductLoop state
    mutable FXMutex myLock;

};
