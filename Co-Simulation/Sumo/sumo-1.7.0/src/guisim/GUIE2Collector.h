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
/// @file    GUIE2Collector.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Okt 2003
///
// The gui-version of the MS_E2_ZS_Collector
/****************************************************************************/
#pragma once
#include <config.h>

#include <microsim/output/MSE2Collector.h>
#include "GUIDetectorWrapper.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIE2Collector
 * @brief The gui-version of the MSE2Collector
 *
 * Allows the building of a wrapper (also declared herein) which draws the
 *  detector on the gl-canvas. Beside this, the method "amVisible" is
 *  overridden to signalise that this detector is not used for simulation-
 *  -internal reasons, but is placed over the simulation by the user.
 */
class GUIE2Collector : public MSE2Collector {
public:
    /** @brief Constructor with given end position and detector length
    *
    * @param[in] id The detector's unique id.
    * @param[in] usage Information how the detector is used
    * @param[in] lane The lane the detector ends
    * @param[in] startPos The start position on the lane the detector is placed at
    * @param[in] endPos The end position on the lane the detector is placed at
    * @param[in] length The length the detector has (heuristic lane selection is done if the continuation is not unique)
    * @param[in] haltingTimeThreshold The time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
    * @param[in] haltingSpeedThreshold The speed a vehicle's speed must be below to be assigned as jammed
    * @param[in] jamDistThreshold The distance between two vehicles in order to not count them to one jam
    * @param[in] vTypes Vehicle types, that the detector takes into account
    * @param[in] friendlyPositioning Whether positions should be corrected to "snap" on lane beginnings or ends if closer than POS_EPSILON
    * @param[in] showDetector Whether the detector should be visible in the GUI
    *
    * @note Exactly one of the arguments startPos, endPos and length should be invalid (i.e. equal to std::numeric_limits<double>::max()).
    *        If length is invalid, it is required that 0 <= startPos < endPos <= lane->length
    *        If endPos is invalid, the detector may span over several lanes downstream of the lane
    *        If pos is invalid, the detector may span over several lanes upstream of the lane
    */
    GUIE2Collector(const std::string& id, DetectorUsage usage,
                   MSLane* lane, double startPos, double endPos, double detLength,
                   SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                   const std::string& vTypes, bool showDetector);


    /** @brief Constructor with a sequence of lanes and given start and end position on the first and last lanes
    *
    * @param[in] id The detector's unique id.
    * @param[in] usage Information how the detector is used
    * @param[in] lanes A sequence of lanes the detector covers (must form a continuous piece)
    * @param[in] startPos The position of the detector start on the first lane the detector is placed at
    * @param[in] endPos The position of the detector end on the last lane the detector is placed at
    * @param[in] haltingTimeThreshold The time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
    * @param[in] haltingSpeedThreshold The speed a vehicle's speed must be below to be assigned as jammed
    * @param[in] jamDistThreshold The distance between two vehicles in order to not count them to one jam
    * @param[in] vTypes Vehicle types, that the detector takes into account
    * @param[in] friendlyPositioning Whether positions should be corrected to "snap" on lane beginnings or ends if closer than POS_EPSILON
    * @param[in] showDetector Whether the detector should be visible in the GUI
    */
    GUIE2Collector(const std::string& id, DetectorUsage usage,
                   std::vector<MSLane*> lanes, double startPos, double endPos,
                   SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                   const std::string& vTypes, bool showDetector);


    /// @brief Destructor
    ~GUIE2Collector();


    /** @brief Returns the wrapper for this detector
     * @return The wrapper representing the detector
     * @see MyWrapper
     */
    virtual GUIDetectorWrapper* buildDetectorGUIRepresentation();

    /// @brief whether the induction loop shall be visible
    bool isVisible() const {
        return myShow;
    }

    /// @brief toggle visibility
    void setVisible(bool show) {
        myShow = show;
    }


private:
    /// @brief Whether the detector shall be drawn in the gui
    bool myShow;

public:
    /**
     * @class GUIE2Collector::MyWrapper
     * A GUIE2Collector-visualiser
     */
    class MyWrapper : public GUIDetectorWrapper {
    public:
        /// @brief Constructor
        MyWrapper(GUIE2Collector& detector);

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
        GUIE2Collector& getDetector();


    private:
        /// @brief The wrapped detector
        GUIE2Collector& myDetector;

        /// @brief The detector's boundary
        Boundary myBoundary;

        /// @brief A sequence of positions in full-geometry mode
        PositionVector myFullGeometry;

        /// @brief A sequence of lengths in full-geometry mode
        std::vector<double> myShapeLengths;

        /// @brief A sequence of rotations in full-geometry mode
        std::vector<double> myShapeRotations;

    private:
        /// @brief Invalidated copy constructor.
        MyWrapper(const MyWrapper&);

        /// @brief Invalidated assignment operator.
        MyWrapper& operator=(const MyWrapper&);

    };

};
