/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUIShapeContainer.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    08.10.2009
///
// Storage for geometrical objects extended by mutexes
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <utils/shapes/ShapeContainer.h>
#include <utils/gui/globjects/GUIGlObject.h>

// ===========================================================================
// class declarations
// ===========================================================================
class SUMORTree;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIShapeContainer
 * @brief Storage for geometrical objects extended by mutexes
 * @see ShapeContainer
 */
class GUIShapeContainer : public ShapeContainer {
public:
    /// @brief Constructor
    GUIShapeContainer(SUMORTree& vis);


    /// @brief Destructor
    virtual ~GUIShapeContainer();

    /** @brief Builds a polygon using the given values and adds it to the container
     * @param[in] id The name of the polygon
     * @param[in] type The (abstract) type of the polygon
     * @param[in] color The color of the polygon
     * @param[in] layer The layer of the polygon
     * @param[in] angle The rotation of the polygon
     * @param[in] imgFile The raster image of the polygon
     * @param[in] relativePath set image file as relative path
     * @param[in] shape The shape of the polygon
     * @param[in] geo specify if shape was loaded as GEO coordinate
     * @param[in] fill Whether the polygon shall be filled
     * @param[in] lineWidth Line width when drawing unfilled polygon
     * @return whether the polygon could be added
     */
    virtual bool addPolygon(const std::string& id, const std::string& type, const RGBColor& color, double layer,
                            double angle, const std::string& imgFile, bool relativePath, const PositionVector& shape, bool geo,
                            bool fill, double lineWidth, bool ignorePruning = false) override;

    /// @brief Adds dynamics to the given Polygon, @see ShapeContainer addPolygonDynamics
    /// @note  Supplies the visualisation RTree to the dynamics for updating the object when moving
    PolygonDynamics* addPolygonDynamics(double simtime,
                                        std::string polyID,
                                        SUMOTrafficObject* trackedObject,
                                        const std::vector<double>& timeSpan,
                                        const std::vector<double>& alphaSpan,
                                        bool looped,
                                        bool rotate) override;

    /// @brief Update PolygonDynamics, @see ShapeContainer
    /// @note  Locks the visualisation RTree
    SUMOTime polygonDynamicsUpdate(SUMOTime t, PolygonDynamics* pd) override;

    /** @brief Builds a POI using the given values and adds it to the container
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
     * @param[in] imgFile The raster image of the POI
     * @param[in] relativePath set image file as relative path
     * @param[in] width The width of the POI image
     * @param[in] height The height of the POI image
     * @return whether the poi could be added
     */
    virtual bool addPOI(const std::string& id, const std::string& type, const RGBColor& color, const Position& pos, bool geo,
                        const std::string& lane, double posOverLane, double posLat, double layer, double angle,
                        const std::string& imgFile, bool relativePath, double width, double height, bool ignorePruning = false) override;



    /** @brief Removes a polygon from the container
     * @param[in] id The id of the polygon
     * @return Whether the polygon could be removed
     */
    virtual bool removePolygon(const std::string& id, bool useLock = true) override;


    /** @brief Removes a PoI from the container
     * @param[in] id The id of the PoI
     * @return Whether the poi could be removed
     */
    virtual bool removePOI(const std::string& id) override;



    /** @brief Assigns a new position to the named PoI
     * @param[in] id The id of the PoI to move
     * @param[in] pos The PoI's new position
     */
    virtual void movePOI(const std::string& id, const Position& pos) override;


    /** @brief Assigns a shape to the named polygon
     * @param[in] id The id of the polygon to reshape
     * @param[in] shape The polygon's new shape
     */
    virtual void reshapePolygon(const std::string& id, const PositionVector& shape) override;



    /// Returns the gl-ids of all pois
    std::vector<GUIGlID> getPOIIds() const;
    /// Returns the gl-ids of all polygons
    std::vector<GUIGlID> getPolygonIDs() const;


    void allowReplacement() {
        myAllowReplacement = true;
    }

private:
    /// @brief The mutex for adding/removing operations
    mutable FXMutex myLock;

    /// @brief The RTree structure to add and remove visualization elements
    SUMORTree& myVis;


    /// @brief whether existing ids shall be replaced
    bool myAllowReplacement;
};
