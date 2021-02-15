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
/// @file    GNEGenericDataFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// The Widget for add genericData elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/data/GNEDataHandler.h>
#include <netedit/elements/data/GNEDataInterval.h>
#include <netedit/elements/data/GNEDataSet.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEGenericDataFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEGenericDataFrame::DataSetSelector) DataSetSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_CREATE,              GNEGenericDataFrame::DataSetSelector::onCmdCreateDataSet),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_DATASET_NEW,         GNEGenericDataFrame::DataSetSelector::onCmdSetNewDataSetID),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_DATASET_SELECTED,    GNEGenericDataFrame::DataSetSelector::onCmdSelectDataSet),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SELECT,              GNEGenericDataFrame::DataSetSelector::onCmdSelectCheckButton)

};

FXDEFMAP(GNEGenericDataFrame::IntervalSelector) IntervalSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_CREATE,                  GNEGenericDataFrame::IntervalSelector::onCmdCreateInterval),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_DATAINTERVAL_SELECTED,   GNEGenericDataFrame::IntervalSelector::onCmdSelectInterval),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_ATTRIBUTE,           GNEGenericDataFrame::IntervalSelector::onCmdSetIntervalAttribute),
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SELECT,                  GNEGenericDataFrame::IntervalSelector::onCmdSelectCheckButton)
};

FXDEFMAP(GNEGenericDataFrame::AttributeSelector) AttributeSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SELECT,  GNEGenericDataFrame::AttributeSelector::onCmdSelectAttribute)
};

// Object implementation
FXIMPLEMENT(GNEGenericDataFrame::DataSetSelector,   FXGroupBox, DataSetSelectorMap,   ARRAYNUMBER(DataSetSelectorMap))
FXIMPLEMENT(GNEGenericDataFrame::IntervalSelector,  FXGroupBox, IntervalSelectorMap,  ARRAYNUMBER(IntervalSelectorMap))
FXIMPLEMENT(GNEGenericDataFrame::AttributeSelector, FXGroupBox, AttributeSelectorMap, ARRAYNUMBER(AttributeSelectorMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEGenericDataFrame::DataSetSelector - methods
// ---------------------------------------------------------------------------

GNEGenericDataFrame::DataSetSelector::DataSetSelector(GNEGenericDataFrame* genericDataFrameParent) :
    FXGroupBox(genericDataFrameParent->myContentFrame, "DataSet", GUIDesignGroupBoxFrame),
    myGenericDataFrameParent(genericDataFrameParent) {
    // create check button for new data set
    myNewDataSetCheckButton = new FXCheckButton(this, "Create new dataSet", this, MID_GNE_SELECT, GUIDesignCheckButton);
    // Create FXComboBox
    myDataSetsComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_DATASET_SELECTED, GUIDesignComboBox);
    // create new id label
    myHorizontalFrameNewID = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myHorizontalFrameNewID, "new dataSet ID", nullptr, GUIDesignLabelAttribute);
    // create new id textField
    myNewDataSetIDTextField = new FXTextField(myHorizontalFrameNewID, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // hide horizontal frame
    myHorizontalFrameNewID->hide();
    // create dataSet button
    myCreateDataSetButton = new FXButton(this, "Create dataSet", GUIIconSubSys::getIcon(GUIIcon::DATASET), this, MID_GNE_CREATE, GUIDesignButton);
    myCreateDataSetButton->hide();
    // refresh interval selector
    refreshDataSetSelector(nullptr);
    // DataSetSelector is always shown
    show();
}


GNEGenericDataFrame::DataSetSelector::~DataSetSelector() {}


void
GNEGenericDataFrame::DataSetSelector::refreshDataSetSelector(const GNEDataSet* currentDataSet) {
    // clear items
    myDataSetsComboBox->clearItems();
    // declare item index
    int currentItemIndex = -1;
    // fill myDataSetsComboBox with all DataSets
    auto dataSetCopy = myGenericDataFrameParent->getViewNet()->getNet()->retrieveDataSets();
    for (const auto& dataSet : dataSetCopy) {
        // check if we have to set currentItemIndex
        if ((currentItemIndex == -1) && (dataSet == currentDataSet)) {
            currentItemIndex = myDataSetsComboBox->getNumItems();
        }
        myDataSetsComboBox->appendItem(dataSet->getID().c_str());
    }
    // Set visible items
    myDataSetsComboBox->setNumVisible((int)myDataSetsComboBox->getNumItems());
    // check if we have to set current element
    if (currentItemIndex != -1) {
        myDataSetsComboBox->setCurrentItem(currentItemIndex, FALSE);
    }
    // recalc frame
    recalc();
    // refresh interval selector
    if (myGenericDataFrameParent->myIntervalSelector) {
        myGenericDataFrameParent->myIntervalSelector->refreshIntervalSelector();
    }
}


GNEDataSet*
GNEGenericDataFrame::DataSetSelector::getDataSet() const {
    if ((myNewDataSetCheckButton->getCheck() == TRUE) || (myDataSetsComboBox->getNumItems() == 0)) {
        return nullptr;
    } else {
        return myGenericDataFrameParent->getViewNet()->getNet()->retrieveDataSet(myDataSetsComboBox->getItem(myDataSetsComboBox->getCurrentItem()).text(), false);
    }
}


long
GNEGenericDataFrame::DataSetSelector::onCmdCreateDataSet(FXObject*, FXSelector, void*) {
    // get string
    const std::string dataSetID = myNewDataSetIDTextField->getText().text();
    // check conditions
    if (myNewDataSetIDTextField->getTextColor() == FXRGB(255, 0, 0)) {
        WRITE_WARNING("Invalid dataSet ID");
    } else if (dataSetID.empty()) {
        WRITE_WARNING("Invalid empty dataSet ID");
    } else if (myGenericDataFrameParent->getViewNet()->getNet()->retrieveDataSet(dataSetID, false) != nullptr) {
        WRITE_WARNING("Invalid duplicated dataSet ID");
    } else {
        // build data set
        const GNEDataSet* dataSet = GNEDataHandler::buildDataSet(myGenericDataFrameParent->getViewNet()->getNet(), true, dataSetID);
        // refresh tag selector
        refreshDataSetSelector(dataSet);
        // change check button
        myNewDataSetCheckButton->setCheck(FALSE, TRUE);
    }
    return 1;
}


long
GNEGenericDataFrame::DataSetSelector::onCmdSetNewDataSetID(FXObject*, FXSelector, void*) {
    //
    return 1;
}


long
GNEGenericDataFrame::DataSetSelector::onCmdSelectDataSet(FXObject*, FXSelector, void*) {
    // update interval modul
    myGenericDataFrameParent->myIntervalSelector->refreshIntervalSelector();
    return 1;
}


long
GNEGenericDataFrame::DataSetSelector::onCmdSelectCheckButton(FXObject*, FXSelector, void*) {
    if (myNewDataSetCheckButton->getCheck() == TRUE) {
        // enable textfield and label
        myHorizontalFrameNewID->show();
        myCreateDataSetButton->show();
        // disable comboBox
        myDataSetsComboBox->hide();
    } else {
        // disable textfield and label
        myHorizontalFrameNewID->hide();
        myCreateDataSetButton->hide();
        // enable comboBox
        myDataSetsComboBox->show();
    }
    // update interval modul
    myGenericDataFrameParent->myIntervalSelector->refreshIntervalSelector();
    return 1;
}

// ---------------------------------------------------------------------------
// GNEGenericDataFrame::IntervalSelector - methods
// ---------------------------------------------------------------------------

GNEGenericDataFrame::IntervalSelector::IntervalSelector(GNEGenericDataFrame* genericDataFrameParent) :
    FXGroupBox(genericDataFrameParent->myContentFrame, "Interval", GUIDesignGroupBoxFrame),
    myGenericDataFrameParent(genericDataFrameParent) {
    // create check button for new interval
    myNewIntervalCheckButton = new FXCheckButton(this, "Create new interval", this, MID_GNE_SELECT, GUIDesignCheckButton);
    // create begin label
    myHorizontalFrameBegin = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myHorizontalFrameBegin, toString(SUMO_ATTR_BEGIN).c_str(), nullptr, GUIDesignLabelAttribute);
    // create begin TextField
    myBeginTextField = new FXTextField(myHorizontalFrameBegin, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myBeginTextField->setText("0");
    // hide horizontal frame begin
    myHorizontalFrameBegin->hide();
    // create end label
    myHorizontalFrameEnd = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(myHorizontalFrameEnd, toString(SUMO_ATTR_END).c_str(), nullptr, GUIDesignLabelAttribute);
    // create end textfield
    myEndTextField = new FXTextField(myHorizontalFrameEnd, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myEndTextField->setText("3600");
    // hide horizontal frame end
    myHorizontalFrameEnd->hide();
    // create interval button
    myCreateIntervalButton = new FXButton(this, "create interval", GUIIconSubSys::getIcon(GUIIcon::DATAINTERVAL), this, MID_GNE_CREATE, GUIDesignButton);
    myCreateIntervalButton->hide();
    // Create three list
    myIntervalsTreelist = new FXTreeList(this, this, MID_GNE_DATAINTERVAL_SELECTED, GUIDesignTreeListFrame);
    // refresh interval selector
    refreshIntervalSelector();
    // IntervalSelector is always shown
    show();
}


GNEGenericDataFrame::IntervalSelector::~IntervalSelector() {}


void
GNEGenericDataFrame::IntervalSelector::refreshIntervalSelector() {
    // first clear items from tree and intervalMap
    myIntervalsTreelist->clearItems();
    myTreeItemIntervalMap.clear();
    // obtain data set
    const GNEDataSet* dataSet = myGenericDataFrameParent->myDataSetSelector->getDataSet();
    // add intervals
    if (dataSet) {
        // insert dataSetItem in Tree list
        FXTreeItem* dataSetItem = myIntervalsTreelist->insertItem(
                                      nullptr, nullptr,
                                      dataSet->getHierarchyName().c_str(),
                                      dataSet->getIcon(),
                                      dataSet->getIcon());
        // by default item is expanded
        dataSetItem->setExpanded(true);
        // iterate over intevals
        for (const auto& interval : dataSet->getDataIntervalChildren()) {
            addIntervalItem(interval.second, dataSetItem);
        }
    }
    // refresh attribute selector
    if (myGenericDataFrameParent->myAttributeSelector) {
        myGenericDataFrameParent->myAttributeSelector->refreshAttributeSelector();
    }
    // recalc frame
    recalc();
}


GNEDataInterval*
GNEGenericDataFrame::IntervalSelector::getDataInterval() const {
    // first check if there is elements in interval tree
    if (myIntervalsTreelist->getNumItems() > 0) {
        for (const auto& treeItem : myTreeItemIntervalMap) {
            if (treeItem.first->isSelected()) {
                return treeItem.second;
            }
        }
    }
    // no GNEDataInterval found, then return nullptr
    return nullptr;
}


long
GNEGenericDataFrame::IntervalSelector::onCmdCreateInterval(FXObject*, FXSelector, void*) {
    // first check that begin and end are valid
    if (GNEAttributeCarrier::canParse<double>(myBeginTextField->getText().text()) &&
            GNEAttributeCarrier::canParse<double>(myEndTextField->getText().text())) {
        // obtain begin and end
        const double begin = GNEAttributeCarrier::parse<double>(myBeginTextField->getText().text());
        const double end = GNEAttributeCarrier::parse<double>(myEndTextField->getText().text());
        // get data set parent
        GNEDataSet* dataSet = myGenericDataFrameParent->myDataSetSelector->getDataSet();
        if (dataSet && dataSet->checkNewInterval(begin, end)) {
            GNEDataHandler::buildDataInterval(myGenericDataFrameParent->getViewNet()->getNet(), true, dataSet, begin, end);
        }
        // disable select interval check button
        myNewIntervalCheckButton->setCheck(FALSE, TRUE);
    }
    return 1;
}


long
GNEGenericDataFrame::IntervalSelector::onCmdSelectInterval(FXObject*, FXSelector, void*) {
    // refresh attribute selector
    myGenericDataFrameParent->myAttributeSelector->refreshAttributeSelector();
    return 1;
}


long
GNEGenericDataFrame::IntervalSelector::onCmdSetIntervalAttribute(FXObject* obj, FXSelector, void*) {
    if (obj == myBeginTextField) {
        // check if begin value can be parsed to double
        if (GNEAttributeCarrier::canParse<double>(myBeginTextField->getText().text())) {
            myBeginTextField->setTextColor(FXRGB(0, 0, 0));
            myBeginTextField->killFocus();
        } else {
            myBeginTextField->setTextColor(FXRGB(255, 0, 0));
        }
    } else if (obj == myEndTextField) {
        // check if end value can be parsed to double
        if (GNEAttributeCarrier::canParse<double>(myEndTextField->getText().text())) {
            myEndTextField->setTextColor(FXRGB(0, 0, 0));
            myEndTextField->killFocus();
        } else {
            myEndTextField->setTextColor(FXRGB(255, 0, 0));
        }
    }
    return 1;
}


long
GNEGenericDataFrame::IntervalSelector::onCmdSelectCheckButton(FXObject*, FXSelector, void*) {
    if (myNewIntervalCheckButton->getCheck() == TRUE) {
        // enable begin and end elements
        myHorizontalFrameBegin->show();
        myHorizontalFrameEnd->show();
        myCreateIntervalButton->show();
        // refresh begin and end text fields
        const GNEDataSet* dataSet = myGenericDataFrameParent->myDataSetSelector->getDataSet();
        if (dataSet) {
            if (dataSet->getDataIntervalChildren().empty()) {
                // set default interval (1 hour)
                myBeginTextField->setText("0");
                myEndTextField->setText("3600");
            } else {
                // obtain last data interval
                const GNEDataInterval* lastDataInterval = dataSet->getDataIntervalChildren().rbegin()->second;
                const double intervalDuration = lastDataInterval->getAttributeDouble(SUMO_ATTR_END) - lastDataInterval->getAttributeDouble(SUMO_ATTR_BEGIN);
                // set new begin end
                myBeginTextField->setText(toString(lastDataInterval->getAttributeDouble(SUMO_ATTR_END)).c_str());
                myEndTextField->setText(toString(lastDataInterval->getAttributeDouble(SUMO_ATTR_END) + intervalDuration).c_str());
            }
        }
    } else {
        // disable begin and end elements
        myHorizontalFrameBegin->hide();
        myHorizontalFrameEnd->hide();
        myCreateIntervalButton->hide();
    }
    // refresh interval seletor
    refreshIntervalSelector();
    return 1;
}


FXTreeItem*
GNEGenericDataFrame::IntervalSelector::addIntervalItem(GNEDataInterval* dataInterval, FXTreeItem* itemParent) {
    // insert item in Tree list
    FXTreeItem* item = myIntervalsTreelist->insertItem(nullptr, itemParent,
                       dataInterval->getHierarchyName().c_str(),
                       dataInterval->getIcon(),
                       dataInterval->getIcon());
    // insert item in map
    myTreeItemIntervalMap[item] = dataInterval;
    // by default item is expanded
    item->setExpanded(true);
    // select first item
    if (myTreeItemIntervalMap.size() == 1) {
        item->setSelected(TRUE);
    }
    // return created FXTreeItem
    return item;
}

// ---------------------------------------------------------------------------
// GNEGenericDataFrame::AttributeSelector - methods
// ---------------------------------------------------------------------------

GNEGenericDataFrame::AttributeSelector::AttributeSelector(GNEGenericDataFrame* genericDataFrameParent, SumoXMLTag tag) :
    FXGroupBox(genericDataFrameParent->myContentFrame, "Data attributes", GUIDesignGroupBoxFrame),
    myGenericDataFrameParent(genericDataFrameParent),
    myMinMaxLabel(nullptr),
    myGenericDataTag(tag) {
    // Create FXComboBox
    myAttributesComboBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SELECT, GUIDesignComboBox);
    // build rainbow
    myMinMaxLabel = GNEFrameModuls::buildRainbow(this);
    // refresh interval selector
    refreshAttributeSelector();
    // AttributeSelector is always shown
    show();
}


GNEGenericDataFrame::AttributeSelector::~AttributeSelector() {}


void
GNEGenericDataFrame::AttributeSelector::refreshAttributeSelector() {
    // first clear items
    myAttributesComboBox->clearItems();
    // restore myMinMaxLabel
    myMinMaxLabel->setText("Scale: Min -> Max");
    // fill myAttributesComboBox depending of data sets
    if (myGenericDataFrameParent->myDataSetSelector->getDataSet() == nullptr) {
        myAttributesComboBox->appendItem("<no dataSet selected>");
        myAttributesComboBox->disable();
    } else {
        // add all item
        myAttributesComboBox->appendItem("<all>");
        // add attributes depending of interval
        if (myGenericDataFrameParent->myIntervalSelector->getDataInterval() == nullptr) {
            const auto parameters = myGenericDataFrameParent->getViewNet()->getNet()->retrieveGenericDataParameters(
                                        myGenericDataFrameParent->myDataSetSelector->getDataSet()->getID(), toString(myGenericDataTag), "", "");
            // add all parameters
            for (const auto& attribute : parameters) {
                myAttributesComboBox->appendItem(attribute.c_str());
            }
        } else {
            // retrieve all parameters within begin and end
            const auto parameters = myGenericDataFrameParent->getViewNet()->getNet()->retrieveGenericDataParameters(
                                        myGenericDataFrameParent->myDataSetSelector->getDataSet()->getID(), toString(myGenericDataTag),
                                        myGenericDataFrameParent->myIntervalSelector->getDataInterval()->getAttribute(SUMO_ATTR_BEGIN),
                                        myGenericDataFrameParent->myIntervalSelector->getDataInterval()->getAttribute(SUMO_ATTR_END));
            // add all parameters
            for (const auto& attribute : parameters) {
                myAttributesComboBox->appendItem(attribute.c_str());
            }
        }
        // enable combo Box
        myAttributesComboBox->enable();
        // adjust visible items
        if (myAttributesComboBox->getNumItems() < 10) {
            myAttributesComboBox->setNumVisible(myAttributesComboBox->getNumItems());
        } else {
            myAttributesComboBox->setNumVisible(10);
        }
    }
    // recalc frame
    recalc();
    // update view net
    myGenericDataFrameParent->getViewNet()->updateViewNet();
}


std::string
GNEGenericDataFrame::AttributeSelector::getFilteredAttribute() const {
    if (myAttributesComboBox->getNumItems() == 0) {
        return "";
    } else if (myAttributesComboBox->getText() == "<all>") {
        return "";
    } else {
        return myAttributesComboBox->getText().text();
    }
}


const RGBColor&
GNEGenericDataFrame::AttributeSelector::getScaledColor(const double min, const double max, const double value) const {
    // update myMinMaxLabel
    myMinMaxLabel->setText(("Min: " + toString(min) + " -> Max: " + toString(max)).c_str());
    // return scaled color
    return GNEViewNetHelper::getRainbowScaledColor(min, max, value);
}


long
GNEGenericDataFrame::AttributeSelector::onCmdSelectAttribute(FXObject*, FXSelector, void*) {
    // empty attribute means <all>
    if (myAttributesComboBox->getText().empty()) {
        myAttributesComboBox->setText("<all>");
    }
    if (myAttributesComboBox->getText() == "<all>") {
        myMinMaxLabel->setText("Scale: Min -> Max");
    }
    // update view
    myGenericDataFrameParent->getViewNet()->updateViewNet();
    return 1;
}

// ---------------------------------------------------------------------------
// GNEGenericDataFrame - methods
// ---------------------------------------------------------------------------

const GNEGenericDataFrame::DataSetSelector*
GNEGenericDataFrame::getDataSetSelector() const {
    return myDataSetSelector;
}


const GNEGenericDataFrame::IntervalSelector*
GNEGenericDataFrame::getIntervalSelector() const {
    return myIntervalSelector;
}


const GNEGenericDataFrame::AttributeSelector*
GNEGenericDataFrame::getAttributeSelector() const {
    return myAttributeSelector;
}


GNEFrameModuls::PathCreator*
GNEGenericDataFrame::getPathCreator() const {
    return myPathCreator;
}


SumoXMLTag
GNEGenericDataFrame::getTag() const {
    return myGenericDataTag;
}


void
GNEGenericDataFrame::show() {
    // first refresh data set selector
    myDataSetSelector->refreshDataSetSelector(nullptr);
    // check if there is an edge path creator
    if (myPathCreator) {
        myPathCreator->showPathCreatorModul(myGenericDataTag, false, false);
    }
    // show frame
    GNEFrame::show();
}


void
GNEGenericDataFrame::hide() {
    if (myPathCreator) {
        // reset candidate edges
        for (const auto& edge : myViewNet->getNet()->getAttributeCarriers()->getEdges()) {
            edge.second->resetCandidateFlags();
        }
    }
    // hide frame
    GNEFrame::hide();
}


GNEGenericDataFrame::GNEGenericDataFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet, SumoXMLTag tag, const bool pathCreator) :
    GNEFrame(horizontalFrameParent, viewNet, toString(tag)),
    myDataSetSelector(nullptr),
    myIntervalSelector(nullptr),
    myAttributeSelector(nullptr),
    myParametersEditor(nullptr),
    myPathCreator(nullptr),
    myGenericDataTag(tag) {
    // create DataSetSelector
    myDataSetSelector = new DataSetSelector(this);
    // create IntervalSelector modul
    myIntervalSelector = new IntervalSelector(this);
    // create AttributeSelector modul
    myAttributeSelector = new AttributeSelector(this, tag);
    // create parameter editor modul
    myParametersEditor = new GNEFrameAttributesModuls::ParametersEditor(this, "Attributes");
    // create PathCreator modul
    if (pathCreator) {
        myPathCreator = new GNEFrameModuls::PathCreator(this);
    }
}


GNEGenericDataFrame::~GNEGenericDataFrame() {}


void
GNEGenericDataFrame::intervalSelected() {
    //
}


void
GNEGenericDataFrame::createPath() {
    // this function has to be reimplemente in all child frames that uses a PathCreator
}

/****************************************************************************/
