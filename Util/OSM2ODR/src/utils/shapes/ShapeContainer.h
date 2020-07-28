/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    ShapeContainer.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2005-09-15
///
// Storage for geometrical objects, sorted by the layers they are in
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <memory>
#include <utils/common/NamedObjectCont.h>
#include "PointOfInterest.h"
#include "SUMOPolygon.h"

// ===========================================================================
// class declarations
// ===========================================================================
class PolygonDynamics;
class SUMOTrafficObject;
template <class T, class S>
class ParametrisedWrappingCommand;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ShapeContainer
 * @brief Storage for geometrical objects
 */
class ShapeContainer {
public:

    /// @brief containers
    typedef NamedObjectCont<SUMOPolygon*> Polygons;
    typedef NamedObjectCont<PointOfInterest*> POIs;

    /// @brief Constructor
    ShapeContainer();

    /// @brief Destructor
    virtual ~ShapeContainer();

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
    virtual bool addPolygon(const std::string& id, const std::string& type,
                            const RGBColor& color, double layer,
                            double angle, const std::string& imgFile,
                            bool relativePath, const PositionVector& shape, bool geo,
                            bool fill, double lineWidth, bool ignorePruning = false);

    /**
     * @brief Adds dynamics (animation / tracking) to the given polygon
     * @param polyID ID of the polygon which should become dynamic
     * @return true if the operation was successful, false if not.
     * @see PolygonDynamics()
     */
    virtual PolygonDynamics* addPolygonDynamics(double simtime,
            std::string polyID,
            SUMOTrafficObject* trackedObject,
            const std::vector<double>& timeSpan,
            const std::vector<double>& alphaSpan,
            bool looped,
            bool rotate);

    /**
     * @brief Remove dynamics (animation / tracking) for the given polygon
     * @param polyID ID of the polygon for which dynamics shall be removed
     * @return true if the operation was successful (dynamics existed for the polygon), false if not.
     */
    virtual bool removePolygonDynamics(const std::string& polyID);

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
                        const std::string& imgFile, bool relativePath, double width, double height, bool ignorePruning = false);

    /** @brief Removes a polygon from the container
     * @param[in] id The id of the polygon
     * @return Whether the polygon could be removed
     */
    virtual bool removePolygon(const std::string& id, bool useLock = true);

    /** @brief Removes a PoI from the container
     * @param[in] id The id of the PoI
     * @return Whether the poi could be removed
     */
    virtual bool removePOI(const std::string& id);

    /** @brief Assigns a new position to the named PoI
     * @param[in] id The id of the PoI to move
     * @param[in] pos The PoI's new position
     */
    virtual void movePOI(const std::string& id, const Position& pos);

    /** @brief Assigns a shape to the named polygon
     * @param[in] id The id of the polygon to reshape
     * @param[in] shape The polygon's new shape
     */
    virtual void reshapePolygon(const std::string& id, const PositionVector& shape);

    /// @brief Returns all polygons
    inline const Polygons& getPolygons() const {
        return myPolygons;
    }

    /// @brief Returns all pois
    inline const POIs& getPOIs() const {
        return myPOIs;
    }

    /** @brief Regular update event for updating polygon dynamics
    * @param[in] t  The time at which the update is called
    * @param[in] pd The dynamics to be updated
    * @returns zero If dynamics has expired, next update time otherwise
    */
    virtual SUMOTime polygonDynamicsUpdate(SUMOTime t, PolygonDynamics* pd);

    /// @brief Register update command (for descheduling at removal)
    virtual void addPolygonUpdateCommand(std::string polyID, ParametrisedWrappingCommand<ShapeContainer, PolygonDynamics*>* cmd);

    /// @brief Remove all tracking polygons for the given object
    virtual void removeTrackers(std::string objectID);

    /// @brief register highlight of the specified type if the given id
    virtual void registerHighlight(const std::string& objectID, const int type, const std::string& polygonID);

protected:
    /// @brief add polygon
    virtual bool add(SUMOPolygon* poly, bool ignorePruning = false);

    /// @brief add poi
    virtual bool add(PointOfInterest* poi, bool ignorePruning = false);

    /** @brief Unschedules the removal and update commands of the given polygon.
    * @param[in] id The id of the polygon
    */
    virtual void cleanupPolygonDynamics(const std::string& id);

    /// @name Management of highlights. For each type, only one highlight can be active,
    /// @see myHighlightPolygons, myHighlightedObjects
    /// @{
    /// @brief Remove any previously added highlight polygon of the specified type
    /// @param[out] toRemove will hold the id of any polygon that was highlighting the given object
    virtual void clearHighlight(const std::string& objectID, const int type, std::string& toRemove);
    /// @brief Clears all highlight information from the maps when the object leaves the net
    ///        (Highlight polygons and dynamics are removed via removeTrackers())
    virtual void clearHighlights(const std::string& objectID, SUMOPolygon* p);
    /// @}

protected:
    /// @brief stored Polygons
    Polygons myPolygons;

    /// @brief stored PolygonDynamics
    std::map<std::string, PolygonDynamics*> myPolygonDynamics;

    /// @brief maps objects to a map of highlight types to highlighting polygons
    std::map<std::string, std::map<int, std::string> > myHighlightPolygons;
    /// @brief inverse map to myHighlightPolygons saves the highlighted object for each polygon
    std::map<std::string, std::string> myHighlightedObjects;

    /// @brief Information about tracked objects
    /// @note  Maps tracked object IDs to set of polygons, which are tracking the object.
    ///        Needed at object removal to cancel tacking (i.e. remove tracking poly).
    std::map<const std::string, std::set<const SUMOPolygon*> > myTrackingPolygons;

    /// @brief stored POIs
    POIs myPOIs;

private:
    /// @brief Command pointers for scheduled polygon update. Maps PolyID->Command
    std::map<const std::string, ParametrisedWrappingCommand<ShapeContainer, PolygonDynamics*>*> myPolygonUpdateCommands;

};
