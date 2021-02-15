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
/// @file    GNENet.h
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// The lop level container for GNE-network-components such as GNEEdge and
// GNEJunction.  Contains an internal instances of NBNetBuilder GNE components
// wrap netbuild-components of this underlying NBNetBuilder and supply
// visualisation and editing capabilities (adapted from GUINet)
//
// WorkrouteFlow (rough draft)
//   wrap NB-components
//   do netedit stuff
//   call NBNetBuilder::buildLoaded to save results
//
/****************************************************************************/
#pragma once

#include "GNENetHelper.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNENet
 * @brief A NBNetBuilder extended by visualisation and editing capabilities
 */
class GNENet : public GUIGlObject {

public:
    /**@brief Constructor
     * @param[in] netbuilder the netbuilder which may already have been filled
     * GNENet becomes responsible for cleaning this up
     **/
    GNENet(NBNetBuilder* netBuilder);

    /// @brief Destructor
    ~GNENet();

    /// @brief retrieve all attribute carriers of Net
    GNENetHelper::AttributeCarriers* getAttributeCarriers() const;

    /// @brief obtain instance of PathCalculator
    GNENetHelper::PathCalculator* getPathCalculator();

    /// @name inherited from GUIGlObject
    /// @{
    /**@brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /**@brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /**@brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;

    /// @brief expand boundary
    void expandBoundary(const Boundary& newBoundary);

    /// @brief Returns the Z boundary (stored in the x() coordinate) values of 0 do not affect the boundary
    const Boundary& getZBoundary() const;

    /// @brief add Z in net boundary
    void addZValueInBoundary(const double z);

    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    /// @}

    /// @brief returns the bounder of the network
    const Boundary& getBoundary() const;

    /**@brief Returns the RTree used for visualisation speed-up
     * @return The visualisation speed-up
     * @note only use in GNEViewNet constructor
     */
    SUMORTree& getGrid();

    /**@brief creates a new junction
     * @param[in] position The position of the new junction
     * @param[in] undoList The undolist in which to mark changes
     * @return the new junction
     */
    GNEJunction* createJunction(const Position& pos, GNEUndoList* undoList);

    /**@brief creates a new edge (unless an edge with the same geometry already
     * exists)
     * @param[in] src The starting junction
     * @param[in] dest The ending junction
     * @param[in] tpl The template edge from which to copy attributes (including lane attrs)
     * @param[in] undoList The undoList in which to mark changes
     * @param[in] suggestedName
     * @param[in] wasSplit Whether the edge was created from a split
     * @param[in] allowDuplicateGeom Whether to create the edge even though another edge with the same geometry exists
     * @param[in] recomputeConnections Whether connections on the affected junctions must be recomputed
     * @return The newly created edge or 0 if no edge was created
     */
    GNEEdge* createEdge(GNEJunction* src, GNEJunction* dest, GNEEdge* tpl, GNEUndoList* undoList,
                        const std::string& suggestedName = "", bool wasSplit = false, bool allowDuplicateGeom = false,
                        bool recomputeConnections = true);

    /**@brief removes junction and all incident edges
     * @param[in] junction The junction to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteJunction(GNEJunction* junction, GNEUndoList* undoList);

    /**@brief removes edge
     * @param[in] edge The edge to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteEdge(GNEEdge* edge, GNEUndoList* undoList, bool recomputeConnections);

    /**@brief replaces edge
     * @param[in] which The edge to be replaced
     * @param[in] by The replacement edge
     * @param[in] undoList The undolist in which to mark changes
     */
    void replaceIncomingEdge(GNEEdge* which, GNEEdge* by, GNEUndoList* undoList);

    /**@brief removes lane
     * @param[in] lane The lane to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteLane(GNELane* lane, GNEUndoList* undoList, bool recomputeConnections);

    /**@brief remove connection
     * @param[in] connection The connection to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteConnection(GNEConnection* connection, GNEUndoList* undoList);

    /**@brief remove crossing
     * @param[in] crossing The crossing to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteCrossing(GNECrossing* crossing, GNEUndoList* undoList);

    /**@brief remove additional
     * @param[in] additional The Shape to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteAdditional(GNEAdditional* additional, GNEUndoList* undoList);

    /**@brief remove shape
     * @param[in] shape The Shape to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteShape(GNEShape* shape, GNEUndoList* undoList);

    /**@brief remove TAZElement
     * @param[in] TAZElement The TAZElement to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteTAZElement(GNETAZElement* TAZElement, GNEUndoList* undoList);

    /**@brief remove demand element
     * @param[in] demandElement The Shape to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteDemandElement(GNEDemandElement* demandElement, GNEUndoList* undoList);

    /**@brief remove data set
     * @param[in] dataSet The data set to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteDataSet(GNEDataSet* dataSet, GNEUndoList* undoList);

    /**@brief remove data interval
     * @param[in] dataInterval The data interval to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteDataInterval(GNEDataInterval* dataInterval, GNEUndoList* undoList);

    /**@brief remove generic data
     * @param[in] genericData The generic data to be removed
     * @param[in] undoList The undolist in which to mark changes
     */
    void deleteGenericData(GNEGenericData* genericData, GNEUndoList* undoList);

    /**@brief duplicates lane
     * @param[in] lane The lane to be duplicated
     * @param[in] undoList The undolist in which to mark changes
     */
    void duplicateLane(GNELane* lane, GNEUndoList* undoList, bool recomputeConnections);

    /**@brief transform lane to restricted lane
     * @param[in] vclass vehicle class to restrict
     * @param[in] lane The lane to be transformed
     * @param[in] undoList The undolist in which to mark changes
     */
    bool restrictLane(SUMOVehicleClass vclass, GNELane* lane, GNEUndoList* undoList);

    /**@brief add restricted lane to edge
     * @param[in] vclass vehicle class to restrict
     * @param[in] edge The edge in which insert restricted lane
     * @param[in] undoList The undolist in which to mark changes
     */
    bool addRestrictedLane(SUMOVehicleClass vclass, GNEEdge* edge, int index, GNEUndoList* undoList);

    /**@brief remove restricted lane
     * @param[in] vclass vehicle class to restrict
     * @param[in] edge the edge in which remove sidewalk
     * @param[in] undoList The undolist in which to mark changes
     */
    bool removeRestrictedLane(SUMOVehicleClass vclass, GNEEdge* edge, GNEUndoList* undoList);

    /**@brief split edge at position by inserting a new junction
     * @param[in] edge The edge to be split
     * @param[in] pos The position on which to insert the new junction
     * @return The new junction
     */
    GNEJunction* splitEdge(GNEEdge* edge, const Position& pos, GNEUndoList* undoList, GNEJunction* newJunction = 0);

    /**@brief split all edges at position by inserting one new junction
     * @param[in] edge The edge to be split
     * @param[in] oppositeEdge The oppositeEdge to be split
     * @param[in] pos The position on which to insert the new junction
     */
    void splitEdgesBidi(GNEEdge* edge, GNEEdge* oppositeEdge, const Position& pos, GNEUndoList* undoList);

    /**@brief reverse edge
     * @param[in] edge The edge to be reversed
     */
    void reverseEdge(GNEEdge* edge, GNEUndoList* undoList);

    /**@brief add reversed edge
     * @param[in] edge The edge of which to add the reverse
     * @return Return the new edge or 0
     */
    GNEEdge* addReversedEdge(GNEEdge* edge, GNEUndoList* undoList);

    /**@brief merge the given junctions
     * edges between the given junctions will be deleted
     * @param[in] moved The junction that will be eliminated
     * @param[in] target The junction that will be enlarged
     * @param[in] undoList The undo list with which to register changes
     */
    void mergeJunctions(GNEJunction* moved, GNEJunction* target, GNEUndoList* undoList);

    /// @brief select all roundabout edges and junctions for the current roundabout
    void selectRoundabout(GNEJunction* junction, GNEUndoList* undoList);

    /// @brief transform the given junction into a roundabout
    void createRoundabout(GNEJunction* junction, GNEUndoList* undoList);

    /**@brief get junction by id
     * @param[in] id The id of the desired junction
     * @param[in] failHard Whether attempts to retrieve a nonexisting junction should result in an exception
     * @throws UnknownElement
     */
    GNEJunction* retrieveJunction(const std::string& id, bool failHard = true) const;

    /**@brief get edge by id
     * @param[in] id The id of the desired edge
     * @param[in] failHard Whether attempts to retrieve a nonexisting edge should result in an exception
     * @throws UnknownElement
     */
    GNEEdge* retrieveEdge(const std::string& id, bool failHard = true) const;

    /**@brief get edge by from and to GNEJunction
     * @param[in] id The id of the desired edge
     * @param[in] failHard Whether attempts to retrieve a nonexisting edge should result in an exception
     * @throws UnknownElement
     */
    GNEEdge* retrieveEdge(GNEJunction* from, GNEJunction* to, bool failHard = true) const;

    /**@brief get Connection by id
    * @param[in] id The id of the desired Connection
    * @param[in] failHard Whether attempts to retrieve a nonexisting Connection should result in an exception
    * @throws UnknownElement
    */
    GNEConnection* retrieveConnection(const std::string& id, bool failHard = true) const;

    /**@brief return all connections
    * @param[in] onlySelected Whether to return only selected connections
    */
    std::vector<GNEConnection*> retrieveConnections(bool onlySelected = false) const;

    /**@brief get Crossing by id
    * @param[in] id The id of the desired Crossing
    * @param[in] failHard Whether attempts to retrieve a nonexisting Crossing should result in an exception
    * @throws UnknownElement
    */
    GNECrossing* retrieveCrossing(const std::string& id, bool failHard = true) const;

    /**@brief return all crossings
    * @param[in] onlySelected Whether to return only selected crossings
    */
    std::vector<GNECrossing*> retrieveCrossings(bool onlySelected = false) const;

    /**@brief get a single attribute carrier based on a GLID
    * @param[in] ids the GL IDs for which to retrive the AC
    * @param[in] failHard Whether attempts to retrieve a nonexisting AttributeCarrier should result in an exception
    * @throws InvalidArgument if GL ID doesn't have a associated Attribute Carrier
    */
    GNEAttributeCarrier* retrieveAttributeCarrier(const GUIGlID id, bool failHard = true) const;

    /**@brief get the attribute carriers based on Type
     * @param[in] type The GUI-type of the AC. SUMO_TAG_NOTHING returns all elements (Warning: bottleneck)
     */
    std::vector<GNEAttributeCarrier*> retrieveAttributeCarriers(SumoXMLTag type = SUMO_TAG_NOTHING);

    /**@brief return all edges
     * @param[in] onlySelected Whether to return only selected edges
     */
    std::vector<GNEEdge*> retrieveEdges(bool onlySelected = false);

    /**@brief return all lanes
     * @param[in] onlySelected Whether to return only selected lanes
     */
    std::vector<GNELane*> retrieveLanes(bool onlySelected = false);

    /**@brief get lane by id
     * @param[in] id The id of the desired lane
     * @param[in] failHard Whether attempts to retrieve a nonexisting lane should result in an exception
     * @param[in] checkVolatileChange Used by additionals after recomputing with volatile options.
     * @throws UnknownElement
     */
    GNELane* retrieveLane(const std::string& id, bool failHard = true, bool checkVolatileChange = false);

    /**@brief return all junctions
    * @param[in] onlySelected Whether to return only selected junctions
    */
    std::vector<GNEJunction*> retrieveJunctions(bool onlySelected = false);

    /**@brief return shape by type shapes
    * @param[in] shapeTag Type of shape.
    * @param[in] onlySelected Whether to return only selected junctions
    */
    std::vector<GNEShape*> retrieveShapes(SumoXMLTag shapeTag, bool onlySelected = false);

    /**@brief return all shapes
    * @param[in] onlySelected Whether to return only selected junctions
    */
    std::vector<GNEShape*> retrieveShapes(bool onlySelected = false);

    /// @brief inform that net has to be saved
    void requireSaveNet(bool value);

    /// @brief return if net has to be saved
    bool isNetSaved() const;

    /**@brief save the network
     * @param[in] oc The OptionsCont which knows how and where to save
     */
    void save(OptionsCont& oc);

    /**@brief save plain xml representation of the network (and nothing else)
     * @param[in] oc The OptionsCont which knows how and where to save
     */
    void savePlain(OptionsCont& oc);

    /**@brief save log of joined junctions (and nothing else)
     * @param[in] oc The OptionsCont which knows how and where to save
     */
    void saveJoined(OptionsCont& oc);

    /// @brief Set the net to be notified of network changes
    void setViewNet(GNEViewNet* viewNet);

    /// @brief add GL Object into net
    void addGLObjectIntoGrid(GNEAttributeCarrier* AC);

    /// @brief add GL Object into net
    void removeGLObjectFromGrid(GNEAttributeCarrier* AC);

    /// @brief modifies endpoins of the given edge
    void changeEdgeEndpoints(GNEEdge* edge, const std::string& newSourceID, const std::string& newDestID);

    /// @brief get view net
    GNEViewNet* getViewNet() const;

    /// @brief get all selected attribute carriers (or only relative to current supermode
    std::vector<GNEAttributeCarrier*> getSelectedAttributeCarriers(bool ignoreCurrentSupermode);

    /// @brief returns the tllcont of the underlying netbuilder
    NBTrafficLightLogicCont& getTLLogicCont();

    /// @brief returns the NBEdgeCont of the underlying netbuilder
    NBEdgeCont& getEdgeCont();

    /// @brief initialize GNEConnections
    void initGNEConnections();

    /// @brief recompute the network and update lane geometries
    void computeAndUpdate(OptionsCont& oc, bool volatileOptions);

    /**@brief trigger full netbuild computation
     * param[in] window The window to inform about delay
     * param[in] force Whether to force recomputation even if not needed
     * param[in] volatileOptions enable or disable volatile options
     * param[in] additionalPath path in wich additionals were saved before recomputing with volatile options
     * param[in] demandPath path in wich demand elements were saved before recomputing with volatile options
     * param[in] dataPath path in wich data elements were saved before recomputing with volatile options
     */
    void computeNetwork(GNEApplicationWindow* window, bool force = false, bool volatileOptions = false, std::string additionalPath = "", std::string demandPath = "", std::string dataPath = "");

    /**@brief compute demand elements
     * param[in] window The window to inform about delay
     */
    void computeDemandElements(GNEApplicationWindow* window);

    /**@brief compute data elements
     * param[in] window The window to inform about delay
     */
    void computeDataElements(GNEApplicationWindow* window);

    /**@brief join selected junctions
     * @note difference to mergeJunctions:
     *  - can join more than 2
     *  - connected edges will keep their geometry (big junction shape is created)
     *  - no hirarchy: if any junction has a traffic light than the resuling junction will
     */
    bool joinSelectedJunctions(GNEUndoList* undoList);

    /// @brief clear invalid crossings
    bool cleanInvalidCrossings(GNEUndoList* undoList);

    /// @brief removes junctions that have no edges
    void removeSolitaryJunctions(GNEUndoList* undoList);

    /// @brief clean unused routes
    void cleanUnusedRoutes(GNEUndoList* undoList);

    /// @brief join routes
    void joinRoutes(GNEUndoList* undoList);

    /// @brief clean invalid demand elements
    void cleanInvalidDemandElements(GNEUndoList* undoList);

    /// @brief replace the selected junction by geometry node(s) and merge the edges
    void replaceJunctionByGeometry(GNEJunction* junction, GNEUndoList* undoList);

    /// @brief replace the selected junction by a list of junctions for each unique edge endpoint
    void splitJunction(GNEJunction* junction, bool reconnect, GNEUndoList* undoList);

    /// @brief clear junction's connections
    void clearJunctionConnections(GNEJunction* junction, GNEUndoList* undoList);

    /// @brief reset junction's connections
    void resetJunctionConnections(GNEJunction* junction, GNEUndoList* undoList);

    /**@brief trigger recomputation of junction shape and logic
     * param[in] window The window to inform about delay
     */
    void computeJunction(GNEJunction* junction);

    /// @brief inform the net about the need for recomputation
    void requireRecompute();

    /// @brief check if net requiere recomputing
    bool isNetRecomputed() const;

    /// @brief check if net has GNECrossings
    bool netHasGNECrossings() const;

    /// @brief get pointer to the main App
    FXApp* getApp();

    /// @brief get net builder
    NBNetBuilder* getNetBuilder() const;

    /// @brief add edge id to the list of explicit turnarounds
    void addExplicitTurnaround(std::string id);

    /// @brief remove edge id from the list of explicit turnarounds
    void removeExplicitTurnaround(std::string id);

    /// @name Functions related to Additional Items
    /// @{

    /**@brief Returns the named additional
     * @param[in] type tag with the type of additional
     * @param[in] id The id of the additional to return.
     * @param[in] failHard Whether attempts to retrieve a nonexisting additional should result in an exception
     */
    GNEAdditional* retrieveAdditional(SumoXMLTag type, const std::string& id, bool hardFail = true) const;

    /**@brief return all additionals
     * @param[in] onlySelected Whether to return only selected additionals
     */
    std::vector<GNEAdditional*> retrieveAdditionals(bool onlySelected = false) const;

    /**@brief Returns the number of additionals of the net
     * @param[in] type type of additional to count. SUMO_TAG_NOTHING will count all additionals
     * @return Number of additionals of the net
     */
    int getNumberOfAdditionals(SumoXMLTag type = SUMO_TAG_NOTHING) const;

    /// @brief inform that additionals has to be saved
    void requireSaveAdditionals(bool value);

    /**@brief save additional elements of the network
    * @param[in] filename name of the file in wich save additionals
    */
    void saveAdditionals(const std::string& filename);

    /// @brief check if additionals are saved
    bool isAdditionalsSaved() const;

    /// @brief generate additional id
    std::string generateAdditionalID(SumoXMLTag type) const;

    /// @}

    /// @name Functions related to DemandElement Items
    /// @{

    /**@brief Returns the named demand element
     * @param[in] type tag with the type of demand element
     * @param[in] id The id of the demand element to return.
     * @param[in] failHard Whether attempts to retrieve a nonexisting demand element should result in an exception
     */
    GNEDemandElement* retrieveDemandElement(SumoXMLTag type, const std::string& id, bool hardFail = true) const;

    /**@brief return all demand elements
     * @param[in] onlySelected Whether to return only selected demand elements
     */
    std::vector<GNEDemandElement*> retrieveDemandElements(bool onlySelected = false) const;

    /**@brief Returns the number of demand elements of the net
     * @param[in] type type of demand element to count. SUMO_TAG_NOTHING will count all demand elements
     * @return Number of demand elements of the net
     */
    int getNumberOfDemandElements(SumoXMLTag type = SUMO_TAG_NOTHING) const;

    /// @brief inform that demand elements has to be saved
    void requireSaveDemandElements(bool value);

    /**@brief save demand element elements of the network
    * @param[in] filename name of the file in wich save demand elements
    */
    void saveDemandElements(const std::string& filename);

    /// @brief check if demand elements are saved
    bool isDemandElementsSaved() const;

    /// @brief generate demand element id
    std::string generateDemandElementID(SumoXMLTag tag) const;

    /// @}

    /// @name Functions related to DataSet Items
    /// @{

    /**@brief Returns the named data set
     * @param[in] id The id of the data set to return.
     * @param[in] failHard Whether attempts to retrieve a nonexisting data set should result in an exception
     */
    GNEDataSet* retrieveDataSet(const std::string& id, bool hardFail = true) const;

    ///@brief return all data sets
    std::vector<GNEDataSet*> retrieveDataSets() const;

    /**@brief return all generic datas
    * @param[in] onlySelected Whether to return only selected generic datas
    */
    std::vector<GNEGenericData*> retrieveGenericDatas(bool onlySelected = false) const;

    /**@brief Returns the number of data sets of the net
     * @return Number of data sets of the net
     */
    int getNumberOfDataSets() const;

    /// @brief inform that data sets has to be saved
    void requireSaveDataElements(bool value);

    /**@brief save data set elements of the network
    * @param[in] filename name of the file in wich save data sets
    */
    void saveDataElements(const std::string& filename);

    /// @brief check if data sets are saved
    bool isDataElementsSaved() const;

    /// @brief generate data set id
    std::string generateDataSetID(const std::string& prefix) const;

    /// @brief return a set of parameters for the given data Interval
    std::set<std::string> retrieveGenericDataParameters(const std::string& genericDataTag, const double begin, const double end) const;

    /// @brief return a set of parameters for the given dataSet, generic data Type, begin and end
    std::set<std::string> retrieveGenericDataParameters(const std::string& dataSetID, const std::string& genericDataTag,
            const std::string& beginStr, const std::string& endStr) const;

    /// @brief get minimum interval
    double getDataSetIntervalMinimumBegin() const;

    /// @brief get maximum interval
    double getDataSetIntervalMaximumEnd() const;

    /// @}

    /// @name Functions related with Shapes
    /// @{

    /**@brief Returns the named shape
     * @param[in] type tag with the type of shape
     * @param[in] id The id of the shape to return.
     * @param[in] failHard Whether attempts to retrieve a nonexisting shape should result in an exception
     */
    GNEShape* retrieveShape(SumoXMLTag type, const std::string& id, bool hardFail = true) const;

    /**@brief return all shapes
     * @param[in] onlySelected Whether to return only selected shapes
     */
    std::vector<GNEShape*> retrieveShapes(bool onlySelected = false) const;

    /// @brief generate Shape ID
    std::string generateShapeID(SumoXMLTag shapeTag) const;

    /**@brief Returns the number of shapes of the net
     * @param[in] type type of shape to count. SUMO_TAG_NOTHING will count all shapes
     * @return Number of shapes of the net
     */
    int getNumberOfShapes(SumoXMLTag type = SUMO_TAG_NOTHING) const;
    /// @}

    /// @name Functions related with TAZElements
    /// @{

    /**@brief Returns the named TAZElement
     * @param[in] type tag with the type of TAZElement
     * @param[in] id The id of the TAZElement to return.
     * @param[in] failHard Whether attempts to retrieve a nonexisting TAZElement should result in an exception
     */
    GNETAZElement* retrieveTAZElement(SumoXMLTag type, const std::string& id, bool hardFail = true) const;

    /**@brief return all TAZElements
     * @param[in] onlySelected Whether to return only selected TAZElements
     */
    std::vector<GNETAZElement*> retrieveTAZElements(bool onlySelected = false) const;

    /// @brief generate TAZElement ID
    std::string generateTAZElementID(SumoXMLTag TAZElementTag) const;

    /**@brief Returns the number of TAZElements of the net
     * @param[in] type type of TAZElement to count. SUMO_TAG_NOTHING will count all TAZElements
     * @return Number of TAZElements of the net
     */
    int getNumberOfTAZElements(SumoXMLTag type = SUMO_TAG_NOTHING) const;
    /// @}

    /// @name Functions related to TLS Programs
    /// @{
    /// @brief inform that TLS Programs has to be saved
    void requireSaveTLSPrograms();

    /**@brief save TLS Programs elements of the network
     * @param[in] filename name of the file in wich save TLS Programs
     */
    void saveTLSPrograms(const std::string& filename);

    /// @brief get number of TLS Programs
    int getNumberOfTLSPrograms() const;
    /// @}

    /// @name Functions related to Enable or disable update geometry of elements after insertio
    /// @{
    /// @brief enable update geometry of elements after inserting or removing an element in net
    void enableUpdateGeometry();

    /// @brief disable update geometry of elements after inserting or removing an element in net
    void disableUpdateGeometry();

    /// @brief check if update geometry after inserting or removing has to be updated
    bool isUpdateGeometryEnabled() const;

    /// @}

protected:
    /// @brief the rtree which contains all GUIGlObjects (so named for historical reasons)
    SUMORTree myGrid;

    /// @brief The net to be notofied of about changes
    GNEViewNet* myViewNet;

    /// @brief The internal netbuilder
    NBNetBuilder* myNetBuilder;

    /// @brief AttributeCarriers of net
    GNENetHelper::AttributeCarriers* myAttributeCarriers;

    /// @brief PathCalculator instance
    GNENetHelper::PathCalculator* myPathCalculator;

    /// @name ID Suppliers for newly created edges and junctions
    // @{
    IDSupplier myEdgeIDSupplier;
    IDSupplier myJunctionIDSupplier;
    // @}

    /// @brief list of edge ids for which turn-arounds must be added explicitly
    std::set<std::string> myExplicitTurnarounds;

    /// @brief whether the net needs recomputation
    bool myNeedRecompute;

    /// @brief Flag to check if net has to be saved
    bool myNetSaved;

    /// @brief Flag to check if additionals has to be saved
    bool myAdditionalsSaved;

    /// @brief Flag to check if shapes has to be saved
    bool myTLSProgramsSaved;

    /// @brief Flag to check if demand elements has to be saved
    bool myDemandElementsSaved;

    /// @brief Flag to check if data elements has to be saved
    bool myDataElementsSaved;

    /// @brief Flag to enable or disable update geometry of elements after inserting or removing element in net
    bool myUpdateGeometryEnabled;

private:
    /// @brief Init Junctions and edges
    void initJunctionsAndEdges();

    /// @brief reserve edge ID (To avoid duplicates)
    void reserveEdgeID(const std::string& id);

    /// @brief reserve junction ID (To avoid duplicates)
    void reserveJunctionID(const std::string& id);

    /// @brief return true if there are already a Junction in the given position, false in other case
    bool checkJunctionPosition(const Position& pos);

    /// @brief save additionals after confirming invalid objects
    void saveAdditionalsConfirmed(const std::string& filename);

    /// @brief save demand elements after confirming invalid objects
    void saveDemandElementsConfirmed(const std::string& filename);

    /// @brief save data elements after confirming invalid objects
    void saveDataElementsConfirmed(const std::string& filename);

    static void replaceInListAttribute(GNEAttributeCarrier* ac, SumoXMLAttr key, const std::string& which, const std::string& by, GNEUndoList* undoList);

    /// @brief the z boundary (stored in the x-coordinate), values of 0 are ignored
    Boundary myZBoundary;

    /// @brief marker for whether the z-boundary is initialized
    static const double Z_INITIALIZED;

    /// @brief map with the Edges and their number of lanes
    std::map<std::string, int> myEdgesAndNumberOfLanes;

    /// @brief Invalidated copy constructor.
    GNENet(const GNENet&) = delete;

    /// @brief Invalidated assignment operator.
    GNENet& operator=(const GNENet&) = delete;
};
