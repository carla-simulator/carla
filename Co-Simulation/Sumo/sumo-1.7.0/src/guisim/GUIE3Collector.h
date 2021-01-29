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
/// @file    GUIE3Collector.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Jan 2004
///
// The gui-version of a MSE3Collector
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include "GUIDetectorWrapper.h"
#include <microsim/output/MSE3Collector.h>
#include <utils/geom/PositionVector.h>
#include <utils/common/ValueSource.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIE3Collector
 * @brief The gui-version of the MSE3Collector.
 *
 * Allows the building of a wrapper (also declared herein) which draws the
 * detector on the gl-canvas.
 */
class GUIE3Collector : public MSE3Collector {
public:
    /// @brief Constructor
    GUIE3Collector(const std::string& id,
                   const CrossSectionVector& entries, const CrossSectionVector& exits,
                   double haltingSpeedThreshold,
                   SUMOTime haltingTimeThreshold, const std::string& vTypes, bool openEntry);

    /// @brief Destructor
    ~GUIE3Collector();


    /** @brief Returns the list of entry points
     * @return The list of entry points
     */
    const CrossSectionVector& getEntries() const;


    /** @brief Returns the list of exit points
     * @return The list of exit points
     */
    const CrossSectionVector& getExits() const;


    /** @brief Returns the wrapper for this detector
     * @return The wrapper representing the detector
     * @see MyWrapper
     */
    GUIDetectorWrapper* buildDetectorGUIRepresentation();


public:
    /**
     * @class GUIE3Collector::MyWrapper
     * A GUIE3Collector-visualiser
     */
    class MyWrapper : public GUIDetectorWrapper {
    public:
        /// @brief Constructor
        MyWrapper(GUIE3Collector& detector);

        /// @brief Destrutor
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
        GUIE3Collector& getDetector();


    protected:
        /** @struct SingleCrossingDefinition
         * @brief Representation of a single crossing point
         */
        struct SingleCrossingDefinition {
            /// @brief The position
            Position myFGPosition;
            /// @brief The rotation
            double myFGRotation;
        };

    protected:
        /// @brief Builds the description about the position of the entry/exit point
        SingleCrossingDefinition buildDefinition(const MSCrossSection& section);

        /// @brief Draws a single entry/exit point
        void drawSingleCrossing(const Position& pos, double rot,
                                double upscale) const;

    private:
        /// @brief The wrapped detector
        GUIE3Collector& myDetector;

        /// @brief The detector's boundary
        Boundary myBoundary;

        /// @brief Definition of a list of cross (entry/exit-point) positions
        typedef std::vector<SingleCrossingDefinition> CrossingDefinitions;

        /// @brief The list of entry positions
        CrossingDefinitions myEntryDefinitions;

        /// @brief The list of exit positions
        CrossingDefinitions myExitDefinitions;

    };

};
