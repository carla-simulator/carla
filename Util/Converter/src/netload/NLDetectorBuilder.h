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
/// @file    NLDetectorBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Clemens Honomichl
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @date    Mon, 15 Apr 2002
///
// Builds detectors for microsim
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <microsim/output/MSCrossSection.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/output/MSE2Collector.h>
// #include <microsim/output/MSMultiLaneE2Collector.h>

// ===========================================================================
// class declarations
// ===========================================================================
class MSDetectorFileOutput;
class MSLane;
class MSEdge;

class MEInductLoop;
class MESegment;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NLDetectorBuilder
 * @brief Builds detectors for microsim
 *
 * The building methods may be overridden, to build guisim-instances of the triggers,
 *  for example.
 */
class NLDetectorBuilder {
public:
    /** @brief Constructor
     *
     * @param[in] net The network to which's detector control built detector shall be added
     */
    NLDetectorBuilder(MSNet& net);


    /// @brief Destructor
    virtual ~NLDetectorBuilder();


    /// @name Value parsing and detector building methods
    /// @{

    /** @brief Builds an e1 detector and adds it to the net
     *
     * Checks the given values, first. If one of the values is invalid
     *  (lane is not known, sampling frequency<=0, position is larger
     *  than lane's length, the id is already in use), an InvalidArgument is thrown.
     *
     * Otherwise the e1 detector is built by calling "createInductLoop".
     *
     * Internally, there is also a distinction whether a mesosim e1 detector
     *  shall be built.
     *
     * @param[in] id The id the detector shall have
     * @param[in] lane The name of the lane the detector is placed at
     * @param[in] pos The definition of the position on the lane the detector shall be placed at
     * @param[in] splInterval The aggregation time span the detector shall use
     * @param[in] device The output device the detector shall write into
     * @param[in] friendlyPos Whether the position information shall be used "friendly" (see user docs)
     * @param[in] vTypes which vehicle types are considered
     * @exception InvalidArgument If one of the values is invalid
     */
    void buildInductLoop(const std::string& id,
                         const std::string& lane, double pos, SUMOTime splInterval,
                         const std::string& device, bool friendlyPos,
                         const std::string& vTypes);


    /** @brief Builds an instantenous induction and adds it to the net
     *
     * Checks the given values, first. If one of the values is invalid
     *  (lane is not known, sampling frequency<=0, position is larger
     *  than lane's length, the id is already in use), an InvalidArgument is thrown.
     *
     * Otherwise the e1 detector is built by calling "createInductLoop".
     *
     * @param[in] id The id the detector shall have
     * @param[in] lane The name of the lane the detector is placed at
     * @param[in] pos The definition of the position on the lane the detector shall be placed at
     * @param[in] device The output device the detector shall write into
     * @param[in] friendlyPos Whether the position information shall be used "friendly" (see user docs)
     * @exception InvalidArgument If one of the values is invalid
     */
    void buildInstantInductLoop(const std::string& id,
                                const std::string& lane, double pos,
                                const std::string& device, bool friendlyPos,
                                const std::string& vTypes);




    /** @brief Builds a new E2 detector and adds it to the net's detector control. Also performs some
    *          consistency checks for the detector positioning and applies "friendly positioning"
    *
    *   @param[in] tlls Traffic light logic associated to the detector
    *   @param[in] toLane Lane associated to the detector (only for tlls != 0)
    *   @param[in] friendlyPos Whether automatic adjustments of the detector position shall be applied in case of erroneous specification
    *   @see For the other parameters see the MSE2Collector constructors
    *
    *   @todo Add parameter showDetector to indicate whether the detector should be visible in the GUI
    *
    */
    void buildE2Detector(const std::string& id, MSLane* lane, double pos, double endPos, double length,
                         const std::string& device, SUMOTime frequency,
                         SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                         const std::string& vTypes, bool friendlyPos, bool showDetector,
                         MSTLLogicControl::TLSLogicVariants* tlls = 0, MSLane* toLane = 0);

    void buildE2Detector(const std::string& id, std::vector<MSLane*> lanes, double pos, double endPos,
                         const std::string& device, SUMOTime frequency,
                         SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                         const std::string& vTypes, bool friendlyPos, bool showDetector,
                         MSTLLogicControl::TLSLogicVariants* tlls = 0, MSLane* toLane = 0);


    /** @brief Stores temporary the initial information about an e3 detector to build
     *
     * If the given sample interval is < 0, an InvalidArgument is thrown. Otherwise,
     *  the values are stored in a new instance of E3DetectorDefinition within
     *  "myE3Definition".
     *
     * @param[in] id The id the detector shall have
     * @param[in] device The output device the detector shall write into
     * @param[in] splInterval The aggregation time span the detector shall use
     * @param[in] haltingTimeThreshold Detector parameter: the time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
     * @param[in] haltingSpeedThreshold Detector parameter: the speed a vehicle's speed must be below to be assigned as jammed
     * @exception InvalidArgument If one of the values is invalid
     */
    void beginE3Detector(const std::string& id, const std::string& device, SUMOTime splInterval,
                         double haltingSpeedThreshold, SUMOTime haltingTimeThreshold,
                         const std::string& vTypes, bool openEntry);


    /** @brief Builds an entry point of an e3 detector
     *
     * If the lane is not known or the position information is not within the lane,
     *  an InvalidArgument is thrown. Otherwise a MSCrossSection is built
     *  using the obtained values and added to the list of entries of the e3 definition
     *  stored in "myE3Definition".
     *
     * @param[in] lane The id of the lane the entry shall be placed at
     * @param[in] pos The position on the lane the entry shall be placed at
     * @param[in] friendlyPos Whether the position information shall be used "friendly" (see user docs)
     * @exception InvalidArgument If one of the values is invalid
     */
    void addE3Entry(const std::string& lane, double pos, bool friendlyPos);


    /** @brief Builds an exit point of an e3 detector
     *
     * If the lane is not known or the position information is not within the lane,
     *  an InvalidArgument is thrown. Otherwise a MSCrossSection is built
     *  using the obtained values and added to the list of exits of the e3 definition
     *  stored in "myE3Definition".
     *
     * @param[in] lane The id of the lane the exit shall be placed at
     * @param[in] pos The position on the lane the exit shall be placed at
     * @param[in] friendlyPos Whether the position information shall be used "friendly" (see user docs)
     * @exception InvalidArgument If one of the values is invalid
     */
    void addE3Exit(const std::string& lane, double pos, bool friendlyPos);


    /** @brief Builds of an e3 detector using collected values
     *
     * The parameter collected are used to build an e3 detector using
     *  "createE3Detector". The resulting detector is added to the net.
     *
     * @param[in] lane The id of the lane the exit shall be placed at
     * @param[in] pos The position on the lane the exit shall be placed at
     * @exception InvalidArgument If one of the values is invalid
     */
    void endE3Detector();


    /** @brief Returns the id of the currently built e3 detector
     *
     * This is used for error-message generation only. If no id is known,
     *  "<unknown>" is returned.
     *
     * @return The id of the currently processed e3 detector
     */
    std::string getCurrentE3ID() const;


    /** @brief Builds a vTypeProbe and adds it to the net
     *
     * Checks the given values, first. If one of the values is invalid
     *  (sampling frequency<=0), an InvalidArgument is thrown.
     *
     * Otherwise the vTypeProbe is built (directly).
     *
     * @param[in] id The id the detector shall have
     * @param[in] vtype The name of the vehicle type the detector shall observe
     * @param[in] frequency The reporting frequency
     * @param[in] device The output device the detector shall write into
     * @exception InvalidArgument If one of the values is invalid
     */
    void buildVTypeProbe(const std::string& id,
                         const std::string& vtype, SUMOTime frequency,
                         const std::string& device);


    /** @brief Builds a routeProbe and adds it to the net
     *
     * Checks the given values, first. If one of the values is invalid
     *  (sampling frequency<=0), an InvalidArgument is thrown.
     *
     * Otherwise the routeProbe is built (directly).
     *
     * @param[in] id The id the detector shall have
     * @param[in] edge The name of the edge the detector shall observe
     * @param[in] frequency The reporting frequency
     * @param[in] begin The start of the first reporting interval
     * @param[in] device The output device the detector shall write into
     * @exception InvalidArgument If one of the values is invalid
     */
    void buildRouteProbe(const std::string& id, const std::string& edge,
                         SUMOTime frequency, SUMOTime begin,
                         const std::string& device,
                         const std::string& vTypes);
    /// @}



    /// @name Detector creating methods
    ///
    /// Virtual, so they may be overwritten, for generating gui-versions of the detectors, for example.
    /// @{

    /** @brief Creates an instance of an e1 detector using the given values
     *
     * Simply calls the MSInductLoop constructor
     *
     * @param[in] id The id the detector shall have
     * @param[in] lane The lane the detector is placed at
     * @param[in] pos The position on the lane the detector is placed at
     * @param[in] vTypes which vehicle types are considered
     * @param[in] show Whether to show the detector in the gui if available
     */
    virtual MSDetectorFileOutput* createInductLoop(const std::string& id,
            MSLane* lane, double pos,
            const std::string& vTypes, bool show = true);


    /** @brief Creates an instance of an e1 detector using the given values
     *
     * Simply calls the MSInductLoop constructor
     *
     * @param[in] id The id the detector shall have
     * @param[in] lane The lane the detector is placed at
     * @param[in] pos The position on the lane the detector is placed at
     * @param[in] od The output device the loop shall use
     */
    virtual MSDetectorFileOutput* createInstantInductLoop(const std::string& id,
            MSLane* lane, double pos, const std::string& od,
            const std::string& vTypes);


    /** @brief Creates a MSE2Collector instance, overridden by GUIE2Collector::createE2Detector()
     *
     * Simply calls the MSE2Collector constructor
     *
     *  @see  MSE2Collector Constructor documentation
     */
    virtual MSE2Collector* createE2Detector(const std::string& id,
                                            DetectorUsage usage, MSLane* lane, double pos, double endPos, double length,
                                            SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                                            const std::string& vTypes, bool showDetector = true);

    virtual MSE2Collector* createE2Detector(const std::string& id,
                                            DetectorUsage usage, std::vector<MSLane*> lanes, double pos, double endPos,
                                            SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                                            const std::string& vTypes, bool showDetector = true);

    /** @brief Creates an instance of an e3 detector using the given values
     *
     * Simply calls the MSE3Collector constructor.
     *
     * @param[in] id The id the detector shall have
     * @param[in] entries The list of this detector's entries
     * @param[in] exits The list of this detector's exits
     * @param[in] haltingSpeedThreshold Detector parameter: the speed a vehicle's speed must be below to be assigned as jammed
     * @param[in] haltingTimeThreshold Detector parameter: the time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
     */
    virtual MSDetectorFileOutput* createE3Detector(const std::string& id,
            const CrossSectionVector& entries, const CrossSectionVector& exits,
            double haltingSpeedThreshold, SUMOTime haltingTimeThreshold,
            const std::string& vTypes, bool openEntry);


    /** @brief Creates edge based mean data collector using the given specification
     *
     * @param[in] id The id the detector shall have
     * @param[in] frequency The aggregation interval the detector shall use
     * @param[in] begin dump begin time
     * @param[in] end dump end time
     * @param[in] type The type of values to be generated
     * @param[in] useLanes Information whether lane-based or edge-based dump shall be generated
     * @param[in] withEmpty Information whether empty lanes/edges shall be written
     * @param[in] withInternal Information whether internal lanes/edges shall be written
     * @param[in] trackVehicles Information whether information shall be collected per vehicle
     * @param[in] detectPersons Whether pedestrians shall be detected instead of vehicles
     * @param[in] maxTravelTime the maximum travel time to output
     * @param[in] minSamples the minimum number of sample seconds before the values are valid
     * @param[in] haltSpeed the maximum speed to consider a vehicle waiting
     * @param[in] vTypes the set of vehicle types to consider
     * @exception InvalidArgument If one of the values is invalid
     */
    void createEdgeLaneMeanData(const std::string& id, SUMOTime frequency,
                                SUMOTime begin, SUMOTime end, const std::string& type,
                                const bool useLanes, const bool withEmpty, const bool printDefaults,
                                const bool withInternal, const bool trackVehicles, const int detectPersons,
                                const double maxTravelTime, const double minSamples,
                                const double haltSpeed, const std::string& vTypes,
                                const std::string& writeAttributes,
                                const std::string& device);
    /// @}



    /** @brief Builds an e2 detector that lies on only one lane
     *
     * @param[in] id The id the detector shall have
     * @param[in] usage Information how the detector is used within the simulation
     * @param[in] lane The lane the detector is placed at
     * @param[in] pos The position on the lane the detector is placed at
     * @param[in] length The length the detector has
     * @param[in] haltingTimeThreshold Detector parameter: the time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
     * @param[in] haltingSpeedThreshold Detector parameter: the speed a vehicle's speed must be below to be assigned as jammed
     * @param[in] jamDistThreshold Detector parameter: the distance between two vehicles in order to not count them to one jam
     * @todo Check whether this method is really needful
     */
    MSE2Collector* buildSingleLaneE2Det(const std::string& id,
                                        DetectorUsage usage, MSLane* lane, double pos, double length,
                                        SUMOTime haltingTimeThreshold, double haltingSpeedThreshold,
                                        double jamDistThreshold,
                                        const std::string& vTypes);


protected:
    /**
     * @class E3DetectorDefinition
     * @brief Holds the incoming definitions of an e3 detector unless the detector is build.
     */
    class E3DetectorDefinition {
    public:
        /** @brief Constructor
         * @param[in] id The id the detector shall have
         * @param[in] device The output device the detector shall write into
         * @param[in] haltingSpeedThreshold Detector parameter: the speed a vehicle's speed must be below to be assigned as jammed
         * @param[in] haltingTimeThreshold Detector parameter: the time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
         * @param[in] splInterval The aggregation time span the detector shall use
         */
        E3DetectorDefinition(const std::string& id,
                             const std::string& device, double haltingSpeedThreshold,
                             SUMOTime haltingTimeThreshold, SUMOTime splInterval,
                             const std::string& vTypes, bool openEntry);

        /// @brief Destructor
        ~E3DetectorDefinition();

        /// @brief The id of the detector
        const std::string myID;
        /// @brief The device the detector shall use
        const std::string myDevice;
        /// @brief The speed a vehicle's speed must be below to be assigned as jammed
        double myHaltingSpeedThreshold;
        /// @brief The time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
        SUMOTime myHaltingTimeThreshold;
        /// @brief List of detector's entries
        CrossSectionVector myEntries;
        /// @brief List of detector's exits
        CrossSectionVector myExits;
        /// @brief The aggregation interval
        SUMOTime mySampleInterval;
        /// @brief The device the detector shall use
        const std::string myVehicleTypes;
        /// @brief Whether the detector is declared as having incomplete entry detectors
        bool myOpenEntry;
        //@}

    private:
        /// @brief Invalidated copy constructor.
        E3DetectorDefinition(const E3DetectorDefinition&);

        /// @brief Invalidated assignment operator.
        E3DetectorDefinition& operator=(const E3DetectorDefinition&);

    };


protected:
    /** @brief Computes the position to use
     *
     * At first, it is checked whether the given position is negative. If so, the
     *  position is added to the lane's length to obtain the position counted
     *  backwards.
     *
     * If the resulting position is beyond or in front (<0) of the lane, it is either
     *  set to the according lane's boundary (.1 or length-.1) if friendlyPos
     *  is set, or, if friendlyPos is not set, an InvalidArgument is thrown.
     *
     * @param[in] pos Definition of the position on the lane
     * @param[in] lane The lane the position must be valid for
     * @param[in] friendlyPos Whether false positions shall be made acceptable
     * @param[in] detid The id of the currently built detector (for error message generation)
     * @exception InvalidArgument If the defined position is invalid
     */
    double getPositionChecking(double pos, MSLane* lane, bool friendlyPos,
                               const std::string& detid);


    /// @name Value checking/adapting methods
    /// @{

    /** @brief Returns the named edge
     * @param[in] edgeID The id of the lane
     * @param[in] type The type of the detector (for error message generation)
     * @param[in] detid The id of the currently built detector (for error message generation)
     * @exception InvalidArgument If the named edge is not known
     */
    MSEdge* getEdgeChecking(const std::string& edgeID, SumoXMLTag type,
                            const std::string& detid);

public:
    /** @brief Returns the named lane
     * @param[in] laneID The id of the lane
     * @param[in] type The type of the detector (for error message generation)
     * @param[in] detid The id of the currently built detector (for error message generation)
     * @exception InvalidArgument If the named lane is not known
     */
    MSLane* getLaneChecking(const std::string& laneID, SumoXMLTag type,
                            const std::string& detid);

protected:
    /** @brief Checks whether the given frequency (sample interval) is valid
     * @param[in] splInterval The sample interval
     * @param[in] type The type of the detector (for error message generation)
     * @param[in] id The id of the detector (for error message generation)
     * @exception InvalidArgument If the given sample interval is invalid (<=0)
     * @todo Why is splInterval an int???
     */
    void checkSampleInterval(SUMOTime splInterval, SumoXMLTag type, const std::string& id);
    /// @}


protected:
    /// @brief The net to fill
    MSNet& myNet;


private:
    /// @brief definition of the currently parsed e3 detector
    E3DetectorDefinition* myE3Definition;


private:
    /// @brief Invalidated copy constructor.
    NLDetectorBuilder(const NLDetectorBuilder&);

    /// @brief Invalidated assignment operator.
    NLDetectorBuilder& operator=(const NLDetectorBuilder&);

};
