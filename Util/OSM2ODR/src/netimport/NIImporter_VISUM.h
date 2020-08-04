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
/// @file    NIImporter_VISUM.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Fri, 19 Jul 2002
///
// A VISUM network importer
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <netbuild/NBCapacity2Lanes.h>
#include <utils/importio/LineHandler.h>
#include <utils/importio/LineReader.h>
#include <utils/importio/NamedColumnsParser.h>
#include "NIVisumTL.h"


// ===========================================================================
// class declaration
// ===========================================================================
class OptionsCont;
class NBNetBuilder;
class NBNodeCont;
class NBEdgeCont;
class NBNode;
class NBEdge;


// ===========================================================================
// class declaration
// ===========================================================================
/**
 * @class NIImporter_VISUM
 * @brief A VISUM network importer
 *
 * This class build an internal list of those VISUM-db entries which are
 *  supported, first. This list is sorted in a way that the parsed dbs can
 *  build upon each other as their related structures within the XML-input.
 *  So, nodes are loaded first, then edges, etc.
 *
 * Because these structures may have a different order within the VISUM-file
 *  than we need, at first the file is scanned and any occurence of one of the
 *  searched dbs is saved. That's where the "Found $XXX at YYY" are printed.
 *  "YYY" is the character position within the file.
 *
 * In a second step, the dbs are parsed in the order we need. It is asked for
 *  each subsequently whether it was found and if so, the proper parse_XXX()
 *  method is called.
 */
class NIImporter_VISUM {
public:
    /** @brief Loads network definition from the assigned option and stores it in the given network builder
     *
     * If the option "visum" is set, the file stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "visum" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);


protected:
    /** @brief constructor
     *
     * Builds the list of typed db parsers ("TypeParser") and stores them in
     *  mySingleDataParsers in the order the according db values must be parsed.
     *
     * @param[in,out] nb the network builder (storage) to fill with parsed values
     * @param[in] file The name of the file to parse
     * @param[in] capacity2Lanes The converter from flow to lanes
     * @param[in] useVisumPrio Information whether the VISUM type's priority shall be used
     */
    NIImporter_VISUM(NBNetBuilder& nb, const std::string& file,
                     NBCapacity2Lanes capacity2Lanes, bool useVisumPrio,
                     const std::string& languageFile);


    /// @brief destructor
    ~NIImporter_VISUM();


    /** @brief Parses the VISUM-network file storing the parsed structures within myNetBuilder
     *
     * At first, it is checked whether the file can be opened. A ProcessError is thrown
     *  if not. Otherwise, the file is scanned for occurences of db table begins. For each found
     *  db, its position within the file, and the column names are stored in the according
     *  TypeParser. After this, the sorted list of type parsers is one through and each
     *  found is used to parse the entries at the found positions using the found column names.
     *
     * @exception ProcessError If the file could not be opened
     */
    void load();

private:
    /** @brief Returns the value from the named column as a float
     *
     * @param[in] fieldName Name of the column to extract the float from
     * @return The parsed real
     * @exception OutOfBoundsException If the current data line has less entries than the float's position
     * @exception NumberFormatException If the float is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    double getNamedFloat(const std::string& fieldName);

    /** @brief The same, but two different names for the field are allowed
     *
     * @param[in] fieldName1 Name of the first column to extract the float from
     * @param[in] fieldName2 Name of the second column to extract the efloat from
     * @return The parsed real
     * @exception OutOfBoundsException If the current data line has less entries than the float's position
     * @exception NumberFormatException If the float is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    double getNamedFloat(const std::string& fieldName1, const std::string& fieldName2);


    /** @brief Returns the value from the named column as a float or the default value if an error occurs
     *
     * @param[in] fieldName Name of the column to extract the float from
     * @param[in] defaultValue The default to return in the case of an error
     * @return The parsed real or the default value if an error while parsing occurred
     */
    double getNamedFloat(const std::string& fieldName, double defaultValue);

    /** @brief The same, but two different names for the field are allowed
     *
     * @param[in] fieldName1 Name of the first column to extract the float from
     * @param[in] fieldName2 Name of the second column to extract the efloat from
     * @param[in] defaultValue The default to return in the case of an error
     * @return The parsed real or the default value if an error while parsing occurred
     */
    double getNamedFloat(const std::string& fieldName1, const std::string& fieldName2,
                         double defaultValue);


    /** @brief Returns the value from the named column as a normalised string
     *
     * "Normalised" means herein that the leading '0' (zeros) are prunned.
     *
     * @param[in] fieldName Name of the column to extract the string from
     * @return The parsed, normalised string
     * @exception OutOfBoundsException If the current data line has less entries than the string's position
     * @exception NumberFormatException If the string is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    std::string getNamedString(const std::string& fieldName);

    /** @brief The same, but two different names for the field are allowed
     *
     * @param[in] fieldName1 Name of the first column to extract the string from
     * @param[in] fieldName2 Name of the second column to extract the string from
     * @return The parsed, normalised string
     * @exception OutOfBoundsException If the current data line has less entries than the string's position
     * @exception NumberFormatException If the string is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    std::string getNamedString(const std::string& fieldName1, const std::string& fieldName2);


    /** @brief tries to get a double which is possibly assigned to a certain modality
     *
     * When the double cannot be extracted using the given name, "(IV)" is
     * appended to the begin of the name. Note that this function does not
     * yet support public traffic.
     *
     * @param[in] name Name of the column to extract the real from
     * @return The real stored under the named column, or if not found the one from name + suffix, or if not found -1
     */
    double getWeightedFloat(const std::string& name, const std::string& suffix);

    /// @brief as above but with two alternative names
    double getWeightedFloat2(const std::string& name, const std::string& name2, const std::string& suffix);

    /// @brief parse permissions
    SVCPermissions getPermissions(const std::string& name, bool warn = false, SVCPermissions unknown = SVCAll);

    /** @brief tries to get a bool which is possibly assigned to a certain modality
     *
     * When the bool cannot be extracted using the given name, "IV" is
     * appended to the begin of the name. Note that this function does not
     * yet support public traffic.
     *
     * @param[in] name Name of the column to extract the bool from
     * @return The bool stored under the named column, or if not found the one from "(IV)"+name, or if not found false
     */
    bool getWeightedBool(const std::string& name);


    /** @brief Tries to get the node which name is stored in the given field
     *
     * If the field can not be parsed, an exception is thrown. Prints an error if the
     *  node could not be found, returning 0. Otherwise, if the field could be parsed
     *  and the node was found, this node is returned.
     *
     * @param[in] fieldName Name of the column to extract the node's name from
     * @return An already known node with the found name
     * @exception OutOfBoundsException If the current data line has less entries than the node id's position
     * @exception NumberFormatException If the node id is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    NBNode* getNamedNode(const std::string& fieldName);
    NBNode* getNamedNodeSecure(const std::string& fieldName, NBNode* fallback = 0);

    /** @brief The same, but two different names for the field are allowed
     *
     * @param[in] fieldName1 Name of the first column to extract the node's name from
     * @param[in] fieldName2 Name of the second column to extract the node's name from
     * @return An already known node with the found name
     * @exception OutOfBoundsException If the current data line has less entries than the node id's position
     * @exception NumberFormatException If the node id is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    NBNode* getNamedNode(const std::string& fieldName1, const std::string& fieldName2);


    /** @brief Tries to get the edge which name is stored in the given field
     *
     * If the field can not be parsed, an exception is thrown. Prints an error if the
     *  edge could not be found, returning 0. Otherwise, if the field could be parsed
     *  and the edge was found, this edge is returned.
     *
     * @param[in] fieldName Name of the column to extract the edge's name from
     * @return An already known edge with the found name
     * @exception OutOfBoundsException If the current data line has less entries than the edge id's position
     * @exception NumberFormatException If the edge id is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    NBEdge* getNamedEdge(const std::string& fieldName);

    /** @brief The same, but two different names for the field are allowed
     *
     * @param[in] fieldName1 Name of the first column to extract the edge's name from
     * @param[in] fieldName2 Name of the second column to extract the edge's name from
     * @return An already known edge with the found name
     * @exception OutOfBoundsException If the current data line has less entries than the edge id's position
     * @exception NumberFormatException If the edge id is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    NBEdge* getNamedEdge(const std::string& fieldName1, const std::string& fieldName2);


    /** @brief Tries to get the edge which name is stored in the given field
     * continuating the search for a subedge that ends at the given node
     *
     * If the field can not be parsed, an exception is thrown. Prints an error if the
     *  edge could not be found, returning 0. Otherwise, if the field could be parsed
     *  and the edge was found, this edge is returned.
     *
     * @param[in] fieldName Name of the column to extract the edge's name from
     * @param[in] node The node the consecutive edge must end at in order to be returned
     * @return The edge's continuation up to the given node, 0 if not found
     * @exception OutOfBoundsException If the current data line has less entries than the edge id's position
     * @exception NumberFormatException If the edge id is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    NBEdge* getNamedEdgeContinuating(const std::string& fieldName, NBNode* node);

    /** @brief The same, but two different names for the field are allowed
     *
     * @param[in] fieldName1 Name of the first column to extract the edge's name from
     * @param[in] fieldName2 Name of the second column to extract the edge's name from
     * @param[in] node The node the consecutive edge must end at in order to be returned
     * @return The edge's continuation up to the given node, 0 if not found
     * @exception OutOfBoundsException If the current data line has less entries than the edge id's position
     * @exception NumberFormatException If the edge id is not numeric
     * @exception UnknownElement If the named data field is not in the line
     */
    NBEdge* getNamedEdgeContinuating(const std::string& fieldName1, const std::string& fieldName2,
                                     NBNode* node);

    /** @brief The same, but for an already given edge
     *
     * @param[in] begin The edge to get the continuation of
     * @param[in] node The node the consecutive edge must end at in order to be returned
     * @return The edge's continuation up to the given node, 0 if not found
     */
    NBEdge* getNamedEdgeContinuating(NBEdge* begin, NBNode* node);


    /** @brief Returns the edge that connects both nodes
     *
     * @param[in] FromNode Name of the node the edge shall start at
     * @param[in] ToNode Name of the node the edge shall end at
     * @return The edge connecting both nodes, 0 if no such edge exists
     */
    NBEdge* getEdge(NBNode* FromNode, NBNode* ToNode);


    /** @brief Returns the opposite direction of the given edge
     *
     * Because the opposite direction edge may be split, not the the plain opposite
     *  edge, the one which name is obtained by adding/removing the leading '-', is returned,
     *  but its continuation until the named node.
     *
     * @param[in] edge Name of the edge to find the opposite of
     * @param[in] node Name of the node the opposite edge's continuation must end at
     * @return The found opposite edge's continuation, 0 if not found
     */
    NBEdge* getReversedContinuating(NBEdge* edge, NBNode* node);


    /** @brief Builds a node for the given district and returns it
     *
     * If the district does not exist, an error is generated and 0 returned. Otherwise
     *  a position for the new node is computed and the new node is built using a combination
     *  of the district name and the node name as id. If Inserting this node into
     *  the net builder fails, zero is returned.
     *
     * @param[in] id Name of the district
     * @param[in] dest Name of the according network node
     * @param[in] isSource Information whether this node will be used as a source
     * @return The built node, zero if an error occurred
     */
    NBNode* buildDistrictNode(const std::string& id, NBNode* dest, bool isSource);


    /** @brief Returns whether both nodes are a valid combination of from/to-nodes
     *
     * They are valid if both are !=0 and differ.
     *
     * @param[in] from The from-node
     * @param[in] from The to-node
     * @return Whether the nodes may be used
     */
    bool checkNodes(NBNode* from, NBNode* to);


private:
    /**
     * @brief Definition of a function for parsing a single line from a certain db
     *
     * This function may assume that both the LineParser is initialised
     *  with the current line.
     */
    typedef void (NIImporter_VISUM::*ParsingFunction)();

    /**
     * @struct TypeParser
     * @brief A complete call description for parsing a single db.
     */
    struct TypeParser {
        /** @brief The name of the db
         *
         * Initialised in the constructor */
        std::string name;

        /** @brief  Pointer to the function used for parsing
         *
         * Initialised in the constructor */
        ParsingFunction function;

        /** @brief Position of the according db within the file
         *
         * Set to -1 in the constructor, and reset to the position while
         *  scaning the file if the according db was found */
        long position;

        /** @brief The column names
         *
         * Set while scaning the file if the according db was found */
        std::string pattern;

    };



    /// @brief Parses VSYS
    void parse_VSysTypes();

    /// @brief Parses STRECKENTYP
    void parse_Types();

    /// @brief Parses KNOTEN
    void parse_Nodes();

    /// @brief Parses BEZIRK
    void parse_Districts();

    /// @brief Parses PUNKT
    void parse_Point();


    /// @brief Parses STRECKE/STRECKEN
    void parse_Edges();

    /// @brief Parses FLAECHENELEMENT
    void parse_PartOfArea();

    /// @brief Parses FLAECHENELEMENT
    void parse_Kante();


    /// @brief Parses ANBINDUNG
    void parse_Connectors();
    void parse_Connectors_legacy();

    /// @brief Parses ABBIEGEBEZIEHUNG/ABBIEGER
    void parse_Turns();

    /// @brief Parses STRECKENPOLY
    void parse_EdgePolys();

    /// @brief Parses FAHRSTREIFEN
    void parse_Lanes();

    /// @brief Parses LSA/SIGNALANLAGE
    void parse_TrafficLights();

    /// @brief Parses KNOTENZULSA/SIGNALANLAGEZUKNOTEN
    void parse_NodesToTrafficLights();

    /// @brief Parses LSASIGNALGRUPPE/SIGNALGRUPPE
    void parse_SignalGroups();

    /// @brief Parses ABBZULSASIGNALGRUPPE/SIGNALGRUPPEZUABBIEGER
    void parse_TurnsToSignalGroups();

    /// @brief Parses ABBZULSASIGNALGRUPPE/SIGNALGRUPPEZUABBIEGER
    void parse_AreaSubPartElement();

    /// @brief Parses LSAPHASE/PHASE
    void parse_Phases();

    /// @brief Parses LSASIGNALGRUPPEZULSAPHASE
    void parse_SignalGroupsToPhases();

    /// @brief Parses FAHRSTREIFENABBIEGER
    void parse_LanesConnections();


    /** @brief Adds a parser into the sorted list of parsers to use
     *
     * @param[in] name db name to assign the parser to
     * @param[in] function The function to use for parsing the named db
     */
    void addParser(const std::string& name, ParsingFunction function);

private:

    /// @brief whether the edge id ends with _nodeID
    static bool isSplitEdge(NBEdge* edge, NBNode* node);


private:
    /// @brief The network builder to fill with loaded values
    NBNetBuilder& myNetBuilder;

    /// @brief The name of the parsed file, for error reporting
    std::string myFileName;

    /// @brief The line reader to use to read from the file
    LineReader myLineReader;

    /** @brief the parser to parse the information from the data lines
     *
     * the order of columns within the visum format seems to vary, so a named parser is needed */
    NamedColumnsParser myLineParser;

    /// @brief The converter to compute the lane number of edges from their capacity
    NBCapacity2Lanes myCapacity2Lanes;

    /// @brief Definition of a storage for vsystypes
    typedef std::map<std::string, std::string> VSysTypeNames;
    /// @brief The used vsystypes
    VSysTypeNames myVSysTypes;

    /// @brief Definition of the list of known parsers
    typedef std::vector<TypeParser> ParserVector;
    /// @brief List of known parsers
    ParserVector mySingleDataParsers;

    /// @brief Definition of a map for loaded traffic lights (id->tls)
    typedef std::map<std::string, NIVisumTL*> NIVisumTL_Map;
    /// @brief List of visum traffic lights
    NIVisumTL_Map myTLS;

    /// @brief Already read edges
    std::vector<std::string > myTouchedEdges;

    /// @brief Information whether VISUM priority information shall be used
    bool myUseVisumPrio;

    /// @brief The name of the currently parsed item used for error reporting
    std::string myCurrentID;


    /// @brief A map of point ids to positions
    std::map<long long int, Position> myPoints;

    /// @brief A map of edge (not road, but "edge" in this case) ids to from/to-points
    std::map<long long int, std::pair<long long int, long long int> > myEdges;

    /// @brief A map from district shape definition name to the district
    std::map<long long int, NBDistrict*> myShapeDistrictMap;

    /// @brief A map from area parts to area ids
    std::map<long long int, std::vector<long long int> > mySubPartsAreas;

    /// @brief A temporary storage for district shapes as they are filled incrementally
    std::map<NBDistrict*, PositionVector> myDistrictShapes;

protected:
    /**
     * @enum VISUM keys
     * @brief Numbers representing VISUM keywords
     */
    enum VISUM_KEY {
        VISUM_SYS,
        VISUM_LINKTYPE,
        VISUM_NODE,
        VISUM_DISTRICT,
        VISUM_POINT,
        VISUM_LINK,
        VISUM_V0,
        VISUM_TYPES,
        VISUM_RANK,
        VISUM_CAPACITY,
        VISUM_XCOORD,
        VISUM_YCOORD,
        VISUM_FROMNODE,
        VISUM_TONODE,
        VISUM_TYPE,
        VISUM_TYP,
        VISUM_ID,
        VISUM_CODE,
        VISUM_DISTRICT_CONNECTION,
        VISUM_SOURCE_DISTRICT,
        VISUM_FROMNODENO,
        VISUM_DIRECTION,
        VISUM_SURFACEID,
        VISUM_FACEID,
        VISUM_FROMPOINTID,
        VISUM_TOPOINTID,
        VISUM_EDGE,
        VISUM_VIANODENO,
        VISUM_NUMLANES,
        VISUM_TURN,
        VISUM_INDEX,
        VISUM_LINKPOLY,
        VISUM_SURFACEITEM,
        VISUM_FACEITEM,
        VISUM_EDGEID,
        VISUM_ORIGIN,
        VISUM_DESTINATION,
        VISUM_NO // must be the last one
    };

    /// Strings for the keywords
    static StringBijection<VISUM_KEY>::Entry KEYS_DE[];

    /// @brief link directions
    static StringBijection<VISUM_KEY> KEYS;

    void loadLanguage(const std::string& file);
};
