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
/// @file    TraCIDefs.h
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @author  Robert Hilbrich
/// @date    30.05.2012
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
// we do not include config.h here, since we should be independent of a special sumo build
#include <libsumo/TraCIConstants.h>
#include <vector>
#include <limits>
#include <map>
#include <string>
#include <stdexcept>
#include <sstream>
#include <memory>


// ===========================================================================
// global definitions
// ===========================================================================

#define LIBSUMO_SUBSCRIPTION_API \
static void subscribe(const std::string& objectID, const std::vector<int>& varIDs = std::vector<int>({-1}), double begin = libsumo::INVALID_DOUBLE_VALUE, double end = libsumo::INVALID_DOUBLE_VALUE); \
static void unsubscribe(const std::string& objectID); \
static void subscribeContext(const std::string& objectID, int domain, double dist, const std::vector<int>& varIDs = std::vector<int>({-1}), double begin = libsumo::INVALID_DOUBLE_VALUE, double end = libsumo::INVALID_DOUBLE_VALUE); \
static void unsubscribeContext(const std::string& objectID, int domain, double dist); \
static const SubscriptionResults getAllSubscriptionResults(); \
static const TraCIResults getSubscriptionResults(const std::string& objectID); \
static const ContextSubscriptionResults getAllContextSubscriptionResults(); \
static const SubscriptionResults getContextSubscriptionResults(const std::string& objectID);

#define LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(CLASS, DOMAIN) \
void \
CLASS::subscribe(const std::string& objectID, const std::vector<int>& varIDs, double begin, double end) { \
    libsumo::Helper::subscribe(CMD_SUBSCRIBE_##DOMAIN##_VARIABLE, objectID, varIDs, begin, end); \
} \
void \
CLASS::unsubscribe(const std::string& objectID) { \
    libsumo::Helper::subscribe(CMD_SUBSCRIBE_##DOMAIN##_VARIABLE, objectID, std::vector<int>(), libsumo::INVALID_DOUBLE_VALUE, libsumo::INVALID_DOUBLE_VALUE); \
} \
void \
CLASS::subscribeContext(const std::string& objectID, int domain, double dist, const std::vector<int>& varIDs, double begin, double end) { \
    libsumo::Helper::subscribe(CMD_SUBSCRIBE_##DOMAIN##_CONTEXT, objectID, varIDs, begin, end, domain, dist); \
} \
void \
CLASS::unsubscribeContext(const std::string& objectID, int domain, double dist) { \
    libsumo::Helper::subscribe(CMD_SUBSCRIBE_##DOMAIN##_CONTEXT, objectID, std::vector<int>(), libsumo::INVALID_DOUBLE_VALUE, libsumo::INVALID_DOUBLE_VALUE, domain, dist); \
} \
const SubscriptionResults \
CLASS::getAllSubscriptionResults() { \
    return mySubscriptionResults; \
} \
const TraCIResults \
CLASS::getSubscriptionResults(const std::string& objectID) { \
    return mySubscriptionResults[objectID]; \
} \
const ContextSubscriptionResults \
CLASS::getAllContextSubscriptionResults() { \
    return myContextSubscriptionResults; \
} \
const SubscriptionResults \
CLASS::getContextSubscriptionResults(const std::string& objectID) { \
    return myContextSubscriptionResults[objectID]; \
}


#define LIBSUMO_GET_PARAMETER_WITH_KEY_API \
static const std::pair<std::string, std::string> getParameterWithKey(const std::string& objectID, const std::string& key);

#define LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(CLASS) \
const std::pair<std::string, std::string> \
CLASS::getParameterWithKey(const std::string& objectID, const std::string& key) { \
    return std::make_pair(key, getParameter(objectID, key)); \
}


// ===========================================================================
// class and type definitions
// ===========================================================================
namespace libsumo {
/**
* @class TraCIException
*/
class TraCIException : public std::runtime_error {
public:
    /** constructor */
    TraCIException(std::string what)
        : std::runtime_error(what) {}
};

/// @name Structures definitions
/// @{

struct TraCIResult {
    virtual ~TraCIResult() {}
    virtual std::string getString() {
        return "";
    }
};

/** @struct TraCIPosition
    * @brief A 3D-position
    */
struct TraCIPosition : TraCIResult {
    std::string getString() {
        std::ostringstream os;
        os << "TraCIPosition(" << x << "," << y << "," << z << ")";
        return os.str();
    }
    double x = INVALID_DOUBLE_VALUE, y = INVALID_DOUBLE_VALUE, z = INVALID_DOUBLE_VALUE;
};

/** @struct TraCIRoadPosition
    * @brief An edgeId, position and laneIndex
    */
struct TraCIRoadPosition : TraCIResult {
    std::string getString() {
        std::ostringstream os;
        os << "TraCIRoadPosition(" << edgeID << "_" << laneIndex << "," << pos << ")";
        return os.str();
    }
    std::string edgeID;
    double pos;
    int laneIndex = INVALID_INT_VALUE;
};

/** @struct TraCIColor
    * @brief A color
    */
struct TraCIColor : TraCIResult {
    TraCIColor() : r(0), g(0), b(0), a(255) {}
    TraCIColor(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a) {}
    std::string getString() {
        std::ostringstream os;
        os << "TraCIColor(" << r << "," << g << "," << b << "," << a << ")";
        return os.str();
    }
    int r, g, b, a;
};


/** @struct TraCILeaderDistance
 * @brief A leaderId and distance to leader
 */
struct TraCILeaderDistance : TraCIResult {
    std::string getString() {
        std::ostringstream os;
        os << "TraCILeaderDistance(" << leaderID << "," << dist << ")";
        return os.str();
    }
    std::string leaderID;
    double dist;
};


/** @struct TraCIPositionVector
    * @brief A list of positions
    */
typedef std::vector<TraCIPosition> TraCIPositionVector;


struct TraCIInt : TraCIResult {
    TraCIInt() : value(0) {}
    TraCIInt(int v) : value(v) {}
    std::string getString() {
        std::ostringstream os;
        os << value;
        return os.str();
    }
    int value;
};


struct TraCIDouble : TraCIResult {
    TraCIDouble() : value(0.) {}
    TraCIDouble(double v) : value(v) {}
    std::string getString() {
        std::ostringstream os;
        os << value;
        return os.str();
    }
    double value;
};


struct TraCIString : TraCIResult {
    TraCIString() : value("") {}
    TraCIString(std::string v) : value(v) {}
    std::string getString() {
        return value;
    }
    std::string value;
};


struct TraCIStringList : TraCIResult {
    std::string getString() {
        std::ostringstream os;
        os << "[";
        for (std::string v : value) {
            os << v << ",";
        }
        os << "]";
        return os.str();
    }
    std::vector<std::string> value;
};


/// @brief {variable->value}
typedef std::map<int, std::shared_ptr<TraCIResult> > TraCIResults;
/// @brief {object->{variable->value}}
typedef std::map<std::string, TraCIResults> SubscriptionResults;
typedef std::map<std::string, SubscriptionResults> ContextSubscriptionResults;


class TraCIPhase {
public:
    TraCIPhase() {}
    TraCIPhase(const double _duration, const std::string& _state, const double _minDur = libsumo::INVALID_DOUBLE_VALUE,
               const double _maxDur = libsumo::INVALID_DOUBLE_VALUE,
               const std::vector<int>& _next = std::vector<int>(),
               const std::string& _name = "") :
        duration(_duration), state(_state), minDur(_minDur), maxDur(_maxDur), next(_next), name(_name) {}
    ~TraCIPhase() {}

    double duration;
    std::string state;
    double minDur, maxDur;
    std::vector<int> next;
    std::string name;
};
}


#ifdef SWIG
%template(TraCIPhaseVector) std::vector<libsumo::TraCIPhase*>; // *NOPAD*
#endif


namespace libsumo {
class TraCILogic {
public:
    TraCILogic() {}
    TraCILogic(const std::string& _programID, const int _type, const int _currentPhaseIndex,
               const std::vector<libsumo::TraCIPhase*>& _phases = std::vector<libsumo::TraCIPhase*>())
        : programID(_programID), type(_type), currentPhaseIndex(_currentPhaseIndex), phases(_phases) {}
    ~TraCILogic() {}

    std::string programID;
    int type;
    int currentPhaseIndex;
    std::vector<TraCIPhase*> phases;
    std::map<std::string, std::string> subParameter;
};


class TraCILink {
public:
    TraCILink(const std::string& _from, const std::string& _via, const std::string& _to)
        : fromLane(_from), viaLane(_via), toLane(_to) {}
    ~TraCILink() {}

    std::string fromLane;
    std::string viaLane;
    std::string toLane;
};


class TraCIConnection {
public:
    TraCIConnection() {} // this is needed by SWIG when building a vector of this type, please don't use it
    TraCIConnection(const std::string& _approachedLane, const bool _hasPrio, const bool _isOpen, const bool _hasFoe,
                    const std::string _approachedInternal, const std::string _state, const std::string _direction, const double _length)
        : approachedLane(_approachedLane), hasPrio(_hasPrio), isOpen(_isOpen), hasFoe(_hasFoe),
          approachedInternal(_approachedInternal), state(_state), direction(_direction), length(_length) {}
    ~TraCIConnection() {}

    std::string approachedLane;
    bool hasPrio;
    bool isOpen;
    bool hasFoe;
    std::string approachedInternal;
    std::string state;
    std::string direction;
    double length;
};


/// @brief mirrors MSInductLoop::VehicleData
struct TraCIVehicleData {
    /// @brief The id of the vehicle
    std::string id;
    /// @brief Length of the vehicle
    double length;
    /// @brief Entry-time of the vehicle in [s]
    double entryTime;
    /// @brief Leave-time of the vehicle in [s]
    double leaveTime;
    /// @brief Type of the vehicle in
    std::string typeID;
};


struct TraCINextTLSData {
    /// @brief The id of the next tls
    std::string id;
    /// @brief The tls index of the controlled link
    int tlIndex;
    /// @brief The distance to the tls
    double dist;
    /// @brief The current state of the tls
    char state;
};


struct TraCINextStopData : TraCIResult {
    std::string getString() {
        std::ostringstream os;
        os << "TraCINextStopData(" << lane << "," << endPos << "," << stoppingPlaceID << "," << stopFlags << "," << duration << "," << until << ")";
        return os.str();
    }

    /// @brief The lane to stop at
    std::string lane;
    /// @brief The stopping position end
    double endPos;
    /// @brief Id assigned to the stop
    std::string stoppingPlaceID;
    /// @brief Stop flags
    int stopFlags;
    /// @brief The stopping duration
    double duration;
    /// @brief The time at which the vehicle may continue its journey
    double until;
};


/** @struct TraCINextStopDataVector
 * @brief A list of vehicle stops
 * @see TraCINextStopData
 */
struct TraCINextStopDataVector : TraCIResult {
    std::string getString() {
        std::ostringstream os;
        os << "TraCINextStopDataVector[";
        for (TraCINextStopData v : value) {
            os << v.getString() << ",";
        }
        os << "]";
        return os.str();
    }

    std::vector<TraCINextStopData> value;
};


struct TraCIBestLanesData {
    /// @brief The id of the lane
    std::string laneID;
    /// @brief The length than can be driven from that lane without lane change
    double length;
    /// @brief The traffic density along length
    double occupation;
    /// @brief The offset of this lane from the best lane
    int bestLaneOffset;
    /// @brief Whether this lane allows continuing the route
    bool allowsContinuation;
    /// @brief The sequence of lanes that best allows continuing the route without lane change
    std::vector<std::string> continuationLanes;
};


class TraCIStage {
public:
    TraCIStage(int type = INVALID_INT_VALUE, const std::string& vType = "", const std::string& line = "", const std::string& destStop = "",
               const std::vector<std::string>& edges = std::vector<std::string>(),
               double travelTime = INVALID_DOUBLE_VALUE, double cost = INVALID_DOUBLE_VALUE, double length = INVALID_DOUBLE_VALUE,
               const std::string& intended = "", double depart = INVALID_DOUBLE_VALUE, double departPos = INVALID_DOUBLE_VALUE,
               double arrivalPos = INVALID_DOUBLE_VALUE, const std::string& description = "") :
        type(type), vType(vType), line(line), destStop(destStop), edges(edges), travelTime(travelTime), cost(cost),
        length(length), intended(intended), depart(depart), departPos(departPos), arrivalPos(arrivalPos), description(description) {}
    /// @brief The type of stage (walking, driving, ...)
    int type;
    /// @brief The vehicle type when using a private car or bike
    std::string vType;
    /// @brief The line or the id of the vehicle type
    std::string line;
    /// @brief The id of the destination stop
    std::string destStop;
    /// @brief The sequence of edges to travel
    std::vector<std::string> edges;
    /// @brief duration of the stage in seconds
    double travelTime;
    /// @brief effort needed
    double cost;
    /// @brief length in m
    double length;
    /// @brief id of the intended vehicle for public transport ride
    std::string intended;
    /// @brief intended depart time for public transport ride or INVALID_DOUBLE_VALUE
    double depart;
    /// @brief position on the lane when starting the stage
    double departPos;
    /// @brief position on the lane when ending the stage
    double arrivalPos;
    /// @brief arbitrary description string
    std::string description;
};
}
