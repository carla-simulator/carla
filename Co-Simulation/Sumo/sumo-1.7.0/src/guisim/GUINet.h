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
/// @file    GUINet.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A MSNet extended by some values for usage within the gui
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utility>
#include <microsim/MSNet.h>
#include <microsim/devices/MSDevice_Tripinfo.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SAXWeightsHandler.h>
#include <foreign/rtree/SUMORTree.h>
#include <foreign/rtree/LayeredRTree.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdgeControl;
class MSJunctionControl;
class MSTLLogicControl;
class MSTrafficLightLogic;
class MSLink;
class GUIJunctionWrapper;
class GUIDetectorWrapper;
class GUICalibrator;
class GUITrafficLightLogicWrapper;
class RGBColor;
class GUIEdge;
class OutputDevice;
class GUIVehicle;
class GUIVehicleControl;
class MSVehicleControl;
class GUIMEVehicleControl;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUINet
 * @brief A MSNet extended by some values for usage within the gui
 *
 * This gui version of the network allows the retrieval of some more
 * information than the normal network version does. Due to this, not only
 * these retrieval, but also some further initialisation methods must have
 * been implemented. Nonethenless, this class has almost the same functions
 * as the MSNet-class.
 *
 * Some microsimulation items are wrapped in certain classes to allow their
 * drawing and their enumerated access. This enumeration is realised by
 * inserting the wrapped items into vectors and is needed to fasten the
 * network's drawing as only visible items are being drawn.
 */
class GUINet : public MSNet, public GUIGlObject {

    friend class GUITrafficLightLogicWrapper; // see createTLWrapper

public:
    /** @brief Constructor
     * @param[in] vc The vehicle control to use
     * @param[in] beginOfTimestepEvents The event control to use for simulation step begin events
     * @param[in] endOfTimestepEvents The event control to use for simulation step end events
     * @param[in] insertionEvents The event control to use for insertion events
     * @exception ProcessError If a network was already constructed
     */
    GUINet(MSVehicleControl* vc, MSEventControl* beginOfTimestepEvents,
           MSEventControl* endOfTimestepEvents,
           MSEventControl* insertionEvents);


    /// @brief Destructor
    ~GUINet();


    /**
     * @brief Returns whether this is a GUI Net
     */
    bool isGUINet() const override {
        return true;
    }


    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) override;


    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent) override;


    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const override;


    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const override;
    //@}


    /// returns the bounder of the network
    const Boundary& getBoundary() const;

    /// returns the position of a junction
    Position getJunctionPosition(const std::string& name) const;

    /// returns the information whether the vehicle still exists
    bool vehicleExists(const std::string& name) const;

    /// Some further steps needed for gui processing
    void guiSimulationStep();

    /** @brief Performs a single simulation step (locking the simulation)
     */
    void simulationStep();

    /// @name functions for performance measurements
    /// @{

    /** @brief Returns the duration of the last step (sim+visualisation+idle) (in ms)
     * @return How long it took to compute and display the last step
     */
    int getWholeDuration() const;


    /** @brief Returns the duration of the last step's simulation part (in ms)
     * @return How long it took to compute the last step
     */
    int getSimDuration() const;


    /// Returns the simulation speed as a factor to real time
    double getRTFactor() const;

    /// Returns the update per seconds rate
    double getUPS() const;

    /// Returns the simulation speed as a factor to real time
    double getMeanRTFactor(int duration) const;

    /// Returns the update per seconds rate
    double getMeanUPS() const;

    // Returns the duration of the last step's visualisation part (in ms)
    //int getVisDuration() const;

    /// Returns the duration of the last step's idle part (in ms)
    int getIdleDuration() const;

    /// Sets the duration of the last step's simulation part
    void setSimDuration(int val);

    // Sets the duration of the last step's visualisation part
    //void setVisDuration(int val);

    /// Sets the duration of the last step's idle part
    void setIdleDuration(int val);
    //}

    double getAvgRouteLength() const {
        return MSDevice_Tripinfo::getAvgRouteLength();
    }
    double getAvgDuration() const {
        return MSDevice_Tripinfo::getAvgDuration();
    }
    double getAvgWaitingTime() const {
        return MSDevice_Tripinfo::getAvgWaitingTime();
    }
    double getAvgTimeLoss() const {
        return MSDevice_Tripinfo::getAvgTimeLoss();
    }
    double getAvgDepartDelay() const {
        return MSDevice_Tripinfo::getAvgDepartDelay();
    }
    double getAvgTripSpeed() const {
        return MSDevice_Tripinfo::getAvgDuration() != 0 ? MSDevice_Tripinfo::getAvgRouteLength() / MSDevice_Tripinfo::getAvgDuration() : 0;
    }
    double getAvgWalkRouteLength() const {
        return MSDevice_Tripinfo::getAvgWalkRouteLength();
    }
    double getAvgWalkDuration() const {
        return MSDevice_Tripinfo::getAvgWalkDuration();
    }
    double getAvgWalkTimeLoss() const {
        return MSDevice_Tripinfo::getAvgWalkTimeLoss();
    }

    /** @brief Returns the person control
     *
     * If the person control does not exist, yet, it is created.
     *
     * @return The person control
     * @see MSPersonControl
     * @see myPersonControl
     */
    MSTransportableControl& getPersonControl() override;


    /** @brief Returns the container control
     *
     * If the container control does not exist, yet, it is created.
     *
     * @return The container control
     * @see MSContainerControl
     * @see myContainerControl
     */
    MSTransportableControl& getContainerControl() override;


    /** Returns the gl-id of the traffic light that controls the given link
     * valid only if the link is controlled by a tls */
    int getLinkTLID(MSLink* link) const;

    /** Returns the index of the link within the junction that controls the given link;
     * Returns -1 if the link is not controlled by a tls */
    int getLinkTLIndex(MSLink* link) const;


    /// @name locator-methods
    //@{

    /* @brief Returns the gl-ids of all junctions within the net
     * @param[in] includeInternal Whether to include ids of internal junctions
     */
    std::vector<GUIGlID> getJunctionIDs(bool includeInternal) const;

    /// Returns the gl-ids of all traffic light logics within the net
    std::vector<GUIGlID> getTLSIDs() const;
    //@}


    /// Initialises gui wrappers
    void initGUIStructures();


    /** @brief Returns the RTree used for visualisation speed-up
     * @return The visualisation speed-up
     */
    SUMORTree& getVisualisationSpeedUp() {
        return myGrid;
    }


    /** @brief Returns the RTree used for visualisation speed-up
     * @return The visualisation speed-up
     */
    const SUMORTree& getVisualisationSpeedUp() const {
        return myGrid;
    }

    /** @brief Returns the vehicle control
     * @return The vehicle control
     * @see MSVehicleControl
     * @see myVehicleControl
     */
    GUIVehicleControl* getGUIVehicleControl();

    /** @brief Returns the vehicle control
     * @return The vehicle control
     * @see MSVehicleControl
     * @see myVehicleControl
     */
    GUIMEVehicleControl* getGUIMEVehicleControl();

    /// @brief retrieve loaded edged weight for the given attribute and the current simulation time
    double getEdgeData(const MSEdge* edge, const std::string& attr);

    /// @brief load edgeData from file
    bool loadEdgeData(const std::string& file);


    /// @brief return list of loaded edgeData attributes
    std::vector<std::string> getEdgeDataAttrs() const;

#ifdef HAVE_OSG
    void updateColor(const GUIVisualizationSettings& s);
#endif

    /// @brief grant exclusive access to the simulation state
    void lock();

    /// @brief release exclusive access to the simulation state
    void unlock();

    /** @brief Returns the pointer to the unique instance of GUINet (singleton).
     * @return Pointer to the unique GUINet-instance
     * @exception ProcessError If a network was not yet constructed
     */
    static GUINet* getGUIInstance();

    /// @brief creates a wrapper for the given logic
    void createTLWrapper(MSTrafficLightLogic* tll) override;

    /// @brief return wheter the given logic (or rather it's wrapper) is selected in the GUI
    bool isSelected(const MSTrafficLightLogic* tll) const override;

private:
    /// @brief Initialises the tl-logic map and wrappers
    void initTLMap();

    friend class GUIOSGBuilder;

protected:
    /// @brief The visualization speed-up
    LayeredRTree myGrid;

    /// @brief The networks boundary
    Boundary myBoundary;

    /// @brief Wrapped MS-edges
    std::vector<GUIEdge*> myEdgeWrapper;

    /// @brief Wrapped MS-junctions
    std::vector<GUIJunctionWrapper*> myJunctionWrapper;

    /// @brief A detector dictionary
    std::vector<GUIDetectorWrapper*> myDetectorWrapper;

    /// @brief A calibrator dictionary
    std::vector<GUICalibrator*> myCalibratorWrapper;

    /// @brief Definition of a link-to-logic-id map
    typedef std::map<MSLink*, std::string> Links2LogicMap;
    /// @brief The link-to-logic-id map
    Links2LogicMap myLinks2Logic;


    /// @brief Definition of a traffic light-to-wrapper map
    typedef std::map<MSTrafficLightLogic*, GUITrafficLightLogicWrapper*> Logics2WrapperMap;
    /// @brief The traffic light-to-wrapper map
    Logics2WrapperMap myLogics2Wrapper;


    /// @brief The step durations (simulation, /*visualisation, */idle)
    int myLastSimDuration, /*myLastVisDuration, */myLastIdleDuration;

    long myLastVehicleMovementCount, myOverallVehicleCount;
    long myOverallSimDuration;

    /// @brief loaded edge data for visualization
    std::map<std::string, MSEdgeWeightsStorage*> myLoadedEdgeData;

    /// @brief class for discovering edge attributes
    class DiscoverAttributes : public SUMOSAXHandler {
    public:
        DiscoverAttributes(const std::string& file):
            SUMOSAXHandler(file), lastIntervalEnd(0) {};
        ~DiscoverAttributes() {};
        void myStartElement(int element, const SUMOSAXAttributes& attrs);
        std::vector<std::string> getEdgeAttrs();
        SUMOTime lastIntervalEnd;
    private:
        std::set<std::string> edgeAttrs;
    };

    class EdgeFloatTimeLineRetriever_GUI : public SAXWeightsHandler::EdgeFloatTimeLineRetriever {
    public:
        /// @brief Constructor
        EdgeFloatTimeLineRetriever_GUI(MSEdgeWeightsStorage* weightStorage) : myWeightStorage(weightStorage) {}

        /// @brief Destructor
        ~EdgeFloatTimeLineRetriever_GUI() { }

        /** @brief Adds an effort for a given edge and time period
         *
         * @param[in] id The id of the object to add a weight for
         * @param[in] val The effort
         * @param[in] beg The begin of the interval the weight is valid for
         * @param[in] end The end of the interval the weight is valid for
         * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever::addEdgeWeight
         */
        void addEdgeWeight(const std::string& id, double val, double beg, double end) const;
        void addEdgeRelWeight(const std::string& from, const std::string& to, double val, double beg, double end) const;

    private:
        /// @brief The storage that  edges shall be added to
        MSEdgeWeightsStorage* myWeightStorage;

    };

private:
    /// The mutex used to avoid concurrent updates of the vehicle buffer
    mutable FXMutex myLock;

};
