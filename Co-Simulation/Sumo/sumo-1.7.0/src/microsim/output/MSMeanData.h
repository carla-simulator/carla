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
/// @file    MSMeanData.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Tue, 17.11.2009
///
// Data collector for edges/lanes
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <list>
#include <limits>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/MSMoveReminder.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class MSEdge;
class MSLane;
class SUMOTrafficObject;

typedef std::vector<MSEdge*> MSEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSMeanData
 * @brief Data collector for edges/lanes
 *
 * This structure does not contain the data itself, it is stored within
 *  MeanDataValues-MoveReminder objects.
 * This class is used to build the output, optionally, in the case
 *  of edge-based dump, aggregated over the edge's lanes.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSMeanData : public MSDetectorFileOutput {
public:
    /**
     * @class MeanDataValues
     * @brief Data structure for mean (aggregated) edge/lane values
     *
     * Structure holding values that describe the emissions (XXX: emissions?) aggregated, refs. #2579
     *  over some seconds.
     */
    class MeanDataValues : public MSMoveReminder {
    public:
        /** @brief Constructor */
        MeanDataValues(MSLane* const lane, const double length, const bool doAdd, const MSMeanData* const parent);

        /** @brief Destructor */
        virtual ~MeanDataValues();


        /** @brief Resets values so they may be used for the next interval
         */
        virtual void reset(bool afterWrite = false) = 0;

        /** @brief Add the values of this to the given one and store them there
         *
         * @param[in] val The meandata to add to
         */
        virtual void addTo(MeanDataValues& val) const = 0;


        /** @brief Called if the vehicle enters the reminder's lane
         *
         * @param[in] veh The entering vehicle.
         * @param[in] reason how the vehicle enters the lane
         * @see MSMoveReminder
         * @see MSMoveReminder::notifyEnter
         * @see MSMoveReminder::Notification
         */
        virtual bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


        /** @brief Checks whether the reminder still has to be notified about the vehicle moves
         *
         * Indicator if the reminders is still active for the passed
         * vehicle/parameters. If false, the vehicle will erase this reminder
         * from it's reminder-container.
         *
         * @param[in] veh Vehicle that asks this reminder.
         * @param[in] oldPos Position before move.
         * @param[in] newPos Position after move with newSpeed.
         * @param[in] newSpeed Moving speed.
         *
         * @return True if vehicle hasn't passed the reminder completely.
         */
        bool notifyMove(SUMOTrafficObject& veh, double oldPos,
                        double newPos, double newSpeed);


        /** @brief Called if the vehicle leaves the reminder's lane
         *
         * @param veh The leaving vehicle.
         * @param[in] lastPos Position on the lane when leaving.
         * @param[in] reason how the vehicle leaves the lane
         * @see MSMoveReminder
         * @see MSMoveReminder::notifyLeave
         */
        virtual bool notifyLeave(SUMOTrafficObject& veh, double lastPos,
                                 MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


        /** @brief Returns whether any data was collected.
         *
         * @return whether no data was collected
         */
        virtual bool isEmpty() const;


        /** @brief Called if a per timestep update is needed. Default does nothing.
         */
        virtual void update();

        /** @brief Writes output values into the given stream
         *
         * @param[in] dev The output device to write the data into
         * @param[in] period Length of the period the data were gathered
         * @param[in] numLanes The total number of lanes for which the data was collected
         * @exception IOError If an error on writing occurs (!!! not yet implemented)
         */
        virtual void write(OutputDevice& dev, long long int attributeMask, const SUMOTime period,
                           const double numLanes, const double defaultTravelTime,
                           const int numVehicles = -1) const = 0;

        /** @brief Returns the number of collected sample seconds.
        * @return the number of collected sample seconds
        */
        virtual double getSamples() const;

        /** @brief Returns the total travelled distance.
        * @return the total travelled distance
        */
        double getTravelledDistance() const {
            return travelledDistance;
        }

        /// @brief write attribute if it passed the attribute mask check
        template <class T>
        static void checkWriteAttribute(OutputDevice& dev, long long int attributeMask, const SumoXMLAttr attr, const T& val) {
            if (attributeMask == 0 || attributeMask & ((long long int)1 << attr)) {
                dev.writeAttr(attr, val);
            }
        }

    protected:
        /// @brief The meandata parent
        const MSMeanData* const myParent;

        /// @brief The length of the lane / edge the data collector is on
        const double myLaneLength;

        /// @name Collected values
        /// @{
        /// @brief The number of sampled vehicle movements (in s)
        double sampleSeconds;

        /// @brief The sum of the distances the vehicles travelled
        double travelledDistance;
        //@}

    };


    /**
     * @class MeanDataValueTracker
     * @brief Data structure for mean (aggregated) edge/lane values for tracked vehicles
     */
    class MeanDataValueTracker : public MeanDataValues {
    public:
        /** @brief Constructor */
        MeanDataValueTracker(MSLane* const lane, const double length,
                             const MSMeanData* const parent);

        /** @brief Destructor */
        virtual ~MeanDataValueTracker();

        /** @brief Resets values so they may be used for the next interval
         */
        void reset(bool afterWrite);

        /** @brief Add the values of this to the given one and store them there
         *
         * @param[in] val The meandata to add to
         */
        void addTo(MSMeanData::MeanDataValues& val) const;

        /// @name Methods inherited from MSMoveReminder
        /// @{

        /** @brief Internal notification about the vehicle moves
         *  @see MSMoveReminder::notifyMoveInternal().
         */
        void notifyMoveInternal(const SUMOTrafficObject& veh, const double frontOnLane, const double timeOnLane, const double meanSpeedFrontOnLane, const double meanSpeedVehicleOnLane, const double travelledDistanceFrontOnLane, const double travelledDistanceVehicleOnLane, const double meanLengthOnLane);


        /** @brief Called if the vehicle leaves the reminder's lane
         *
         * @param veh The leaving vehicle.
         * @param[in] lastPos Position on the lane when leaving.
         * @param[in] isArrival whether the vehicle arrived at its destination
         * @param[in] isLaneChange whether the vehicle changed from the lane
         * @see MSMoveReminder
         * @see MSMoveReminder::notifyLeave
         */
        bool notifyLeave(SUMOTrafficObject& veh, double lastPos, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


        /** @brief Computes current values and adds them to their sums
         *
         * The fraction of time the vehicle is on the lane is computed and
         *  used as a weight for the vehicle's current values.
         *  The "emitted" field is incremented, additionally.
         *
         * @param[in] veh The entering vehicle.
         * @param[in] reason how the vehicle enters the lane
         * @see MSMoveReminder::notifyEnter
         * @return Always true
         */
        bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
        //@}

        bool isEmpty() const;

        /** @brief Writes output values into the given stream
         *
         * @param[in] dev The output device to write the data into
         * @param[in] period Length of the period the data were gathered
         * @param[in] numLanes The total number of lanes for which the data was collected
         * @exception IOError If an error on writing occurs (!!! not yet implemented)
         */
        void write(OutputDevice& dev, long long int attributeMask, const SUMOTime period,
                   const double numLanes, const double defaultTravelTime,
                   const int numVehicles = -1) const;

        int getNumReady() const;

        void clearFirst();

        double getSamples() const;

    private:
        class TrackerEntry {
        public:
            /** @brief Constructor */
            TrackerEntry(MeanDataValues* const values)
                : myNumVehicleEntered(0), myNumVehicleLeft(0), myValues(values) {}

            /** @brief Constructor */
            virtual ~TrackerEntry() {
                delete myValues;
            }

            /// @brief The number of vehicles which entered in the current interval
            int myNumVehicleEntered;

            /// @brief The number of vehicles which left in the current interval
            int myNumVehicleLeft;

            /// @brief The number of vehicles which left in the current interval
            MeanDataValues* myValues;
        };

        /// @brief The map of vehicles to data entries
        std::map<const SUMOTrafficObject*, TrackerEntry*> myTrackedData;

        /// @brief The currently active meandata "intervals"
        std::list<TrackerEntry*> myCurrentData;

    };


public:
    /** @brief Constructor
     *
     * @param[in] id The id of the detector
     * @param[in] dumpBegin Begin time of dump
     * @param[in] dumpEnd End time of dump
     * @param[in] useLanes Information whether lane-based or edge-based dump shall be generated
     * @param[in] withEmpty Information whether empty lanes/edges shall be written
     * @param[in] withInternal Information whether internal lanes/edges shall be written
     * @param[in] trackVehicles Information whether vehicles shall be tracked
     * @param[in] detectPersons Whether pedestrians shall be detected instead of vehicles
     * @param[in] maxTravelTime the maximum travel time to use when calculating per vehicle output
     * @param[in] defaultEffort the value to use when calculating defaults
     * @param[in] minSamples the minimum number of sample seconds before the values are valid
     * @param[in] vTypes the set of vehicle types to consider
     */
    MSMeanData(const std::string& id,
               const SUMOTime dumpBegin, const SUMOTime dumpEnd,
               const bool useLanes, const bool withEmpty,
               const bool printDefaults, const bool withInternal,
               const bool trackVehicles, const int detectPersons,
               const double minSamples,
               const double maxTravelTime,
               const std::string& vTypes,
               const std::string& writeAttributes);


    /// @brief Destructor
    virtual ~MSMeanData();

    /** @brief Adds the value collectors to all relevant edges.
     */
    void init();

    /// @name Methods inherited from MSDetectorFileOutput.
    /// @{

    /** @brief Writes collected values into the given stream
     *
     * At first, it is checked whether the values for the current interval shall be written.
     *  If not, a reset is performed, only, using "resetOnly". Otherwise,
     *  both the list of single-lane edges and the list of multi-lane edges
     *  are gone through and each edge is written using "writeEdge".
     *
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @see MSDetectorFileOutput::writeXMLOutput
     * @see write
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLOutput(OutputDevice& dev, SUMOTime startTime, SUMOTime stopTime);

    /** @brief Opens the XML-output using "netstats" as root element
     *
     * @param[in] dev The output device to write the root into
     * @see MSDetectorFileOutput::writeXMLDetectorProlog
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    virtual void writeXMLDetectorProlog(OutputDevice& dev) const;
    /// @}

    /** @brief Updates the detector
     */
    virtual void detectorUpdate(const SUMOTime step);

    double getMinSamples() const {
        return myMinSamples;
    }

    double getMaxTravelTime() const {
        return myMaxTravelTime;
    }

    bool isEdgeData() const {
        return myAmEdgeBased;
    }


protected:
    /** @brief Create an instance of MeanDataValues
     *
     * @param[in] lane The lane to create for
     * @param[in] doAdd whether to add the values as reminder to the lane
     */
    virtual MSMeanData::MeanDataValues* createValues(MSLane* const lane, const double length, const bool doAdd) const = 0;

    /** @brief Resets network value in order to allow processing of the next interval
     *
     * Goes through the lists of edges and starts "resetOnly" for each edge.
     * @param[in] edge The last time step that is reported
     */
    void resetOnly(SUMOTime stopTime);

    /** @brief Return the relevant edge id
     *
     * @param[in] edge The edge to retrieve the id for
     */
    virtual std::string getEdgeID(const MSEdge* const edge);

    /** @brief Writes edge values into the given stream
     *
     * microsim: It is checked whether the dump shall be generated edge-
     *  or lane-wise. In the first case, the lane-data are collected
     *  and aggregated and written directly. In the second case, "writeLane"
     *  is used to write each lane's state.
     *
     * @param[in] dev The output device to write the data into
     * @param[in] edgeValues List of this edge's value collectors
     * @param[in] edge The edge to write the dump of
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeEdge(OutputDevice& dev, const std::vector<MeanDataValues*>& edgeValues,
                   MSEdge* edge, SUMOTime startTime, SUMOTime stopTime);

    /** @brief Writes the interval opener
     *
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     */
    virtual void openInterval(OutputDevice& dev, const SUMOTime startTime, const SUMOTime stopTime);

    /** @brief Checks for emptiness and writes prefix into the given stream
     *
     * @param[in] dev The output device to write the data into
     * @param[in] values The values to check for emptiness
     * @param[in] tag The xml tag to write (lane / edge)
     * @param[in] id The id for the lane / edge to write
     * @return whether further output should be generated
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    virtual bool writePrefix(OutputDevice& dev, const MeanDataValues& values,
                             const SumoXMLTag tag, const std::string id) const;

protected:
    /// @brief the minimum sample seconds
    const double myMinSamples;

    /// @brief the maximum travel time to write
    const double myMaxTravelTime;

    /// @brief Value collectors; sorted by edge, then by lane
    std::vector<std::vector<MeanDataValues*> > myMeasures;

    /// @brief Whether empty lanes/edges shall be written
    const bool myDumpEmpty;

private:
    static long long int initWrittenAttributes(const std::string writeAttributes, const std::string& id);

    /// @brief Information whether the output shall be edge-based (not lane-based)
    const bool myAmEdgeBased;

    /// @brief The first and the last time step to write information (-1 indicates always)
    const SUMOTime myDumpBegin, myDumpEnd;

    /// @brief The corresponding first edges
    MSEdgeVector myEdges;

    /// @brief Whether empty lanes/edges shall be written
    const bool myPrintDefaults;

    /// @brief Whether internal lanes/edges shall be written
    const bool myDumpInternal;

    /// @brief Whether vehicles are tracked
    const bool myTrackVehicles;

    /// @brief bit mask for checking attributes to be written
    const long long int myWrittenAttributes;

    /// @brief The intervals for which output still has to be generated (only in the tracking case)
    std::list< std::pair<SUMOTime, SUMOTime> > myPendingIntervals;

private:
    /// @brief Invalidated copy constructor.
    MSMeanData(const MSMeanData&);

    /// @brief Invalidated assignment operator.
    MSMeanData& operator=(const MSMeanData&);

};
