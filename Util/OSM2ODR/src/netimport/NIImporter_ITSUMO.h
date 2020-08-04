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
/// @file    NIImporter_ITSUMO.h
/// @author  Daniel Krajzewicz
/// @date    2011-09-16
///
// Importer for networks stored in ITSUMO format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <netbuild/NBCapacity2Lanes.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBEdgeCont;
class NBNetBuilder;
class NBNode;
class NBNodeCont;
class NBTrafficLightLogicCont;
class NBTypeCont;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_ITSUMO
 * @brief Importer for networks stored in ITSUMO format
 *
 */
class NIImporter_ITSUMO {
public:
    /** @brief Loads content of the optionally given ITSUMO network files
     *
     * If the option "itsumo-files" is set, the file(s) stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "itsumo-files" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);


private:
    /**
     * @class NodesHandler
     * @brief A class which parses an ITSUMO file
     */
    class Handler : public GenericSAXHandler {
    public:
        /** @brief Contructor
         * @param[in] toFill The container to fill
         */
        Handler(NBNetBuilder& toFill);


        /// @brief Destructor
        ~Handler();


    protected:
        /// @name inherited from GenericSAXHandler
        //@{

        /** @brief Called on the opening of a tag;
         *
         * @param[in] element ID of the currently opened element
         * @param[in] attrs Attributes within the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myStartElement
         */
        void myStartElement(int element, const SUMOSAXAttributes& attrs);


        /**
         * @brief Callback method for characters to implement by derived classes
         *
         * Called by "endElement" (see there).
         * @param[in] element The opened element, given as a int
         * @param[in] chars The complete embedded character string
         * @exceptions ProcessError These method may throw a ProcessError if something fails
         */
        void myCharacters(int element, const std::string& chars);


        /** @brief Callback method for a closing tag to implement by derived classes
         *
         * Called by "endElement" (see there).
         * @param[in] element The closed element, given as a int
         * @exceptions ProcessError These method may throw a ProcessError if something fails
         */
        void myEndElement(int element);
        //@}


    private:
        /// @brief The container to fill
        NBNetBuilder& myNetBuilder;

        /// @brief A temporary parameter map
        std::map<std::string, std::string> myParameter;


        struct Lane {
        public:
            Lane(const std::string& _id, int _idx, double _v)
                : id(_id), index(_idx), v(_v) {}
            std::string id;
            int index;
            double v;
        };

        std::vector<Lane> myCurrentLanes;

        struct LaneSet {
        public:
            LaneSet(const std::string& _id, const std::vector<Lane>& _lanes, double _v, int _pos, NBNode* _from, NBNode* _to)
                : id(_id), lanes(_lanes), v(_v), position(_pos), from(_from), to(_to) {}
            std::string id;
            std::vector<Lane> lanes;
            double v;
            int position;
            NBNode* from;
            NBNode* to;
        };

        std::map<std::string, LaneSet*> myLaneSets;
        std::vector<LaneSet*> myCurrentLaneSets;

        struct Section {
        public:
            Section(const std::string& _id, const std::vector<LaneSet*>& _laneSets)
                : id(_id), laneSets(_laneSets) {}
            std::string id;
            std::vector<LaneSet*> laneSets;
        };

        std::vector<Section*> mySections;


    private:
        /** @brief invalidated copy constructor */
        Handler(const Handler& s);

        /** @brief invalidated assignment operator */
        Handler& operator=(const Handler& s);

    };



    /**
     * @enum ItsumoXMLTag
     * @brief Numbers representing ITSUMO-XML - element names
     * @see GenericSAXHandler
     */
    enum ItsumoXMLTag {
        ITSUMO_TAG_NOTHING = 0,
        ITSUMO_TAG_SIMULATION,
        ITSUMO_TAG_NETWORK_ID,
        ITSUMO_TAG_NETWORK_NAME,
        ITSUMO_TAG_NODES,
        ITSUMO_TAG_NODE,
        ITSUMO_TAG_NODE_ID,
        ITSUMO_TAG_NODE_NAME,
        ITSUMO_TAG_X_COORD,
        ITSUMO_TAG_Y_COORD,
        ITSUMO_TAG_SOURCES,
        ITSUMO_TAG_SINKS,
        ITSUMO_TAG_TRAFFIC_LIGHTS,
        ITSUMO_TAG_STREETS,
        ITSUMO_TAG_STREET,
        ITSUMO_TAG_STREET_ID,
        ITSUMO_TAG_STREET_NAME,
        ITSUMO_TAG_SECTIONS,
        ITSUMO_TAG_SECTION,
        ITSUMO_TAG_SECTION_ID,
        ITSUMO_TAG_SECTION_NAME,
        ITSUMO_TAG_IS_PREFERENCIAL,
        ITSUMO_TAG_DELIMITING_NODE,
        ITSUMO_TAG_LANESETS,
        ITSUMO_TAG_LANESET,
        ITSUMO_TAG_LANESET_ID,
        ITSUMO_TAG_LANESET_POSITION,
        ITSUMO_TAG_START_NODE,
        ITSUMO_TAG_END_NODE,
        ITSUMO_TAG_TURNING_PROBABILITIES,
        ITSUMO_TAG_DIRECTION,
        ITSUMO_TAG_DESTINATION_LANESET,
        ITSUMO_TAG_PROBABILITY,
        ITSUMO_TAG_LANES,
        ITSUMO_TAG_LANE,
        ITSUMO_TAG_LANE_ID,
        ITSUMO_TAG_LANE_POSITION,
        ITSUMO_TAG_MAXIMUM_SPEED,
        ITSUMO_TAG_DECELERATION_PROB
    };


    /**
     * @enum ItsumoXMLAttr
     * @brief Numbers representing MATSIM-XML - attributes
     * @see GenericSAXHandler
     */
    enum ItsumoXMLAttr {
        ITSUMO_ATTR_NOTHING = 0
    };

    /// The names of MATSIM-XML elements (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry itsumoTags[];

    /// The names of MATSIM-XML attributes (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry itsumoAttrs[];


};
