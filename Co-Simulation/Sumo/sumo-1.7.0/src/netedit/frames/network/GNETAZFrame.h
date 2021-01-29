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
/// @file    GNETAZFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2018
///
// The Widget for add TAZ elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>


// ===========================================================================
// class definitions
// ===========================================================================
class GNETAZ;
class GNETAZSourceSink;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNETAZFrame
 * The Widget for setting internal attributes of TAZ elements
 */
class GNETAZFrame : public GNEFrame {

public:
    // ===========================================================================
    // class TAZCurrent
    // ===========================================================================

    class TAZCurrent : protected FXGroupBox {

    public:
        /// @brief struct for edges and the source/sink colors
        struct TAZEdge {
            /// @brief constructor
            TAZEdge(TAZCurrent* TAZCurrentParent, GNEEdge* _edge, GNETAZSourceSink* _TAZSource, GNETAZSourceSink* _TAZSink);

            /// @brief destructor (needed because RGBColors has to be deleted)
            ~TAZEdge();

            /// @brief update colors
            void updateColors();

            /// @brief TAZ edge
            GNEEdge* edge;

            /// @brief source TAZ
            GNETAZSourceSink* TAZSource;

            /// @brif sink TAZ
            GNETAZSourceSink* TAZSink;

            /// @brief color by source [0-9]
            int sourceColor;

            /// @brief color by sink [0-9]
            int sinkColor;

            /// @brief color by source + sink [0-9]
            int sourcePlusSinkColor;

            /// @brief color by source - sink [0-9]
            int sourceMinusSinkColor;

        private:
            /// @brief pointer to TAZCurrentParent
            TAZCurrent* myTAZCurrentParent;
        };

        /// @brief constructor
        TAZCurrent(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZCurrent();

        /// @brief set current TAZ
        void setTAZ(GNETAZ* editedTAZ);

        /// @brief get current TAZ
        GNETAZ* getTAZ() const;

        /// @brief check if given edge belongs to current TAZ
        bool isTAZEdge(GNEEdge* edge) const;

        /// @brief get current net edges
        const std::vector<GNEEdge*>& getNetEdges() const;

        /// @brief get current selected edges
        const std::vector<GNEEdge*>& getSelectedEdges() const;

        /// @brief get TAZEdges
        const std::vector<TAZCurrent::TAZEdge>& getTAZEdges() const;

        /// @brief refresh TAZEdges
        void refreshTAZEdges();

    protected:
        /// @brief add TAZChild
        void addTAZChild(GNETAZSourceSink* additional);

    private:
        /// @brief pointer to TAZ Frame
        GNETAZFrame* myTAZFrameParent;

        /// @brief current edited TAZ
        GNETAZ* myEditedTAZ;

        /// @brief vector with pointers to edges (it's used to avoid slowdowns during Source/Sinks manipulations)
        std::vector<GNEEdge*> myNetEdges;

        /// @brief vector with pointers to selected edges
        std::vector<GNEEdge*> mySelectedEdges;

        /// @brief vector with TAZ's edges
        std::vector<TAZEdge> myTAZEdges;

        /// @brief Label for current TAZ
        FXLabel* myTAZCurrentLabel;

        /// @brief maximum source plus sink value of current TAZ Edges
        double myMaxSourcePlusSinkWeight;

        /// @brief minimum source plus sink value of current TAZ Edges
        double myMinSourcePlusSinkWeight;

        /// @brief maximum source minus sink value of current TAZ Edges
        double myMaxSourceMinusSinkWeight;

        /// @brief minimum source minus sink value of current TAZ Edges
        double myMinSourceMinusSinkWeight;
    };

    // ===========================================================================
    // class TAZCommonStatistics
    // ===========================================================================

    class TAZCommonStatistics : protected FXGroupBox {

    public:
        /// @brief constructor
        TAZCommonStatistics(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZCommonStatistics();

        /// @brief show TAZ Common Statistics Modul
        void showTAZCommonStatisticsModul();

        /// @brief hide TAZ Common Statistics Modul
        void hideTAZCommonStatisticsModul();

    protected:
        /// @brief update Statistics label
        void updateStatistics();

    private:
        /// @brief pointer to TAZFrame parent
        GNETAZFrame* myTAZFrameParent;

        /// @brief Statistics labels
        FXLabel* myStatisticsLabel;
    };

    // ===========================================================================
    // class TAZSaveChanges
    // ===========================================================================

    class TAZSaveChanges : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNETAZFrame::TAZSaveChanges)

    public:
        /// @brief constructor
        TAZSaveChanges(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZSaveChanges();

        /// @brief show TAZ Save Changes Modul
        void showTAZSaveChangesModul();

        /// @brief hide TAZ Save Changes Modul
        void hideTAZSaveChangesModul();

        /// @brief enable buttons save and cancel changes (And begin Undo List)
        void enableButtonsAndBeginUndoList();

        /// @brief return true if there is changes to save
        bool isChangesPending() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user press the button save changes
        long onCmdSaveChanges(FXObject*, FXSelector, void*);

        /// @brief Called when the user press the button cancel changes
        long onCmdCancelChanges(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(TAZSaveChanges)

    private:
        /// @brief pointer to TAZFrame parent
        GNETAZFrame* myTAZFrameParent;

        /// @field FXButton for save changes in TAZEdges
        FXButton* mySaveChangesButton;

        /// @field FXButton for cancel changes in TAZEdges
        FXButton* myCancelChangesButton;
    };

    // ===========================================================================
    // class TAZChildDefaultParameters
    // ===========================================================================

    class TAZChildDefaultParameters : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNETAZFrame::TAZChildDefaultParameters)

    public:
        /// @brief constructor
        TAZChildDefaultParameters(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZChildDefaultParameters();

        /// @brief show TAZ child default parameters Modul
        void showTAZChildDefaultParametersModul();

        /// @brief hide TAZ child default parameters Modul
        void hideTAZChildDefaultParametersModul();

        /// @brief update "select edges button"
        void updateSelectEdgesButton();

        /// @brief get default TAZSource weight
        double getDefaultTAZSourceWeight() const;

        /// @brief default TAZSink weight
        double getDefaultTAZSinkWeight() const;

        /// @brief check if toggle membership is enabled
        bool getToggleMembership() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user changes default values
        long onCmdSetDefaultValues(FXObject* obj, FXSelector, void*);

        /// @brief Called when the user press use selected edges
        long onCmdUseSelectedEdges(FXObject* obj, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(TAZChildDefaultParameters)

    private:
        /// @brief pointer to TAZFrame parent
        GNETAZFrame* myTAZFrameParent;

        /// @brief CheckButton to enable or disable Toggle edge Membership
        FXCheckButton* myToggleMembership;

        /// @brief Horizontal Frame for default TAZ Source Weight
        FXHorizontalFrame* myDefaultTAZSourceFrame;

        /// @brief textField to set a default value for TAZ Sources
        FXTextField* myTextFieldDefaultValueTAZSources;

        /// @brief Horizontal Frame for default TAZ Sink Weight
        FXHorizontalFrame* myDefaultTAZSinkFrame;

        /// @brief textField to set a default value for TAZ Sinks
        FXTextField* myTextFieldDefaultValueTAZSinks;

        /// @brief button for use selected edges
        FXButton* myUseSelectedEdges;

        /// @brief information label
        FXLabel* myInformationLabel;

        /// @brief default TAZSource weight
        double myDefaultTAZSourceWeight;

        /// @brief default TAZSink weight
        double myDefaultTAZSinkWeight;
    };

    // ===========================================================================
    // class TAZSelectionStatistics
    // ===========================================================================

    class TAZSelectionStatistics : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNETAZFrame::TAZSelectionStatistics)

    public:
        /// @brief constructor
        TAZSelectionStatistics(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZSelectionStatistics();

        /// @brief show TAZ Selection Statistics Modul
        void showTAZSelectionStatisticsModul();

        /// @brief hide TAZ Selection Statistics Modul
        void hideTAZSelectionStatisticsModul();

        /// @brief add an edge and their TAZ Children in the list of selected items
        bool selectEdge(const TAZCurrent::TAZEdge& edge);

        /// @brief un select an edge (and their TAZ Children)
        bool unselectEdge(GNEEdge* edge);

        /// @brief check if an edge is selected
        bool isEdgeSelected(GNEEdge* edge);

        /// @brief clear current TAZ children
        void clearSelectedEdges();

        /// @brief get map with edge and TAZChildren
        const std::vector<TAZCurrent::TAZEdge>& getEdgeAndTAZChildrenSelected() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user set a new value
        long onCmdSetNewValues(FXObject* obj, FXSelector, void*);

        /// @brief Called when the user press select edges
        long onCmdSelectEdges(FXObject* obj, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(TAZSelectionStatistics)

        /// @brief update TAZSelectionStatistics
        void updateStatistics();

    private:
        /// @brief pointer to TAZFrame parent
        GNETAZFrame* myTAZFrameParent;

        /// @brief Horizontal Frame for default TAZ Source Weight
        FXHorizontalFrame* myTAZSourceFrame;

        /// @brief textField for TAZ Source weight
        FXTextField* myTextFieldTAZSourceWeight;

        /// @brief Horizontal Frame for default TAZ Sink Weight
        FXHorizontalFrame* myTAZSinkFrame;

        /// @brief textField for TAZ Sink weight
        FXTextField* myTextFieldTAZSinkWeight;

        /// @brief Statistics labels
        FXLabel* myStatisticsLabel;

        /// @brief vector with the current selected edges and their associated children
        std::vector<TAZCurrent::TAZEdge> myEdgeAndTAZChildrenSelected;
    };

    // ===========================================================================
    // class TAZParameters
    // ===========================================================================

    class TAZParameters : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNETAZFrame::TAZParameters)

    public:
        /// @brief constructor
        TAZParameters(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZParameters();

        /// @brief show TAZ parameters and set the default value of parameters
        void showTAZParametersModul();

        /// @brief hide TAZ parameters
        void hideTAZParametersModul();

        /// @brief check if current parameters are valid
        bool isCurrentParametersValid() const;

        /// @brief check if edges within has to be used after TAZ Creation
        bool isAddEdgesWithinEnabled() const;

        /// @brief get a map with attributes and their values
        std::map<SumoXMLAttr, std::string> getAttributesAndValues() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user press the "Color" button
        long onCmdSetColorAttribute(FXObject*, FXSelector, void*);

        /// @brief Called when user set a value
        long onCmdSetAttribute(FXObject*, FXSelector, void*);

        /// @brief Called when help button is pressed
        long onCmdHelp(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(TAZParameters)

    private:
        /// @brief pointer to GNETAZFrame parent
        GNETAZFrame* myTAZFrameParent;

        /// @brief Button for open color editor
        FXButton* myColorEditor;

        /// @brief textField to modify the default value of color parameter
        FXTextField* myTextFieldColor;

        /// @brief CheckButton to enable or disable use edges within TAZ after creation
        FXCheckButton* myAddEdgesWithinCheckButton;

        /// @brief button for help
        FXButton* myHelpTAZAttribute;
    };

    // ===========================================================================
    // class TAZEdgesGraphic
    // ===========================================================================

    class TAZEdgesGraphic : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNETAZFrame::TAZEdgesGraphic)

    public:
        /// @brief constructor
        TAZEdgesGraphic(GNETAZFrame* TAZFrameParent);

        /// @brief destructor
        ~TAZEdgesGraphic();

        /// @brief show TAZ Edges Graphic Modul
        void showTAZEdgesGraphicModul();

        /// @brief hide TAZ Edges Graphic Modul
        void hideTAZEdgesGraphicModul();

        /// @brief update edge colors;
        void updateEdgeColors();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user select one kind of representation
        long onCmdChoosenBy(FXObject* obj, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(TAZEdgesGraphic)

    private:
        /// @brief pointer to TAZFrame parent
        GNETAZFrame* myTAZFrameParent;

        /// @brief add radio button "color by source"
        FXRadioButton* myColorBySourceWeight;

        /// @brief add radio button "color by sink"
        FXRadioButton* myColorBySinkWeight;

        /// @brief add radio button "color source + sink"
        FXRadioButton* myColorBySourcePlusSinkWeight;

        /// @brief add radio button "color source - Sink"
        FXRadioButton* myColorBySourceMinusSinkWeight;

        /// @brief default RGBColor for all edges
        RGBColor myEdgeDefaultColor;

        /// @brief RGBColor color for selected egdes
        RGBColor myEdgeSelectedColor;
    };

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNETAZFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNETAZFrame();

    /// @brief hide TAZ frame
    void hide();

    /**@brief process click over Viewnet
    * @param[in] clickedPosition clicked position over ViewNet
    * @param[in] objectsUnderCursor objects under cursors
    * @return true if something (select TAZ or add edge) was sucefully done
    */
    bool processClick(const Position& clickedPosition, const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor);

    /// @brief process selection of edges in view net
    void processEdgeSelection(const std::vector<GNEEdge*>& edges);

    /// @brief get drawing mode modul
    GNEFrameModuls::DrawingShape* getDrawingShapeModul() const;

    /// @brief get Current TAZ modul
    TAZCurrent* getTAZCurrentModul() const;

    /// @brief get TAZ Selection Statistics modul
    TAZSelectionStatistics* getTAZSelectionStatisticsModul() const;

    /// @brief get TAZ Save Changes modul
    TAZSaveChanges* getTAZSaveChangesModul() const;

protected:
    /**@brief build a shaped element using the drawed shape
     * return true if was sucesfully created
     * @note called when user stop drawing shape
     */
    bool shapeDrawed();

    /// @brief add or remove a TAZSource and a TAZSink, or remove it if edge is in the list of TAZ Children
    bool addOrRemoveTAZMember(GNEEdge* edge);

    /// @brief drop all TAZSources and TAZ Sinks of current TAZ
    void dropTAZMembers();

private:
    /// @brief current TAZ
    TAZCurrent* myTAZCurrent;

    /// @brief TAZ Edges common parameters
    TAZCommonStatistics* myTAZCommonStatistics;

    /// @brief TAZ parameters
    TAZParameters* myTAZParameters;

    /// @brief Netedit parameter
    GNEFrameAttributesModuls::NeteditAttributes* myNeteditAttributes;

    /// @brief Drawing shape
    GNEFrameModuls::DrawingShape* myDrawingShape;

    /// @brief save TAZ Edges
    TAZSaveChanges* myTAZSaveChanges;

    /// @brief TAZ child defaults parameters
    TAZChildDefaultParameters* myTAZChildDefaultParameters;

    /// @brief TAZ Edges selection parameters
    TAZSelectionStatistics* myTAZSelectionStatistics;

    /// @brief TAZ Edges Graphic
    TAZEdgesGraphic* myTAZEdgesGraphic;
};
