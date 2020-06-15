/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
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
/// @file    PCLoaderOSM.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Melanie Knocke
/// @date    Wed, 19.11.2008
///
// A reader of pois and polygons stored in OSM-format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include "PCPolyContainer.h"
#include "PCTypeMap.h"
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class definitions
// ===========================================================================
class OptionsCont;


// ===========================================================================
// class declarations
// ===========================================================================
/**
 * @class PCLoaderOSM
 * @brief A reader of pois and polygons stored in OSM-format
 *
 * Reads pois stored as XML definition as given by the OpenStreetMap-API.
 */
class PCLoaderOSM : public SUMOSAXHandler {
public:
    /** @brief Loads pois/polygons assumed to be stored as OSM-XML
     *
     * If the option "osm-files" is set within the given options container,
     *  an instance of PCLoaderOSM is built and used as a handler for the
     *  files given in this option.
     *
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys/pois to
     * @param[in] tm The type map to use for setting values of loaded polys/pois
     * @exception ProcessError if something fails
     */
    static void loadIfSet(OptionsCont& oc, PCPolyContainer& toFill,
                          PCTypeMap& tm);


protected:

    /** @brief An internal representation of an OSM-node
     */
    struct PCOSMNode {
        /// @brief The node's id
        long long int id;
        /// @brief The longitude the node is located at
        double lon;
        /// @brief The latitude the node is located at
        double lat;
        /// @brief The nodes name (if any)
        std::string name;
        /// @brief Additional attributes
        std::map<std::string, std::string> myAttributes;
    };


    /** @brief An internal definition of a loaded relation
     */
    struct PCOSMRelation {
        /// @brief The relation's id
        long long int id;
        /// @brief The relation's name (if any)
        std::string name;
        /// @brief The list of ways this relation is made of
        std::vector<long long int> myWays;
        /// @brief Additional attributes
        std::map<std::string, std::string> myAttributes;
        /// @brief whether this relation is a valid polygon
        bool keep;
    };


    /** @brief An internal definition of a loaded edge
     */
    struct PCOSMEdge {
        /// @brief The edge's id
        long long int id;
        /// @brief The edge's name (if any)
        std::string name;
        /// @brief Information whether this area is closed
        bool myIsClosed;
        /// @brief The list of nodes this edge is made of
        std::vector<long long int> myCurrentNodes;
        /// @brief Additional attributes
        std::map<std::string, std::string> myAttributes;
        // @brief Wether this way constitutes a complete polygon object
        bool standalone;
    };

    typedef std::vector<PCOSMRelation*> Relations;
    typedef std::map<long long int, PCOSMRelation*> RelationsMap;
    typedef std::map<long long int, PCOSMEdge*> EdgeMap;

protected:
    /// @brief try add the polygon and return the next index on success
    static int addPolygon(const PCOSMEdge* edge, const PositionVector& vec, const PCTypeMap::TypeDef& def,
                          const std::string& fullType, int index, bool useName, PCPolyContainer& toFill, bool ignorePruning, bool withAttributes);

    /// @brief try add the POI and return the next index on success
    static int addPOI(const PCOSMNode* node, const Position& pos, const PCTypeMap::TypeDef& def,
                      const std::string& fullType, int index, bool useName, PCPolyContainer& toFill, bool ignorePruning, bool withAttributes);


protected:
    static const std::set<std::string> MyKeysToInclude;

private:
    static std::set<std::string> initMyKeysToInclude();

    /// @brief retrieve cartesian coordinate for given node
    static Position convertNodePosition(PCOSMNode* n);

protected:
    /**
     * @class NodesHandler
     * @brief A class which extracts OSM-nodes from a parsed OSM-file
     */
    class NodesHandler : public SUMOSAXHandler {
    public:
        /** @brief Contructor
         * @param[in] toFill The nodes container to fill
         * @param[in] withAttributes Whether all attributes shall be stored
         * @param[in] errorHandler The handler to report errors to (WarningHandler for ignoring errors)
         */
        NodesHandler(std::map<long long int, PCOSMNode*>& toFill, bool withAttributes,
                     MsgHandler& errorHandler);


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


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element);
        //@}


    private:
        /// @brief Whether all attributes shall be stored
        bool myWithAttributes;

        /// @brief The handler to report errors to (will be the WarningsHandler if --ignore-errors was set)
        MsgHandler& myErrorHandler;

        /// @brief The nodes container to fill
        std::map<long long int, PCOSMNode*>& myToFill;

        /// @brief Current path in order to know to what occuring values belong
        std::vector<int> myParentElements;

        /// @brief The id of the last parsed node
        long long int myLastNodeID;

    private:
        /// @brief Invalidated copy constructor
        NodesHandler(const NodesHandler& s);

        /// @brief Invalidated assignment operator
        NodesHandler& operator=(const NodesHandler& s);

    };

    /**
     * @class RelationsHandler
     * @brief A class which extracts relevant way-ids from relations in a parsed OSM-file
     */
    class RelationsHandler : public SUMOSAXHandler {
    public:
        /** @brief Constructor
         *
         * @param[in] osmNodes The previously parsed (osm-)nodes
         * @param[in] toFill The edges container to fill with read edges
         * @param[in] withAttributes Whether all attributes shall be stored
         * @param[in] errorHandler The handler to report errors to (WarningHandler for ignoring errors)
         */
        RelationsHandler(RelationsMap& additionalWays,
                         Relations& relations,
                         bool withAttributes,
                         MsgHandler& errorHandler);


        /// @brief Destructor
        ~RelationsHandler();


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


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element);
        //@}


    private:
        /// @brief additional ways which are reference by relations
        RelationsMap& myAdditionalWays;

        /// @brief the loaded relations
        Relations& myRelations;

        /// @brief Whether all attributes shall be stored
        bool myWithAttributes;

        /// @brief The handler to report errors to (will be the WarningsHandler if --ignore-errors was set)
        MsgHandler& myErrorHandler;

        /// @brief The currently parsed relation
        PCOSMRelation* myCurrentRelation;

        /// @brief the ways within the current relation
        std::vector<long long int> myCurrentWays;

        /// @brief Current path in order to know to what occuring values belong
        std::vector<long long int> myParentElements;

        /// @brief whether the last edge (way) should be kept because it had a key from the inclusion list
        bool myKeep;

    private:
        /// @brief Invalidated copy constructor
        RelationsHandler(const RelationsHandler& s);

        /// @brief Invalidated assignment operator
        RelationsHandler& operator=(const RelationsHandler& s);

    };


    /**
     * @class EdgesHandler
     * @brief A class which extracts OSM-edges from a parsed OSM-file
     */
    class EdgesHandler : public SUMOSAXHandler {
    public:
        /** @brief Constructor
         *
         * @param[in] osmNodes The previously parsed (osm-)nodes
         * @param[in] toFill The edges container to fill with read edges
         * @param[in] withAttributes Whether all attributes shall be stored
         * @param[in] additionalWays Additional ways which were identified as polygons to import
         * @param[in] errorHandler The handler to report errors to (WarningHandler for ignoring errors)
         */
        EdgesHandler(const std::map<long long int, PCOSMNode*>& osmNodes,
                     EdgeMap& toFill,
                     const RelationsMap& additionalWays,
                     bool withAttributes,
                     MsgHandler& errorHandler);


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


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element);
        //@}


    private:
        /// @brief Whether all attributes shall be stored
        bool myWithAttributes;

        /// @brief The handler to report errors to (will be the WarningsHandler if --ignore-errors was set)
        MsgHandler& myErrorHandler;

        /// @brief The previously parsed nodes
        const std::map<long long int, PCOSMNode*>& myOSMNodes;

        /// @brief A map of built edges
        EdgeMap& myEdgeMap;

        /// @brief additional ways which are reference by relations
        const RelationsMap& myAdditionalWays;

        /// @brief The currently built edge
        PCOSMEdge* myCurrentEdge;

        /// @brief Current path in order to know to what occuring values belong
        std::vector<int> myParentElements;

        /// @brief whether the last edge (way) should be kept because it had a key from the inclusion list
        bool myKeep;

    private:
        /// @brief Invalidated copy constructor
        EdgesHandler(const EdgesHandler& s);

        /// @brief Invalidated assignment operator
        EdgesHandler& operator=(const EdgesHandler& s);

    };

};
