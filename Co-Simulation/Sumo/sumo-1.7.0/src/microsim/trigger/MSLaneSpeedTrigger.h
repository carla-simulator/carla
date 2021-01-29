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
/// @file    MSLaneSpeedTrigger.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Changes the speed allowed on a set of lanes
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/common/Command.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOSAXHandler.h>
#include "MSTrigger.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLaneSpeedTrigger
 * @brief Changes the speed allowed on a set of lanes
 *
 * This trigger reads the next maximum velocity of the lane he is
 * responsible for from a file and sets it.
 * Lanes with variable speeds are so possible.
 */
class MSLaneSpeedTrigger : public MSTrigger, public SUMOSAXHandler, public Command {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the lane speed trigger
     * @param[in] net The net the lane speed trigger belongs to
     * @param[in] destLanes List of lanes affected by this speed trigger
     * @param[in] file Name of the file to read the speeds to set from
     */
    MSLaneSpeedTrigger(const std::string& id,
                       const std::vector<MSLane*>& destLanes,
                       const std::string& file);


    /** @brief Destructor */
    virtual ~MSLaneSpeedTrigger();


    /** @brief Executes a switch command
     *
     * Calls processCommand()
     * @param[in] currentTime The current simulation time
     * @see Command
     * @see MSEventControl
     */
    SUMOTime execute(SUMOTime currentTime);


    SUMOTime processCommand(bool move2next, SUMOTime currentTime);

    double getDefaultSpeed() const;

    void setOverriding(bool val);

    void setOverridingValue(double val);

    double getLoadedSpeed();

    /// Returns the current speed
    double getCurrentSpeed() const;

    /// @brief return all MSLaneSpeedTrigger instances
    static const std::map<std::string, MSLaneSpeedTrigger*>& getInstances() {
        return myInstances;
    }

    const std::vector<MSLane*>& getLanes() {
        return myDestLanes;
    }

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
    virtual void myStartElement(int element,
                                const SUMOSAXAttributes& attrs);

    /** @brief Called on the closing of a tag;
     *
     * @param[in] element ID of the currently closed element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    virtual void myEndElement(int element);
    //@}


protected:
    /** the lane the trigger is responsible for */
    std::vector<MSLane*> myDestLanes;

    /** the speed that will be set on the next call */
    double myCurrentSpeed;

    /// The original speed allowed on the lanes
    double myDefaultSpeed;

    /// The information whether the read speed shall be overridden
    bool myAmOverriding;

    /// The speed to use if overriding the read speed
    double mySpeedOverrideValue;

    std::vector<std::pair<SUMOTime, double> > myLoadedSpeeds;
    std::vector<std::pair<SUMOTime, double> >::iterator myCurrentEntry;
    static std::map<std::string, MSLaneSpeedTrigger*> myInstances;

private:
    /// @brief Invalidated copy constructor.
    MSLaneSpeedTrigger(const MSLaneSpeedTrigger&);

    /// @brief Invalidated assignment operator.
    MSLaneSpeedTrigger& operator=(const MSLaneSpeedTrigger&);

    void init();


private:
    /// The information whether init was called
    bool myDidInit;

};
