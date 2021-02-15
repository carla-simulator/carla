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
/// @file    MSPerson.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// The class for modelling person-movements
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <set>
#include <utils/common/SUMOTime.h>
#include <utils/common/Command.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <microsim/transportables/MSTransportable.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSEdge;
class MSLane;
class OutputDevice;
class SUMOVehicleParameter;
class MSStoppingPlace;
class SUMOVehicle;
class MSVehicleType;
class MSPModel;
class MSTransportableStateAdapter;

typedef std::vector<const MSEdge*> ConstMSEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
/**
  * @class MSPerson
  *
  * The class holds a simulated person together with her movement stages
  */
class MSPerson : public MSTransportable {
public:
    /**
     * A "real" stage performing the walking to an edge
     * The walking does not need any route as it is not simulated.
     * Only the duration is needed
     */
    class MSPersonStage_Walking : public MSStageMoving {
    public:
        /// constructor
        MSPersonStage_Walking(const std::string& personID, const ConstMSEdgeVector& route, MSStoppingPlace* toStop, SUMOTime walkingTime,
                              double speed, double departPos, double arrivalPos, double departPosLat);

        /// destructor
        ~MSPersonStage_Walking();

        MSStage* clone() const;

        /// proceeds to the next step
        virtual void proceed(MSNet* net, MSTransportable* person, SUMOTime now, MSStage* previous);

        /// abort this stage (TraCI)
        void abort(MSTransportable*);

        /// sets the walking speed (ignored in other stages)
        void setSpeed(double speed);

        /// @brief get travel distance in this stage
        double getDistance() const {
            return walkDistance();
        }

        std::string getStageDescription(const bool isPerson) const {
            UNUSED_PARAMETER(isPerson);
            return "walking";
        }

        std::string getStageSummary(const bool isPerson) const;

        /** @brief Called on writing tripinfo output
         * @param[in] os The stream to write the information into
         * @exception IOError not yet implemented
         */
        virtual void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const;

        /** @brief Called on writing vehroute output
         * @param[in] os The stream to write the information into
         * @param[in] withRouteLength whether route length shall be written
         * @exception IOError not yet implemented
         */
        virtual void routeOutput(const bool isPerson, OutputDevice& os, const bool withRouteLength, const MSStage* const previous) const;

        /// @brief move forward and return whether the person arrived
        bool moveToNextEdge(MSTransportable* person, SUMOTime currentTime, MSEdge* nextInternal = nullptr);

        /// @brief accessors to be used by MSPModel
        //@{
        double getMaxSpeed(const MSTransportable* const person) const;

        inline double getArrivalPos() const {
            return myArrivalPos;
        }

        inline const MSEdge* getNextRouteEdge() const {
            return myRouteStep == myRoute.end() - 1 ? nullptr : *(myRouteStep + 1);
        }
        //@}


    private:

        /// @brief compute total walking distance
        double walkDistance() const;

        /* @brief compute average speed if the total walking duration is given
         * @note Must be called when the previous stage changes myDepartPos from the default*/
        double computeAverageSpeed() const;


    private:
        /// the time the person is walking
        SUMOTime myWalkingTime;

        /// the time the person entered the edge
        SUMOTime myLastEdgeEntryTime;

        class arrival_finder {
        public:
            /// constructor
            explicit arrival_finder(SUMOTime time) : myTime(time) {}

            /// comparison operator
            bool operator()(double t) const {
                return myTime > t;
            }

        private:
            /// the searched arrival time
            SUMOTime myTime;
        };

    private:
        /// @brief Invalidated copy constructor.
        MSPersonStage_Walking(const MSPersonStage_Walking&);

        /// @brief Invalidated assignment operator.
        MSPersonStage_Walking& operator=(const MSPersonStage_Walking&);

    };

    /**
     * An intermediate stage performing the access from or to public transport as given
     * by the access elements of the public transport stop. The travel time is computed by the simulation
     */
    class MSPersonStage_Access : public MSStage {
    public:
        /// constructor
        MSPersonStage_Access(const MSEdge* destination, MSStoppingPlace* toStop,
                             const double arrivalPos, const double dist, const bool isExit);

        /// destructor
        ~MSPersonStage_Access();

        MSStage* clone() const;

        /// proceeds to the next step
        virtual void proceed(MSNet* net, MSTransportable* person, SUMOTime now, MSStage* previous);

        /// @brief returns the stage description as a string
        std::string getStageDescription(const bool isPerson) const;
        std::string getStageSummary(const bool isPerson) const;

        Position getPosition(SUMOTime now) const;

        double getAngle(SUMOTime now) const;

        /// @brief get travel distance in this stage
        double getDistance() const {
            return myDist;
        }

        /** @brief Called on writing tripinfo output
        *
        * @param[in] os The stream to write the information into
        * @param[in] transportable The person to write information about
        * @exception IOError not yet implemented
        */
        void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const;

        /// @brief Called on writing vehroute output. Currently does nothing.
        void routeOutput(const bool, OutputDevice&, const bool, const MSStage* const) const {};

    private:
        class ProceedCmd : public Command {
        public:
            ProceedCmd(MSTransportable* person, MSEdge* edge) : myPerson(person), myStopEdge(edge) {}
            ~ProceedCmd() {}
            SUMOTime execute(SUMOTime currentTime);
        private:
            MSTransportable* const myPerson;
            MSEdge* myStopEdge;
        private:
            /// @brief Invalidated assignment operator.
            ProceedCmd& operator=(const ProceedCmd&);
        };

    private:
        const double myDist;
        const bool myAmExit;
        SUMOTime myEstimatedArrival;
        PositionVector myPath;
    };

public:
    /// constructor
    MSPerson(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan, const double speedFactor);

    /// destructor
    virtual ~MSPerson();

    void checkAccess(const MSStage* const prior, const bool isDisembark = true);

    /// @brief return the list of internal edges if this person is walking and the pedestrian model allows it
    const std::string& getNextEdge() const;

    /// @brief returns the next edge ptr if this person is walking and the pedestrian model allows it
    const MSEdge* getNextEdgePtr() const;

    /// @brief whether this person is selected in the GUI
    virtual bool isSelected() const {
        return false;
    }

    inline double getSpeedFactor() const {
        return myChosenSpeedFactor;
    }

    /// @brief set new walk and replace the stages with relative indices in the interval [firstIndex, nextIndex[
    void reroute(ConstMSEdgeVector& newEdges, double departPos, int firstIndex, int nextIndex);


    /** @class Influencer
     * @brief Changes the wished person speed and position
     *
     * The class is used for passing velocities or positions obtained via TraCI to the person.
     */
    class Influencer {
    public:
        /// @brief Constructor
        Influencer();


        /// @brief Destructor
        ~Influencer();


        void setRemoteControlled(Position xyPos, MSLane* l, double pos, double posLat, double angle, int edgeOffset, const ConstMSEdgeVector& route, SUMOTime t);

        SUMOTime getLastAccessTimeStep() const {
            return myLastRemoteAccess;
        }

        void postProcessRemoteControl(MSPerson* p);

        bool isRemoteControlled() const;

        bool isRemoteAffected(SUMOTime t) const;

    private:
        Position myRemoteXYPos;
        MSLane* myRemoteLane;
        double myRemotePos;
        double myRemotePosLat;
        double myRemoteAngle;
        int myRemoteEdgeOffset;
        ConstMSEdgeVector myRemoteRoute;
        SUMOTime myLastRemoteAccess;
    };


    /** @brief Returns the velocity/lane influencer
     *
     * If no influencer was existing before, one is built, first
     * @return Reference to this vehicle's speed influencer
     */
    Influencer& getInfluencer();

    const Influencer* getInfluencer() const;

    bool hasInfluencer() const {
        return myInfluencer != 0;
    }

    /// @brief sets position outside the road network
    void setRemoteState(Position xyPos);

private:
    /// @brief An instance of a speed/position influencing instance; built in "getInfluencer"
    Influencer* myInfluencer;

    const double myChosenSpeedFactor;

private:
    /// @brief Invalidated copy constructor.
    MSPerson(const MSPerson&);

    /// @brief Invalidated assignment operator.
    MSPerson& operator=(const MSPerson&);

};
