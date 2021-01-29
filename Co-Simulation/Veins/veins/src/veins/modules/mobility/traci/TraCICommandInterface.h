//
// Copyright (C) 2006 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include <list>
#include <string>
#include <stdint.h>

#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightProgram.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace veins {

class VEINS_API TraCICommandInterface : public HasLogProxy {
public:
    TraCICommandInterface(cComponent* owner, TraCIConnection& c, bool ignoreGuiCommands);
    bool isIgnoringGuiCommands();

    enum DepartTime {
        DEPART_TIME_TRIGGERED = -1,
        DEPART_TIME_CONTAINER_TRIGGERED = -2,
        DEPART_TIME_NOW = -3, // Not yet documented and fully implemented (Sumo 0.30.0)
    };

    enum DepartSpeed {
        DEPART_SPEED_RANDOM = -2,
        DEPART_SPEED_MAX = -3,
    };

    enum DepartPosition {
        DEPART_POSITION_RANDOM = -2,
        DEPART_POSITION_FREE = -3,
        DEPART_POSITION_BASE = -4,
        DEPART_POSITION_LAST = -5,
        DEPART_POSITION_RANDOM_FREE = -6,
    };

    enum DepartLane {
        DEPART_LANE_RANDOM = -2, // A random lane
        DEPART_LANE_FREE = -3, // The least occupied lane
        DEPART_LANE_ALLOWED = -4, // The least occupied lane which allows continuation
        DEPART_LANE_BEST = -5, // The least occupied of the best lanes
        DEPART_LANE_FIRST = -6, // The rightmost valid
    };

    // General methods that do not deal with a particular object in the simulation
    std::pair<uint32_t, std::string> getVersion();
    void setApiVersion(uint32_t apiVersion);
    std::pair<double, double> getLonLat(const Coord&);

    unsigned getApiVersion() const
    {
        return versionConfig.version;
    }
    uint8_t getTimeType() const
    {
        return versionConfig.timeType;
    }
    uint8_t getNetBoundaryType() const
    {
        return versionConfig.netBoundaryType;
    }
    uint8_t getTimeStepCmd() const
    {
        return versionConfig.timeStepCmd;
    }

    std::pair<TraCICoord, TraCICoord> initNetworkBoundaries(int margin);

    /**
     * Convert Cartesian coordination to road map position
     * @param coord Cartesian coordination
     * @return a tuple of <RoadId, Pos, LaneId> where:
     *     RoadId identifies a road segment (edge)
     *     Pos describes the position of the node in longitudinal direction (ranging from 0 to the road's length)
     *     LaneId identifies the driving lane on the edge.
     */
    std::tuple<std::string, double, uint8_t> getRoadMapPos(const Coord& coord);

    /**
     * Get the distance between two arbitrary positions.
     *
     * @param position1 OMNeT coordinate of first position
     * @param position2 OMNeT coordinate of second position
     * @param returnDrivingDistance whether to return the driving distance or the air distance
     * @return the distance between the two positions
     */
    double getDistance(const Coord& position1, const Coord& position2, bool returnDrivingDistance);

    // Vehicle methods
    /**
     * @brief Adds a vehicle to the simulation.
     *
     * @param vehicleId The new vehicle's ID.
     * @param vehicleTypeId The new vehicle's type identifier.
     * @param routeId Identifier of the new vehicle's route.
     * @param emitTime_st Time at which to spawn the new vehicle or a value from DepartTime.
     * @param emitPosition Position of the new vehicle on its lane. Valid values are between 0 and 1 (start and
     *                        end of edge) and special values from DepartPosition.
     * @param emitSpeed Speed in meters per second of the new vehicle. Also accepts special values from DepartSpeed.
     * @param emitLane The new vehicle's lane. Special Also accepts special values from DepartLane.
     * @return Success indication
     */
    bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = DEPART_TIME_TRIGGERED, double emitPosition = DEPART_POSITION_BASE, double emitSpeed = DEPART_SPEED_MAX, int8_t emitLane = DEPART_LANE_BEST);
    class VEINS_API Vehicle {
    public:
        Vehicle(TraCICommandInterface* traci, std::string nodeId)
            : traci(traci)
            , nodeId(nodeId)
        {
            connection = &traci->connection;
        }

        void setSpeedMode(int32_t bitset);
        void setSpeed(double speed);
        void setMaxSpeed(double speed);
        TraCIColor getColor();
        void setColor(const TraCIColor& color);
        void slowDown(double speed, simtime_t time);
        void newRoute(std::string roadId);
        void setParking();
        std::string getRoadId();
        std::string getLaneId();
        double getMaxSpeed();
        double getLanePosition();
        std::list<std::string> getPlannedRoadIds();
        std::string getRouteId();
        void changeRoute(std::string roadId, simtime_t travelTime);
        void stopAt(std::string roadId, double pos, uint8_t laneid, double radius, simtime_t waittime);
        int32_t getLaneIndex();
        std::string getTypeId();
        bool changeVehicleRoute(const std::list<std::string>& roads);
        double getLength();
        double getWidth();
        double getHeight();
        double getAccel();
        double getDeccel();

        void setParameter(const std::string& parameter, int value);
        void setParameter(const std::string& parameter, double value);
        void setParameter(const std::string& parameter, const std::string& value);
        void getParameter(const std::string& parameter, int& value);
        void getParameter(const std::string& parameter, double& value);
        void getParameter(const std::string& parameter, std::string& value);

        /**
         * Returns the vehicle type of a vehicle
         */
        std::string getVType();

        /**
         * Get the vehicle's CO2 emissions in mg during this time step.
         *
         * @return the vehicle's CO2 emissions, -1001 in case of error
         */
        double getCO2Emissions() const;

        /**
         * Get the vehicle's CO emissions in mg during this time step.
         *
         * @return the vehicle's CO emissions, -1001 in case of error
         */
        double getCOEmissions() const;

        /**
         * Get the vehicle's HC emissions in mg during this time step.
         *
         * @return the vehicle's HC emissions, -1001 in case of error
         */
        double getHCEmissions() const;

        /**
         * Get the vehicle's PMx emissions in mg during this time step.
         *
         * @return the vehicle's PMx emissions, -1001 in case of error
         */
        double getPMxEmissions() const;

        /**
         * Get the vehicle's NOx emissions in mg during this time step.
         *
         * @return the vehicle's NOx emissions, -1001 in case of error
         */
        double getNOxEmissions() const;

        /**
         * Get the vehicle's fuel consumption in ml during this time step.
         *
         * @return the vehicle's fuel consumption, -1001 in case of error
         */
        double getFuelConsumption() const;

        /**
         * Get the noise generated by the vehicle's in dbA during this time step.
         *
         * @return the noise, -1001 in case of error
         */
        double getNoiseEmission() const;

        /**
         * Get the vehicle's electricity consumption in kWh during this time step.
         *
         * @return the vehicle's electricity consumption, -1001 in case of error
         */
        double getElectricityConsumption() const;

        /**
         * Get the vehicle's waiting time in s.
         * The waiting time of a vehicle is defined as the time (in seconds) spent with a speed below 0.1m/s since the last time it was faster than 0.1m/s.
         * (basically, the waiting time of a vehicle is reset to 0 every time it moves).
         * A vehicle that is stopping intentionally with a "stop" command does not accumulate waiting time.
         *
         * @return the vehicle's waiting time
         */
        double getWaitingTime() const;

        /**
         * Get the vehicle's accumulated waiting time in s within the previous time interval.
         * The length of the interval is configurable and 100s per default.
         *
         * @return the accumulated waiting time
         */
        double getAccumulatedWaitingTime() const;

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string nodeId;
    };
    Vehicle vehicle(std::string nodeId)
    {
        return Vehicle(this, nodeId);
    }

    // Road methods
    std::list<std::string> getRoadIds();
    class VEINS_API Road {
    public:
        Road(TraCICommandInterface* traci, std::string roadId)
            : traci(traci)
            , roadId(roadId)
        {
            connection = &traci->connection;
        }

        double getCurrentTravelTime();
        double getMeanSpeed();

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string roadId;
    };
    Road road(std::string roadId)
    {
        return Road(this, roadId);
    }

    // Lane methods
    std::list<std::string> getLaneIds();
    class VEINS_API Lane {
    public:
        Lane(TraCICommandInterface* traci, std::string laneId)
            : traci(traci)
            , laneId(laneId)
        {
            connection = &traci->connection;
        }

        std::list<Coord> getShape();
        std::string getRoadId();
        double getLength();
        double getMaxSpeed();
        double getMeanSpeed();

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string laneId;
    };
    Lane lane(std::string laneId)
    {
        return Lane(this, laneId);
    }

    // Trafficlight methods
    std::list<std::string> getTrafficlightIds();
    class VEINS_API Trafficlight {
    public:
        Trafficlight(TraCICommandInterface* traci, std::string trafficLightId)
            : traci(traci)
            , trafficLightId(trafficLightId)
        {
            connection = &traci->connection;
        }

        std::string getCurrentState() const;
        simtime_t getDefaultCurrentPhaseDuration() const;
        std::list<std::string> getControlledLanes() const;
        std::list<std::list<TraCITrafficLightLink>> getControlledLinks() const;
        int32_t getCurrentPhaseIndex() const;
        std::string getCurrentProgramID() const;
        TraCITrafficLightProgram getProgramDefinition() const;
        simtime_t getAssumedNextSwitchTime() const;

        void setProgram(std::string program); /**< set/switch to different program */
        void setPhaseIndex(int32_t index); /**< set/switch to different phase within the program  */
        void setState(std::string state);
        void setPhaseDuration(simtime_t duration); /**< set remaining duration of current phase */
        void setProgramDefinition(TraCITrafficLightProgram::Logic program, int32_t programNr);

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string trafficLightId;
    };
    Trafficlight trafficlight(std::string trafficLightId)
    {
        return Trafficlight(this, trafficLightId);
    }

    // LaneAreaDetector methods
    std::list<std::string> getLaneAreaDetectorIds();
    class VEINS_API LaneAreaDetector {
    public:
        LaneAreaDetector(TraCICommandInterface* traci, std::string laneAreaDetectorId)
            : traci(traci)
            , laneAreaDetectorId(laneAreaDetectorId)
        {
            connection = &traci->connection;
        }

        int getLastStepVehicleNumber();

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string laneAreaDetectorId;
    };
    LaneAreaDetector laneAreaDetector(std::string laneAreaDetectorId)
    {
        return LaneAreaDetector(this, laneAreaDetectorId);
    }

    // Polygon methods
    std::list<std::string> getPolygonIds();
    void addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points);
    class VEINS_API Polygon {
    public:
        Polygon(TraCICommandInterface* traci, std::string polyId)
            : traci(traci)
            , polyId(polyId)
        {
            connection = &traci->connection;
        }

        std::string getTypeId();
        std::list<Coord> getShape();
        void setShape(const std::list<Coord>& points);
        void remove(int32_t layer);

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string polyId;
    };
    Polygon polygon(std::string polyId)
    {
        return Polygon(this, polyId);
    }

    // Poi methods
    std::list<std::string> getPoiIds();
    void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos);
    class VEINS_API Poi {
    public:
        Poi(TraCICommandInterface* traci, std::string poiId)
            : traci(traci)
            , poiId(poiId)
        {
            connection = &traci->connection;
        }

        void remove(int32_t layer);

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string poiId;
    };
    Poi poi(std::string poiId)
    {
        return Poi(this, poiId);
    }

    // Junction methods
    std::list<std::string> getJunctionIds();
    class VEINS_API Junction {
    public:
        Junction(TraCICommandInterface* traci, std::string junctionId)
            : traci(traci)
            , junctionId(junctionId)
        {
            connection = &traci->connection;
        }

        Coord getPosition();

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string junctionId;
    };
    Junction junction(std::string junctionId)
    {
        return Junction(this, junctionId);
    }

    // Route methods
    std::list<std::string> getRouteIds();
    class VEINS_API Route {
    public:
        Route(TraCICommandInterface* traci, std::string routeId)
            : traci(traci)
            , routeId(routeId)
        {
            connection = &traci->connection;
        }

        std::list<std::string> getRoadIds();

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string routeId;
    };
    Route route(std::string routeId)
    {
        return Route(this, routeId);
    }

    // Vehicletype methods
    std::list<std::string> getVehicleTypeIds();

    // GuiView methods
    std::list<std::string> getGuiViewIds();
    class VEINS_API GuiView : public HasLogProxy {
    public:
        GuiView(TraCICommandInterface* traci, std::string viewId)
            : HasLogProxy(traci->owner)
            , traci(traci)
            , viewId(viewId)
        {
            connection = &traci->connection;
        }

        std::string getScheme();
        void setScheme(std::string name);
        double getZoom();
        void setZoom(double zoom);
        void setBoundary(Coord p1, Coord p2);
        void takeScreenshot(std::string filename = "", int32_t width = -1, int32_t height = -1);

        /**
         * Track the vehicle identified by vehicleId in the Sumo GUI.
         */
        void trackVehicle(std::string vehicleId);

    protected:
        TraCICommandInterface* traci;
        TraCIConnection* connection;
        std::string viewId;
    };
    GuiView guiView(std::string viewId)
    {
        return GuiView(this, viewId);
    }

private:
    struct VersionConfig {
        unsigned version;
        uint8_t timeType;
        uint8_t netBoundaryType;
        uint8_t timeStepCmd;
    };

    TraCIConnection& connection;
    bool ignoreGuiCommands;
    static const std::map<uint32_t, VersionConfig> versionConfigs;
    VersionConfig versionConfig;

    std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
    Coord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
    double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
    simtime_t genericGetTime(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
    int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
    std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
    std::list<Coord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId, TraCIConnection::Result* result = nullptr);
};

} // namespace veins
