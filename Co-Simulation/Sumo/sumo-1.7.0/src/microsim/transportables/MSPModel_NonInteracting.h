/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPModel_NonInteracting.h
/// @author  Jakob Erdmann
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model (prototype)
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <limits>
#include <utils/common/SUMOTime.h>
#include <utils/common/Command.h>
#include "MSPerson.h"
#include "MSPModel.h"

// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSLink;
class MSLane;
class MSJunction;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSPModel_NonInteracting
 * @brief The pedestrian following model
 *
 */
class MSPModel_NonInteracting : public MSPModel {
public:

    /// @brief Constructor (it should not be necessary to construct more than one instance)
    MSPModel_NonInteracting(const OptionsCont& oc, MSNet* net);

    ~MSPModel_NonInteracting();

    /// @brief register the given transportable
    MSTransportableStateAdapter* add(MSTransportable* transportable, MSStageMoving* stage, SUMOTime now);

    /// @brief remove the specified person from the pedestrian simulation
    void remove(MSTransportableStateAdapter* state);

    /// @brief whether movements on intersections are modelled
    bool usingInternalLanes() {
        return false;
    }

    /// @brief return the number of active objects
    int getActiveNumber() {
        return myNumActivePedestrians;
    }

    void registerArrived() {
        myNumActivePedestrians--;
    }

private:
    class MoveToNextEdge : public Command {
    public:
        MoveToNextEdge(MSTransportable* transportable, MSStageMoving& walk, MSPModel_NonInteracting* model) :
            myParent(walk), myTransportable(transportable), myModel(model) {}
        virtual ~MoveToNextEdge();
        SUMOTime execute(SUMOTime currentTime);
        void abortWalk() {
            myTransportable = nullptr;
        }
        const MSTransportable* getTransportable() const {
            return myTransportable;
        }

    private:
        MSStageMoving& myParent;
        MSTransportable* myTransportable;
        MSPModel_NonInteracting* myModel;

    private:
        /// @brief Invalidated assignment operator.
        MoveToNextEdge& operator=(const MoveToNextEdge&);
    };


    /// @brief implementation of callbacks to retrieve various state information from the model
    class PState : public MSTransportableStateAdapter {
    public:
        PState(MoveToNextEdge* cmd): myCommand(cmd) {};

        /// @brief abstract methods inherited from PedestrianState
        /// @{
        /// @brief return the offset from the start of the current edge measured in its natural direction
        double getEdgePos(const MSStageMoving& stage, SUMOTime now) const;
        virtual Position getPosition(const MSStageMoving& stage, SUMOTime now) const;
        virtual double getAngle(const MSStageMoving& stage, SUMOTime now) const;
        SUMOTime getWaitingTime(const MSStageMoving& stage, SUMOTime now) const;
        double getSpeed(const MSStageMoving& stage) const;
        const MSEdge* getNextEdge(const MSStageMoving& stage) const;
        /// @}

        /// @brief compute walking time on edge and update state members
        SUMOTime computeWalkingTime(const MSEdge* prev, const MSStageMoving& stage, SUMOTime currentTime);
        MoveToNextEdge* getCommand() {
            return myCommand;
        }

    protected:
        SUMOTime myLastEntryTime;
        SUMOTime myCurrentDuration;
        double myCurrentBeginPos;
        double myCurrentEndPos;
        MoveToNextEdge* myCommand;
    };


    class CState : public PState {
    public:
        CState(MoveToNextEdge* cmd) : PState(cmd) {};

        /// @brief the offset for computing container positions when being transhiped
        static const double LATERAL_OFFSET;

        /// @brief return the network coordinate of the container
        Position getPosition(const MSStageMoving& stage, SUMOTime now) const;
        /// @brief return the direction in which the container heading to
        double getAngle(const MSStageMoving& stage, SUMOTime now) const;
        /// @brief compute tranship time on edge and update state members
        SUMOTime computeTranshipTime(const MSEdge* prev, const MSStageMoving& stage, SUMOTime currentTime);

    private:
        Position myCurrentBeginPosition;  //the position the container is moving from during its tranship stage
        Position myCurrentEndPosition;  //the position the container is moving to during its tranship stage
    };

private:
    /// @brief the net to which to issue moveToNextEdge commands
    MSNet* myNet;

    /// @brief the total number of active pedestrians
    int myNumActivePedestrians;

};



