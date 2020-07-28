/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIImporter_ArcView.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// Importer for networks stored in ArcView-shape format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class OGRFeature;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_ArcView
 * @brief Importer for networks stored in ArcView-shape format
 *
 * The current importer works only if SUMO was compiled with GDAL-support.
 *  If not, an error message is generated.
 *
 * @todo reinsert import via shapelib
 */
class NIImporter_ArcView {
public:
    /** @brief Loads content of the optionally given ArcView Shape files
     *
     * If the option "shapefile-prefix" is set, the file stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "shapefile-prefix" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);


protected:
    /** @brief Constructor
     * @param[in] oc Options container to read options from
     * @param[in] nc The node container to store nodes into
     * @param[in] ec The edge container to store edges into
     * @param[in] tc The type container to get edge types from
     * @param[in] dbf_name The name of the according database file
     * @param[in] shp_name The name of the according shape file
     * @param[in] speedInKMH Whether the speed shall be assumed to be given in km/h
     */
    NIImporter_ArcView(const OptionsCont& oc,
                       NBNodeCont& nc, NBEdgeCont& ec, NBTypeCont& tc,
                       const std::string& dbf_name, const std::string& shp_name,
                       bool speedInKMH);

    /// @brief Destructor
    ~NIImporter_ArcView();


    /** @brief Loads the shape files
     */
    void load();


private:
#ifdef HAVE_GDAL
    /** @brief Parses the maximum speed allowed on the edge currently processed
     * @param[in] f The entry to read the speed from
     * @param[in] edgeid The id of the edge for error output
     */
    double getSpeed(OGRFeature& f, const std::string& edgeid);


    /** @brief Parses the number of lanes of the edge currently processed
     * @param[in] f The entry to read the lane number from
     * @param[in] edgeid The id of the edge for error output
     * @param[in] speed The edge's speed used to help determinig the edge's lane number
     */
    int getLaneNo(OGRFeature& f,
                  const std::string& edgeid, double speed);

    /** @brief Parses the priority of the edge currently processed
     * @param[in] f The entry to read the priority from
     * @param[in] edgeid The id of the edge for error output
     */
    int getPriority(OGRFeature& f, const std::string& edgeid);


    /** @brief Checks whether the lane spread shall be changed
     *
     * If for the given edge an edge into the vice direction is already
     *  stored, both edges' lane spread functions are set to LaneSpreadFunction::RIGHT.
     *
     * @param[in] e The edge to check
     */
    void checkSpread(NBEdge* e);


    /** @brief Sets the value from the named field into "into"
     *
     * If the field's name was set on the command line, the value is tried to be retrieved, returning true on success.
     * If it cannot be retrieved, false is retuned, and the field's name is inserted into "into".
     *
     * If no field name was given, the standard value (defaultName) is used. In this case, an empty string is returned
     *  if the field does not exist.
     * @param[in] poFeature The feature to read from
     * @param[in] optionName The name of the option at which an optional field name is stored
     * @param[in] defaultName The field's default name
     * @param[in] prune Whether the value shall be prunned
     * @param[out] into The read value/missing field is stored here
     */
    bool getStringEntry(OGRFeature* poFeature, const std::string& optionName, const char* defaultName, bool prune, std::string& into);

    /// @brief return all fields support by the given feature
    std::vector<std::string> getFieldNames(OGRFeature* poFeature) const;

    /// @brief add list of parameters to edge
    void addParams(NBEdge* edge, OGRFeature* poFeature, const std::vector<std::string>& params) const;

#endif

private:
    /// @brief The options to use
    const OptionsCont& myOptions;

    /// @brief The name of the shape file
    std::string mySHPName;

    /// @brief A running number to assure unique edge ids
    int myNameAddition;

    /// @brief The container to add nodes to
    NBNodeCont& myNodeCont;

    /// @brief The container to add edges to
    NBEdgeCont& myEdgeCont;

    /// @brief The container to get the types from
    NBTypeCont& myTypeCont;

    /// @brief Whether the speed is given in km/h
    bool mySpeedInKMH;

    /// @brief A running number to assure unique ids (as fallback)
    int myRunningEdgeID;
    int myRunningNodeID;


private:
    /// @brief Invalidated copy constructor.
    NIImporter_ArcView(const NIImporter_ArcView&);

    /// @brief Invalidated assignment operator.
    NIImporter_ArcView& operator=(const NIImporter_ArcView&);

};
