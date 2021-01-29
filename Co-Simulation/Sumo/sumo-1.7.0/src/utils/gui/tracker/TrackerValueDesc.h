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
/// @file    TrackerValueDesc.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Representation of a timeline of floats with their names and moments
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <string>
#include <vector>
#include <utils/common/RGBColor.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ValueRetriever.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TrackerValueDesc
 * @brief Representation of a timeline of floats with their names and moments
 *
 * This class contains the information needed to display a time line of
 *  float values.
 */
class TrackerValueDesc : public ValueRetriever<double> {
public:
    /// Constructor
    TrackerValueDesc(const std::string& name, const RGBColor& col,
                     SUMOTime recordBegin,
                     double aggregationSeconds);

    /// Destructor
    ~TrackerValueDesc();

    /// returns the maximum value range
    double getRange() const;

    /// Returns the values minimum
    double getMin() const;

    /// Returns the values maximum
    double getMax() const;

    /// Returns the center of the value
    double getYCenter() const;

    /// Returns the color to use to display the value
    const RGBColor& getColor() const;

    /** @brief returns the vector of collected values
        The values will be locked - no further addition will be perfomed until
        the method "unlockValues" will be called */
    const std::vector<double>& getValues();

    /** @brief returns the vector of aggregated values
        The values will be locked - no further addition will be perfomed until
        the method "unlockValues" will be called */
    const std::vector<double>& getAggregatedValues();

    /// Returns the name of the value
    const std::string& getName() const;

    /// Adds a new value to the list
    void addValue(double value);

    /// Releases the locking after the values have been drawn
    void unlockValues();

    /// set the aggregation amount
    void setAggregationSpan(SUMOTime as);

    /// get the aggregation amount
    SUMOTime getAggregationSpan() const;

    /// Returns the timestep the recording started
    SUMOTime getRecordingBegin() const;


private:
    /// The name of the value
    std::string myName;

    /// The color to use when the value is set as "active"
    RGBColor myActiveCol;

    /// The color to use when the value is set as "inactive"
    RGBColor myInactiveCol;

    /// Values collected
    std::vector<double> myValues;

    /// Collected values in their aggregated form
    std::vector<double> myAggregatedValues;

    /// The minimum and the maximum of the value
    double myMin, myMax;

    // Mutex to avoid parallel drawing and insertion of new items
    FXMutex myLock;

    /// The aggregation interval in simulation steps
    int myAggregationInterval;

    /// Values like this shall not be counted on aggregation
    double myInvalidValue;

    /// Counter for valid numbers within the current aggregation interval
    int myValidNo;

    /// The time step the values are added from
    SUMOTime myRecordingBegin;

    /// Temporary storage for the last aggregation interval
    double myTmpLastAggValue;

};
