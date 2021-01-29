/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    ODMatrix.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    05. Apr. 2006
///
// An O/D (origin/destination) matrix
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <set>
#include <string>
#include <utils/common/SUMOTime.h>
#include "ODCell.h"
#include "ODDistrictCont.h"
#include <utils/distribution/Distribution_Points.h>
#include <utils/importio/LineReader.h>
#include <utils/common/SUMOTime.h>

// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class OutputDevice;
class SUMOSAXHandler;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ODMatrix
 * @brief An O/D (origin/destination) matrix
 *
 * This class is the internal representation of a loaded O/D-matrix. Beside
 *  being the storage for ODCells, the matrix also contains information about
 *  the numbers of loaded, discarded, and written vehicles.
 *
 * The matrix has a reference to the container of districts stored. This allows
 *  to validate added cell descriptions in means that using existing origins/
 *  destinations only is assured.
 *
 * In addition of being a storage, the matrix is also responsible for writing
 *  the results and contains methods for splitting the entries over time.
 */
class ODMatrix {
public:
    /** @brief Constructor
     *
     * @param[in] dc The district container to obtain referenced districts from
     */
    ODMatrix(const ODDistrictCont& dc);


    /// Destructor
    ~ODMatrix();


    /** @brief Builds a single cell from the given values, verifying them
     *
     * At first, the number of loaded vehicles (myNoLoaded) is incremented
     *  by vehicleNumber.
     *
     * It is checked whether both the origin and the destination exist within
     *  the assigned district container (myDistricts). If one of them is missing,
     *  an error is generated, if both, a warning, because in the later case
     *  the described flow may lay completely beside the processed area. In both
     *  cases the given number of vehicles (vehicleNumber) is added to myNoDiscarded.
     *
     * If the origin/destination districts are known, a cell is built using the
     *  given values. This cell is added to the list of known cells (myContainer).
     *
     * @param[in] vehicleNumber The number of vehicles to store within the cell
     * @param[in] begin The begin of the interval the cell is valid for
     * @param[in] end The end of the interval the cell is valid for
     * @param[in] origin The origin district to use for the cell's flows
     * @param[in] destination The destination district to use for the cell's flows
     * @param[in] vehicleType The vehicle type to use for the cell's flows
     * @return whether the cell could be added
     */
    bool add(double vehicleNumber, SUMOTime begin,
             SUMOTime end, const std::string& origin, const std::string& destination,
             const std::string& vehicleType,
             const bool originIsEdge = false, const bool destinationIsEdge = false);

    /** @brief Adds a single vehicle with departure time
     *
     * If there is no existing ODCell for the given parameters one is generated
     * using add(...)
     *
     * @param[in] id The id of the vehicle
     * @param[in] depart The departure time of the vehicle
     * @param[in] od The origin and destination district to use for the cell's flows
     * @param[in] vehicleType The vehicle type to use for the cell's flows
     * @return whether the vehicle could be added
     */
    bool add(const std::string& id, const SUMOTime depart,
             const std::string& fromTaz, const std::string& toTaz,
             const std::string& vehicleType,
             const bool originIsEdge = false, const bool destinationIsEdge = false);

    /** @brief Helper function for flow and trip output writing the depart
     *   and arrival attributes
     *
     * @param[in] dev The stream to write the generated vehicle trips to
     * @param[in] noVtype Whether vtype information shall not be written
     * @param[in] cell The OD cell containing the vtype
     */
    void writeDefaultAttrs(OutputDevice& dev, const bool noVtype,
                           const ODCell* const cell);

    /** @brief Writes the vehicles stored in the matrix assigning the sources and sinks
     *
     * The cells stored in myContainer are sorted, first. Then, for each time
     *  step to generate vehicles for, it is checked whether the topmost cell
     *  is valid for this time step. If so, vehicles are generated from this
     *  cell's description using "computeDeparts" and stored in an internal vector.
     *  The pointer is moved and the check is repeated until the current cell
     *  is not valid for the current time or no further cells exist.
     *
     * Then, for the current time step, the internal list of vehicles is sorted and
     *  all vehicles that start within this time step are written.
     *
     * The left fraction of vehicles to insert is saved for each O/D-dependency
     *  over time and the number of vehicles to generate is increased as soon
     *  as this value is larger than 1, decrementing it.
     *
     * @param[in] begin The begin time to generate vehicles for
     * @param[in] end The end time to generate vehicles for
     * @param[in] dev The stream to write the generated vehicle trips to
     * @param[in] uniform Information whether departure times shallbe uniformly spread or random
     * @param[in] differSourceSink whether source and sink shall be different edges
     * @param[in] noVtype Whether vtype information shall not be written
     * @param[in] prefix A prefix for the vehicle names
     * @param[in] stepLog Whether processed time shall be written
     * @param[in] pedestrians Writes trips for pedestrians
     * @param[in] persontrips Writes trips for persontrips
     */
    void write(SUMOTime begin, const SUMOTime end,
               OutputDevice& dev, const bool uniform,
               const bool differSourceSink, const bool noVtype,
               const std::string& prefix, const bool stepLog,
               bool pedestrians, bool persontrips,
               const std::string& modes);


    /** @brief Writes the flows stored in the matrix
     *
     * @param[in] begin The begin time to generate vehicles for
     * @param[in] end The end time to generate vehicles for
     * @param[in] dev The stream to write the generated vehicle trips to
     * @param[in] noVtype Whether vtype information shall not be written
     * @param[in] prefix A prefix for the flow names
     * @param[in] asProbability Write probability to spawn per second instead of number of vehicles
     * @param[in] pedestrians Writes flows for pedestrians
     * @param[in] persontrips Writes flows for persontrips
     */
    void writeFlows(const SUMOTime begin, const SUMOTime end,
                    OutputDevice& dev, const bool noVtype,
                    const std::string& prefix,
                    bool asProbability = false, bool pedestrians = false, bool persontrips = false,
                    const std::string& modes = "");


    /** @brief Returns the number of loaded vehicles
     *
     * Returns the value of myNoLoaded
     *
     * @return The number of loaded vehicles
     */
    double getNumLoaded() const;


    /** @brief Returns the number of written vehicles
     *
     * Returns the value of myNoWritten
     *
     * @return The number of written vehicles
     */
    double getNumWritten() const;


    /** @brief Returns the number of discarded vehicles
     *
     * Returns the value of myNoDiscarded
     *
     * @return The number of discarded vehicles
     */
    double getNumDiscarded() const;


    /** @brief Splits the stored cells dividing them on the given time line
     * @todo Describe
     */
    void applyCurve(const Distribution_Points& ps);


    /** @brief read a VISUM-matrix with the O Format
     *  @todo Describe
     */
    void readO(LineReader& lr, double scale,
               std::string vehType, bool matrixHasVehType);

    /** @brief read a VISUM-matrix with the V Format
     *  @todo Describe
     */
    void readV(LineReader& lr, double scale,
               std::string vehType, bool matrixHasVehType);

    /** @brief read a matrix in one of several formats
     *  @todo Describe
     */
    void loadMatrix(OptionsCont& oc);

    /** @brief read SUMO routes
     *  @todo Describe
     */
    void loadRoutes(OptionsCont& oc, SUMOSAXHandler& handler);

    /** @brief split the given timeline
     *  @todo Describe
     */
    Distribution_Points parseTimeLine(const std::vector<std::string>& def, bool timelineDayInHours);

    const std::vector<ODCell*>& getCells() {
        return myContainer;
    }

    void sortByBeginTime();

    SUMOTime getBegin() const {
        return myBegin;
    }

    SUMOTime getEnd() const {
        return myEnd;
    }

protected:
    /**
     * @struct ODVehicle
     * @brief An internal representation of a single vehicle
     */
    struct ODVehicle {
        /// @brief The id of the vehicle
        std::string id;
        /// @brief The departure time of the vehicle
        SUMOTime depart;
        /// @brief The cell of the ODMatrix which generated the vehicle
        ODCell* cell;
        /// @brief The edge the vehicles shall start at
        std::string from;
        /// @brief The edge the vehicles shall end at
        std::string to;

    };


    /** @brief Computes the vehicle departs stored in the given cell and saves them in "into"
     *
     * At first, the number of vehicles to insert is computed using the
     *  integer value of the vehicleNumber information from the given cell.
     *  In the case vehicleNumber has a fraction, an additional vehicle
     *  may be added in the case a chosen random number is lower than this fraction.
     *
     * If uniform is true, the departure times of the generated vehicles
     *  are spread uniformly, otherwise the departure time are chosen randomly from
     *  the interval.
     *
     * The vehicle names are generated by putting the value of vehName after the
     *  given prefix. The value of vehName is incremented with each generated vehicle.
     *
     * The number of left vehicles (the fraction if no additional vehicle was
     *  generated) is returned.
     *
     * @param[in] cell The cell to use
     * @param[in,out] vehName An incremented index of the generated vehicle
     * @param[out] into The storage to put generated vehicles into
     * @param[in] uniform Information whether departure times shallbe uniformly spread or random
     * @param[in] differSourceSink whether source and sink shall be different edges
     * @param[in] prefix A prefix for the vehicle names
     * @return The number of left vehicles to insert
     */
    double computeDeparts(ODCell* cell,
                          int& vehName, std::vector<ODVehicle>& into,
                          const bool uniform, const bool differSourceSink,
                          const std::string& prefix);


    /** @brief Splits the given cell dividing it on the given time line and
     *          storing the results in the given container
     *
     * For the given cell, a list of clones is generated. The number of these
     *  is equal to the number of "areas" within the given distribution
     *  description (time line in this case) and each clone's vehicleNumber
     *  is equal to the given cell's vehicle number multiplied with the area's
     *  probability. The clones are stored in the given cell vector.
     *
     * @see Distribution_Points
     * @param[in] ps The time line to apply
     * @param[in] cell The cell to split
     * @param[out] newCells The storage to put generated cells into
     * @todo describe better!!!
     */
    void applyCurve(const Distribution_Points& ps, ODCell* cell,
                    std::vector<ODCell*>& newCells);


private:
    /** @used in the functions readV and readO
     * @todo Describe
     */
    std::string getNextNonCommentLine(LineReader& lr);

    /** @used in the functions readV and readO
     * @todo Describe
     */
    SUMOTime parseSingleTime(const std::string& time);

    /** @used in the functions readV and readO
     * @todo Describe
     */
    std::pair<SUMOTime, SUMOTime> readTime(LineReader& lr);

    /** @used in the functions readV and readO
     * @todo Describe
     */
    double readFactor(LineReader& lr, double scale);


private:
    /// @brief The loaded cells
    std::vector<ODCell*> myContainer;

    /// @brief The loaded cells indexed by origin and destination
    std::map<const std::pair<const std::string, const std::string>, std::vector<ODCell*> > myShortCut;

    /// @brief The districts to retrieve sources/sinks from
    const ODDistrictCont& myDistricts;

    /// @brief The missing districts already warned about
    std::set<std::string> myMissingDistricts;

    /// @brief Number of loaded vehicles
    double myNumLoaded;

    /// @brief Number of written vehicles
    double myNumWritten;

    /// @brief Number of discarded vehicles
    double myNumDiscarded;

    /// @brief parsed time bounds
    SUMOTime myBegin, myEnd;

    /**
     * @class cell_by_begin_comparator
     * @brief Used for sorting the cells by the begin time they describe
     */
    class cell_by_begin_comparator {
    public:
        /// @brief constructor
        explicit cell_by_begin_comparator() { }


        /** @brief Comparing operator
         *
         * Compares two cells by the begin of the time they describe. The sort is stabilized
         * (with secondary sort keys being origin and destination) to get comparable results
         * with different platforms / compilers.
         *
         * @param[in] p1 First cell to compare
         * @param[in] p2 Second cell to compare
         * @return Whether the begin time of the first cell is lower than the one of the second
         */
        int operator()(ODCell* p1, ODCell* p2) const {
            if (p1->begin == p2->begin) {
                if (p1->origin == p2->origin) {
                    return p1->destination < p2->destination;
                }
                return p1->origin < p2->origin;
            }
            return p1->begin < p2->begin;
        }

    };


    /**
     * @class descending_departure_comperator
     * @brief Used for sorting vehicles by their departure (latest first)
     *
     * A reverse operator to what may be expected is used in order to allow
     *  prunning the sorted vector from its tail.
     */
    class descending_departure_comperator {
    public:
        /// @brief constructor
        descending_departure_comperator() { }


        /** @brief Comparing operator
         *
         * Compares two vehicles by their departure time
         *
         * @param[in] p1 First vehicle to compare
         * @param[in] p2 Second vehicle to compare
         * @return Whether the departure time of the first vehicle is larger than the one of the second
         */
        bool operator()(const ODVehicle& p1, const ODVehicle& p2) const {
            if (p1.depart == p2.depart) {
                return p1.id > p2.id;
            }
            return p1.depart > p2.depart;
        }

    };

private:
    /** @brief invalid copy constructor */
    ODMatrix(const ODMatrix& s);

    /** @brief invalid assignment operator */
    ODMatrix& operator=(const ODMatrix& s) = delete;

};
