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
/// @file    MSDetectorControl.h
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Clemens Honomichl
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    2005-09-15
///
// Detectors container; responsible for string and output generation
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/common/NamedObjectCont.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <microsim/output/MSE2Collector.h>
#include <microsim/output/MSE3Collector.h>
#include <microsim/output/MSInductLoop.h>
#include <microsim/output/MSRouteProbe.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSMeanData;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDetectorControl
 * @brief Detectors container; responsible for string and output generation
 */
class MSDetectorControl {
public:
    /** @brief Constructor
     */
    MSDetectorControl();


    /** @brief Destructor
     *
     * Deletes all stored detectors.
     */
    ~MSDetectorControl();


    /** @brief Closes the detector outputs
     *
     * Goes through the intervals to write and writes the last step using
     *  the current simulation time as end.
     *
     * @param[in] step The time step (the simulation has ended at)
     */
    void close(SUMOTime step);


    /** @brief Adds a detector/output combination into the containers
     *
     * The detector is tried to be added into "myDetectors". If the detector
     *  is already known (the id was already used for a similar detector),
     *  a ProcessError is thrown.
     *
     * Otherwise, the Detector2File-mechanism is instantiated for the detector.
     *
     * Please note, that the detector control gets responsible for the detector.
     *
     * @param[in] type The type of the detector
     * @param[in] d The detector to add
     * @param[in] device The device the detector uses
     * @param[in] splInterval The sample interval of the detector
     * @exception ProcessError If the detector is already known
     */
    void add(SumoXMLTag type, MSDetectorFileOutput* d, const std::string& device, SUMOTime splInterval, SUMOTime begin = -1);



    /** @brief Adds only the detector into the containers
     *
     * The detector is tried to be added into "myDetectors". If the detector
     *  is already known (the id was already used for a similar detector),
     *  a ProcessError is thrown.
     *
     * Please note, that the detector control gets responsible for the detector.
     *
     * @param[in] type The type of the detector
     * @param[in] d The detector to add
     * @exception ProcessError If the detector is already known
     */
    void add(SumoXMLTag type, MSDetectorFileOutput* d);


    /** @brief Adds a mean data object
     *
     * The detector is pushed into the internal list.
     *
     * Please note, that the detector control gets responsible for the detector.
     *
     * @param[in] mn The mean data to add
     * @param[in] device The output device to use
     * @param[in] frequency The frequency for output generation
     * @param[in] begin The begin of the first interval
     */
    void add(MSMeanData* mn, const std::string& device,
             SUMOTime frequency, SUMOTime begin);



    /** @brief Adds one of the detectors as a new MSDetectorFileOutput
     *
     * @param[in] det The generator to add
     * @param[in] device The device to use
     * @param[in] interval The sample interval to use
     * @param[in] begin The start of the first sample interval to use
     */
    void addDetectorAndInterval(MSDetectorFileOutput* det,
                                OutputDevice* device,
                                SUMOTime interval, SUMOTime begin = -1);



    /** @brief Returns the list of available detector types
     *
     * @return The list of types
     */
    const std::vector<SumoXMLTag> getAvailableTypes() const;


    /** @brief Returns the list of detectors of the given type
     *
     * @param[in] type The type of detectors to be retrieved
     * @return The container of detectors
     */
    const NamedObjectCont<MSDetectorFileOutput*>& getTypedDetectors(SumoXMLTag type) const;

    const std::map<std::string, std::vector<MSMeanData*> >& getMeanData() const {
        return myMeanData;
    }

    /** @brief Computes detector values
     *
     * Some detectors need to be touched each time step in order to compute
     *  values from the vehicles stored in their containers. This method
     *  goes through all of these detectors and forces a recomputation of
     *  the values.
     * @param[in] step The current time step
     */
    void updateDetectors(const SUMOTime step);


    /** @brief Writes the output to be generated within the given time step
     *
     * Goes through the list of intervals. If one interval has ended within the
     *  given step or if the closing-flag is set and the output was not
     *  written in this step already, the writeXMLOutput method is called
     *  for all MSDetectorFileOutputs within this interval.
     *
     * @param[in] step The current time step
     * @param[in] closing Whether the device is closed
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeOutput(SUMOTime step, bool closing);

    /** @brief Remove all vehicles before quick-loading state */
    void clearState();

protected:
    /// @name Structures needed for assigning detectors to intervals
    /// @{

    /// @brief A pair of a Detector with it's associated file-stream.
    typedef std::pair< MSDetectorFileOutput*, OutputDevice* > DetectorFilePair;

    /// @brief Container holding DetectorFilePair (with the same interval).
    typedef std::vector< DetectorFilePair > DetectorFileVec;

    /// @brief Definition of the interval key
    typedef std::pair<SUMOTime, SUMOTime> IntervalsKey;

    /// @brief Association of intervals to DetectorFilePair containers.
    typedef std::map< IntervalsKey, DetectorFileVec > Intervals;
    /// @}

    /**
     * @struct detectorEquals
     * @brief Returns true if detectors are equal.
     *
     * Binary predicate that compares the passed DetectorFilePair's
     * detector to a fixed one. Returns true if detectors are
     * equal. (Used to prevent multiple inclusion of a detector for
     * the same interval.)
     *
     * @see addDetectorAndInterval
     */
    struct detectorEquals : public std::binary_function< DetectorFilePair, MSDetectorFileOutput*, bool > {
        /** @brief Returns true if detectors are equal. */
        bool operator()(const DetectorFilePair& pair, const MSDetectorFileOutput* det) const {
            return pair.first == det;
        }
    };

protected:
    /// @brief The detectors map, first by detector type, then using NamedObjectCont (@see NamedObjectCont)
    std::map<SumoXMLTag, NamedObjectCont< MSDetectorFileOutput*> > myDetectors;


    /// @brief Map that hold DetectorFileVec for given intervals
    Intervals myIntervals;

    /// @brief The map that holds the last call for each sample interval
    std::map<IntervalsKey, SUMOTime> myLastCalls;

    /// @brief List of meanData  detectors
    std::map<std::string, std::vector<MSMeanData*> > myMeanData;

    /// @brief An empty container to return in getTypedDetectors() if no detectors of the asked type exist
    NamedObjectCont< MSDetectorFileOutput*> myEmptyContainer;


private:
    /// @brief Invalidated copy constructor.
    MSDetectorControl(const MSDetectorControl&);

    /// @brief Invalidated assignment operator.
    MSDetectorControl& operator=(const MSDetectorControl&);


};
