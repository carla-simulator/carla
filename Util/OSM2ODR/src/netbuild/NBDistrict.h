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
/// @file    NBDistrict.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A class representing a single district
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utility>
#include "NBCont.h"
#include <utils/common/Named.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBDistrict
 * @brief A class representing a single district
 *
 * A "district" is an area within the network which may be referenced by
 *  O/D-matrices. It stems from importing VISUM-networks. Work with VISUM-
 *  -networks also made it necessary that a district knows the edges at
 *  which new vehicles shall approach the simulated network (sources) and
 *  those to use when leaving the network (sinks). These connections to the
 *  network are weighted.
 *
 * Later work on VISUM required also parsing the shape of a district. This
 *  information is used by some external tools only, it is even not shown
 *  within the GUI.
 *
 * @todo Recheck whether this can be somehow joined with ODDistrict
 */
class NBDistrict : public Named {
public:
    /** @brief Constructor with id, and position
     *
     * @param[in] id The id of the district
     * @param[in] pos The position of the district
     */
    NBDistrict(const std::string& id, const Position& pos);


    /** @brief Constructor without position
     *
     * The position must be computed later
     *
     * @param[in] id The id of the district
     */
    NBDistrict(const std::string& id);


    /// @brief Destructor
    ~NBDistrict();


    /** @brief Adds a source
     *
     * It is checked whether the edge has already been added as a source. false
     *  is returned in this case. Otherwise, the source is pushed into
     *  the list of sources and the weight into the list of source weights.
     *  both lists stay sorted this way. true is returned.
     *
     * @param[in] source An edge that shall be used as source
     * @param[in] weight The weight of the source
     * @return Whether the source could be added (was not added before)
     * @todo Consider using only one list for sources/weights
     */
    bool addSource(NBEdge* const source, double weight);


    /** @brief Adds a sink
     *
     * It is checked whether the edge has already been added as a sink. false
     *  is returned in this case. Otherwise, the sink is pushed into
     *  the list of sink and the weight into the list of sink weights.
     *  both lists stay sorted this way. true is returned.
     *
     * @param[in] sink An edge that shall be used as sink
     * @param[in] weight The weight of the sink
     * @return Whether the sink could be added (was not added before)
     * @todo Consider using only one list for sinks/weights
     */
    bool addSink(NBEdge* const sink, double weight);


    /** @brief Returns the position of this district's center
     *
     * @return The position of this district's center
     * @todo Recheck when this information is set/needed
     */
    const Position& getPosition() const {
        return myPosition;
    }


    /** @brief Sets the center coordinates
     *
     * @param[in] pos The new center to assign
     * @todo Recheck when this information is set/needed
     */
    void setCenter(const Position& pos);


    /** @brief Replaces incoming edges from the vector (sinks) by the given edge
     *
     * When an edge is split/joined/removed/etc., it may get necessary to replace prior
     *  edges by new ones. This method replaces all occurences of the edges from
     *  "which" within incoming edges (sinks) by the given edge.
     *
     * The new sink edge's weight is the sum of the weights of the replaced edges.
     *
     * @param[in] which List of edges to replace
     * @param[in] by The replacement
     */
    void replaceIncoming(const EdgeVector& which, NBEdge* const by);


    /** @brief Replaces outgoing edges from the vector (source) by the given edge
     *
     * When an edge is split/joined/removed/etc., it may get necessary to replace prior
     *  edges by new ones. This method replaces all occurences of the edges from
     *  "which" within outgoing edges (sources) by the given edge.
     *
     * The new source edge's weight is the sum of the weights of the replaced edges.
     *
     * @param[in] which List of edges to replace
     * @param[in] by The replacement
     */
    void replaceOutgoing(const EdgeVector& which, NBEdge* const by);


    /** @brief Removes the given edge from the lists of sources and sinks
     *
     * The according weights are removed, too.
     *
     * @param[in] e The edge to remove from sinks/sources
     */
    void removeFromSinksAndSources(NBEdge* const e);


    /** @brief Sets the shape of this district
     *
     * @param[in] p The new shape
     */
    void addShape(const PositionVector& p);


    /** @brief Returns the weights of the sources
     * @return The source weights
     */
    const std::vector<double>& getSourceWeights() const {
        return mySourceWeights;
    }


    /** @brief Returns the sources
     * @return The source edges
     */
    const std::vector<NBEdge*>& getSourceEdges() const {
        return mySources;
    }


    /** @brief Returns the weights of the sinks
     * @return The sink weights
     */
    const std::vector<double>& getSinkWeights() const {
        return mySinkWeights;
    }


    /** @brief Returns the sinks
     * @return The sink edges
     */
    const std::vector<NBEdge*>& getSinkEdges() const {
        return mySinks;
    }


    /** @brief Returns the shape
     * @return The district's shape
     */
    const PositionVector& getShape() const {
        return myShape;
    }



    /// @name Applying offset
    /// @{

    /** @brief Applies an offset to the district
     * @param[in] xoff The x-offset to apply
     * @param[in] yoff The y-offset to apply
     */
    void reshiftPosition(double xoff, double yoff);

    /// @brief mirror coordinates along the x-axis
    void mirrorX();
    /// @}





private:
    /// @brief Definition of a vector of connection weights
    typedef std::vector<double> WeightsCont;

    /// @brief The sources (connection from district to network)
    EdgeVector mySources;

    /// @brief The weights of the sources
    WeightsCont mySourceWeights;

    /// @brief The sinks (connection from network to district)
    EdgeVector mySinks;

    /// @brief The weights of the sinks
    WeightsCont mySinkWeights;

    /// @brief The position of the district
    Position myPosition;

    /// @brief The shape of the dsitrict
    PositionVector myShape;


private:
    /** invalid copy constructor */
    NBDistrict(const NBDistrict& s);

    /** invalid assignment operator */
    NBDistrict& operator=(const NBDistrict& s);


};
