/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    ODDistrict.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A district (origin/destination)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utility>
#include <utils/common/Named.h>
#include <utils/common/UtilExceptions.h>
#include <utils/distribution/RandomDistributor.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ODDistrict
 * @brief A district (origin/destination)
 *
 * Class representing a district which has some ingoing and outgoing connections
 *  to the road network which may be weighted.
 */
class ODDistrict : public Named {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the district
     */
    ODDistrict(const std::string& id);


    /// @brief Destructor
    ~ODDistrict();


    /** @brief Adds a source connection
     *
     * A source is an edge where vehicles leave the district from to reach
     *  the network. The weight is used when a random source shall be
     *  chosen.
     *
     * BTW, it is possible to add a source twice. In this case it will occure
     *  twice within the distribution so that the behaviour is as adding
     *  both given probabilities.
     *
     * @param[in] id The id of the source
     * @param[in] weight The weight (probability to be chosen) of the source
     */
    void addSource(const std::string& id, double weight);


    /** @brief Adds a sink connection
     *
     * A sink connection is an edge which is used by vehicles to leave the
     *  network and reach the district.  The weight is used when a random
     *  sink shall be chosen.
     *
     * BTW, it is possible to add a sink twice. In this case it will occure
     *  twice within the distribution so that the behaviour is as adding
     *  both given probabilities.
     *
     * @param[in] id The id of the sink
     * @param[in] weight The weight (probability to be chosen) of the sink
     */
    void addSink(const std::string& id, double weight);


    /** @brief Returns the id of a source to use
     *
     * If the list of this district's sources is empty, an OutOfBoundsException
     *  -exception is thrown.
     *
     * @return One of this district's sources chosen randomly regarding their weights
     * @exception OutOfBoundsException If this district has no sources
     */
    std::string getRandomSource() const;


    /** @brief Returns the id of a sink to use
     *
     * If the list of this district's sinks is empty, an OutOfBoundsException
     *  -exception is thrown.
     *
     * @return One of this district's sinks chosen randomly regarding their weights
     * @exception OutOfBoundsException If this district has no sinks
     */
    std::string getRandomSink() const;


    /** @brief Returns the number of sinks
     *
     * @return The number of known sinks
     */
    int sinkNumber() const;


    /** @brief Returns the number of sources
     *
     * @return The number of known sources
     */
    int sourceNumber() const;


private:
    /// @brief Container of weighted sources
    RandomDistributor<std::string> mySources;

    /// @brief Container of weighted sinks
    RandomDistributor<std::string> mySinks;


private:
    /// @brief invalidated copy constructor
    ODDistrict(const ODDistrict& s);

    /// @brief invalidated assignment operator
    ODDistrict& operator=(const ODDistrict& s);


};
