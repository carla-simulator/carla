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
/// @file    ROJTRTurnDefLoader.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Loader for the of turning percentages and source/sink definitions
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <string>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/importio/NamedColumnsParser.h>
#include <utils/importio/LineHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROJTREdge;
class RONet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROJTRTurnDefLoader
 * @brief Loader for the of turning percentages and source/sink definitions
 *
 * This handler parses XML-descriptions of jtrrouter-definitions, including
 *  percentage ratios at junctions and definitions of sink/source edges.
 *
 * All read values are stored directly into the given network's structures
 *   (edges).
 */
class ROJTRTurnDefLoader : public SUMOSAXHandler {
public:
    /** @brief Constructor
     *
     * @param[in] net The net to add loaded turning percentages into
     */
    ROJTRTurnDefLoader(RONet& net);


    /// @brief Destructor
    ~ROJTRTurnDefLoader();


protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element, const SUMOSAXAttributes& attrs);
    //@}


private:
    /** @brief Begins the processing of a incoming edge definition
     *
     * Tries to retrieve the currently described incoming edge. If the
     *  edge id is not given in the attributes or the edge is not known,
     *  an error is reported.
     *
     * If everything is ok, the edge's address is stored in myEdge.
     *
     * @param[in] attrs The SAX-attributes to parse incoming edge from
     */
    void beginFromEdge(const SUMOSAXAttributes& attrs);


    /** @brief Parses the probability to use a certain outgoing edge
     *
     * Tries to retreive the outgoing edge and then the probability to
     *  use it. If one of both operations could not be accomplished,
     *  an error is generated.
     *
     * If everything is ok, this means the destination edge is defined
     *  and known and the probability is valid, too, this probability
     *  is added to "myEdge", the last parsed incoming edge. As time,
     *  the previously parsed interval begin/end is used.
     *
     * @param[in] attrs The SAX-attributes to parse the destination edge and the probability to use it from
     */
    void addToEdge(const SUMOSAXAttributes& attrs);

    /** @brief Parses the probability to use a certain incoming-outgoing edge relation
     * @param[in] attrs The SAX-attributes to parse the destination edge and the probability to use it from
     */
    void addEdgeRel(const SUMOSAXAttributes& attrs);

private:
    /// @brief The network to set the information into
    RONet& myNet;

    /// @brief The begin and the end of the current interval
    double myIntervalBegin, myIntervalEnd;

    /// @brief The current incoming edge the turning probabilities are set into
    ROJTREdge* myEdge;

    /// @brief whether all sources are sinks
    bool mySourcesAreSinks;

    /// @brief whether upstream flows should be discounted from source flows
    bool myDiscountSources;

    /// @brief whether the warning for the deprecated format has been issued
    bool myHaveWarnedAboutDeprecatedFormat;

};
