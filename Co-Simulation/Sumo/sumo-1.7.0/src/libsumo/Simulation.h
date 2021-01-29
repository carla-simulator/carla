/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    Simulation.h
/// @author  Robert Hilbrich
/// @date    15.09.2017
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <libsumo/TraCIDefs.h>
#include <libsumo/TraCIConstants.h>


// ===========================================================================
// class declarations
// ===========================================================================
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Simulation
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class Simulation {
public:
    /// @brief load a simulation with the given arguments
    static void load(const std::vector<std::string>& args);

    /// @brief return whether a simulation (network) is present
    static bool isLoaded();

    /// @brief Advances by one step (or up to the given time)
    static void step(const double time = 0.);

    /// @brief close simulation
    static void close(const std::string& reason = "Libsumo requested termination.");

    /// @brief return TraCI API and SUMO version
    static std::pair<int, std::string> getVersion();

    static int getCurrentTime();
    static double getTime();

    static int getLoadedNumber();
    static std::vector<std::string> getLoadedIDList();
    static int getDepartedNumber();
    static std::vector<std::string> getDepartedIDList();
    static int getArrivedNumber();
    static std::vector<std::string> getArrivedIDList();
    static int getParkingStartingVehiclesNumber();
    static std::vector<std::string> getParkingStartingVehiclesIDList();
    static int getParkingEndingVehiclesNumber();
    static std::vector<std::string> getParkingEndingVehiclesIDList();
    static int getStopStartingVehiclesNumber();
    static std::vector<std::string> getStopStartingVehiclesIDList();
    static int getStopEndingVehiclesNumber();
    static std::vector<std::string> getStopEndingVehiclesIDList();
    static int getCollidingVehiclesNumber();
    static std::vector<std::string> getCollidingVehiclesIDList();
    static int getEmergencyStoppingVehiclesNumber();
    static std::vector<std::string> getEmergencyStoppingVehiclesIDList();
    static int getStartingTeleportNumber();
    static std::vector<std::string> getStartingTeleportIDList();
    static int getEndingTeleportNumber();
    static std::vector<std::string> getEndingTeleportIDList();

    static std::vector<std::string> getBusStopIDList();
    static int getBusStopWaiting(const std::string& id);

    /** @brief Returns the IDs of the transportables on a given bus stop.
     */
    static std::vector<std::string> getBusStopWaitingIDList(const std::string& id);


    static double getDeltaT();

    static TraCIPositionVector getNetBoundary();

    static TraCIPosition convert2D(const std::string& edgeID, double pos, int laneIndex = 0, bool toGeo = false);

    static TraCIPosition convert3D(const std::string& edgeID, double pos, int laneIndex = 0, bool toGeo = false);

    static TraCIRoadPosition convertRoad(double x, double y, bool isGeo = false, const std::string& vClass = "ignoring");

    static TraCIPosition convertGeo(double x, double y, bool fromGeo = false);

    static double getDistance2D(double x1, double y1, double x2, double y2, bool isGeo = false, bool isDriving = false);
    static double getDistanceRoad(const std::string& edgeID1, double pos1, const std::string& edgeID2, double pos2, bool isDriving = false);

    static int getMinExpectedNumber();

    static TraCIStage findRoute(const std::string& fromEdge, const std::string& toEdge, const std::string& vType = "", const double depart = -1., const int routingMode = 0);

    /* @note: default arrivalPos is not -1 because this would lead to very short walks when moving against the edge direction,
     * instead the middle of the edge is used. DepartPos is treated differently so that 1-edge walks do not have length 0.
     */
    static std::vector<TraCIStage> findIntermodalRoute(const std::string& fromEdge, const std::string& toEdge, const std::string& modes = "",
            double depart = -1., const int routingMode = 0, double speed = -1., double walkFactor = -1.,
            double departPos = 0, double arrivalPos = INVALID_DOUBLE_VALUE, const double departPosLat = 0,
            const std::string& pType = "", const std::string& vType = "", const std::string& destStop = "");

    static std::string getParameter(const std::string& objectID, const std::string& key);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    static void clearPending(const std::string& routeID = "");
    static void saveState(const std::string& fileName);
    /// @brief quick-load simulation state from file and return the state time
    static double loadState(const std::string& fileName);
    static void writeMessage(const std::string& msg);

    static void subscribe(const std::vector<int>& varIDs = std::vector<int>(), double begin = INVALID_DOUBLE_VALUE, double end = INVALID_DOUBLE_VALUE);
    static const TraCIResults getSubscriptionResults();

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

    /// @brief invalidated standard constructor
    Simulation() = delete;
};


}
