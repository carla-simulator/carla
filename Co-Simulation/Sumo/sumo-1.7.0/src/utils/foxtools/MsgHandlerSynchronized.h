/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2003-2020 German Aerospace Center (DLR) and others.
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
/// @file    MsgHandlerSynchronized.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Tue, 17 Jun 2003
///
// Retrieves messages about the process and gives them further to output
/****************************************************************************/
#pragma once
#include <config.h>

#ifdef HAVE_FOX
#include <fx.h>
#endif
#include <string>
#include <vector>
#include <iostream>
#include <utils/common/MsgHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * MsgHandlerSynchronized
 */
class MsgHandlerSynchronized : public MsgHandler {
public:
    static MsgHandler* create(MsgType type) {
        return new MsgHandlerSynchronized(type);
    }

    /// @brief adds a new error to the list
    void inform(std::string msg, bool addType = true) {
        FXMutexLock locker(myLock);
        MsgHandler::inform(msg, addType);
    }

    /** @brief Begins a process information
     *
     * When a longer action is started, this method should be used to inform the user about it.
     * There will be no newline printed, but the message handler will be informed that
     *  a process message has been begun. If an error occurs, a newline will be printed.
     * After the action has been performed, use endProcessMsg to inform the user about it.
     */
    void beginProcessMsg(std::string msg, bool addType = true) {
        FXMutexLock locker(myLock);
        MsgHandler::beginProcessMsg(msg, addType);
    }

    /// @brief Ends a process information
    void endProcessMsg(std::string msg) {
        FXMutexLock locker(myLock);
        MsgHandler::endProcessMsg(msg);
    }

    /// @brief Clears information whether an error occurred previously
    void clear(bool resetInformed = true) {
        FXMutexLock locker(myLock);
        MsgHandler::clear(resetInformed);
    }

    /// @brief Adds a further retriever to the instance responsible for a certain msg type
    void addRetriever(OutputDevice* retriever) {
        FXMutexLock locker(myLock);
        MsgHandler::addRetriever(retriever);
    }

    /// @brief Removes the retriever from the handler
    void removeRetriever(OutputDevice* retriever) {
        FXMutexLock locker(myLock);
        MsgHandler::removeRetriever(retriever);
    }

protected:
    bool aggregationThresholdReached(const std::string& format) {
        FXMutexLock locker(myLock);
        return MsgHandler::aggregationThresholdReached(format);
    }

private:
    /// @brief standard constructor
    MsgHandlerSynchronized(MsgType type) :
        MsgHandler(type), myLock(true) {
    }

    /// @brief destructor
    ~MsgHandlerSynchronized() {
    }

    /// @brief The lock for synchronizing all outputs using handlers of this class
    FXMutex myLock;

private:
    /// @brief invalid copy constructor
    MsgHandlerSynchronized(const MsgHandlerSynchronized& s) = delete;

    /// @brief invalid assignment operator
    MsgHandlerSynchronized& operator=(const MsgHandlerSynchronized& s) = delete;
};
