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
/// @file    RODFRouteCont.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A container for routes
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <utils/common/UtilExceptions.h>
#include "RODFRouteDesc.h"


// ===========================================================================
// class declarations
// ===========================================================================
class RODFNet;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFRouteCont
 * @brief A container for DFROUTER-routes
 *
 * The route id is (re)set as soon as the route is added.
 *
 * As sometimes several routes can be used between two edges and have to be
 *  identified, the number of routes connecting them is stored for each
 *  edge pair "myConnectionOccurences" and the route is named using this
 *  information, @see addRouteDesc.
 *
 * @see RODFRouteDesc
 */
class RODFRouteCont {
public:
    /// @brief Constructor
    RODFRouteCont();

    /// @brief Destructor
    ~RODFRouteCont();


    /** @brief Adds a route to the container
     *
     * If the same route is already known, its "overallProb" is increased
     *  by the value stored in the given route.
     *
     * An id for the route is generated if it is unset, yet. The id is
     *  computed and set via "setID".
     *
     * @param[in] desc The route description to add
     * @see setID
     */
    void addRouteDesc(RODFRouteDesc& desc);


    /** @brief Removes the given route description from the container
     *
     * All routes are regarded as being same if they pass the same edges.
     *  This is done via the "route_finder".
     *
     * @param[in] desc The route description to remove
     * @return Whether the route was removed (a similar was found)
     * @see RODFRouteCont::route_finder
     */
    bool removeRouteDesc(RODFRouteDesc& desc);


    /** @brief Saves routes
     *
     * @param[in, out] saved The list of ids of routes that shall not be saved (were saved before)
     * @param[in] prependix The prependix for route names
     * @param[out] out The device the routes shall written to
     * @return Whether at least one route was saved
     * @exception IOError not yet implemented
     */
    bool save(std::vector<std::string>& saved,
              const std::string& prependix, OutputDevice& out);


    /** @brief Returns the container of stored routes
     * @return The stored routes
     */
    std::vector<RODFRouteDesc>& get() {
        return myRoutes;
    }


    /** @brief Sorts routes by their distance (length)
     *
     * Done using by_distance_sorter.
     * @see RODFRouteCont::by_distance_sorter
     */
    void sortByDistance();


    /** @brief Removes "illegal" routes
     *
     * "illegal" routes means edge combinations that shall not be passed.
     *
     * @param[in] illegals List of edge combinations that shall not be passed
     * @todo Not used, yet
     */
    void removeIllegal(const std::vector<ROEdgeVector >& illegals);


protected:
    /** @brief Computes and sets the id of a route
     *
     * The id is <FIRST_EDGE>_to_<LAST_EDGE>_<RUNNING> where <RUNNING>
     *  is the number of routes which connect <FIRST_EDGE> and <LAST_EDGE>.
     *
     * @param[in] desc The route description to add
     */
    void setID(RODFRouteDesc& desc) const;


    /** @brief A class for sorting route descriptions by their length */
    class by_distance_sorter {
    public:
        /// @brief Constructor
        explicit by_distance_sorter() { }

        /// @brief Sorting function; compares RODFRouteDesc::distance2Last
        int operator()(const RODFRouteDesc& p1, const RODFRouteDesc& p2) {
            return p1.distance2Last < p2.distance2Last;
        }
    };


    /** @brief A class for finding a same route (one that passes the same edges) */
    class route_finder {
    public:
        /** @brief onstructor
         * @param[in] desc The route description to which a same shall be found
         */
        explicit route_finder(const RODFRouteDesc& desc) : myDesc(desc) { }

        /**  @brief The comparing function; compares passed edges */
        bool operator()(const RODFRouteDesc& desc) {
            return myDesc.edges2Pass == desc.edges2Pass;
        }

    private:
        /// @brief The route description for which a same shall be found
        const RODFRouteDesc& myDesc;

    private:
        /// @brief invalidated assignment operator
        route_finder& operator=(const route_finder&);
    };

protected:
    /// @brief Stored route descriptions
    std::vector<RODFRouteDesc> myRoutes;

    /// @brief Counts how many routes connecting the key-edges were already stored
    mutable std::map<std::pair<ROEdge*, ROEdge*>, int> myConnectionOccurences;


};
