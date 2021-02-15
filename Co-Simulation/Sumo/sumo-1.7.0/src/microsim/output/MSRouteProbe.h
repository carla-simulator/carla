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
/// @file    MSRouteProbe.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Tino Morenz
/// @date    Thu, 04.12.2008
///
// Writes route distributions at a certain edge
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/MSMoveReminder.h>
#include <utils/distribution/RandomDistributor.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSRoute;
class MSVehicle;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRouteProbe
 * @brief Writes routes of vehicles passing a certain edge
 *
 * This device allows to create route distributions of all vehicles
 *  running over a certain edge.
 *
 * A frequency can be specified to generate the output in certain intervals,
 *  (e.g. every 900 seconds) and is used via the detector control by
 *  calling the appropriate methods derived from MSDetectorFileOutput.
 *
 * @see MSDetectorFileOutput
 * @see Named
 */

class MSRouteProbe : public MSDetectorFileOutput, public MSMoveReminder {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the route probe
     * @param[in] edge The edge where the distribution shall be estimated
     * @param[in] distID The id of the distribution to add values to
     * @param[in] lastID The id of the last completed distribution to use for getRoute
     */
    MSRouteProbe(const std::string& id, const MSEdge* edge,
                 const std::string& distID, const std::string& lastID,
                 const std::string& vTypes);


    /// @brief Destructor
    virtual ~MSRouteProbe();


    /// @name Methods inherited from MSMoveReminder
    /// @{

    /** @brief Returns whether the vehicle shall be aware of this entry
     *
     * Returns true if the vehicle is in front of the entry, so that it
     *  may enter it in later steps.
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return False, if vehicle passed the entry, else true.
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
    /// @}


    /// @name Methods inherited from MSDetectorFileOutput.
    /// @{

    /** @brief Writes values into the given stream
     *
     * This method writes the distribution of routes collected
     *  in the last interval.
     * As a side effect the distribution is added to the global
     *  route distribution container.
     *
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @see MSDetectorFileOutput::writeXMLOutput
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLOutput(OutputDevice& dev,
                        SUMOTime startTime, SUMOTime stopTime);


    /** @brief Opens the XML-output using "detector" as root element
     *
     * @param[in] dev The output device to write the root into
     * @see MSDetectorFileOutput::writeXMLDetectorProlog
     * @todo What happens with the additional information if several detectors use the same output?
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLDetectorProlog(OutputDevice& dev) const;
    /// @}

    /* @brief sample a route from the routeDistribution
     * @param[in] last Retrieve route from the previous (complete) collection interval
     */
    const MSRoute* sampleRoute(bool last = true) const;

    const MSEdge* getEdge() {
        return myEdge;
    }

private:
    /// @brief The previous distribution of routes (probability->route)
    std::pair<std::string, RandomDistributor<const MSRoute*>*> myLastRouteDistribution;

    /// @brief The current distribution of routes (probability->route)
    std::pair<std::string, RandomDistributor<const MSRoute*>*> myCurrentRouteDistribution;

    /// @brief the edge of this route probe
    const MSEdge* myEdge;


private:
    /// @brief Invalidated copy constructor.
    MSRouteProbe(const MSRouteProbe&);

    /// @brief Invalidated assignment operator.
    MSRouteProbe& operator=(const MSRouteProbe&);


};
