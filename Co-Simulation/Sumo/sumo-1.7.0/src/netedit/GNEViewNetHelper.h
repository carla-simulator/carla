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
/// @file    GNEViewNetHelper.h
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2019
///
// A file used to reduce the size of GNEViewNet.h grouping structs and classes
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOVehicleClass.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <utils/gui/settings/GUIPropertyScheme.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>


// ===========================================================================
// enum
// ===========================================================================

/// @brie enum for supermodes
enum class Supermode {
    /// @brief Network mode (Edges, junctions, etc..)
    NETWORK,
    ///@brief Demand mode (Routes, Vehicles etc..)
    DEMAND,
    ///@brief Data mode (edgeData, LaneData etc..)
    DATA
};

/// @brie enum for network edit modes
enum class NetworkEditMode {
    /// @brief empty Network mode
    NETWORK_NONE,
    ///@brief mode for inspecting network elements
    NETWORK_INSPECT,
    ///@brief mode for deleting network elements
    NETWORK_DELETE,
    ///@brief mode for selecting network elements
    NETWORK_SELECT,
    ///@brief mode for moving network elements
    NETWORK_MOVE,
    ///@brief mode for creating new edges
    NETWORK_CREATE_EDGE,
    ///@brief mode for connecting lanes
    NETWORK_CONNECT,
    ///@brief mode for editing tls
    NETWORK_TLS,
    ///@brief Mode for editing additionals
    NETWORK_ADDITIONAL,
    ///@brief Mode for editing crossing
    NETWORK_CROSSING,
    ///@brief Mode for editing TAZ
    NETWORK_TAZ,
    ///@brief Mode for editing Polygons
    NETWORK_POLYGON,
    ///@brief Mode for editing connection prohibitions
    NETWORK_PROHIBITION
};

/// @brie enum for demand edit modes
enum class DemandEditMode {
    /// @brief empty Demand mode
    DEMAND_NONE,
    ///@brief mode for inspecting demand elements
    DEMAND_INSPECT,
    ///@brief mode for deleting demand elements
    DEMAND_DELETE,
    ///@brief mode for selecting demand elements
    DEMAND_SELECT,
    ///@brief mode for moving demand elements
    DEMAND_MOVE,
    ///@brief Mode for editing routes
    DEMAND_ROUTE,
    ///@brief Mode for editing vehicles
    DEMAND_VEHICLE,
    ///@brief Mode for editing vehicle types
    DEMAND_VEHICLETYPES,
    ///@brief Mode for editing stops
    DEMAND_STOP,
    ///@brief Mode for editing person types
    DEMAND_PERSONTYPES,
    ///@brief Mode for editing person
    DEMAND_PERSON,
    ///@brief Mode for editing person plan
    DEMAND_PERSONPLAN
};

/// @brie enum for data edit modes
enum class DataEditMode {
    /// @brief empty Data mode
    DATA_NONE,
    /// @brief mode for inspecting data elements
    DATA_INSPECT,
    /// @brief mode for deleting data elements
    DATA_DELETE,
    /// @brief mode for selecting data elements
    DATA_SELECT,
    /// @brief mode for create edgeData elements
    DATA_EDGEDATA,
    /// @brief mode for create edgeRelData elements
    DATA_EDGERELDATA,
    /// @brief mode for create TAZRelData elements
    DATA_TAZRELDATA
};

// ===========================================================================
// class declarations
// ===========================================================================

// main elements
class GNEAttributeCarrier;
class GNEFrame;
class GNENet;
class GNEUndoList;
class GNEViewNet;
class GNEViewParent;
// network elements
class GNENetworkElement;
class GNEJunction;
class GNEEdge;
class GNELane;
class GNEConnection;
class GNECrossing;
class GNEInternalLane;
// additional elements
class GNEAdditional;
class GNETAZElement;
class GNEShape;
class GNEPoly;
class GNEPOI;
class GNETAZElement;
class GNETAZ;
class GNETAZSourceSink;
// demand elements
class GNEDemandElement;
class GNEVehicleType;
class GNEFlow;
class GNEVehicle;
class GNEtrip;
// data elements
class GNEDataSet;
class GNEDataInterval;
class GNEGenericData;
class GNEEdgeData;
class GNEEdgeRelData;

// ===========================================================================
// classes and structs definitions
// ===========================================================================

struct GNEViewNetHelper {

    /// @brief class used to group all variables related with objects under cursor after a click over view
    class ObjectsUnderCursor {

    public:
        /// @brief constructor
        ObjectsUnderCursor(GNEViewNet* viewNet);

        /// @brief update objects under cursor (Called only in onLeftBtnPress(...) function)
        void updateObjectUnderCursor(const std::vector<GUIGlObject*>& GUIGlObjects);

        /// @brief swap lane to edge
        void swapLane2Edge();

        /// @brief get front GUI GL ID or a pointer to nullptr
        GUIGlID getGlIDFront() const;

        /// @brief get front GUI GL object type or a pointer to nullptr
        GUIGlObjectType getGlTypeFront() const;

        /// @brief get front attribute carrier or a pointer to nullptr
        GUIGlObject* getGUIGlObjectFront() const;

        /// @brief get front attribute carrier or a pointer to nullptr
        GNEAttributeCarrier* getAttributeCarrierFront() const;

        /// @brief get front network element or a pointer to nullptr
        GNENetworkElement* getNetworkElementFront() const;

        /// @brief get front additional element or a pointer to nullptr
        GNEAdditional* getAdditionalFront() const;

        /// @brief get front shape element or a pointer to nullptr
        GNEShape* getShapeFront() const;

        /// @brief get front TAZElement or a pointer to nullptr
        GNETAZElement* getTAZElementFront() const;

        /// @brief get front demand element or a pointer to nullptr
        GNEDemandElement* getDemandElementFront() const;

        /// @brief get generic data element or a pointer to nullptr
        GNEGenericData* getGenericDataElementFront() const;

        /// @brief get front junction or a pointer to nullptr
        GNEJunction* getJunctionFront() const;

        /// @brief get front edge or a pointer to nullptr
        GNEEdge* getEdgeFront() const;

        /// @brief get front lane or a pointer to nullptr
        GNELane* getLaneFront() const;

        /// @brief get front crossing or a pointer to nullptr
        GNECrossing* getCrossingFront() const;

        /// @brief get front connection or a pointer to nullptr
        GNEConnection* getConnectionFront() const;

        /// @brief get front internal lane or a pointer to nullptr
        GNEInternalLane* getInternalLaneFront() const;

        /// @brief get front TAZ or a pointer to nullptr
        GNETAZ* getTAZFront() const;

        /// @brief get front POI or a pointer to nullptr
        GNEPOI* getPOIFront() const;

        /// @brief get front Poly or a pointer to nullptr
        GNEPoly* getPolyFront() const;

        /// @brief get edge data element or a pointer to nullptr
        GNEEdgeData* getEdgeDataElementFront() const;

        /// @brief get edge rel data element or a pointer to nullptr
        GNEEdgeRelData* getEdgeRelDataElementFront() const;

        /// @brief get vector with clicked ACs
        const std::vector<GNEAttributeCarrier*>& getClickedAttributeCarriers() const;

    protected:
        /// @brief objects container
        class ObjectsContainer {

        public:
            /// @brief constructor
            ObjectsContainer();

            /// @brief clear elements
            void clearElements();

            /// @brief vector with the clicked GUIGlObjects
            std::vector<GUIGlObject*> GUIGlObjects;

            /// @brief vector with the clicked attribute carriers
            std::vector<GNEAttributeCarrier*> attributeCarriers;

            /// @brief vector with the clicked network elements
            std::vector<GNENetworkElement*> networkElements;

            /// @brief vector with the clicked additional elements
            std::vector<GNEAdditional*> additionals;

            /// @brief vector with the clicked shape elements (Poly and POIs)
            std::vector<GNEShape*> shapes;

            /// @brief vector with the clicked TAZ elements
            std::vector<GNETAZElement*> TAZElements;

            /// @brief vector with the clicked demand elements
            std::vector<GNEDemandElement*> demandElements;

            /// @brief vector with the clicked generic datas
            std::vector<GNEGenericData*> genericDatas;

            /// @brief vector with the clicked junctions
            std::vector<GNEJunction*> junctions;

            /// @brief vector with the clicked edges
            std::vector<GNEEdge*> edges;

            /// @brief vector with the clicked lanes
            std::vector<GNELane*> lanes;

            /// @brief vector with the clicked crossings
            std::vector<GNECrossing*> crossings;

            /// @brief vector with the clicked connections
            std::vector<GNEConnection*> connections;

            /// @brief vector with the clicked internal lanes
            std::vector<GNEInternalLane*> internalLanes;

            /// @brief vector with the clicked TAZ elements
            std::vector<GNETAZ*> TAZs;

            /// @brief vector with the clicked POIs
            std::vector<GNEPOI*> POIs;

            /// @brief vector with the clicked polys
            std::vector<GNEPoly*> polys;

            /// @brief vector with the clicked edge datas
            std::vector<GNEEdgeData*> edgeDatas;

            /// @brief vector with the clicked edge relation datas
            std::vector<GNEEdgeRelData*> edgeRelDatas;

        private:
            /// @brief Invalidated copy constructor.
            ObjectsContainer(const ObjectsContainer&) = delete;

            /// @brief Invalidated assignment operator.
            ObjectsContainer& operator=(const ObjectsContainer&) = delete;
        };

        /// @brief pointer to viewNet
        const GNEViewNet* myViewNet;

        /// @brief objectContainer for objects selecting edges
        ObjectsContainer myEdgeObjects;

        /// @brief objectContainer for objects selecting lanes
        ObjectsContainer myLaneObjects;

        /// @brief flag to enable/disable swap lane to edge
        bool mySwapLane2edge;

    private:
        /// @brief sort by altitude and update GUIGlObjects
        void sortGUIGlObjects(const std::vector<GUIGlObject*>& GUIGlObjects);

        /// @brief update attribute carrier elements
        void updateAttributeCarriers(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief update network elements
        void updateNetworkElements(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief update additional elements
        void updateAdditionalElements(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief update TAZ elements
        void updateTAZElements(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief update shape elements
        void updateShapeElements(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief update demand elements
        void updateDemandElements(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief update generic data elements
        void updateGenericDataElements(ObjectsContainer& container, GNEAttributeCarrier* AC);

        /// @brief updateGUIGlObjects
        void updateGUIGlObjects(ObjectsContainer& container);

        /// @brief default constructor
        ObjectsUnderCursor();

        /// @brief Invalidated copy constructor.
        ObjectsUnderCursor(const ObjectsUnderCursor&) = delete;

        /// @brief Invalidated assignment operator.
        ObjectsUnderCursor& operator=(const ObjectsUnderCursor&) = delete;
    };

    /// @brief class used to group all variables related with key pressed after certain events
    struct KeyPressed {

        /// @brief constructor
        KeyPressed();

        /// @brief update status of KeyPressed
        void update(void* eventData);

        /// @brief check if SHIFT key was pressed during click
        bool shiftKeyPressed() const;

        /// @brief check if CONTROL key was pressed during click
        bool controlKeyPressed() const;

    private:
        /// @brief information of event
        FXEvent* myEventInfo;

        /// @brief Invalidated copy constructor.
        KeyPressed(const KeyPressed&) = delete;

        /// @brief Invalidated assignment operator.
        KeyPressed& operator=(const KeyPressed&) = delete;
    };

    /// @brief struct used to group all variables related with save elements
    struct SaveElements {

        /// @brief default constructor
        SaveElements(GNEViewNet* viewNet);

        /// @brief build save buttons
        void buildSaveElementsButtons();

        /// @brief chekable button for save network
        FXButton* saveNetwork;

        /// @brief chekable button for save additional elements
        FXButton* saveAdditionalElements;

        /// @brief chekable button for save demand elements
        FXButton* saveDemandElements;

        /// @brief chekable button for save data elements
        FXButton* saveDataElements;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief Invalidated copy constructor.
        SaveElements(const SaveElements&) = delete;

        /// @brief Invalidated assignment operator.
        SaveElements& operator=(const SaveElements&) = delete;
    };

    /// @brief struct used to group all variables related with Supermodes
    struct EditModes {

        /// @brief default constructor
        EditModes(GNEViewNet* viewNet);

        /// @brief build checkable buttons
        void buildSuperModeButtons();

        /// @brief set supermode
        void setSupermode(Supermode supermode, const bool force);

        /// @brief set Network edit mode
        void setNetworkEditMode(NetworkEditMode networkMode, const bool force = false);

        /// @brief set Demand edit mode
        void setDemandEditMode(DemandEditMode demandMode, const bool force = false);

        /// @brief set Data edit mode
        void setDataEditMode(DataEditMode dataMode, const bool force = false);

        /// @check if current supermode is Network
        bool isCurrentSupermodeNetwork() const;

        /// @check if current supermode is Demand
        bool isCurrentSupermodeDemand() const;

        /// @check if current supermode is Data
        bool isCurrentSupermodeData() const;

        /// @brief the current Network edit mode
        NetworkEditMode networkEditMode;

        /// @brief the current Demand edit mode
        DemandEditMode demandEditMode;

        /// @brief the current Data edit mode
        DataEditMode dataEditMode;

        /// @brief chekable button for supermode Network
        MFXCheckableButton* networkButton;

        /// @brief chekable button for supermode Demand
        MFXCheckableButton* demandButton;

        /// @brief chekable button for supermode Data
        MFXCheckableButton* dataButton;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief the current supermode
        Supermode myCurrentSupermode;

        /// @brief Invalidated copy constructor.
        EditModes(const EditModes&) = delete;

        /// @brief Invalidated assignment operator.
        EditModes& operator=(const EditModes&) = delete;
    };

    /// @brief struct used to group all variables related to view options in supermode Network
    struct NetworkViewOptions {

        /// @brief default constructor
        NetworkViewOptions(GNEViewNet* viewNet);

        /// @brief build menu checks
        void buildNetworkViewOptionsMenuChecks();

        /// @brief hide all options menu checks
        void hideNetworkViewOptionsMenuChecks();

        /// @brief get visible network menu commands
        void getVisibleNetworkMenuCommands(std::vector<FXMenuCheck*>& commands) const;

        /// @brief check if vehicles must be drawn spread
        bool drawSpreadVehicles() const;

        /// @brief check if show demand elements checkbox is enabled
        bool showDemandElements() const;

        /// @brief check if select edges checkbox is enabled
        bool selectEdges() const;

        /// @brief check if select show connections checkbox is enabled
        bool showConnections() const;

        /// @brief check if we're editing elevation
        bool editingElevation() const;

        /// @brief menu check to show grid button
        FXMenuCheck* menuCheckShowGrid;

        /// @brief menu check to draw vehicles in begin position or spread in lane
        FXMenuCheck* menuCheckDrawSpreadVehicles;

        /// @brief menu check to show Demand Elements
        FXMenuCheck* menuCheckShowDemandElements;

        /// @brief menu check to select only edges
        FXMenuCheck* menuCheckSelectEdges;

        /// @brief menu check to show connections
        FXMenuCheck* menuCheckShowConnections;

        /// @brief menu check to hide connections in connect mode
        FXMenuCheck* menuCheckHideConnections;

        /// @brief menu check to extend to edge nodes
        FXMenuCheck* menuCheckExtendSelection;

        /// @brief menu check to set change all phases
        FXMenuCheck* menuCheckChangeAllPhases;

        /// @brief menu check to we should warn about merging junctions
        FXMenuCheck* menuCheckWarnAboutMerge;

        /// @brief menu check to show connection as buuble in "Move" mode.
        FXMenuCheck* menuCheckShowJunctionBubble;

        /// @brief menu check to apply movement to elevation
        FXMenuCheck* menuCheckMoveElevation;

        /// @brief menu check to the endpoint for a created edge should be set as the new source
        FXMenuCheck* menuCheckChainEdges;

        /// @brief menu check to create auto create opposite edge
        FXMenuCheck* menuCheckAutoOppositeEdge;
    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief Invalidated copy constructor.
        NetworkViewOptions(const NetworkViewOptions&) = delete;

        /// @brief Invalidated assignment operator.
        NetworkViewOptions& operator=(const NetworkViewOptions&) = delete;
    };

    /// @brief struct used to group all variables related to view options in supermode Demand
    struct DemandViewOptions {

        /// @brief default constructor
        DemandViewOptions(GNEViewNet* viewNet);

        /// @brief build menu checks
        void buildDemandViewOptionsMenuChecks();

        /// @brief hide all options menu checks
        void hideDemandViewOptionsMenuChecks();

        /// @brief get visible demand menu commands
        void getVisibleDemandMenuCommands(std::vector<FXMenuCheck*>& commands) const;

        /// @brief check if vehicles must be drawn spread
        bool drawSpreadVehicles() const;

        /// @brief check if non inspected element has to be hidden
        bool showNonInspectedDemandElements(const GNEDemandElement* demandElement) const;

        /// @brief check if shapes has to be drawn
        bool showShapes() const;

        /// @brief check all person plans has to be show
        bool showAllPersonPlans() const;

        /// @brief lock person
        void lockPerson(const GNEDemandElement* person);

        /// @brief unlock person
        void unlockPerson();

        /// @brief get locked person
        const GNEDemandElement* getLockedPerson() const;

        /// @brief menu check to show grid button
        FXMenuCheck* menuCheckShowGrid;

        /// @brief menu check to draw vehicles in begin position or spread in lane
        FXMenuCheck* menuCheckDrawSpreadVehicles;

        /// @brief Hide shapes (Polygons and POIs)
        FXMenuCheck* menuCheckHideShapes;

        /// @brief Hide non inspected demand elements
        FXMenuCheck* menuCheckHideNonInspectedDemandElements;

        /// @brief show all person plans
        FXMenuCheck* menuCheckShowAllPersonPlans;

        /// @brief Lock Person
        FXMenuCheck* menuCheckLockPerson;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief pointer to locked person
        const GNEDemandElement* myLockedPerson;

        /// @brief Invalidated copy constructor.
        DemandViewOptions(const DemandViewOptions&) = delete;

        /// @brief Invalidated assignment operator.
        DemandViewOptions& operator=(const DemandViewOptions&) = delete;
    };

    /// @brief struct used to group all variables related to view options in supermode Data
    struct DataViewOptions {

        /// @brief default constructor
        DataViewOptions(GNEViewNet* viewNet);

        /// @brief build menu checks
        void buildDataViewOptionsMenuChecks();

        /// @brief hide all options menu checks
        void hideDataViewOptionsMenuChecks();

        /// @brief get visible demand menu commands
        void getVisibleDataMenuCommands(std::vector<FXMenuCheck*>& commands) const;

        /// @brief check if additionals has to be drawn
        bool showAdditionals() const;

        /// @brief check if shapes has to be drawn
        bool showShapes() const;

        /// @brief check if show demand elements checkbox is enabled
        bool showDemandElements() const;

        /// @brief menu check to show Additionals
        FXMenuCheck* menuCheckShowAdditionals;

        /// @brief menu check to show Shapes
        FXMenuCheck* menuCheckShowShapes;

        /// @brief menu check to show Demand Elements
        FXMenuCheck* menuCheckShowDemandElements;
    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief Invalidated copy constructor.
        DataViewOptions(const DataViewOptions&) = delete;

        /// @brief Invalidated assignment operator.
        DataViewOptions& operator=(const DataViewOptions&) = delete;
    };

    /// @brief class used to group all variables related to interval bar
    class IntervalBar {

    public:
        /// @brief default constructor
        IntervalBar(GNEViewNet* viewNet);

        /// @brief build interval bar elements
        void buildIntervalBarElements();

        /// @brief enable interval bar
        void enableIntervalBar();

        /// @brief disable interval bar
        void disableIntervalBar();

        /// @brief enable interval bar update
        void enableIntervalBarUpdate();

        /// @brief enable interval bar update
        void disableIntervalBarUpdate();

        /// @brief show interval option bar
        void showIntervalBar();

        /// @brief hide all options menu checks
        void hideIntervalBar();

        /// @brief update interval bar
        void updateIntervalBar();

        /// @name get functions (called by GNEViewNet)
        /// @{

        /// @brief get generic data type
        std::string getGenericDataTypeStr() const;

        /// @brief get dataSet
        std::string getDataSetStr() const;

        /// @brief get begin
        std::string getBeginStr() const;

        /// @brief get end
        std::string getEndStr() const;

        /// @brief set attribute
        std::string getAttributeStr() const;

        /// @}

        /// @name set functions (called by GNEViewNet)
        /// @{

        /// @brief set generic data type
        void setGenericDataType();

        /// @brief set dataSet
        void setDataSet();

        /// @brief update limit by interval
        void setInterval();

        /// @brief set begin
        void setBegin();

        /// @brief set end
        void setEnd();

        /// @brief set attribute
        void setAttribute();

        /// @}

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief flag to enable or disable update interval bar
        bool myIntervalBarUpdate;

        /// @brief combo box for generic data types
        FXComboBox* myGenericDataTypesComboBox;

        /// @brief combo box for data sets
        FXComboBox* myDataSetsComboBox;

        /// @brief checkbox for limit data elements by interval
        FXCheckButton* myLimitByIntervalCheckBox;

        /// @brief text field for interval begin
        FXTextField* myBeginTextField;

        /// @brief text field for interval end
        FXTextField* myEndTextField;

        /// @brief combo box for filtered attributes
        FXComboBox* myFilteredAttributesComboBox;

        /// @brief set for attribuets
        std::set<std::string> myFilteredAttributes;

        /// @brief string with wildcard for no generic datas
        const FXString myNoGenericDatas;

        /// @brief string with wildcard for all generic datas
        const FXString myAllGenericDatas;

        /// @brief string with wildcard for no dataSets
        const FXString myNoDataSets;

        /// @brief string with wildcard for all dataSets
        const FXString myAllDataSets;

        /// @brief string with wildcard for all attributes
        const FXString myAllAttributes;

    private:
        /// @brief update combo box attributes
        void updateComboBoxAttributes();

        /// @brief Invalidated copy constructor.
        IntervalBar(const IntervalBar&) = delete;

        /// @brief Invalidated assignment operator.
        IntervalBar& operator=(const IntervalBar&) = delete;
    };

    /// @brief struct used to group all variables related with movement of single elements
    struct MoveSingleElementValues {

        /// @brief constructor
        MoveSingleElementValues(GNEViewNet* viewNet);

        /// @brief begin move network elementshape
        bool beginMoveNetworkElementShape();

        /// @brief begin move single element in Network mode
        bool beginMoveSingleElementNetworkMode();

        /// @brief begin move single element in Demand mode
        bool beginMoveSingleElementDemandMode();

        /// @brief move single element in Network AND Demand mode
        void moveSingleElement();

        /// @brief finish moving single elements in Network AND Demand mode
        void finishMoveSingleElement();

    private:
        /// @brief calculate network element movement values (Position, Index, etc.)
        bool calculateJunctionValues();

        /// @brief calculate crossing element movement values (Position, Index, etc.)
        bool calculateCrossingValues();

        /// @brief calculate connection element movement values (Position, Index, etc.)
        bool calculateConnectionValues();

        /// @brief calculate Poly movement values (Position, Index, etc.)
        bool calculatePolyValues();

        /// @brief calculate Edge movement values (Position, Index, etc.)
        bool calculateEdgeValues();

        /// @brief calculate TAZ movement values (Position, Index, etc.)
        bool calculateTAZValues();

        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief relative position of Clicked Position regarding to originalGeometryPointPosition (Used when user doesn't click exactly over the center of element)
        Position myRelativeClickedPosition;

        /// @brief bool to indicate that startPos of an edge is being moved
        bool myMovingStartPos;

        /// @brief bool to indicate that end pos of an edge is being moved
        bool myMovingEndPos;

        /// @brief the junction to be moved
        GNEJunction* myJunctionToMove;

        /// @brief the crossing to be moved.
        GNECrossing* myCrossingToMove;

        /// @brief the connection to be moved.
        GNEConnection* myConnectionToMove;

        /// @brief the edge of which geometry is being moved
        GNEEdge* myEdgeToMove;

        /// @brief the poly of which geometry is being moved
        GNEPoly* myPolyToMove;

        /// @brief the poi which position is being moved
        GNEPOI* myPOIToMove;

        /// @brief the additional element which position is being moved
        GNEAdditional* myAdditionalToMove;

        /// @brief the demand element which position is being moved
        GNEDemandElement* myDemandElementToMove;

        /// @brief the TAZ element which their Shape is being moved (it's the only additional with a shape instead a position)
        GNETAZ* myTAZElementToMove;
    };

    /// @brief struct used to group all variables related with movement of groups of elements
    struct MoveMultipleElementValues {

        /// @brief constructor
        MoveMultipleElementValues(GNEViewNet* viewNet);

        /// @brief begin move selection
        void beginMoveSelection(GNEAttributeCarrier* originAC);

        /// @brief move selection
        void moveSelection();

        /// @brief finish moving selection
        void finishMoveSelection();

        /// @brief check if currently there is element being moved
        bool isMovingSelection() const;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief original clicked position when moveSelection is called (used for calculate offset during moveSelection())
        Position myClickedPosition;

        /// @brief flag to check if a selection is being moved
        bool myMovingSelection;

        /// @brief container used for move junctions
        std::vector<GNEJunction*> myMovedJunctions;

        /// @brief container used for move edges
        std::set<GNEEdge*> myMovedEdges;
    };

    /// @brief struct used to group all variables related with movement of groups of elements
    struct VehicleOptions {

        /// @brief constructor
        VehicleOptions(GNEViewNet* viewNet);

        /// @brief build menu checks
        void buildVehicleOptionsMenuChecks();

        /// @brief hide all options menu checks
        void hideVehicleOptionsMenuChecks();

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with movement of groups of elements
    struct VehicleTypeOptions {

        /// @brief constructor
        VehicleTypeOptions(GNEViewNet* viewNet);

        /// @brief build menu checks
        void buildVehicleTypeOptionsMenuChecks();

        /// @brief hide all options menu checks
        void hideVehicleTypeOptionsMenuChecks();

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with selecting using a square or polygon
    /// @note in the future the variables used for selecting throught a polygon will be placed here
    struct SelectingArea {

        /// @brief default constructor
        SelectingArea(GNEViewNet* viewNet);

        /// @brief begin rectangle selection
        void beginRectangleSelection();

        /// @brief move rectangle selection
        void moveRectangleSelection();

        /// @brief finish rectangle selection
        void finishRectangleSelection();

        /// @brief process rectangle Selection
        void processRectangleSelection();

        /// @brief process rectangle Selection (only limited to Edges)
        std::vector<GNEEdge*> processEdgeRectangleSelection();

        /// @brief process shape selection
        void processShapeSelection(const PositionVector& shape);

        /// @brief draw rectangle selection
        void drawRectangleSelection(const RGBColor& color) const;

        /// @brief whether we have started rectangle-selection
        bool selectingUsingRectangle;

        /// @brief whether we have started rectangle-selection
        bool startDrawing;

    private:
        /// @brief Process boundary Selection
        void processBoundarySelection(const Boundary& boundary);

        /// @brief firstcorner of the rectangle-selection
        Position selectionCorner1;

        /// @brief second corner of the rectangle-selection
        Position selectionCorner2;

        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with testing
    struct TestingMode {

        /// @brief default constructor
        TestingMode(GNEViewNet* viewNet);

        /// @brief init testing mode
        void initTestingMode();

        /// @brief draw testing element
        void drawTestingElements(GUIMainWindow* mainWindow);

        /// @brief check if testing mode is enabled
        bool isTestingEnabled() const;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief flag to enable or disable testing mode
        bool myTestingEnabled;

        /// @brief Width of net in testing mode
        int myTestingWidth;

        /// @brief Height of net in testing mode
        int myTestingHeight;
    };

    /// @brief struct used to group all variables related with common chekable Buttons
    struct CommonCheckableButtons {

        /// @brief default constructor
        CommonCheckableButtons(GNEViewNet* viewNet);

        /// @brief build checkable buttons
        void buildCommonCheckableButtons();

        /// @brief show all Common Checkable Buttons
        void showCommonCheckableButtons();

        /// @brief hide all Common Checkable Buttons
        void hideCommonCheckableButtons();

        /// @brief hide all options menu checks
        void disableCommonCheckableButtons();

        /// @brief update Common checkable buttons
        void updateCommonCheckableButtons();

        /// @brief chekable button for edit mode inspect
        MFXCheckableButton* inspectButton;

        /// @brief chekable button for edit mode delete
        MFXCheckableButton* deleteButton;

        /// @brief chekable button for edit mode select
        MFXCheckableButton* selectButton;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with Network chekable Buttons
    struct NetworkCheckableButtons {

        /// @brief default constructor
        NetworkCheckableButtons(GNEViewNet* viewNet);

        /// @brief build checkable buttons
        void buildNetworkCheckableButtons();

        /// @brief show all Network Checkable Buttons
        void showNetworkCheckableButtons();

        /// @brief hide all Network Checkable Buttons
        void hideNetworkCheckableButtons();

        /// @brief hide all options menu checks
        void disableNetworkCheckableButtons();

        /// @brief update network checkable buttons
        void updateNetworkCheckableButtons();

        /// @brief chekable button for edit mode "move network elements"
        MFXCheckableButton* moveNetworkElementsButton;

        /// @brief chekable button for edit mode create edge
        MFXCheckableButton* createEdgeButton;

        /// @brief chekable button for edit mode connection
        MFXCheckableButton* connectionButton;

        /// @brief chekable button for edit mode traffic light
        MFXCheckableButton* trafficLightButton;

        /// @brief chekable button for edit mode additional
        MFXCheckableButton* additionalButton;

        /// @brief chekable button for edit mode crossing
        MFXCheckableButton* crossingButton;

        /// @brief chekable button for edit mode TAZ
        MFXCheckableButton* TAZButton;

        /// @brief chekable button for edit mode shape
        MFXCheckableButton* shapeButton;

        /// @brief checkable button for edit mode polygon
        MFXCheckableButton* prohibitionButton;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with Demand checkable Buttons
    struct DemandCheckableButtons {

        /// @brief default constructor
        DemandCheckableButtons(GNEViewNet* viewNet);

        /// @brief build checkable buttons
        void buildDemandCheckableButtons();

        /// @brief show all Demand Checkable Buttons
        void showDemandCheckableButtons();

        /// @brief hide all Demand Checkable Buttons
        void hideDemandCheckableButtons();

        /// @brief hide all options menu checks
        void disableDemandCheckableButtons();

        /// @brief update Demand checkable buttons
        void updateDemandCheckableButtons();

        /// @brief chekable button for edit mode "move demand elements"
        MFXCheckableButton* moveDemandElementsButton;

        /// @brief chekable button for edit mode create routes
        MFXCheckableButton* routeButton;

        /// @brief chekable button for edit mode create vehicles
        MFXCheckableButton* vehicleButton;

        /// @brief chekable button for edit mode create vehicle type
        MFXCheckableButton* vehicleTypeButton;

        /// @brief chekable button for edit mode create stops
        MFXCheckableButton* stopButton;

        /// @brief chekable button for edit mode create person type
        MFXCheckableButton* personTypeButton;

        /// @brief chekable button for edit mode create persons
        MFXCheckableButton* personButton;

        /// @brief chekable button for edit mode create person plans
        MFXCheckableButton* personPlanButton;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with Data checkable Buttons
    struct DataCheckableButtons {

        /// @brief default constructor
        DataCheckableButtons(GNEViewNet* viewNet);

        /// @brief build checkable buttons
        void buildDataCheckableButtons();

        /// @brief show all Data Checkable Buttons
        void showDataCheckableButtons();

        /// @brief hide all Data Checkable Buttons
        void hideDataCheckableButtons();

        /// @brief hide all options menu checks
        void disableDataCheckableButtons();

        /// @brief update Data checkable buttons
        void updateDataCheckableButtons();

        /// @brief chekable button for edit mode "edgeData"
        MFXCheckableButton* edgeDataButton;

        /// @brief chekable button for edit mode "edgeRelData"
        MFXCheckableButton* edgeRelDataButton;

        /// @brief chekable button for edit mode "TAZRelData"
        MFXCheckableButton* TAZRelDataButton;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;
    };

    /// @brief struct used to group all variables related with edit shapes of NetworkElements
    struct EditNetworkElementShapes {

        /// @brief default constructor
        EditNetworkElementShapes(GNEViewNet* viewNet);

        /// @brief start edit custom shape
        void startEditCustomShape(GNENetworkElement* element);

        /// @brief edit edit shape
        void stopEditCustomShape();

        /// @brief save edited shape
        void commitEditedShape();

        /// @brief pointer to edited network element
        GNENetworkElement* getEditedNetworkElement() const;

    private:
        /// @brief pointer to net
        GNEViewNet* myViewNet;

        /// @brief pointer to edited network element
        GNENetworkElement* myEditedNetworkElement;

        /// @brief the previous edit mode before edit NetworkElement's shapes
        NetworkEditMode myPreviousNetworkEditMode;

    };

    /// @brief get scaled rainbow colors
    static const std::vector<RGBColor>& getRainbowScaledColors();

    /// @brief get rainbow scaled color
    static const RGBColor& getRainbowScaledColor(const double min, const double max, const double value);

private:
    /// @brief scale (rainbow) colors
    static std::vector<RGBColor> myRainbowScaledColors;
};
