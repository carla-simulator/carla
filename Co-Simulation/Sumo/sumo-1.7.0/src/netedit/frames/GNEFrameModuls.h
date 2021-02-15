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
/// @file    GNEFrameModuls.h
/// @author  Pablo Alvarez Lopez
/// @date    Aug 2019
///
// Auxiliar class for GNEFrame Moduls
/****************************************************************************/
#pragma once
#include <config.h>

#include <netedit/elements/GNEAttributeCarrier.h>
#include <netedit/GNEViewNetHelper.h>

// ===========================================================================
// class declaration
// ===========================================================================

class GNEFrame;

// ===========================================================================
// class definitions
// ===========================================================================

class GNEFrameModuls {

public:
    // ===========================================================================
    // class TagSelector
    // ===========================================================================

    class TagSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEFrameModuls::TagSelector)

    public:
        /// @brief constructor
        TagSelector(GNEFrame* frameParent, GNETagProperties::TagType type, bool onlyDrawables = true);

        /// @brief destructor
        ~TagSelector();

        /// @brief show item selector
        void showTagSelector();

        /// @brief hide item selector
        void hideTagSelector();

        /// @brief get current type tag
        const GNETagProperties& getCurrentTagProperties() const;

        /// @brief set current type manually
        void setCurrentTagType(GNETagProperties::TagType tagType);

        /// @brief set current type manually
        void setCurrentTag(SumoXMLTag newTag);

        /// @brief due myCurrentTagProperties is a Reference, we need to refresh it when frameParent is show
        void refreshTagProperties();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user select an group in ComboBox
        long onCmdSelectTagType(FXObject*, FXSelector, void*);

        /// @brief Called when the user select an elementin ComboBox
        long onCmdSelectTag(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(TagSelector)

    private:
        /// @brief pointer to Frame Parent
        GNEFrame* myFrameParent;

        /// @brief comboBox with tag type
        FXComboBox* myTagTypesMatchBox;

        /// @brief comboBox with the list of tags
        FXComboBox* myTagsMatchBox;

        /// @brief current tag properties
        GNETagProperties myCurrentTagProperties;

        /// @brief list of tags types that will be shown in Match Box
        std::vector<std::pair<std::string, GNETagProperties::TagType> > myListOfTagTypes;

        /// @brief list of tags that will be shown in Match Box
        std::vector<std::pair<SumoXMLTag, const std::string> > myListOfTags;

        /// @brief dummy tag properties used if user select an invalid tag
        GNETagProperties myInvalidTagProperty;
    };

    // ===========================================================================
    // class DemandElementSelector
    // ===========================================================================

    class DemandElementSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEFrameModuls::DemandElementSelector)

    public:
        /// @brief constructor with a single tag
        DemandElementSelector(GNEFrame* frameParent, SumoXMLTag demandElementTag);

        /// @brief constructor with tag type
        DemandElementSelector(GNEFrame* frameParent, const std::vector<GNETagProperties::TagType>& tagTypes);

        /// @brief destructor
        ~DemandElementSelector();

        /// @brief get current demand element
        GNEDemandElement* getCurrentDemandElement() const;

        // @brief obtain allowed tags (derived from tagTypes)
        const std::vector<SumoXMLTag>& getAllowedTags() const;

        /// @brief set current demand element
        void setDemandElement(GNEDemandElement* demandElement);

        /// @brief show demand element selector
        void showDemandElementSelector();

        /// @brief hide demand element selector
        void hideDemandElementSelector();

        /// @brief check if demand element selector is shown
        bool isDemandElementSelectorShown() const;

        /// @brief refresh demand element selector
        void refreshDemandElementSelector();

        /// @brief get previous edge for the current selected demand elemnt
        GNEEdge* getPersonPlanPreviousEdge() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user select another demand elementelement in ComboBox
        long onCmdSelectDemandElement(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(DemandElementSelector)

    private:
        /// @brief pointer to frame Parent
        GNEFrame* myFrameParent;

        /// @brief comboBox with the list of elements type
        FXComboBox* myDemandElementsMatchBox;

        /// @brief current demand element
        GNEDemandElement* myCurrentDemandElement;

        /// @brief demand element tags
        std::vector<SumoXMLTag> myDemandElementTags;
    };

    // ===========================================================================
    // class HierarchicalElementTree
    // ===========================================================================

    class HierarchicalElementTree : private FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEFrameModuls::HierarchicalElementTree)

    public:
        /// @brief constructor
        HierarchicalElementTree(GNEFrame* frameParent);

        /// @brief destructor
        ~HierarchicalElementTree();

        /// @brief show HierarchicalElementTree
        void showHierarchicalElementTree(GNEAttributeCarrier* AC);

        /// @brief hide HierarchicalElementTree
        void hideHierarchicalElementTree();

        /// @brief refresh HierarchicalElementTree
        void refreshHierarchicalElementTree();

        /// @brief if given AttributeCarrier is the same of myHE, set it as nullptr
        void removeCurrentEditedAttributeCarrier(const GNEAttributeCarrier* HE);

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user press right click over an item of list of children
        long onCmdShowChildMenu(FXObject*, FXSelector, void* data);

        /// @brief called when user click over option "center" of child Menu
        long onCmdCenterItem(FXObject*, FXSelector, void*);

        /// @brief called when user click over option "inspect" of child menu
        long onCmdInspectItem(FXObject*, FXSelector, void*);

        /// @brief called when user click over option "delete" of child menu
        long onCmdDeleteItem(FXObject*, FXSelector, void*);

        /// @brief called when user click over option "Move up" of child menu
        long onCmdMoveItemUp(FXObject*, FXSelector, void*);

        /// @brief called when user click over option "Move down" of child menu
        long onCmdMoveItemDown(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(HierarchicalElementTree)

        // @brief create pop-up menu in the positions X-Y for the clicked attribute carrier
        void createPopUpMenu(int X, int Y, GNEAttributeCarrier* clickedAC);

        /// @brief show child of current attributeCarrier
        FXTreeItem* showAttributeCarrierParents();

        /// @brief show children of given hierarchical element
        void showHierarchicalElementChildren(GNEHierarchicalElement* HE, FXTreeItem* itemParent);

        /// @brief add item into list
        FXTreeItem* addListItem(GNEAttributeCarrier* AC, FXTreeItem* itemParent = nullptr, std::string prefix = "", std::string sufix = "");

        /// @brief add item into list
        FXTreeItem* addListItem(FXTreeItem* itemParent, const std::string& text, FXIcon* icon, bool expanded);

    private:
        /// @brief frame Parent
        GNEFrame* myFrameParent;

        /// @brief hierarchical element
        GNEHierarchicalElement* myHE;

        /// @brief pointer to current clicked Attribute Carrier
        GNEAttributeCarrier* myClickedAC;

        /// @brief junction (casted from myClickedAC)
        GNEJunction* myClickedJunction;

        /// @brief edge (casted from myClickedAC)
        GNEEdge* myClickedEdge;

        /// @brief lane (casted from myClickedAC)
        GNELane* myClickedLane;

        /// @brief crossing (casted from myClickedAC)
        GNECrossing* myClickedCrossing;

        /// @brief junction (casted from myClickedAC)
        GNEConnection* myClickedConnection;

        /// @brief shape (casted from myClickedAC)
        GNEShape* myClickedShape;

        /// @brief TAZElement (casted from myClickedAC)
        GNETAZElement* myClickedTAZElement;

        /// @brief additional (casted from myClickedAC)
        GNEAdditional* myClickedAdditional;

        /// @brief demand element (casted from myClickedAC)
        GNEDemandElement* myClickedDemandElement;

        /// @brief data set element (casted from myClickedAC)
        GNEDataSet* myClickedDataSet;

        /// @brief data interval element (casted from myClickedAC)
        GNEDataInterval* myClickedDataInterval;

        /// @brief generic data element (casted from myClickedAC)
        GNEGenericData* myClickedGenericData;

        /// @brief tree list to show the children of the element to erase
        FXTreeList* myTreelist;

        /// @brief map used to save the FXTreeItems items with their vinculated AC
        std::map<FXTreeItem*, GNEAttributeCarrier*> myTreeItemToACMap;

        /// @brief set used to save tree items without AC assigned, the Incoming/Outcoming connections
        std::set<FXTreeItem*> myTreeItemsConnections;
    };

    // ===========================================================================
    // class DrawingShape
    // ===========================================================================

    class DrawingShape : private FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEFrameModuls::DrawingShape)

    public:
        /// @brief constructor
        DrawingShape(GNEFrame* frameParent);

        /// @brief destructor
        ~DrawingShape();

        /// @brief show Drawing mode
        void showDrawingShape();

        /// @brief hide Drawing mode
        void hideDrawingShape();

        /// @brief start drawing
        void startDrawing();

        /// @brief stop drawing and check if shape can be created
        void stopDrawing();

        /// @brief abort drawing
        void abortDrawing();

        /// @brief add new point to temporal shape
        void addNewPoint(const Position& P);

        /// @brief remove last added point
        void removeLastPoint();

        /// @brief get Temporal shape
        const PositionVector& getTemporalShape() const;

        /// @brief return true if currently a shape is drawed
        bool isDrawing() const;

        /// @brief enable or disable delete last created point
        void setDeleteLastCreatedPoint(bool value);

        /// @brief get flag delete last created point
        bool getDeleteLastCreatedPoint();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user press start drawing button
        long onCmdStartDrawing(FXObject*, FXSelector, void*);

        /// @brief Called when the user press stop drawing button
        long onCmdStopDrawing(FXObject*, FXSelector, void*);

        /// @brief Called when the user press abort drawing button
        long onCmdAbortDrawing(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(DrawingShape)

    private:
        /// @brief pointer to frame parent
        GNEFrame* myFrameParent;

        /// @brief flag to enable/disable delete point mode
        bool myDeleteLastCreatedPoint;

        /// @brief current drawed shape
        PositionVector myTemporalShape;

        /// @brief button for start drawing
        FXButton* myStartDrawingButton;

        /// @brief button for stop drawing
        FXButton* myStopDrawingButton;

        /// @brief button for abort drawing
        FXButton* myAbortDrawingButton;

        /// @brief Label with information
        FXLabel* myInformationLabel;
    };

    // ===========================================================================
    // class SelectorParent
    // ===========================================================================

    class SelectorParent : protected FXGroupBox {
    public:
        /// @brief constructor
        SelectorParent(GNEFrame* frameParent);

        /// @brief destructor
        ~SelectorParent();

        /// @brief get currently parent additional selected
        std::string getIdSelected() const;

        /// @brief select manually a element of the list
        void setIDSelected(const std::string& id);

        /// @brief Show list of SelectorParent Modul
        bool showSelectorParentModul(SumoXMLTag additionalTypeParent);

        /// @brief hide SelectorParent Modul
        void hideSelectorParentModul();

        /// @brief Refresh list of Additional Parents Modul
        void refreshSelectorParentModul();

    private:
        /// @brief pointer to Frame Parent
        GNEFrame* myFrameParent;

        /// @brief current parent additional tag
        SumoXMLTag myParentTag;

        /// @brief Label with parent name
        FXLabel* myParentsLabel;

        /// @brief List of parents
        FXList* myParentsList;
    };

    // ===========================================================================
    // class OverlappedInspection
    // ===========================================================================

    class OverlappedInspection : private FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEFrameModuls::OverlappedInspection)

    public:
        /// @brief constructor
        OverlappedInspection(GNEFrame* frameParent);

        /// @brief constructor (used for filter objects under cusor
        OverlappedInspection(GNEFrame* frameParent, const SumoXMLTag filteredTag);

        /// @brief destructor
        ~OverlappedInspection();

        /// @brief show template editor
        void showOverlappedInspection(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const Position& clickedPosition);

        /// @brief hide template editor
        void hideOverlappedInspection();

        /// @brief check if overlappedInspection modul is shown
        bool overlappedInspectionShown() const;

        /// @brief get number of overlapped ACSs
        int getNumberOfOverlappedACs() const;

        /// @brief check if given position is near to saved position
        bool checkSavedPosition(const Position& clickedPosition) const;

        /// @brief try to go to next element if clicked position is near to saved position
        bool nextElement(const Position& clickedPosition);

        /// @brief try to go to previous element if clicked position is near to saved position
        bool previousElement(const Position& clickedPosition);

        /// @name FOX-callbacks
        /// @{

        /// @brief Inspect next Element (from top to bot)
        long onCmdNextElement(FXObject*, FXSelector, void*);

        /// @brief Inspect previous element (from top to bot)
        long onCmdPreviousElement(FXObject*, FXSelector, void*);

        /// @brief show list of overlapped elements
        long onCmdShowList(FXObject*, FXSelector, void*);

        /// @brief called when a list item is selected
        long onCmdListItemSelected(FXObject*, FXSelector, void*);

        /// @brief Called when user press the help button
        long onCmdOverlappingHelp(FXObject*, FXSelector, void*);
        /// @}

    protected:
        /// @brief FOX needs this
        OverlappedInspection();

        /// @brief build Fox Toolkit elemements
        void buildFXElements();

    private:
        /// @brief current frame parent
        GNEFrame* myFrameParent;

        /// @brief Previous element button
        FXButton* myPreviousElement;

        /// @brief Button for current index
        FXButton* myCurrentIndexButton;

        /// @brief Next element button
        FXButton* myNextElement;

        /// @brief list of overlapped elements
        FXList* myOverlappedElementList;

        /// @brief button for help
        FXButton* myHelpButton;

        /// @brief filtered tag
        const SumoXMLTag myFilteredTag;

        /// @brief objects under cursor
        std::vector<GNEAttributeCarrier*> myOverlappedACs;

        /// @brief current index item
        size_t myItemIndex;

        /// @brief saved clicked position
        Position mySavedClickedPosition;
    };

    // ===========================================================================
    // class PathCreator
    // ===========================================================================

    class PathCreator : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEFrameModuls::PathCreator)

    public:
        /// @brief class for path
        class Path {

        public:
            /// @brief constructor for single edge
            Path(const SUMOVehicleClass vClass, GNEEdge* edge);

            /// @brief constructor for multiple edges
            Path(GNEViewNet* viewNet, const SUMOVehicleClass vClass, GNEEdge* edgeFrom, GNEEdge* edgeTo);

            /// @brief get sub path
            const std::vector<GNEEdge*>& getSubPath() const;

            /// @brief get from additional
            GNEAdditional* getFromBusStop() const;

            /// @brief to additional
            GNEAdditional* getToBusStop() const;

            /// @brief check if current path is conflict due vClass
            bool isConflictVClass() const;

            /// @brief check if current path is conflict due is disconnected
            bool isConflictDisconnected() const;

        protected:
            /// @brief sub path
            std::vector<GNEEdge*> mySubPath;

            /// @brief from additional (usually a busStop)
            GNEAdditional* myFromBusStop;

            /// @brief to additional (usually a busStop)
            GNEAdditional* myToBusStop;

            /// @brief flag to mark this path as conflicted
            bool myConflictVClass;

            /// @brief flag to mark this path as disconnected
            bool myConflictDisconnected;

        private:
            /// @brief default constructor
            Path();

            /// @brief Invalidated copy constructor.
            Path(Path*) = delete;

            /// @brief Invalidated assignment operator.
            Path& operator=(Path*) = delete;
        };

        /// @brief default constructor
        PathCreator(GNEFrame* frameParent);

        /// @brief destructor
        ~PathCreator();

        /// @brief show PathCreator for the given tag
        void showPathCreatorModul(SumoXMLTag tag, const bool firstElement, const bool consecutives);

        /// @brief show PathCreator
        void hidePathCreatorModul();

        /// @brief get vClass
        SUMOVehicleClass getVClass() const;

        /// @brief set vClass
        void setVClass(SUMOVehicleClass vClass);

        /// @brief add edge
        bool addEdge(GNEEdge* edge, const bool shiftKeyPressed, const bool controlKeyPressed);

        /// @brief get current selected additionals
        std::vector<GNEEdge*> getSelectedEdges() const;

        /// @brief add stoppingPlace
        bool addStoppingPlace(GNEAdditional* stoppingPlace, const bool shiftKeyPressed, const bool controlKeyPressed);

        /// @brief get from stoppingPlace
        GNEAdditional* getFromStoppingPlace(SumoXMLTag expectedTag) const;

        /// @brief get to stoppingPlace
        GNEAdditional* getToStoppingPlace(SumoXMLTag expectedTag) const;

        /// @brief add route
        bool addRoute(GNEDemandElement* route, const bool shiftKeyPressed, const bool controlKeyPressed);

        /// @brief get route
        GNEDemandElement* getRoute() const;

        /// @brief get path route
        const std::vector<Path>& getPath() const;

        /// @brief draw candidate edges with special color (Only for candidates, special and conflicted)
        bool drawCandidateEdgesWithSpecialColor() const;

        /// @brief update edge colors
        void updateEdgeColors();

        /// @brief draw temporal route
        void drawTemporalRoute(const GUIVisualizationSettings* s) const;

        /// @brief create path
        void createPath();

        /// @brief abort path creation
        void abortPathCreation();

        /// @brief remove path element
        void removeLastElement();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user click over button "Finish route creation"
        long onCmdCreatePath(FXObject*, FXSelector, void*);

        /// @brief Called when the user click over button "Abort route creation"
        long onCmdAbortPathCreation(FXObject*, FXSelector, void*);

        /// @brief Called when the user click over button "Remove las inserted edge"
        long onCmdRemoveLastElement(FXObject*, FXSelector, void*);

        /// @brief Called when the user click over check button "show candidate edges"
        long onCmdShowCandidateEdges(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(PathCreator)

        // @brief creation mode
        enum Mode {
            CONSECUTIVE_EDGES       = 1 << 0,   // Path's edges are consecutives
            NONCONSECUTIVE_EDGES    = 1 << 1,   // Path's edges aren't consecutives
            START_EDGE              = 1 << 2,   // Path begins in an edge
            END_EDGE                = 1 << 3,   // Path ends in an edge
            SINGLE_ELEMENT          = 1 << 4,   // Path only had one element
            ONLY_FROMTO             = 1 << 5,   // Path only had two elements (first and last)
            START_BUSSTOP           = 1 << 6,   // Path begins in a busStop
            END_BUSSTOP             = 1 << 7,   // Path ends in a busStop
            ROUTE                   = 1 << 8,   // Path uses a route
            REQUIERE_FIRSTELEMENT   = 1 << 9,   // Path start always in a previous element
            SHOW_CANDIDATE_EDGES    = 1 << 10,  // disable candidate edges
        };

        /// @brief update InfoRouteLabel
        void updateInfoRouteLabel();

        /// @brief clear edges (and restore colors)
        void clearPath();

        /// @brief recalculate path
        void recalculatePath();

        /// @brief set special candidates (This function will be called recursively)
        void setSpecialCandidates(GNEEdge* originEdge);

        /// @brief set edgereachability (This function will be called recursively)
        void setPossibleCandidates(GNEEdge* originEdge, const SUMOVehicleClass vClass);

        /// @brief current frame parent
        GNEFrame* myFrameParent;

        /// @brief current vClass
        SUMOVehicleClass myVClass;

        /// @brief current creation mode
        int myCreationMode;

        /// @brief vector with selected edges
        std::vector<GNEEdge*> mySelectedEdges;

        /// @brief from additional (usually a busStop)
        GNEAdditional* myFromStoppingPlace;

        /// @brief to additional (usually a busStop)
        GNEAdditional* myToStoppingPlace;

        /// @brief route (usually a busStop)
        GNEDemandElement* myRoute;

        /// @brief vector with current path
        std::vector<Path> myPath;

        /// @brief label with route info
        FXLabel* myInfoRouteLabel;

        /// @brief button for finish route creation
        FXButton* myFinishCreationButton;

        /// @brief button for abort route creation
        FXButton* myAbortCreationButton;

        /// @brief button for removing last inserted element
        FXButton* myRemoveLastInsertedElement;

        /// @brief CheckBox for show candidate edges
        FXCheckButton* myShowCandidateEdges;

        /// @brief label for shift information
        FXLabel* myShiftLabel;

        /// @brief label for control information
        FXLabel* myControlLabel;

    private:
        /// @brief Invalidated copy constructor.
        PathCreator(PathCreator*) = delete;

        /// @brief Invalidated assignment operator.
        PathCreator& operator=(PathCreator*) = delete;
    };

    // ===========================================================================
    // class PathLegend
    // ===========================================================================

    class PathLegend : protected FXGroupBox {

    public:
        /// @brief constructor
        PathLegend(GNEFrame* frameParent);

        /// @brief destructor
        ~PathLegend();

        /// @brief show Legend modul
        void showPathLegendModul();

        /// @brief hide Legend modul
        void hidePathLegendModul();
    };

    // ===========================================================================
    // Functions
    // ===========================================================================

    /// @brief build rainbow in frame modul
    static FXLabel* buildRainbow(FXComposite* parent);
};
