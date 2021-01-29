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
/// @file    GUIRunThread.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The thread that runs the simulation
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fx.h>
#include <utils/foxtools/FXSingleEventThread.h>
#include <utils/foxtools/FXThreadEvent.h>
#include <utils/foxtools/FXSynchQue.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUINet;
class GUIEvent;
class OutputDevice;


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class GUIRunThread
 * This thread executes the given simulation stepwise to allow parallel
 * visualisation.
 * The avoidance of collisions between the simulation execution and its
 * visualisation is done individually for every lane using mutexes
 */
class GUIRunThread : public FXSingleEventThread {
public:
    /// constructor
    GUIRunThread(FXApp* app, MFXInterThreadEventClient* mw,
                 double& simDelay, FXSynchQue<GUIEvent*>& eq, FXEX::FXThreadEvent& ev);

    /// destructor
    virtual ~GUIRunThread();

    /// initialises the thread with the new simulation
    virtual bool init(GUINet* net, SUMOTime start, SUMOTime end);

    /// starts the execution
    virtual FXint run();

    /** called when the user presses the "resume"-button,
        this method resumes the execution after a break */
    void resume();

    /** called when the user presses the "single step"-button,
    this method allows the thread to perform a single simulation step */
    void singleStep();

    /** starts the simulation (execution of one step after another) */
    virtual void begin();

    /** halts the simulation execution */
    void stop();

    /** returns the information whether a simulation has been loaded */
    bool simulationAvailable() const;

    virtual bool simulationIsStartable() const;
    virtual bool simulationIsStopable() const;
    virtual bool simulationIsStepable() const;

    /** deletes the existing simulation */
    virtual void deleteSim();

    /** returns the loaded network */
    GUINet& getNet() const;

    /** halts the thread before it shall be deleted */
    void prepareDestruction();

    /// Retrieves messages from the loading module
    void retrieveMessage(const MsgHandler::MsgType type, const std::string& msg);

    SUMOTime getSimEndTime() const {
        return mySimEndTime;
    }

    std::vector<SUMOTime>& getBreakpoints() {
        return myBreakpoints;
    }

    FXMutex& getBreakpointLock() {
        return myBreakpointLock;
    }

protected:
    void makeStep();

    void waitForSnapshots(const SUMOTime snapshotTime);

protected:
    /// the loaded simulation network
    GUINet* myNet;

    /// the times the simulation starts and ends with
    SUMOTime mySimStartTime, mySimEndTime;

    /// information whether the simulation is halting (is not being executed)
    bool myHalting;

    /** information whether the thread shall be stopped
    (if not, the thread stays in an endless loop) */
    bool myQuit;

    /** information whether a simulation step is being performed
    (otherwise the thread may be waiting or the simulation is maybe not
    performed at all) */
    bool mySimulationInProgress;

    bool myOk;

    /** information whether the thread is running in single step mode */
    bool mySingle;

    /// @brief whether the simulation already ended
    bool myHaveSignaledEnd;

    /** @brief The instances of message retriever encapsulations
        Needed to be deleted from the handler later on */
    OutputDevice* myErrorRetriever, *myMessageRetriever, *myWarningRetriever;

    double& mySimDelay;

    FXSynchQue<GUIEvent*>& myEventQue;

    FXEX::FXThreadEvent& myEventThrow;

    FXMutex mySimulationLock;

    /// @brief List of breakpoints
    std::vector<SUMOTime> myBreakpoints;

    /// @brief Lock for modifying the list of breakpoints
    FXMutex myBreakpointLock;

};
