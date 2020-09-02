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
/// @file    NIImporter_Vissim.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Lukas Grohmann (AIT)
/// @author  Gerald Richter (AIT)
/// @date    Sept 2002
///
// Importer for networks stored in Vissim format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <utils/common/RGBColor.h>
#include <utils/geom/Position.h>
#include "tempstructs/NIVissimExtendedEdgePoint.h"
#include "NIVissimElements.h"
#include <utils/xml/SUMOSAXHandler.h>
#include "tempstructs/NIVissimEdge.h"
#include "tempstructs/NIVissimConnection.h"
#include "tempstructs/NIVissimConflictArea.h"

#include <utils/common/StringBijection.h>
#include <utils/common/StringTokenizer.h>
#include <list>

// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class NBNetBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_Vissim
 * @brief Importer for networks stored in Vissim format
 */
class NIImporter_Vissim {
public:
    /** @brief Loads network definition from the assigned option and stores it in the given network builder
     *
     * If the option "vissim-file" is set, the file stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "vissim-file" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);



private:

    typedef std::map<std::string, std::list<std::string> > nodeMap;
    nodeMap elementData;

    /**
     * @class NIVissimSingleTypeXMLHandler_Streckendefinition
     * @brief A class which extracts VISSIM-Strecken from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_Streckendefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         * @param[in] strecken_dic  The strecken dictionary to fill
         */
        //NIVissimXMLHandler_Streckendefinition(std::map<int, VissimXMLEdge>& toFill);
        NIVissimXMLHandler_Streckendefinition(nodeMap& elemData);


        /// @brief Destructor
        ~NIVissimXMLHandler_Streckendefinition();

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

        void myEndElement(int element);
        //@}

    private:

        //std::map<int, VissimXMLEdge> myToFill;
        nodeMap& myElemData;

        /// @brief The current hierarchy level
        int myHierarchyLevel;

        /// @brief check if the link is a connector
        bool isConnector;

        /// @brief ID of the currently parsed node, for reporting mainly
        int myLastNodeID;

        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_Streckendefinition(const NIVissimXMLHandler_Streckendefinition& s);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_Streckendefinition& operator=(const NIVissimXMLHandler_Streckendefinition& s);
    };


private:
    /**
     * @class NIVissimSingleTypeXMLHandler_Zuflussdefinition
     * @brief A class which extracts VISSIM-Zuflüsse from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_Zuflussdefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         */
        NIVissimXMLHandler_Zuflussdefinition();


        /// @brief Destructor
        ~NIVissimXMLHandler_Zuflussdefinition();

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



        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_Zuflussdefinition(const NIVissimXMLHandler_Zuflussdefinition& z);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_Zuflussdefinition& operator=(const NIVissimXMLHandler_Zuflussdefinition& z);
    };


private:
    /**
     * @class NIVissimSingleTypeXMLHandler_Parkplatzdefinition
     * @brief A class which extracts VISSIM-Parkplätze from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_Parkplatzdefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         */
        NIVissimXMLHandler_Parkplatzdefinition();


        /// @brief Destructor
        ~NIVissimXMLHandler_Parkplatzdefinition();

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



        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_Parkplatzdefinition(const NIVissimXMLHandler_Parkplatzdefinition& z);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_Parkplatzdefinition& operator=(const NIVissimXMLHandler_Parkplatzdefinition& z);
    };


private:
    /**
     * @class NIVissimSingleTypeXMLHandler_Fahrzeugklassendefinition
     * @brief A class which extracts VISSIM-Fahrzeugklassen from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_Fahrzeugklassendefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         * @param[in] elemData  The string container to fill
         */

        NIVissimXMLHandler_Fahrzeugklassendefinition(nodeMap& elemData);


        /// @brief Destructor
        ~NIVissimXMLHandler_Fahrzeugklassendefinition();

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

        void myEndElement(int element);
        //@}

    private:

        //std::map<int, VissimXMLEdge> myToFill;
        nodeMap& myElemData;

        /// @brief The current hierarchy level
        int myHierarchyLevel;

        /// @brief ID of the currently parsed node, for reporting mainly
        int myLastNodeID;

        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_Fahrzeugklassendefinition(const NIVissimXMLHandler_Fahrzeugklassendefinition& f);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_Fahrzeugklassendefinition& operator=(const NIVissimXMLHandler_Fahrzeugklassendefinition& f);
    };

private:
    /**
     * @class NIVissimSingleTypeXMLHandler_VWunschentscheidungsdefinition
     * @brief A class which extracts VISSIM-VWunschentscheidungen from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_VWunschentscheidungsdefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         * @param[in] elemData  The string container to fill
         */

        NIVissimXMLHandler_VWunschentscheidungsdefinition(nodeMap& elemData);


        /// @brief Destructor
        ~NIVissimXMLHandler_VWunschentscheidungsdefinition();

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

        void myEndElement(int element);
        //@}

    private:

        //std::map<int, VissimXMLEdge> myToFill;
        nodeMap& myElemData;

        /// @brief The current hierarchy level
        int myHierarchyLevel;

        /// @brief ID of the currently parsed node, for reporting mainly
        int myLastNodeID;

        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_VWunschentscheidungsdefinition(const NIVissimXMLHandler_VWunschentscheidungsdefinition& vW);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_VWunschentscheidungsdefinition& operator=(const NIVissimXMLHandler_VWunschentscheidungsdefinition& vW);
    };




private:
    /**
     * @class NIVissimSingleTypeXMLHandler_Geschwindigkeitsverteilungsdefinition
     * @brief A class which extracts VISSIM-Geschwindigkeitsverteilung from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         * @param[in] elemData  The string container to fill
         */

        NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition(nodeMap& elemData);


        /// @brief Destructor
        ~NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition();

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

        void myEndElement(int element);
        //@}

    private:

        //std::map<int, VissimXMLEdge> myToFill;
        nodeMap& myElemData;

        /// @brief The current hierarchy level
        int myHierarchyLevel;

        /// @brief ID of the currently parsed node, for reporting mainly
        int myLastNodeID;

        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition(const NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition& vW);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition& operator=(const NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition& vW);
    };


private:
    /**
     * @class NIVissimXMLHandler_Routenentscheidungsdefinition
     * @brief A class which extracts VISSIM-Routes from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_Routenentscheidungsdefinition : public GenericSAXHandler {
    public:
        /** @brief Constructor
         * @param[in] elemData  The string container to fill
         */

        NIVissimXMLHandler_Routenentscheidungsdefinition(nodeMap& elemData);


        /// @brief Destructor
        ~NIVissimXMLHandler_Routenentscheidungsdefinition();

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

        void myEndElement(int element);
        //@}

    private:

        //std::map<int, VissimXMLEdge> myToFill;
        nodeMap& myElemData;

        /// @brief The current hierarchy level
        int myHierarchyLevel;

        /// @brief ID of the currently parsed node, for reporting mainly
        int myLastNodeID;

        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_Routenentscheidungsdefinition(const NIVissimXMLHandler_Routenentscheidungsdefinition& r);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_Routenentscheidungsdefinition& operator=(const NIVissimXMLHandler_Routenentscheidungsdefinition& r);
    };


private:
    /**
     * @class NIVissimSingleTypeXMLHandler_ConflictArea
     * @brief A class which extracts VISSIM-ConflictAreas from a parsed VISSIM-file
     */
    class NIVissimXMLHandler_ConflictArea : public GenericSAXHandler {
    public:
        /** @brief Constructor
         */
        NIVissimXMLHandler_ConflictArea();


        /// @brief Destructor
        ~NIVissimXMLHandler_ConflictArea();


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


        /** @brief invalidated copy constructor */
        NIVissimXMLHandler_ConflictArea(const NIVissimXMLHandler_ConflictArea& c);

        /** @brief invalidated assignment operator */
        NIVissimXMLHandler_ConflictArea& operator=(const NIVissimXMLHandler_ConflictArea& c);
    };


protected:
    /// constructor
    NIImporter_Vissim(NBNetBuilder& nb);

    /// destructor
    ~NIImporter_Vissim();

    /// loads the vissim file
    void load(const OptionsCont& options);

    bool admitContinue(const std::string& tag);

public:
    class VissimSingleTypeParser {
    public:
        /// Constructor
        VissimSingleTypeParser(NIImporter_Vissim& parent);

        /// Destructor
        virtual ~VissimSingleTypeParser();

        /** @brief Parses a single data type.
            Returns whether no error occurred */
        virtual bool parse(std::istream& from) = 0;

    protected:
        /// reads from the stream and returns the lower case version of the read value
        std::string myRead(std::istream& from);

        /// as myRead, but returns "DATAEND" when the current field has ended
        std::string readEndSecure(std::istream& from,
                                  const std::string& excl = "");

        std::string readEndSecure(std::istream& from,
                                  const std::vector<std::string>& excl);

        /// overrides the optional label definition; returns the next tag as done by readEndSecure
        std::string overrideOptionalLabel(std::istream& from,
                                          const std::string& tag = "");

        /// returns the 2d-position saved as next within the stream
        Position getPosition(std::istream& from);

        /** @brief parses a listof vehicle types assigned to the current data field
            One should remeber, that -1 means "all" vehicle types */
        std::vector<int> parseAssignedVehicleTypes(std::istream& from,
                const std::string& next);

        NIVissimExtendedEdgePoint readExtEdgePointDef(std::istream& from);

        /** @brief Reads the structures name
            We cannot use the "<<" operator, as names may contain more than one word
            which are joined using '"'. */
        std::string readName(std::istream& from);

        /** @brief Overreads the named parameter (if) given and skips the rest until "DATAEND"
         */
        bool skipOverreading(std::istream& from, const std::string& name = "");

        /// Reads from the stream until the keywor occurs
        void readUntil(std::istream& from, const std::string& name);

    private:
        NIImporter_Vissim& myVissimParent;

    private:
        /// @brief Invalidated assignment operator.
        VissimSingleTypeParser& operator=(const VissimSingleTypeParser&);

    };


    /// definition of a map from color names to color definitions
    typedef std::map<std::string, RGBColor> ColorMap;

private:
    bool readContents(std::istream& strm);
    void postLoadBuild(double offset);


    /// adds name-to-id - relationships of known elements into myKnownElements
    void insertKnownElements();

    /// adds id-to-parser - relationships of elements to parse into myParsers
    void buildParsers();

private:
    /// Definition of a map from element names to their numerical representation
    typedef std::map<std::string, NIVissimElement> ToElemIDMap;

    /// Map from element names to their numerical representation
    ToElemIDMap myKnownElements;

    /// Definition of a map from an element's numerical id to his parser
    typedef std::map<NIVissimElement, VissimSingleTypeParser*> ToParserMap;

    /// Parsers by element id
    ToParserMap myParsers;

    /// a map from color names to color definitions
    ColorMap myColorMap;

    std::string myLastSecure;

    NBNetBuilder& myNetBuilder;

private:
    /// @brief Invalidated copy constructor.
    NIImporter_Vissim(const NIImporter_Vissim&);

    /// @brief Invalidated assignment operator.
    NIImporter_Vissim& operator=(const NIImporter_Vissim&);



    /**
     * @enum VissimXMLTag
     * @brief Numbers representing VISSIM-XML - element names
     * @see GenericSAXHandler
     */
    enum VissimXMLTag {
        VISSIM_TAG_NOTHING = 0,
        VISSIM_TAG_NETWORK,
        VISSIM_TAG_LANES,
        VISSIM_TAG_LANE,
        VISSIM_TAG_LINK,
        VISSIM_TAG_LINKS,
        VISSIM_TAG_POINTS3D,
        VISSIM_TAG_POINT3D,
        VISSIM_TAG_LINKPOLYPOINT,
        VISSIM_TAG_LINKPOLYPTS,
        VISSIM_TAG_FROM,
        VISSIM_TAG_TO,
        VISSIM_TAG_VEHICLE_INPUT,
        VISSIM_TAG_PARKINGLOT,
        VISSIM_TAG_VEHICLE_CLASS,
        VISSIM_TAG_INTOBJECTREF,
        VISSIM_TAG_SPEED_DECISION,
        VISSIM_TAG_SPEED_DIST,
        VISSIM_TAG_DATAPOINT,
        VISSIM_TAG_DECISION_STATIC,
        VISSIM_TAG_ROUTE_STATIC,
        VISSIM_TAG_CA
    };


    /**
     * @enum VissimXMLAttr
     * @brief Numbers representing VISSIM-XML - attributes
     * @see GenericSAXHandler
     */
    enum VissimXMLAttr {
        VISSIM_ATTR_NOTHING = 0,
        VISSIM_ATTR_NO,
        VISSIM_ATTR_NAME,
        VISSIM_ATTR_X,
        VISSIM_ATTR_Y,
        VISSIM_ATTR_ZOFFSET,
        VISSIM_ATTR_ZUSCHLAG1,
        VISSIM_ATTR_ZUSCHLAG2,
        VISSIM_ATTR_WIDTH,
        VISSIM_ATTR_LINKBEHAVETYPE,
        VISSIM_ATTR_LANE,
        VISSIM_ATTR_POS,
        VISSIM_ATTR_LINK,
        VISSIM_ATTR_INTLINK,
        VISSIM_ATTR_PERCENTAGE,
        VISSIM_ATTR_DISTRICT,
        VISSIM_ATTR_COLOR,
        VISSIM_ATTR_KEY,
        VISSIM_ATTR_FX,
        VISSIM_ATTR_DESTLINK,
        VISSIM_ATTR_DESTPOS,
        VISSIM_ATTR_LINK1,
        VISSIM_ATTR_LINK2,
        VISSIM_ATTR_STATUS
    };

    /// The names of VISSIM-XML elements (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry vissimTags[];

    /// The names of VISSIM-XML attributes (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry vissimAttrs[];


};
