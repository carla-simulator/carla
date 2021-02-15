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
/// @file    GNENetHelper.h
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2020
///
// Helper for GNENet
//
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <foreign/rtree/SUMORTree.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include <netbuild/NBVehicle.h>
#include <netedit/changes/GNEChange.h>
#include <utils/common/IDSupplier.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIShapeContainer.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/shapes/ShapeContainer.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================

class NBNetBuilder;
class GNEAdditional;
class GNEDataSet;
class GNEDemandElement;
class GNEApplicationWindow;
class GNEAttributeCarrier;
class GNEConnection;
class GNECrossing;
class GNEEdge;
class GNEJunction;
class GNELane;
class GNENetworkElement;
class GNEPOI;
class GNEPoly;
class GNEShape;
class GNETAZElement;
class GNEUndoList;
class GNEViewNet;

// ===========================================================================
// class definitions
// ===========================================================================

struct GNENetHelper {

    /// @brief struct used for saving all attribute carriers of net, in different formats
    class AttributeCarriers : ShapeContainer {

        /// @brief declare friend class
        friend class GNEAdditionalHandler;
        friend class GNERouteHandler;
        friend class GNEChange_Junction;
        friend class GNEChange_Edge;
        friend class GNEChange_Additional;
        friend class GNEChange_Shape;
        friend class GNEChange_TAZElement;
        friend class GNEChange_DemandElement;
        friend class GNEChange_DataSet;
        friend class GNEChange_DataInterval;
        friend class GNEChange_GenericData;

    public:
        /// @brief constructor
        AttributeCarriers(GNENet* net);

        /// @brief destructor
        ~AttributeCarriers();

        /// @brief update ID
        void updateID(GNEAttributeCarrier* AC, const std::string newID);

        /// @brief remap junction and edge IDs
        void remapJunctionAndEdgeIds();

        /// @name function for junctions
        /// @{
        /// @brief registers a junction with GNENet containers
        GNEJunction* registerJunction(GNEJunction* junction);

        /// @brief map with the ID and pointer to junctions of net
        const std::map<std::string, GNEJunction*>& getJunctions() const;

        /// @brief clear junctions
        void clearJunctions();

        /// @}

        /// @name function for edges
        /// @{
        /// @brief registers an edge with GNENet containers
        GNEEdge* registerEdge(GNEEdge* edge);

        /// @brief map with the ID and pointer to edges of net
        const std::map<std::string, GNEEdge*>& getEdges() const;

        /// @brief clear junctions
        void clearEdges();

        /// @}

        /// @name inherited from ShapeHandler
        /// @{
        /**@brief Builds a polygon using the given values and adds it to the container
        * @param[in] id The name of the polygon
        * @param[in] type The (abstract) type of the polygon
        * @param[in] color The color of the polygon
        * @param[in] layer The layer of the polygon
        * @param[in] angle The rotation of the polygon
        * @param[in] imgFile The raster image of the polygon
        * @param[in] relativePath set image file as relative path
        * @param[in] shape The shape of the polygon
        * @param[in] geo specify if shape was loaded as GEO coordinate
        * @param[in] fill Whether the polygon shall be filled
        * @param[in] lineWidth The widht for drawing unfiled polygon
        * @return whether the polygon could be added
        */
        bool addPolygon(const std::string& id, const std::string& type, const RGBColor& color, double layer,
                        double angle, const std::string& imgFile, bool relativePath, const PositionVector& shape,
                        bool geo, bool fill, double lineWidth, bool ignorePruning = false);

        /**@brief Builds a POI using the given values and adds it to the container
        * @param[in] id The name of the POI
        * @param[in] type The (abstract) type of the POI
        * @param[in] color The color of the POI
        * @param[in] pos The position of the POI
        * @param[in[ geo use GEO coordinates (lon/lat)
        * @param[in] lane The Lane in which this POI is placed
        * @param[in] posOverLane The position over Lane
        * @param[in] posLat The position lateral over Lane
        * @param[in] layer The layer of the POI
        * @param[in] angle The rotation of the POI
        * @param[in] imgFile The raster image of the POI
        * @param[in] relativePath set image file as relative path
        * @param[in] width The width of the POI image
        * @param[in] height The height of the POI image
        * @return whether the poi could be added
        */
        bool addPOI(const std::string& id, const std::string& type, const RGBColor& color, const Position& pos, bool geo,
                    const std::string& lane, double posOverLane, double posLat, double layer, double angle,
                    const std::string& imgFile, bool relativePath, double width, double height, bool ignorePruning = false);

        /// @}

        /// @name function for additionals
        /// @{
        /// @brief get additionals
        const std::map<SumoXMLTag, std::map<std::string, GNEAdditional*> >& getAdditionals() const;

        /// @brief clear additionals
        void clearAdditionals();

        /// @}

        /// @name function for shapes
        /// @{
        /// @brief get shapes
        const std::map<SumoXMLTag, std::map<std::string, GNEShape*> >& getShapes() const;

        /// @brief clear shapes
        void clearShapes();

        /// @}

        /// @name function for TAZElements
        /// @{
        /// @brief get TAZElements
        const std::map<SumoXMLTag, std::map<std::string, GNETAZElement*> >& getTAZElements() const;

        /// @brief clear TAZElements
        void clearTAZElements();

        /// @}

        /// @name function for demand elements
        /// @{
        /// @brief get demand elements
        const std::map<SumoXMLTag, std::map<std::string, GNEDemandElement*> >& getDemandElements() const;

        /// @brief clear demand elements
        void clearDemandElements();

        /// @brief add default VTypes
        void addDefaultVTypes();

        /// @}

        /// @name function for demand elements
        /// @{
        /// @brief get demand elements
        const std::map<std::string, GNEDataSet*>& getDataSets() const;

        /// @brief clear demand elements
        void clearDataSets();

        /// @}

        /// @name function for data Sets
        /// @{
        /// @brief retrieve generic datas within the given interval
        std::vector<GNEGenericData*> retrieveGenericDatas(const SumoXMLTag genericDataTag, const double begin, const double end);

        /// @}

    protected:
        /// @name Insertion and erasing of GNEJunctions
        /// @{
        /// @brief inserts a single junction into the net and into the underlying netbuild-container
        void insertJunction(GNEJunction* junction);

        /// @brief deletes a single junction
        void deleteSingleJunction(GNEJunction* junction);

        /// @brief update junction ID in container
        void updateJunctionID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @{

        /// @name Insertion and erasing of GNEEdges
        /// @{
        /// @brief inserts a single edge into the net and into the underlying netbuild-container
        void insertEdge(GNEEdge* edge);

        /// @brief deletes a single edge
        void deleteSingleEdge(GNEEdge* edge);

        /// @brief update edge ID in container
        void updateEdgeID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @{

        /// @name Insertion and erasing of GNEAdditionals items
        /// @{

        /// @brief return true if given additional exist
        bool additionalExist(const GNEAdditional* additional) const;

        /**@brief Insert a additional element int GNENet container.
         * @throw processError if route was already inserted
         */
        void insertAdditional(GNEAdditional* additional);

        /**@brief delete additional element of GNENet container
         * @throw processError if additional wasn't previously inserted
         */
        void deleteAdditional(GNEAdditional* additional);

        /// @brief update additional ID in container
        void updateAdditionalID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @}

        /// @name Insertion and erasing of GNEShapes items
        /// @{

        /// @brief return true if given shape exist
        bool shapeExist(const GNEShape* shape) const;

        /**@brief Insert a shape element int GNENet container.
         * @throw processError if route was already inserted
         */
        void insertShape(GNEShape* shape);

        /**@brief delete shape element of GNENet container
         * @throw processError if shape wasn't previously inserted
         */
        void deleteShape(GNEShape* shape);

        /// @brief update shape ID in container
        void updateShapeID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @name Insertion and erasing of GNETAZElements items
        /// @{

        /// @brief return true if given TAZElement exist
        bool TAZElementExist(const GNETAZElement* TAZElement) const;

        /**@brief Insert a TAZElement element int GNENet container.
         * @throw processError if route was already inserted
         */
        void insertTAZElement(GNETAZElement* TAZElement);

        /**@brief delete TAZElement element of GNENet container
         * @throw processError if TAZElement wasn't previously inserted
         */
        void deleteTAZElement(GNETAZElement* TAZElement);

        /// @brief update TAZElement ID in container
        void updateTAZElementID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @}

        /// @name Insertion and erasing of GNEDemandElements items
        /// @{

        /// @brief return true if given demand element exist
        bool demandElementExist(const GNEDemandElement* demandElement) const;

        /**@brief Insert a demand element element int GNENet container.
         * @throw processError if route was already inserted
         */
        void insertDemandElement(GNEDemandElement* demandElement);

        /**@brief delete demand element element of GNENet container
         * @throw processError if demand element wasn't previously inserted
         */
        void deleteDemandElement(GNEDemandElement* demandElement);

        /// @brief update demand element ID in container
        void updateDemandElementID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @}

        /// @name Insertion and erasing of GNEDataSets items
        /// @{

        /// @brief return true if given demand element exist
        bool dataSetExist(const GNEDataSet* dataSet) const;

        /**@brief Insert a demand element element int GNENet container.
         * @throw processError if route was already inserted
         */
        void insertDataSet(GNEDataSet* dataSet);

        /**@brief delete demand element element of GNENet container
         * @throw processError if demand element wasn't previously inserted
         */
        void deleteDataSet(GNEDataSet* dataSet);

        /// @brief update data element ID in container
        void updateDataSetID(GNEAttributeCarrier* AC, const std::string& newID);

        /// @}

    private:
        /// @brief map with the ID and pointer to junctions of net
        std::map<std::string, GNEJunction*> myJunctions;

        /// @brief map with the ID and pointer to edges of net
        std::map<std::string, GNEEdge*> myEdges;

        /// @brief map with the ID and pointer to additional elements of net
        std::map<SumoXMLTag, std::map<std::string, GNEAdditional*> > myAdditionals;

        /// @brief map with the ID and pointer to shape elements of net
        std::map<SumoXMLTag, std::map<std::string, GNEShape*> > myShapes;

        /// @brief map with the ID and pointer to TAZElement elements of net
        std::map<SumoXMLTag, std::map<std::string, GNETAZElement*> > myTAZElements;

        /// @brief map with the ID and pointer to demand elements of net
        std::map<SumoXMLTag, std::map<std::string, GNEDemandElement*> > myDemandElements;

        /// @brief map with the ID and pointer to data sets of net
        std::map<std::string, GNEDataSet*> myDataSets;

        /// @brief pointer to net
        GNENet* myNet;

        /// @brief flag used to indicate if created shape can be undo
        bool myAllowUndoShapes;

        /// @brief Invalidated copy constructor.
        AttributeCarriers(const AttributeCarriers&) = delete;

        /// @brief Invalidated assignment operator.
        AttributeCarriers& operator=(const AttributeCarriers&) = delete;
    };

    /// @brief class used to calculate paths in nets
    class PathCalculator {

    public:
        /// @brief constructor
        PathCalculator(const GNENet* net);

        /// @brief destructor
        ~PathCalculator();

        /// @brief update path calculator (called when SuperModes Demand or Data is selected)
        void updatePathCalculator();

        /// @brief calculate Dijkstra path between a list of partial edges
        std::vector<GNEEdge*> calculatePath(const SUMOVehicleClass vClass, const std::vector<GNEEdge*>& partialEdges) const;

        /// @brief calculate reachability for given edge
        void calculateReachability(const SUMOVehicleClass vClass, GNEEdge* originEdge);

        /// @brief check if exist a path between the two given consecutives edges for the given VClass
        bool consecutiveEdgesConnected(const SUMOVehicleClass vClass, const GNEEdge* from, const GNEEdge* to) const;

        /// @brief check if exist a path between the given busStop and edge (Either a valid lane or an acces) for pedestrians
        bool busStopConnected(const GNEAdditional* busStop, const GNEEdge* edge) const;

    private:
        /// @brief pointer to net
        const GNENet* myNet;

        /// @brief SUMO Abstract myDijkstraRouter
        SUMOAbstractRouter<NBRouterEdge, NBVehicle>* myDijkstraRouter;
    };

    /// @brief class for GNEChange_ReplaceEdgeInTLS
    class GNEChange_ReplaceEdgeInTLS : public GNEChange {
        FXDECLARE_ABSTRACT(GNEChange_ReplaceEdgeInTLS)

    public:
        /// @brief constructor
        GNEChange_ReplaceEdgeInTLS(NBTrafficLightLogicCont& tllcont, NBEdge* replaced, NBEdge* by);

        /// @bief destructor
        ~GNEChange_ReplaceEdgeInTLS();

        /// @brief undo name
        FXString undoName() const;

        /// @brief get Redo name
        FXString redoName() const;

        /// @brief undo action
        void undo();

        /// @brief redo action
        void redo();

        /// @brief wether original and new value differ
        bool trueChange();

    private:
        /// @brief container for traffic light logic
        NBTrafficLightLogicCont& myTllcont;

        /// @brief replaced NBEdge
        NBEdge* myReplaced;

        /// @brief replaced by NBEdge
        NBEdge* myBy;
    };
};
