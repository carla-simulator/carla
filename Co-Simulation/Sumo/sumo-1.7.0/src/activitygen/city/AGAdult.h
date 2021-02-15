/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    AGAdult.h
/// @author  Piotr Woznica
/// @author  Walter Bamberger
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    July 2010
///
// Person in working age: can be linked to a work position.
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <stdexcept>
#include "AGPerson.h"


// ===========================================================================
// class declarations
// ===========================================================================
class AGWorkPosition;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class AGAdult
 * @brief An adult person who can have a job.
 *
 * AGAdult extends AGPerson by various methods to handle work life.
 */
class AGAdult : public AGPerson {
public:
    /** @brief Initialises the base class and the own attributes.
     *
     * @param[in] the age of the AGPerson
     */
    AGAdult(int age);

    /** @brief Puts out a summary of the attributes.
     */
    void print() const;

    /** @brief States whether this person occupies a work position at present.
     *
     * @return true if she has a work position
     */
    bool isWorking() const;

    /** @brief Tries to get a new work position.
     *
     * Depending on the employment rate, this adult randomly gets unemployed
     * or employed. If it gets employed, it randomly chooses one of the free
     * work positions and occupies it.
     *
     * The new state (employed or unemployed) is chosen independently from the
     * previous state. If the adult was employed, her previous job is given up.
     *
     * @param[in]: employmentRate (1 - unemploymentRate)
     * @param[in]: wps the list of work positions (open or not) in the city
     */
    void tryToWork(double employmentRate, std::vector<AGWorkPosition>* wps);

    /** @brief Called when the adult has lost her job.
     *
     * This method is called from AGWorkPosition, whenever the adult lost
     * her job, be it because it got fired or because its resignation has
     * been accepted.
     */
    void lostWorkPosition();

    /** @brief Called when the adult should resign her job.
     *
     * This method asks the WorkPosition to quit the job. The WorkPosition in
     * turn calls AGAdult::lostWorkPosition.
     */
    void resignFromWorkPosition();

    /** @brief Provides the work position of the adult.
     *
     * You should test before, whether the adult has a job. If you call this
     * method and the adult has no job, then a runtime exception is thrown.
     *
     * @return the work position
     * @throw runtime_error the adult has no work position
     */
    const AGWorkPosition& getWorkPosition() const;

private:
    /** The work position of this adult.
     *
     * A pointer to the work position or 0 if the adult is unemployed at present.
     */
    AGWorkPosition* work;

    /** @brief Randomly selects a free work position from the list.
     *
     * @param[in] the list of work positions (free or not)
     * @return the chosen free work position
     */
    static AGWorkPosition* randomFreeWorkPosition(std::vector<AGWorkPosition>* wps);
};
