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
/// @file    GNEAdditionalFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Dec 2015
///
// The Widget for add additional elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEAdditionalFrame
 * The Widget for setting internal attributes of additional elements
 */
class GNEAdditionalFrame : public GNEFrame {

public:

    // ===========================================================================
    // class SelectorParentLanes
    // ===========================================================================

    class SelectorParentLanes : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEAdditionalFrame::SelectorParentLanes)
    public:
        /// @brief constructor
        SelectorParentLanes(GNEAdditionalFrame* additionalFrameParent);

        /// @brief destructor
        ~SelectorParentLanes();

        /// @brief show SelectorParentLanes modul
        void showSelectorParentLanesModul();

        /// @brief hide SelectorParentLanes
        void hideSelectorParentLanesModul();

        /// @brief start selection of consecutive lanes
        void startConsecutiveLaneSelector(GNELane* lane, const Position& clickedPosition);

        /// @brief stop selection of consecutive lanes
        bool stopConsecutiveLaneSelector();

        /// @brief abort selection of consecutive lanes
        void abortConsecutiveLaneSelector();

        /// @brief return true if lane can be selected as consecutive lane
        bool addSelectedLane(GNELane* lane, const Position& clickedPosition);

        /// @brief remove last added point
        void removeLastSelectedLane();

        /// @brief return true if modul is selecting lane
        bool isSelectingLanes() const;

        /// @brief return true if modul is shown
        bool isShown() const;

        /// @brief get current selected lanes
        const std::vector<std::pair<GNELane*, double> >& getSelectedLanes() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user press stop selection button
        long onCmdStopSelection(FXObject*, FXSelector, void*);

        /// @brief Called when the user press abort selection button
        long onCmdAbortSelection(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(SelectorParentLanes)

    private:
        /// @brief pointer to additionalFrameParent
        GNEAdditionalFrame* myAdditionalFrameParent;

        /// @brief button for stop selecting
        FXButton* myStopSelectingButton;

        /// @brief button for abort selecting
        FXButton* myAbortSelectingButton;

        /// @brief Vector with the selected lanes and the clicked position
        std::vector<std::pair<GNELane*, double> > mySelectedLanes;

        /// @brief Vector with the colored lanes
        std::vector<GNELane*> myCandidateLanes;

        /// @brief check if certain lane is selected
        bool isLaneSelected(GNELane* lane) const;
    };

    // ===========================================================================
    // class SelectorChildEdges
    // ===========================================================================

    class SelectorChildEdges : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEAdditionalFrame::SelectorChildEdges)

    public:
        /// @brief constructor
        SelectorChildEdges(GNEAdditionalFrame* additionalFrameParent);

        /// @brief destructor
        ~SelectorChildEdges();

        /// @brief get list of selecte id's in string format
        std::string getEdgeIdsSelected() const;

        /// @brief Show SelectorChildEdges Modul
        void showSelectorChildEdgesModul(std::string search = "");

        /// @brief hide SelectorChildEdges Modul
        void hideSelectorChildEdgesModul();

        /// @brief Update use selectedEdges
        void updateUseSelectedEdges();

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user trigger checkBox of useSelectedEdges
        long onCmdUseSelectedEdges(FXObject*, FXSelector, void*);

        /// @brief called when user type in search box
        long onCmdTypeInSearchBox(FXObject*, FXSelector, void*);

        /// @brief called when user select a edge of the list
        long onCmdSelectEdge(FXObject*, FXSelector, void*);

        /// @brief called when clear selection button is pressed
        long onCmdClearSelection(FXObject*, FXSelector, void*);

        /// @brief called when invert selection button is pressed
        long onCmdInvertSelection(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(SelectorChildEdges)

    private:
        /// @brief pointer to additional frame parent
        GNEAdditionalFrame* myAdditionalFrameParent;

        /// @brief CheckBox for selected edges
        FXCheckButton* myUseSelectedEdgesCheckButton;

        /// @brief List of SelectorChildEdges
        FXList* myList;

        /// @brief text field for search edge IDs
        FXTextField* myEdgesSearch;

        /// @brief button for clear selection
        FXButton* myClearEdgesSelection;

        /// @brief button for invert selection
        FXButton* myInvertEdgesSelection;
    };

    // ===========================================================================
    // class SelectorChildLanes
    // ===========================================================================

    class SelectorChildLanes : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEAdditionalFrame::SelectorChildLanes)

    public:
        /// @brief constructor
        SelectorChildLanes(GNEAdditionalFrame* additionalFrameParent);

        /// @brief destructor
        ~SelectorChildLanes();

        /// @brief get list of selecte lane ids in string format
        std::string getLaneIdsSelected() const;

        /// @brief Show list of SelectorChildLanes Modul
        void showSelectorChildLanesModul(std::string search = "");

        /// @brief hide SelectorChildLanes Modul
        void hideSelectorChildLanesModul();

        // @brief Update use selectedLanes
        void updateUseSelectedLanes();

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user trigger checkBox of useSelectedLanes
        long onCmdUseSelectedLanes(FXObject*, FXSelector, void*);

        /// @brief called when user type in search box
        long onCmdTypeInSearchBox(FXObject*, FXSelector, void*);

        /// @brief called when user select a lane of the list
        long onCmdSelectLane(FXObject*, FXSelector, void*);

        /// @brief called when clear selection button is pressed
        long onCmdClearSelection(FXObject*, FXSelector, void*);

        /// @brief called when invert selection button is pressed
        long onCmdInvertSelection(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(SelectorChildLanes)

    private:
        /// @brief pointer to additional frame parent
        GNEAdditionalFrame* myAdditionalFrameParent;

        /// @brief CheckBox for selected lanes
        FXCheckButton* myUseSelectedLanesCheckButton;

        /// @brief List of SelectorChildLanes
        FXList* myList;

        /// @brief text field for search lane IDs
        FXTextField* myLanesSearch;

        /// @brief button for clear selection
        FXButton* clearLanesSelection;

        /// @brief button for invert selection
        FXButton* invertLanesSelection;
    };

    // ===========================================================================
    // class E2MultilaneLaneSelector
    // ===========================================================================

    class E2MultilaneLaneSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEAdditionalFrame::E2MultilaneLaneSelector)

    public:
        /// @brief default constructor
        E2MultilaneLaneSelector(GNEAdditionalFrame* additionalFrameParent);

        /// @brief destructor
        ~E2MultilaneLaneSelector();

        /// @brief show E2MultilaneLaneSelector
        void showE2MultilaneLaneSelectorModul();

        /// @brief show E2MultilaneLaneSelector
        void hideE2MultilaneLaneSelectorModul();

        /// @brief add lane
        bool addLane(GNELane* lane);

        /// @brief draw candidate lanes with special color (Only for candidates, special and conflicted)
        bool drawCandidateLanesWithSpecialColor() const;

        /// @brief update lane colors
        void updateLaneColors();

        /// @brief draw temporal E2Multilane
        void drawTemporalE2Multilane(const GUIVisualizationSettings& s) const;

        /// @brief create path
        bool createPath();

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

        /// @brief Called when the user click over button "Remove las inserted lane"
        long onCmdRemoveLastElement(FXObject*, FXSelector, void*);

        /// @brief Called when the user click over check button "show candidate lanes"
        long onCmdShowCandidateLanes(FXObject*, FXSelector, void*);
        /// @}

    protected:
        /// @brief fox need this
        FOX_CONSTRUCTOR(E2MultilaneLaneSelector)

        /// @brief update InfoRouteLabel
        void updateInfoRouteLabel();

        /// @brief clear lanes (and restore colors)
        void clearPath();

        /// @brief current additional frame parent
        GNEAdditionalFrame* myAdditionalFrameParent;

        /// @brief vector with lanes and clicked positions
        std::vector<std::pair<GNELane*, double> > myLanePath;

        /// @brief label with route info
        FXLabel* myInfoRouteLabel;

        /// @brief button for finish route creation
        FXButton* myFinishCreationButton;

        /// @brief button for abort route creation
        FXButton* myAbortCreationButton;

        /// @brief button for removing last inserted element
        FXButton* myRemoveLastInsertedElement;

        /// @brief CheckBox for show candidate lanes
        FXCheckButton* myShowCandidateLanes;

    private:
        /// @brief Invalidated copy constructor.
        E2MultilaneLaneSelector(E2MultilaneLaneSelector*) = delete;

        /// @brief Invalidated assignment operator.
        E2MultilaneLaneSelector& operator=(E2MultilaneLaneSelector*) = delete;
    };

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEAdditionalFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEAdditionalFrame();

    /// @brief show Frame
    void show();

    /**@brief add additional element
     * @param objectsUnderCursor collection of objects under cursor after click over view
     * @return true if additional was sucesfully added
     */
    bool addAdditional(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor);

    /// @brief show selector child lane and update use selected edges/lanes
    void showSelectorChildLanesModul();

    /// @brief get consecutive Lane Selector
    GNEAdditionalFrame::SelectorParentLanes* getConsecutiveLaneSelector() const;

    /// @brief getConsecutive Lane Selector
    GNEAdditionalFrame::E2MultilaneLaneSelector* getE2MultilaneLaneSelector() const;

protected:
    /// @brief Tag selected in TagSelector
    void tagSelected();

private:
    /// @brief generate a ID for an additiona element
    std::string generateID(GNENetworkElement* networkElement) const;

    /// @brief build common additional attributes
    bool buildAdditionalCommonAttributes(std::map<SumoXMLAttr, std::string>& valuesMap, const GNETagProperties& tagValues);

    /// @brief build slave additional
    bool buildSlaveAdditional(std::map<SumoXMLAttr, std::string>& valuesMap, GNEAdditional* parent, const GNETagProperties& tagValues);

    /// @brief build additional over an edge (parent of lane)
    bool buildAdditionalOverEdge(std::map<SumoXMLAttr, std::string>& valuesMap, GNELane* lane, const GNETagProperties& tagValues);

    /// @brief build additional over a single lane
    bool buildAdditionalOverLane(std::map<SumoXMLAttr, std::string>& valuesMap, GNELane* lane, const GNETagProperties& tagValues);

    /// @brief build additional over view
    bool buildAdditionalOverView(std::map<SumoXMLAttr, std::string>& valuesMap, const GNETagProperties& tagValues);

    /// @brief item selector
    GNEFrameModuls::TagSelector* myAdditionalTagSelector;

    /// @brief internal additional attributes
    GNEFrameAttributesModuls::AttributesCreator* myAdditionalAttributes;

    /// @brief Netedit parameter
    GNEFrameAttributesModuls::NeteditAttributes* myNeteditAttributes;

    /// @brief Modul for select parent lanes (currently only consecutives)
    SelectorParentLanes* mySelectorParentLanes;

    /// @brief Modul for select a single parent additional
    GNEFrameModuls::SelectorParent* myParentAdditional;

    /// @brief Modul for select child edges
    SelectorChildEdges* mySelectorChildEdges;

    /// @brief Modul for select child lanes
    SelectorChildLanes* mySelectorChildLanes;

    /// @brief Modul for E2Multilane lane selector
    E2MultilaneLaneSelector* myE2MultilaneLaneSelector;
};
