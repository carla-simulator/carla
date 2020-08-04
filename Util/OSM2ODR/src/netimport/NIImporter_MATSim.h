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
/// @file    NIImporter_MATSim.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Tue, 26.04.2011
///
// Importer for networks stored in MATSim format
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
 * @class NIImporter_MATSim
 * @brief Importer for networks stored in MATSim format
 *
 */
class NIImporter_MATSim {
public:
    /** @brief Loads content of the optionally given MATSIM network files
     *
     * If the option "matsim-files" is set, the file(s) stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "matsim-files" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);


private:
    /**
     * @class NodesHandler
     * @brief A class which extracts MATSIM-nodes from a parsed MATSIM-file
     */
    class NodesHandler : public GenericSAXHandler {
    public:
        /** @brief Contructor
         * @param[in] toFill The nodes container to fill
         */
        NodesHandler(NBNodeCont& toFill);


        /// @brief Destructor
        ~NodesHandler();


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
        //@}


    private:
        /// @brief The nodes container to fill
        NBNodeCont& myNodeCont;


    private:
        /** @brief invalidated copy constructor */
        NodesHandler(const NodesHandler& s);

        /** @brief invalidated assignment operator */
        NodesHandler& operator=(const NodesHandler& s);

    };



    /**
     * @class EdgesHandler
     * @brief A class which extracts MATSIM-edges from a parsed MATSIM-file
     */
    class EdgesHandler : public GenericSAXHandler {
    public:
        /** @brief Constructor
         *
         * @param[in] nc The node container to retrieve nodes form
         * @param[in, out] toFill The edges container to fill with read edges
         * @param[in] keepEdgeLengths Whether the loaded lengths shal be used
         * @param[in] lanesFromCapacity Whether the lane number shall be computed from the capacity
         * @param[in] capacity2Lanes The converter from flow to lanes
         */
        EdgesHandler(const NBNodeCont& nc, NBEdgeCont& toFill,
                     bool keepEdgeLengths, bool lanesFromCapacity,
                     NBCapacity2Lanes capacity2Lanes);


        /// @brief Destructor
        ~EdgesHandler();


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
        //@}


    private:
        /// @brief The previously parsed nodes
        const NBNodeCont& myNodeCont;

        /// @brief The edge container to fill
        NBEdgeCont& myEdgeCont;

        /// @brief The capacity norming
        double myCapacityNorm;

        /// @brief Whether the loaded lengths shal be used
        bool myKeepEdgeLengths;

        /// @brief Whether the lane number shall be computed from the capacity
        bool myLanesFromCapacity;

        /// @brief The converter from flow to lanes
        NBCapacity2Lanes myCapacity2Lanes;


    private:
        /** @brief invalidated copy constructor */
        EdgesHandler(const EdgesHandler& s);

        /** @brief invalidated assignment operator */
        EdgesHandler& operator=(const EdgesHandler& s);

    };


    /**
     * @enum MatsimXMLTag
     * @brief Numbers representing MATSIM-XML - element names
     * @see GenericSAXHandler
     */
    enum MatsimXMLTag {
        MATSIM_TAG_NOTHING = 0,
        MATSIM_TAG_NETWORK,
        MATSIM_TAG_NODE,
        MATSIM_TAG_LINK,
        MATSIM_TAG_LINKS
    };


    /**
     * @enum MatsimXMLAttr
     * @brief Numbers representing MATSIM-XML - attributes
     * @see GenericSAXHandler
     */
    enum MatsimXMLAttr {
        MATSIM_ATTR_NOTHING = 0,
        MATSIM_ATTR_ID,
        MATSIM_ATTR_X,
        MATSIM_ATTR_Y,
        MATSIM_ATTR_FROM,
        MATSIM_ATTR_TO,
        MATSIM_ATTR_LENGTH,
        MATSIM_ATTR_FREESPEED,
        MATSIM_ATTR_CAPACITY,
        MATSIM_ATTR_PERMLANES,
        MATSIM_ATTR_ONEWAY,
        MATSIM_ATTR_MODES,
        MATSIM_ATTR_ORIGID,
        MATSIM_ATTR_CAPPERIOD,
        MATSIM_ATTR_CAPDIVIDER
    };

    /// The names of MATSIM-XML elements (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry matsimTags[];

    /// The names of MATSIM-XML attributes (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry matsimAttrs[];


};
