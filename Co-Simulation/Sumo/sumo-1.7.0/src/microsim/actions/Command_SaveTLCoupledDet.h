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
/// @file    Command_SaveTLCoupledDet.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    15 Feb 2004
///
// Writes e2 state on each tls switch
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <microsim/traffic_lights/MSTLLogicControl.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSTrafficLightLogic;
class MSDetectorFileOutput;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Command_SaveTLCoupledDet
 * @brief Writes e2 state on each tls switch
 *
 * @todo Problem: The detector may not save the last state (on simulation end)
 */
class Command_SaveTLCoupledDet : public MSTLLogicControl::OnSwitchAction {
public:
    /** @brief Constructor
     *
     * @param[in] tlls The logic to observe
     * @param[in] dtf The detector used to generate the values
     * @param[in] begin The begin simulation time
     * @param[in] device The output device to write the detector values into
     */
    Command_SaveTLCoupledDet(MSTLLogicControl::TLSLogicVariants& tlls,
                             MSDetectorFileOutput* dtf, SUMOTime begin, OutputDevice& device);


    /// @brief Destructor
    virtual ~Command_SaveTLCoupledDet();


    /** @brief Executes the command
     *
     * Called when an active tls program switches, this method calls
     *  "writeXMLOutput" of its detector (e2; the values are resetted there).
     *
     * @see MSDetectorFileOutput::writeXMLOutput
     * @see MSE2Collector::writeXMLOutput
     */
    virtual void execute();


protected:
    /// @brief The file to write the output to
    OutputDevice& myDevice;

    /// @brief The logic to use
    const MSTLLogicControl::TLSLogicVariants& myLogics;

    /// @brief The detector to use
    MSDetectorFileOutput* myDetector;

    /// @brief The last time the values were written
    SUMOTime myStartTime;


private:
    /// @brief Invalidated copy constructor.
    Command_SaveTLCoupledDet(const Command_SaveTLCoupledDet&);

    /// @brief Invalidated assignment operator.
    Command_SaveTLCoupledDet& operator=(const Command_SaveTLCoupledDet&);

};
