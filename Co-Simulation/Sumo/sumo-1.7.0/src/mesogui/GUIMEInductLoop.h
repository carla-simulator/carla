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
/// @file    GUIMEInductLoop.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 21.07.2005
///
// The gui-version of the MEInductLoop
/****************************************************************************/
#pragma once
#include <config.h>

#include <mesosim/MEInductLoop.h>
#include <utils/geom/Position.h>
#include <guisim/GUIDetectorWrapper.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUILane;
class MESegment;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIMEInductLoop
 * The gui-version of the MSInductLoop.
 * Allows the building of a wrapper (also declared herein) which draws the
 * detector on the gl-canvas.
 */
class GUIMEInductLoop : public MEInductLoop {
public:
    /// Construtor
    GUIMEInductLoop(const std::string& id, MESegment* s,
                    double position, const std::string& vTypes);

    /// Destructor
    ~GUIMEInductLoop();

    /** @brief Returns this detector's visualisation-wrapper
     * @return The wrapper representing the detector
     */
    virtual GUIDetectorWrapper* buildDetectorGUIRepresentation();


public:
    /**
     * @class GUIMEInductLoop::MyWrapper
     * A MSInductLoop-visualiser
     */
    class MyWrapper : public GUIDetectorWrapper {
    public:
        /// Constructor
        MyWrapper(GUIMEInductLoop& detector,
                  double pos);

        /// Destructor
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


        /** @brief Draws the object
         *
         * @param[in] s Current visualization settings
         * @see GUIGlObject::drawGL
         */
        void drawGL(const GUIVisualizationSettings& s) const;


        /** @brief Returns the boundary to which the view shall be centered in order to show the object
         *
         * @return The boundary the object is within
         * @see GUIGlObject::getCenteringBoundary
         */
        Boundary getCenteringBoundary() const;
        //@}


        /// Returns the detector itself
        GUIMEInductLoop& getLoop();

    private:
        /// The wrapped detector
        GUIMEInductLoop& myDetector;

        /// The detector's boundary
        Boundary myBoundary;

        /// The position in full-geometry mode
        Position myFGPosition;

        /// The rotation in full-geometry mode
        double myFGRotation;

        /// The position on the lane
        double myPosition;

    private:
        /// @brief Invalidated copy constructor.
        MyWrapper(const MyWrapper&);

        /// @brief Invalidated assignment operator.
        MyWrapper& operator=(const MyWrapper&);

    };

};
