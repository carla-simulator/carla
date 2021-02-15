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
/// @file    MSPModel_Remote.h
/// @author  Gregor Laemmel
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model for remote controlled pedestrian movement
/****************************************************************************/
#pragma once

#include <utils/options/OptionsCont.h>
#include <microsim/MSNet.h>
#include <utils/geom/Boundary.h>
#include "MSPModel.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSPModel_Remote
 * @brief The pedestrian following model connected to the external JuPedSim simulation
 */
class MSPModel_Remote : public MSPModel {
public:
    MSPModel_Remote(const OptionsCont& oc, MSNet* net);

    ~MSPModel_Remote();
    MSTransportableStateAdapter* add(MSTransportable* person, MSStageMoving* stage, SUMOTime now) override;
    void remove(MSTransportableStateAdapter* state) override;
    bool usingInternalLanes();

    SUMOTime execute(SUMOTime time);
    class Event : public Command {
    public:
        explicit Event(MSPModel_Remote* remoteModel)
            : myRemoteModel(remoteModel) { }
        SUMOTime execute(SUMOTime currentTime) override {
            return myRemoteModel->execute(currentTime);
        }
    private:
        MSPModel_Remote* myRemoteModel;
    };

    /// @brief return the number of active objects
    int getActiveNumber() {
        return (int)remoteIdPStateMapping.size();
    }

private:
    /**
    * @class PState
    * @brief Container for pedestrian state and individual position update function
    */
    class PState : public MSTransportableStateAdapter {
    public:
        PState(MSPerson* person, MSStageMoving* stage);
        ~PState() override;
        double getEdgePos(const MSStageMoving& stage, SUMOTime now) const override;
        Position getPosition(const MSStageMoving& stage, SUMOTime now) const override;
        double getAngle(const MSStageMoving& stage, SUMOTime now) const override;
        SUMOTime getWaitingTime(const MSStageMoving& stage, SUMOTime now) const override;
        double getSpeed(const MSStageMoving& stage) const override;
        const MSEdge* getNextEdge(const MSStageMoving& stage) const override;
        MSStageMoving* getStage();
        MSPerson* getPerson();

        void setPosition(double x, double y);
        void setPhi(double phi);
    private:
        Position myPosition;
        double myPhi;
        MSStageMoving* myStage;
        MSPerson* myPerson;
    };


    MSNet* myNet;
    Boundary myBoundary;
    void initialize();

    std::map<int, PState*> remoteIdPStateMapping;
    std::map<const MSEdge*, std::tuple<int, int>> edgesTransitionsMapping;
    std::map<int, const MSEdge*> transitionsEdgesMapping;
    int myLastId = 0;
    int myLastTransitionId = 0;

    MSLane* getFirstPedestrianLane(const MSEdge* const& edge);
};
