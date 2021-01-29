/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    ROPerson.h
/// @author  Robert Hilbrich
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A person as used by router
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <iostream>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOTime.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include "RORoutable.h"
#include "RORouteDef.h"
#include "ROVehicle.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class ROEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROPerson
 * @brief A person as used by router
 */
class ROPerson : public RORoutable {

public:
    /** @brief Constructor
     *
     * @param[in] pars Parameter of this person
     * @param[in] type The type of the person
     */
    ROPerson(const SUMOVehicleParameter& pars, const SUMOVTypeParameter* type);

    /// @brief Destructor
    virtual ~ROPerson();

    void addTrip(const ROEdge* const from, const ROEdge* const to, const SVCPermissions modeSet,
                 const std::string& vTypes, const double departPos, const double arrivalPos,
                 const std::string& busStop, double walkFactor, const std::string& group);

    void addRide(const ROEdge* const from, const ROEdge* const to, const std::string& lines,
                 double arrivalPos, const std::string& destStop, const std::string& group);

    void addWalk(const ConstROEdgeVector& edges, const double duration, const double speed,
                 const double departPos, const double arrivalPos, const std::string& busStop);

    void addStop(const SUMOVehicleParameter::Stop& stopPar, const ROEdge* const stopEdge);

    class TripItem;
    /**
     * @brief Every person has a plan comprising of multiple planItems
     *
     */
    class PlanItem {
    public:
        /// @brief Destructor
        virtual ~PlanItem() {}

        virtual PlanItem* clone() const = 0;

        virtual void addTripItem(TripItem* /* tripIt */) {
            throw ProcessError();
        }
        virtual const ROEdge* getOrigin() const = 0;
        virtual const ROEdge* getDestination() const = 0;
        virtual double getDestinationPos() const = 0;
        virtual void saveVehicles(OutputDevice& /* os */, OutputDevice* const /* typeos */, bool /* asAlternatives */, OptionsCont& /* options */) const {}
        virtual void saveAsXML(OutputDevice& os, const bool extended, const bool asTrip, const bool writeGeoTrip) const = 0;
        virtual bool isStop() const {
            return false;
        }
        virtual bool needsRouting() const {
            return false;
        }

        virtual SUMOTime getDuration() const = 0;
    };

    /**
     * @brief A planItem can be a Stop
     *
     */
    class Stop : public PlanItem {
    public:
        Stop(const SUMOVehicleParameter::Stop& stop, const ROEdge* const stopEdge)
            : stopDesc(stop), edge(stopEdge) {}

        PlanItem* clone() const {
            return new Stop(stopDesc, edge);
        }

        const ROEdge* getOrigin() const {
            return edge;
        }
        const ROEdge* getDestination() const {
            return edge;
        }
        double getDestinationPos() const {
            return (stopDesc.startPos + stopDesc.endPos) / 2;
        }
        void saveAsXML(OutputDevice& os, const bool /* extended */, const bool /*asTrip*/, const bool /*writeGeoTrip*/) const {
            stopDesc.write(os);
        }
        bool isStop() const {
            return true;
        }
        SUMOTime getDuration() const {
            return stopDesc.duration;
        }

    private:
        SUMOVehicleParameter::Stop stopDesc;
        const ROEdge* const edge;

    private:
        /// @brief Invalidated assignment operator
        Stop& operator=(const Stop& src);

    };

    /**
     * @brief A TripItem is part of a trip, e.g., go from here to here by car
     *
     */
    class TripItem {
    public:
        TripItem(const double _cost)
            : cost(_cost) {}

        /// @brief Destructor
        virtual ~TripItem() {}

        virtual TripItem* clone() const = 0;

        virtual const ROEdge* getOrigin() const = 0;
        virtual const ROEdge* getDestination() const = 0;
        virtual double getDestinationPos() const = 0;
        virtual void saveAsXML(OutputDevice& os, const bool extended) const = 0;
        SUMOTime getDuration() const {
            return TIME2STEPS(cost);
        }

        double getCost() const {
            return cost;
        }
    protected:
        double cost;
    };

    /**
     * @brief A ride is part of a trip, e.g., go from here to here by car or bus
     *
     */
    class Ride : public TripItem {
    public:
        Ride(const ROEdge* const _from, const ROEdge* const _to,
             const std::string& _lines, const std::string& _group, const double _cost, const double arrivalPos,
             const std::string& _destStop = "", const std::string& _intended = "", const SUMOTime _depart = -1) :
            TripItem(_cost),
            from(_from), to(_to),
            lines(_lines),
            group(_group),
            destStop(_destStop),
            intended(_intended),
            depart(_depart),
            arr(arrivalPos) {
        }

        TripItem* clone() const {
            return new Ride(from, to, lines, group, cost, arr, destStop, intended, depart);
        }

        const ROEdge* getOrigin() const {
            return from;
        }
        const ROEdge* getDestination() const {
            return to;
        }
        double getDestinationPos() const {
            return arr == std::numeric_limits<double>::infinity() ? -NUMERICAL_EPS : arr;
        }
        void saveAsXML(OutputDevice& os, const bool extended) const;

    private:
        const ROEdge* const from;
        const ROEdge* const to;
        const std::string lines;
        const std::string group;
        const std::string destStop;
        const std::string intended;
        const SUMOTime depart;
        const double arr;

    private:
        /// @brief Invalidated assignment operator
        Ride& operator=(const Ride& src);

    };

    /**
     * @brief A walk is part of a trip, e.g., go from here to here by foot
     *
     */
    class Walk : public TripItem {
    public:
        Walk(const ConstROEdgeVector& _edges, const double _cost,
             double departPos = std::numeric_limits<double>::infinity(),
             double arrivalPos = std::numeric_limits<double>::infinity(),
             const std::string& _destStop = "")
            : TripItem(_cost), edges(_edges), dur(-1), v(-1), dep(departPos), arr(arrivalPos), destStop(_destStop) {}
        Walk(const ConstROEdgeVector& edges, const double _cost, const double duration, const double speed,
             const double departPos, const double arrivalPos, const std::string& _destStop)
            : TripItem(_cost), edges(edges), dur(duration), v(speed), dep(departPos), arr(arrivalPos), destStop(_destStop) {}

        TripItem* clone() const {
            return new Walk(edges, cost, dep, arr, destStop);
        }

        const ROEdge* getOrigin() const {
            return edges.front();
        }
        const ROEdge* getDestination() const {
            return edges.back();
        }
        double getDestinationPos() const {
            return arr == std::numeric_limits<double>::infinity() ? 0 : arr;
        }
        void saveAsXML(OutputDevice& os, const bool extended) const;

    private:
        const ConstROEdgeVector edges;
        const double dur, v, dep, arr;
        const std::string destStop;

    private:
        /// @brief Invalidated assignment operator
        Walk& operator=(const Walk& src);

    };

    /**
     * @brief A planItem can be a Trip which contains multiple tripItems
     *
     */
    class PersonTrip : public PlanItem {
    public:
        PersonTrip()
            : from(0), to(0), modes(SVC_PEDESTRIAN), dep(0), arr(0), stopDest(""), walkFactor(1.0) {}
        PersonTrip(const ROEdge* const from, const ROEdge* const to, const SVCPermissions modeSet,
                   const double departPos, const double arrivalPos, const std::string& _stopDest, double _walkFactor, const std::string& _group)
            : from(from), to(to), modes(modeSet), dep(departPos), arr(arrivalPos), stopDest(_stopDest), walkFactor(_walkFactor), group(_group) {}
        /// @brief Destructor
        virtual ~PersonTrip() {
            for (std::vector<TripItem*>::const_iterator it = myTripItems.begin(); it != myTripItems.end(); ++it) {
                delete *it;
            }
            for (std::vector<ROVehicle*>::const_iterator it = myVehicles.begin(); it != myVehicles.end(); ++it) {
                delete (*it)->getRouteDefinition();
                delete *it;
            }
        }

        PlanItem* clone() const;

        virtual void addTripItem(TripItem* tripIt) {
            myTripItems.push_back(tripIt);
        }
        void addVehicle(ROVehicle* veh) {
            myVehicles.push_back(veh);
        }
        std::vector<ROVehicle*>& getVehicles() {
            return myVehicles;
        }
        const ROEdge* getOrigin() const {
            return from != 0 ? from : myTripItems.front()->getOrigin();
        }
        const ROEdge* getDestination() const {
            return to;
        }
        double getDestinationPos() const {
            if (myTripItems.empty()) {
                return getArrivalPos(true);
            } else {
                return myTripItems.back()->getDestinationPos();
            }
        }
        double getDepartPos(bool replaceDefault = true) const {
            return dep == std::numeric_limits<double>::infinity() && replaceDefault ? 0 : dep;
        }
        double getArrivalPos(bool replaceDefault = true) const {
            return arr == std::numeric_limits<double>::infinity() && replaceDefault ? 0 : arr;
        }
        SVCPermissions getModes() const {
            return modes;
        }

        const std::string& getGroup() const {
            return group;
        }

        const std::string& getStopDest() const {
            return stopDest;
        }
        virtual bool needsRouting() const {
            return myTripItems.empty();
        }
        void saveVehicles(OutputDevice& os, OutputDevice* const typeos, bool asAlternatives, OptionsCont& options) const;
        void saveAsXML(OutputDevice& os, const bool extended, const bool asTrip, const bool writeGeoTrip) const;

        double getWalkFactor() const {
            return walkFactor;
        }

        /// @brief return duration sum of all trip items
        SUMOTime getDuration() const;

    private:
        const ROEdge* from;
        const ROEdge* to;
        const SVCPermissions modes;
        const double dep, arr;
        const std::string stopDest;
        /// @brief the fully specified trips
        std::vector<TripItem*> myTripItems;
        /// @brief the vehicles which may be used for routing
        std::vector<ROVehicle*> myVehicles;
        /// @brief walking speed factor
        double walkFactor;
        /// @brief group id for travelling in groups
        const std::string group;

    private:
        /// @brief Invalidated assignment operator
        PersonTrip& operator=(const PersonTrip& src);

    };


    /** @brief Returns the first edge the person takes
     *
     * @return The person's departure edge
     */
    const ROEdge* getDepartEdge() const {
        return myPlan.front()->getOrigin();
    }


    void computeRoute(const RORouterProvider& provider,
                      const bool removeLoops, MsgHandler* errorHandler);


    /** @brief Saves the complete person description.
     *
     * Saves the person itself including the trips and stops.
     *
     * @param[in] os The routes or alternatives output device to store the routable's description into
     * @param[in] typeos The types - output device to store additional types into
     * @param[in] asAlternatives Whether the route shall be saved as route alternatives
     * @param[in] options to find out about defaults and whether exit times for the edges shall be written
     * @exception IOError If something fails (not yet implemented)
     */
    void saveAsXML(OutputDevice& os, OutputDevice* const typeos, bool asAlternatives, OptionsCont& options) const;

    std::vector<PlanItem*>& getPlan() {
        return myPlan;
    }

private:
    bool computeIntermodal(SUMOTime time, const RORouterProvider& provider,
                           PersonTrip* const trip, const ROVehicle* const veh, MsgHandler* const errorHandler);

private:
    /**
     * @brief The plan of the person
     */
    std::vector<PlanItem*> myPlan;


private:
    /// @brief Invalidated copy constructor
    ROPerson(const ROPerson& src);

    /// @brief Invalidated assignment operator
    ROPerson& operator=(const ROPerson& src);

};
