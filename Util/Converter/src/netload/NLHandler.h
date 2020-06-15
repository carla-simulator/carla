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
/// @file    NLHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Clemens Honomichl
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The XML-Handler for network loading
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/geom/Boundary.h>
#include <utils/shapes/ShapeHandler.h>
#include <microsim/MSRouteHandler.h>
#include "NLDiscreteEventBuilder.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NLContainer;
class NLDetectorBuilder;
class NLEdgeControlBuilder;
class NLJunctionControlBuilder;
class NLTriggerBuilder;
class MSTrafficLightLogic;


// ===========================================================================
// class definitions
// ===========================================================================


/**
 * @class NLShapeHandler
 * @brief The XML-Handler for shapes loading network loading
 *
 * This subclasses ShapeHandler with MSLane specific function
 */
class NLShapeHandler : public ShapeHandler {
public:
    NLShapeHandler(const std::string& file, ShapeContainer& sc) :
        ShapeHandler(file, sc) {}

    /// @brief Destructor
    virtual ~NLShapeHandler() {}

    Position getLanePos(const std::string& poiID, const std::string& laneID, double lanePos, double lanePosLat);

    virtual bool addLanePosParams() {
        return true;
    }
};


/**
 * @class NLHandler
 * @brief The XML-Handler for network loading
 *
 * The SAX2-handler responsible for parsing networks and routes to load.
 * This is an extension of the MSRouteHandler as routes and vehicles may also
 *  be loaded from network descriptions.
 */
class NLHandler : public MSRouteHandler {
public:
    /// @brief Definition of a lane vector
    typedef std::vector<MSLane*> LaneVector;

public:
    /** @brief Constructor
     *
     * @param[in] file Name of the parsed file
     * @param[in, out] net The network to fill
     * @param[in] detBuilder The detector builder to use
     * @param[in] triggerBuilder The trigger builder to use
     * @param[in] edgeBuilder The builder of edges to use
     * @param[in] junctionBuilder The builder of junctions to use
     */
    NLHandler(const std::string& file, MSNet& net,
              NLDetectorBuilder& detBuilder, NLTriggerBuilder& triggerBuilder,
              NLEdgeControlBuilder& edgeBuilder,
              NLJunctionControlBuilder& junctionBuilder);


    /// @brief Destructor
    virtual ~NLHandler();

    bool haveSeenInternalEdge() const {
        return myHaveSeenInternalEdge;
    }

    bool haveSeenDefaultLength() const {
        return myHaveSeenDefaultLength;
    }

    bool haveSeenNeighs() const {
        return myHaveSeenNeighs;
    }

    bool haveSeenAdditionalSpeedRestrictions() const {
        return myHaveSeenAdditionalSpeedRestrictions;
    }

    double networkVersion() const {
        return myNetworkVersion;
    }

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     * @todo Refactor/describe
     */
    virtual void myStartElement(int element,
                                const SUMOSAXAttributes& attrs);


    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     * @todo Refactor/describe
     */
    virtual void myEndElement(int element);
    //@}


protected:
    void addParam(const SUMOSAXAttributes& attrs);

    /** @brief Builds an e1 detector using the given specification
     * @param[in] attrs The attributes that define the detector
     */
    virtual void addE1Detector(const SUMOSAXAttributes& attrs);

    /** @brief Builds an e1 detector using the given specification
     * @param[in] attrs The attributes that define the detector
     */
    virtual void addInstantE1Detector(const SUMOSAXAttributes& attrs);

    /** @brief Builds an e2 detector using the given specification
     * @param[in] attrs The attributes that define the detector
     */
    virtual void addE2Detector(const SUMOSAXAttributes& attrs);

    /** @brief Starts building of an e3 detector using the given specification
     * @param[in] attrs The attributes that define the detector
     */
    void beginE3Detector(const SUMOSAXAttributes& attrs);

    /** @brief Adds an entry to the currently processed e3 detector
     * @param[in] attrs The attributes that define the entry
     */
    void addE3Entry(const SUMOSAXAttributes& attrs);

    /** @brief Adds an exit to the currently processed e3 detector
     * @param[in] attrs The attributes that define the exit
     */
    void addE3Exit(const SUMOSAXAttributes& attrs);

    /// Builds of an e3 detector using collected values
    virtual void endE3Detector();

    /** @brief Builds a vtype-detector using the given specification
     * @param[in] attrs The attributes that define the detector
     */
    virtual void addVTypeProbeDetector(const SUMOSAXAttributes& attrs);

    /** @brief Builds a routeProbe-detector using the given specification
     * @param[in] attrs The attributes that define the detector
     */
    virtual void addRouteProbeDetector(const SUMOSAXAttributes& attrs);

    /** @brief Builds edge or lane base mean data collector using the given specification
     * @param[in] attrs The attributes that define the detector
     * @param[in] objecttype The type (lane/edge) of the parsed mean data definition
     */
    virtual void addEdgeLaneMeanData(const SUMOSAXAttributes& attrs, int objecttype);

    /// Closes the process of building an edge
    virtual void closeEdge();


protected:
    /// The net to fill (preinitialised)
    MSNet& myNet;


private:
    /// begins the processing of an edge
    void beginEdgeParsing(const SUMOSAXAttributes& attrs);

    /// adds a lane to the previously opened edge
    void addLane(const SUMOSAXAttributes& attrs);

    ///  adds a request item to the current junction logic
    void addRequest(const SUMOSAXAttributes& attrs);

    /// begins the reading of a junction row logic
    void initJunctionLogic(const SUMOSAXAttributes& attrs);

    /// begins the reading of a traffic lights logic
    void initTrafficLightLogic(const SUMOSAXAttributes& attrs);

    /// adds a phase to the traffic lights logic currently build
    void addPhase(const SUMOSAXAttributes& attrs);


    /// opens a junction for processing
    virtual void openJunction(const SUMOSAXAttributes& attrs);

    void parseLanes(const std::string& junctionID, const std::string& def, std::vector<MSLane*>& into, bool& ok);

    /// adds a connection
    void addConnection(const SUMOSAXAttributes& attrs);

    virtual void openWAUT(const SUMOSAXAttributes& attrs);
    void addWAUTSwitch(const SUMOSAXAttributes& attrs);
    void addWAUTJunction(const SUMOSAXAttributes& attrs);

    /// Parses network location description
    void setLocation(const SUMOSAXAttributes& attrs);

    /** @begin Parses a district and creates a pseudo edge for it
     *
     * Called on the occurence of a "district" element, this method
     *  retrieves the id of the district and creates a district type
     *  edge with this id.
     *
     * @param[in] attrs The attributes (of the "district"-element) to parse
     * @exception ProcessError If an edge given in district@edges is not known
     */
    void addDistrict(const SUMOSAXAttributes& attrs);


    /** @begin Parses a district edge and connects it to the district
     *
     * Called on the occurence of a "dsource" or "dsink" element, this method
     *  retrieves the id of the approachable edge. If this edge is known
     *  and valid, the approaching edge is informed about it.
     *
     * @param[in] attrs The attributes to parse
     * @param[in] isSource whether a "dsource or a "dsink" was given
     * @todo No exception?
     */
    void addDistrictEdge(const SUMOSAXAttributes& attrs, bool isSource);

    /** @begin Parses a roundabout and sets flags for the edges
     * @param[in] attrs The attributes to parse
     */
    void addRoundabout(const SUMOSAXAttributes& attrs);


    void closeWAUT();

    /// @brief Parses the given character into an enumeration typed link direction
    LinkDirection parseLinkDir(const std::string& dir);

    /// @brief Parses the given character into an enumeration typed link state
    LinkState parseLinkState(const std::string& state);

protected:
    /// @brief A builder for object actions
    NLDiscreteEventBuilder myActionBuilder;

    /// @brief Information whether the currently parsed edge is internal and not wished, here
    bool myCurrentIsInternalToSkip;


    /// @brief The detector builder to use
    NLDetectorBuilder& myDetectorBuilder;

    /// @brief The trigger builder to use
    NLTriggerBuilder& myTriggerBuilder;

    /// @brief The edge builder to use
    NLEdgeControlBuilder& myEdgeControlBuilder;

    /// @brief The junction builder to use
    NLJunctionControlBuilder& myJunctionControlBuilder;

    /// The id of the current district
    std::string myCurrentDistrictID;

    /// internal information whether a tls-logic is currently read
    bool myAmParsingTLLogicOrJunction;

    /// The id of the currently processed WAUT
    std::string myCurrentWAUTID;

    /// The id of the currently processed edge type
    std::string myCurrentTypeID;

    /// The network offset
    Position myNetworkOffset;

    /// The network's boundaries
    Boundary myOrigBoundary, myConvBoundary;

    bool myCurrentIsBroken;

    bool myHaveWarnedAboutInvalidTLType;

    Parameterised myLastEdgeParameters;
    std::vector<Parameterised*> myLastParameterised;

    /// @brief whether the loaded network contains internal lanes
    bool myHaveSeenInternalEdge;

    /// @brief whether the loaded network contains edges with default lengths
    bool myHaveSeenDefaultLength;

    /// @brief whether the loaded network contains explicit neighbor lanes
    bool myHaveSeenNeighs;

    /// @brief whether additional files contained type-specific speed limits
    bool myHaveSeenAdditionalSpeedRestrictions;

    /// @brief the loaded network version
    double myNetworkVersion;

    /// @brief whether the location element was already loadee
    bool myNetIsLoaded;

    /// @brief temporary data for building the junction graph after network parsing is finished
    typedef std::map<std::string, std::pair<std::string, std::string> > JunctionGraph;
    JunctionGraph myJunctionGraph;

private:
    /** invalid copy constructor */
    NLHandler(const NLHandler& s);

    /** invalid assignment operator */
    NLHandler& operator=(const NLHandler& s);

};
