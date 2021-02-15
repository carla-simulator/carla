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
/// @file    GNEDataHandler.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// Builds data objects for netedit
/****************************************************************************/
#pragma once

// ===========================================================================
// included modules
// ===========================================================================

#include <config.h>

#include <utils/geom/Position.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GNENet;
class GNEEdge;
class GNETAZElement;
class GNEEdgeData;
class GNEEdgeRelData;
class GNETAZRelData;
class GNEGenericData;
class GNEDataSet;
class GNEDataInterval;
class GNEDataSet;
class GNEUndoList;

// ===========================================================================
// class definitions
// ===========================================================================

/// @class GNEDataHandler
/// @brief Builds data objects for GNENet (busStops, chargingStations, detectors, etc..)
class GNEDataHandler : public SUMOSAXHandler {
public:

    /// @brief Stack used to save the last inserted element
    struct HierarchyInsertedDatas {

        /// @brief insert new element (called only in function myStartElement)
        void insertElement(SumoXMLTag tag);

        /// @brief commit element insertion (used to save last correct data interval element)
        void commitDataIntervalInsertion(GNEDataInterval* dataIntervalCreated);

        /// @brief commit element insertion (used to save last correct generic data element)
        void commitGenericDataInsertion(GNEGenericData* dataElementCreated);

        /// @brief pop last inserted element (used only in function myEndElement)
        void popElement();

        /// @brief retrieve parent generic data correspond to current status of myInsertedElements
        GNEDataInterval* retrieveParentDataInterval(SumoXMLTag expectedTag) const;

        /// @brief return last generic data inserted
        GNEDataInterval* getLastInsertedDataInterval() const;

        /// @brief retrieve parent generic data correspond to current status of myInsertedElements
        GNEGenericData* retrieveParentGenericData(SumoXMLTag expectedTag) const;

        /// @brief return last generic data inserted
        GNEGenericData* getLastInsertedGenericData() const;

    private:
        /// @brief vector used as stack
        std::vector<std::pair<SumoXMLTag, std::pair<GNEDataInterval*, GNEGenericData*> > > myInsertedElements;
    };

    /// @brief Constructor
    GNEDataHandler(const std::string& file, GNENet* net);

    /// @brief Destructor
    ~GNEDataHandler();

    /// @name inherited from ShapeHandler
    /// @{
    /**@brief Called on the opening of a tag;
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element, const SUMOSAXAttributes& attrs);

    /** @brief Called when a closing tag occurs
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);

    /// @}

    /// @name building methods
    ///
    /// Called with parsed values, these methods build the data.
    /// @{
    /**@brief Build datas
     * @param[in] viewNet pointer to viewNet in wich data will be created
     * @param[in] allowUndoRedo enable or disable remove created data with ctrl + Z / ctrl + Y
     * @param[in] tag tag of the data to create
     * @param[in] attrs SUMOSAXAttributes with attributes
     * @param[in] HierarchyInsertedDatas pointer to HierarchyInsertedDatas (can be null)
     * @return true if was sucesfully created, false in other case
     */
    static bool buildData(GNENet* net, bool allowUndoRedo, SumoXMLTag tag, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas);

    /**@brief Builds DataSet
     * @param[in] net GNENet in which element will be inserted
     * @param[in] allowUndoRedo enable or disable remove created data with ctrl + Z / ctrl + Y
     * @param[in] dataSetID data set interval
     * @exception InvalidArgument If the DataSet can not be added to the net
     */
    static GNEDataSet* buildDataSet(GNENet* net, bool allowUndoRedo, const std::string& dataSetID);

    /**@brief Builds DataInterval
     * @param[in] net GNENet in which element will be inserted
     * @param[in] allowUndoRedo enable or disable remove created data with ctrl + Z / ctrl + Y
     * @param[in] dataSet GNEDataInterval in which this DataInterval is saved
     * @param[in] begin interval begin
     * @param[in] end interval end
     * @exception InvalidArgument If the DataInterval can not be added to the net
     */
    static GNEDataInterval* buildDataInterval(GNENet* net, bool allowUndoRedo, GNEDataSet* dataSetParent,
            const double begin, const double end);

    /**@brief Builds edgeData
     * @param[in] net GNENet in which element will be inserted
     * @param[in] allowUndoRedo enable or disable remove created data with ctrl + Z / ctrl + Y
     * @param[in] dataIntervalParent GNEDataInterval in which this edgeData is saved
     * @param[in] edge GNEEdge parent
     * @param[in] parameters parameters map
     * @exception InvalidArgument If the edgeData can not be added to the net
     */
    static GNEGenericData* buildEdgeData(GNENet* net, bool allowUndoRedo, GNEDataInterval* dataIntervalParent,
                                         GNEEdge* edge, const std::map<std::string, std::string>& parameters);

    /**@brief Builds edgeRelationData
     * @param[in] net GNENet in which element will be inserted
     * @param[in] allowUndoRedo enable or disable remove created data with ctrl + Z / ctrl + Y
     * @param[in] dataIntervalParent GNEDataInterval in which this edgeRelationData is saved
     * @param[in] fromEdge GNEEdge from
     * @param[in] toEdge GNEEdge to
     * @param[in] parameters parameters map
     * @exception InvalidArgument If the edgeRelationData can not be added to the net
     */
    static GNEGenericData* buildEdgeRelationData(GNENet* net, bool allowUndoRedo, GNEDataInterval* dataIntervalParent,
            GNEEdge* fromEdge, GNEEdge* toEdge, const std::map<std::string, std::string>& parameters);

    /**@brief Builds TAZRelationData
     * @param[in] net GNENet in which element will be inserted
     * @param[in] allowUndoRedo enable or disable remove created data with ctrl + Z / ctrl + Y
     * @param[in] dataIntervalParent GNEDataInterval in which this TAZRelationData is saved
     * @param[in] fromTAZ GNETAZ from
     * @param[in] toTAZ GNETAZ to
     * @param[in] parameters parameters map
     * @exception InvalidArgument If the TAZRelationData can not be added to the net
     */
    static GNEGenericData* buildTAZRelationData(GNENet* net, bool allowUndoRedo, GNEDataInterval* dataIntervalParent,
            GNETAZElement* fromTAZ, GNETAZElement* toTAZ, const std::map<std::string, std::string>& parameters);

protected:
    /// @name parsing methods
    ///
    /// These methods parse the attributes for each of the described data
    /// and call the according methods to build the data
    /// @{

    /**@brief Parses his values and builds a interval
     * @param[in] attrs SAX-attributes which define the data
     */
    static bool parseAndBuildInterval(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas);

    /**@brief Parses his values and builds a edge data
     * @param[in] attrs SAX-attributes which define the data
     */
    static bool parseAndBuildEdgeData(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas);

    /**@brief Parses his values and builds a edge relation data
     * @param[in] attrs SAX-attributes which define the data
     */
    static bool parseAndBuildEdgeRelationData(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas);

    /**@brief Parses his values and builds a TAZ relation data
     * @param[in] attrs SAX-attributes which define the data
     */
    static bool parseAndBuildTAZRelationData(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, HierarchyInsertedDatas* insertedDatas);

    /// @}

private:
    /**@brief Parse parameter and insert it in the last created data
     * @param[in] attrs SAX-attributes which define the parameter
     */
    void parseParameter(const SUMOSAXAttributes& attrs);

    /// @brief pointer to net
    GNENet* myNet;

    /// @brief HierarchyInsertedDatas used for insert children
    HierarchyInsertedDatas myHierarchyInsertedGenericDatas;

    /// @brief invalidate copy constructor
    GNEDataHandler(const GNEDataHandler& s) = delete;

    /// @brief invalidate assignment operator
    GNEDataHandler& operator=(const GNEDataHandler& s) = delete;
};
