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
/// @file    GNEGenericDataFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// The Widget for add genericData elements
/****************************************************************************/
#pragma once
#include <config.h>

#include <netedit/frames/GNEFrame.h>


// ===========================================================================
// class declaration
// ===========================================================================
class GNEDataInterval;
class GNEDataSet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEGenericDataFrame
 * The Widget for setting internal attributes of additional elements
 */
class GNEGenericDataFrame : public GNEFrame {

public:
    // ===========================================================================
    // class DataSetSelector
    // ===========================================================================

    class DataSetSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEGenericDataFrame::DataSetSelector)

    public:
        /// @brief constructor
        DataSetSelector(GNEGenericDataFrame* genericDataFrameParent);

        /// @brief destructor
        ~DataSetSelector();

        /// @brief refresh interval selector
        void refreshDataSetSelector(const GNEDataSet* currentDataSet);

        /// @brief get current select data set ID
        GNEDataSet* getDataSet() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user press button "create dataSet"
        long onCmdCreateDataSet(FXObject*, FXSelector, void*);

        /// @brief Called when the user set a new data set ID
        long onCmdSetNewDataSetID(FXObject*, FXSelector, void*);

        /// @brief Called when the user select an existent data set
        long onCmdSelectDataSet(FXObject* obj, FXSelector, void*);

        /// @brief Called when the user select check button
        long onCmdSelectCheckButton(FXObject* obj, FXSelector, void*);

        /// @}

    protected:
        FOX_CONSTRUCTOR(DataSetSelector)

    private:
        /// @brief pointer to genericData frame Parent
        GNEGenericDataFrame* myGenericDataFrameParent;

        /// @brief check button to create a new dataSet
        FXCheckButton* myNewDataSetCheckButton;

        /// @brief horizontal frame new id
        FXHorizontalFrame* myHorizontalFrameNewID;

        /// @brief interval new id
        FXTextField* myNewDataSetIDTextField;

        /// @brief create dataSet button
        FXButton* myCreateDataSetButton;

        /// @brief comboBox with intervals
        FXComboBox* myDataSetsComboBox;
    };

    // ===========================================================================
    // class IntervalSelector
    // ===========================================================================

    class IntervalSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEGenericDataFrame::IntervalSelector)

    public:
        /// @brief constructor
        IntervalSelector(GNEGenericDataFrame* genericDataFrameParent);

        /// @brief destructor
        ~IntervalSelector();

        /// @brief refresh interval selector
        void refreshIntervalSelector();

        /// @brief get current select data set ID
        GNEDataInterval* getDataInterval() const;

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user press button "create interval"
        long onCmdCreateInterval(FXObject*, FXSelector, void*);

        /// @brief Called when the user select an interval in the list
        long onCmdSelectInterval(FXObject*, FXSelector, void*);

        /// @brief Called when the user changes begin or end
        long onCmdSetIntervalAttribute(FXObject*, FXSelector, void*);

        /// @brief Called when the user select check button
        long onCmdSelectCheckButton(FXObject* obj, FXSelector, void*);

        /// @}

    protected:
        FOX_CONSTRUCTOR(IntervalSelector)

    private:
        /// @brief add interval item into list
        FXTreeItem* addIntervalItem(GNEDataInterval* dataInterval, FXTreeItem* itemParent = nullptr);

        /// @brief pointer to genericData frame Parent
        GNEGenericDataFrame* myGenericDataFrameParent;

        /// @brief check button to create a new interval
        FXCheckButton* myNewIntervalCheckButton;

        /// @brief interval begin horizontal frame
        FXHorizontalFrame* myHorizontalFrameBegin;

        /// @brief interval begin text field
        FXTextField* myBeginTextField;

        /// @brief interval end horizontal frame
        FXHorizontalFrame* myHorizontalFrameEnd;

        /// @brief interval end
        FXTextField* myEndTextField;

        /// @brief create interval button
        FXButton* myCreateIntervalButton;

        /// @brief tree list to show the interval list
        FXTreeList* myIntervalsTreelist;

        /// @brief map used to save the Tree items with their GNEDataInterval
        std::map<FXTreeItem*, GNEDataInterval*> myTreeItemIntervalMap;
    };

    // ===========================================================================
    // class AttributeSelector
    // ===========================================================================

    class AttributeSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEGenericDataFrame::AttributeSelector)

    public:
        /// @brief constructor
        AttributeSelector(GNEGenericDataFrame* genericDataFrameParent, SumoXMLTag tag);

        /// @brief destructor
        ~AttributeSelector();

        /// @brief refresh interval selector
        void refreshAttributeSelector();

        /// @brief get filtered attribute
        std::string getFilteredAttribute() const;

        /// @brief get color for the given value
        const RGBColor& getScaledColor(const double min, const double max, const double value) const;

        /// @name FOX-callbacks
        /// @{

        /// @brief Called when the user select a attribute in the combo boz
        long onCmdSelectAttribute(FXObject*, FXSelector, void*);

        /// @}

    protected:
        FOX_CONSTRUCTOR(AttributeSelector)

    private:
        /// @brief pointer to genericData frame Parent
        GNEGenericDataFrame* myGenericDataFrameParent;

        /// @brief combo box for attributes
        FXComboBox* myAttributesComboBox;

        /// @brief label for min/max attribute
        FXLabel* myMinMaxLabel;

        /// @brief generic data tag
        SumoXMLTag myGenericDataTag;
    };

    /// @brief get dataSet selector modul
    const DataSetSelector* getDataSetSelector() const;

    /// @brief get interval selector modul
    const IntervalSelector* getIntervalSelector() const;

    /// @brief getattribute selector modul
    const AttributeSelector* getAttributeSelector() const;

    /// @brief get PathCreator modul
    GNEFrameModuls::PathCreator* getPathCreator() const;

    /// @bried get element type of this data frame
    SumoXMLTag getTag() const;

    /// @brief show Frame
    void show();

    /// @brief hide Frame
    void hide();

protected:
    /**@brief Constructor (protected due GNEGenericDataFrame is abtract)
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet GNEViewNet that uses this GNEFrame
     * @brief tag generic data tag
     * @brief pathCreator flag to create pathCreator
     */
    GNEGenericDataFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet, SumoXMLTag tag, const bool pathCreator);

    /// @brief Destructor
    ~GNEGenericDataFrame();

    /// @brief interval selected
    void intervalSelected();

    /// @brief create path
    virtual void createPath();

    /// @brief dataSet selector modul
    DataSetSelector* myDataSetSelector;

    /// @brief interval selector modul
    IntervalSelector* myIntervalSelector;

    /// @brief attribute selector modul
    AttributeSelector* myAttributeSelector;

    /// @brief parameters editor
    GNEFrameAttributesModuls::ParametersEditor* myParametersEditor;

    /// @brief edge path creator (used for Walks, rides and trips)
    GNEFrameModuls::PathCreator* myPathCreator;

    /// @brief generic data tag
    SumoXMLTag myGenericDataTag;

private:
    /// @brief Invalidated copy constructor.
    GNEGenericDataFrame(const GNEGenericDataFrame&) = delete;

    /// @brief Invalidated assignment operator.
    GNEGenericDataFrame& operator=(const GNEGenericDataFrame&) = delete;
};
