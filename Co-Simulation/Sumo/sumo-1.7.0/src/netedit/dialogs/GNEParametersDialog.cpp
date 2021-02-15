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
/// @file    GNEParametersDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2018
///
// Dialog for edit parameters
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/xml/XMLSubSys.h>
#include <netedit/GNEViewNet.h>
#include <netedit/frames/GNEFrame.h>

#include "GNEParametersDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEParametersDialog) GNEParametersDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_ACCEPT,  GNEParametersDialog::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_CANCEL,  GNEParametersDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_RESET,   GNEParametersDialog::onCmdReset),
    FXMAPFUNC(SEL_CHORE,    FXDialogBox::ID_CANCEL, GNEParametersDialog::onCmdCancel),
    FXMAPFUNC(SEL_TIMEOUT,  FXDialogBox::ID_CANCEL, GNEParametersDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,  FXDialogBox::ID_CANCEL, GNEParametersDialog::onCmdCancel),
    FXMAPFUNC(SEL_CLOSE,    0,                      GNEParametersDialog::onCmdCancel),
};

FXDEFMAP(GNEParametersDialog::ParametersValues) ParametersValuesMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,      GNEParametersDialog::ParametersValues::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_REMOVE_ATTRIBUTE,   GNEParametersDialog::ParametersValues::onCmdButtonPress),
    FXMAPFUNC(SEL_PAINT,    0,                          GNEParametersDialog::ParametersValues::onPaint),
};

FXDEFMAP(GNEParametersDialog::ParametersOptions) ParametersOptionsMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_LOAD,    GNEParametersDialog::ParametersOptions::onCmdLoadParameters),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_SAVE,    GNEParametersDialog::ParametersOptions::onCmdSaveParameters),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_CLEAR,   GNEParametersDialog::ParametersOptions::onCmdClearParameters),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_BUTTON_SORT,    GNEParametersDialog::ParametersOptions::onCmdSortParameters),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,               GNEParametersDialog::ParametersOptions::onCmdHelpParameter),
};

// Object implementation
FXIMPLEMENT(GNEParametersDialog,                    FXDialogBox,   GNEParametersDialogMap,     ARRAYNUMBER(GNEParametersDialogMap))
FXIMPLEMENT(GNEParametersDialog::ParametersValues,  FXGroupBox,    ParametersValuesMap,        ARRAYNUMBER(ParametersValuesMap))
FXIMPLEMENT(GNEParametersDialog::ParametersOptions, FXGroupBox,    ParametersOptionsMap,       ARRAYNUMBER(ParametersOptionsMap))

// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEParametersDialog::ParametersValues - methods
// ---------------------------------------------------------------------------

GNEParametersDialog::ParametersValues::ParametersValues(FXHorizontalFrame* frame, GNEParametersDialog* ParameterDialogParent) :
    FXGroupBox(frame, " Parameters", GUIDesignGroupBoxFrameFill),
    myParameterDialogParent(ParameterDialogParent) {
    // create labels for keys and values
    FXHorizontalFrame* horizontalFrameLabels = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myKeyLabel = new FXLabel(horizontalFrameLabels, "key", nullptr, GUIDesignLabelThick100);
    new FXLabel(horizontalFrameLabels, "value", nullptr, GUIDesignLabelCenterThick);
    // create scroll windows
    FXScrollWindow* scrollWindow = new FXScrollWindow(this, LAYOUT_FILL);
    // create vertical frame for rows
    myVerticalFrameRow = new FXVerticalFrame(scrollWindow, GUIDesignAuxiliarFrame);
    // update values
    updateValues();
}


GNEParametersDialog::ParametersValues::~ParametersValues() {}


void
GNEParametersDialog::ParametersValues::updateValues() {
    // first show the correct number of rows
    while ((myParameterDialogParent->myEditedParameters.size() + 1) < myParameterRows.size()) {
        delete myParameterRows.back();
        myParameterRows.pop_back();
    }
    while ((myParameterDialogParent->myEditedParameters.size() + 1) > myParameterRows.size()) {
        myParameterRows.push_back(new ParameterRow(this, myVerticalFrameRow));
    }
    // fill rows
    for (int i = 0; i < (int)myParameterDialogParent->myEditedParameters.size(); i++) {
        myParameterRows.at(i)->enableRow(myParameterDialogParent->myEditedParameters.at(i).first, myParameterDialogParent->myEditedParameters.at(i).second);
    }
    // set last myParameterRows with the add button
    myParameterRows.back()->toogleAddButton();
}


void
GNEParametersDialog::ParametersValues::setParameters(const std::vector<std::pair<std::string, std::string> >& newParameters) {
    myParameterDialogParent->myEditedParameters = newParameters;
    // update values
    updateValues();
}


void
GNEParametersDialog::ParametersValues::addParameter(std::pair<std::string, std::string> newParameter) {
    myParameterDialogParent->myEditedParameters.push_back(newParameter);
    // update values
    updateValues();
}


void
GNEParametersDialog::ParametersValues::clearParameters() {
    myParameterDialogParent->myEditedParameters.clear();
    // update values
    updateValues();
}


long
GNEParametersDialog::ParametersValues::onPaint(FXObject* o, FXSelector f, void* p) {
    // size of key label has to be updated in every interation
    if (myParameterRows.size() > 0) {
        myKeyLabel->setWidth(myParameterRows.front()->keyField->getWidth());
    }
    return FXGroupBox::onPaint(o, f, p);
}


long
GNEParametersDialog::ParametersValues::onCmdSetAttribute(FXObject* obj, FXSelector, void*) {
    // find what value was changed
    for (int i = 0; i < (int)myParameterRows.size(); i++) {
        if (myParameterRows.at(i)->keyField == obj) {
            // change key of Parameter
            myParameterDialogParent->myEditedParameters.at(i).first = myParameterRows.at(i)->keyField->getText().text();
            // change color of text field depending if key is valid or empty
            if (myParameterDialogParent->myEditedParameters.at(i).first.empty() || SUMOXMLDefinitions::isValidParameterKey(myParameterDialogParent->myEditedParameters.at(i).first)) {
                myParameterRows.at(i)->keyField->setTextColor(FXRGB(0, 0, 0));
            } else {
                myParameterRows.at(i)->keyField->setTextColor(FXRGB(255, 0, 0));
                myParameterRows.at(i)->keyField->killFocus();
            }
        } else if (myParameterRows.at(i)->valueField == obj) {
            // change value of Parameter
            myParameterDialogParent->myEditedParameters.at(i).second = myParameterRows.at(i)->valueField->getText().text();
        }
    }
    return 1;
}


long
GNEParametersDialog::ParametersValues::onCmdButtonPress(FXObject* obj, FXSelector, void*) {
    // first check if add button was pressed
    if (myParameterRows.back()->button == obj) {
        // create new parameter
        myParameterDialogParent->myEditedParameters.push_back(std::make_pair("", ""));
        // update values and finish
        updateValues();
        return 1;
    } else {
        // in other case, button press was a "remove button". Find id and remove the Parameter
        for (int i = 0;  i < (int)myParameterRows.size(); i++) {
            if (myParameterRows.at(i)->button == obj && i < (int)myParameterDialogParent->myEditedParameters.size()) {
                // remove parameter
                myParameterDialogParent->myEditedParameters.erase(myParameterDialogParent->myEditedParameters.begin() + i);
                // update values and finish
                updateValues();
                return 1;
            }
        }
    }
    // Nothing to do
    return 1;
}


GNEParametersDialog::ParametersValues::ParameterRow::ParameterRow(ParametersValues* ParametersValues, FXVerticalFrame* verticalFrameParent) {
    horizontalFrame = new FXHorizontalFrame(verticalFrameParent, GUIDesignAuxiliarHorizontalFrame);
    keyField = new FXTextField(horizontalFrame, GUIDesignTextFieldNCol, ParametersValues, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    valueField = new FXTextField(horizontalFrame, GUIDesignTextFieldNCol, ParametersValues, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    button = new FXButton(horizontalFrame, "", GUIIconSubSys::getIcon(GUIIcon::REMOVE), ParametersValues, MID_GNE_REMOVE_ATTRIBUTE, GUIDesignButtonIcon);
    // only create elements if vertical frame was previously created
    if (verticalFrameParent->id()) {
        horizontalFrame->create();
    }
    // by defaults rows are disabled
    disableRow();
}


GNEParametersDialog::ParametersValues::ParameterRow::~ParameterRow() {
    // simply delete horizontalFrame (rest of elements will be automatic deleted due they are children of horizontal frame)
    delete horizontalFrame;
}


void
GNEParametersDialog::ParametersValues::ParameterRow::disableRow() {
    // hide all
    keyField->setText("");
    keyField->disable();
    valueField->setText("");
    valueField->disable();
    button->disable();
    button->setIcon(GUIIconSubSys::getIcon(GUIIcon::REMOVE));
}


void
GNEParametersDialog::ParametersValues::ParameterRow::enableRow(const std::string& parameter, const std::string& value) const {
    // restore color and enable key field
    keyField->setText(parameter.c_str());
    if (parameter.empty() || SUMOXMLDefinitions::isValidParameterKey(parameter)) {
        keyField->setTextColor(FXRGB(0, 0, 0));
    } else {
        keyField->setTextColor(FXRGB(255, 0, 0));
    }
    keyField->enable();
    // restore color and enable value field
    valueField->setText(value.c_str());
    valueField->enable();
    // enable button and set icon remove
    button->enable();
    button->setIcon(GUIIconSubSys::getIcon(GUIIcon::REMOVE));
}


void
GNEParametersDialog::ParametersValues::ParameterRow::toogleAddButton() {
    // clear and disable parameter and value fields
    keyField->setText("");
    keyField->disable();
    valueField->setText("");
    valueField->disable();
    // enable remove button and set "add" icon and focus
    button->enable();
    button->setIcon(GUIIconSubSys::getIcon(GUIIcon::ADD));
    button->setFocus();
}


bool
GNEParametersDialog::ParametersValues::ParameterRow::isButtonInAddMode() const {
    return (button->getIcon() == GUIIconSubSys::getIcon(GUIIcon::ADD));
}


void
GNEParametersDialog::ParametersValues::ParameterRow::copyValues(const ParameterRow& other) {
    keyField->setText(other.keyField->getText());
    valueField->setText(other.valueField->getText());
}

// ---------------------------------------------------------------------------
// GNEParametersDialog::ParametersOptions - methods
// ---------------------------------------------------------------------------

GNEParametersDialog::ParametersOptions::ParametersOptions(FXHorizontalFrame* frame, GNEParametersDialog* ParameterDialogParent) :
    FXGroupBox(frame, "Options", GUIDesignGroupBoxFrame100),
    myParameterDialogParent(ParameterDialogParent) {
    // create buttons
    mySortButton =  new FXButton(this, "Sort",  GUIIconSubSys::getIcon(GUIIcon::RELOAD), this, MID_GNE_BUTTON_SORT, GUIDesignButtonRectangular100x23);
    myClearButton = new FXButton(this, "Clear", GUIIconSubSys::getIcon(GUIIcon::CLEANJUNCTIONS), this, MID_GNE_BUTTON_CLEAR, GUIDesignButtonRectangular100x23);
    myLoadButton =  new FXButton(this, "Load",  GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, MID_GNE_BUTTON_LOAD, GUIDesignButtonRectangular100x23);
    mySaveButton =  new FXButton(this, "Save",  GUIIconSubSys::getIcon(GUIIcon::SAVE), this, MID_GNE_BUTTON_SAVE, GUIDesignButtonRectangular100x23);
    myHelpButton =  new FXButton(this, "Help",  GUIIconSubSys::getIcon(GUIIcon::HELP), this, MID_HELP, GUIDesignButtonRectangular100x23);
}


GNEParametersDialog::ParametersOptions::~ParametersOptions() {}


long
GNEParametersDialog::ParametersOptions::onCmdLoadParameters(FXObject*, FXSelector, void*) {
    // get the Additional file name
    FXFileDialog opendialog(this, "Open Parameter Template");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::GREENVEHICLE));
    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList(" Parameter Template files (*.xml)\nAll files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        std::string file = opendialog.getFilename().text();
        // save current number of parameters
        int numberOfParametersbeforeLoad = (int)myParameterDialogParent->myEditedParameters.size();
        // Create additional handler and run parser
        GNEParameterHandler handler(this, file);
        if (!XMLSubSys::runParser(handler, file, false)) {
            WRITE_MESSAGE("Loading of Parameters From " + file + " failed.");
        }
        // show loaded attributes
        WRITE_MESSAGE("Loaded " + toString((int)myParameterDialogParent->myEditedParameters.size() - numberOfParametersbeforeLoad) + " Parameters.");
        // update values
        myParameterDialogParent->myParametersValues->updateValues();
    }
    return 1;
}


long
GNEParametersDialog::ParametersOptions::onCmdSaveParameters(FXObject*, FXSelector, void*) {
    // obtain file to save parameters
    FXString file = MFXUtils::getFilename2Write(this,
                    "Select name of the Parameter Template file", ".xml",
                    GUIIconSubSys::getIcon(GUIIcon::GREENVEHICLE),
                    gCurrentFolder);
    if (file == "") {
        // None parameter file was selected, then stop function
        return 1;
    } else {
        OutputDevice& device = OutputDevice::getDevice(file.text());
        device.writeXMLHeader("Parameter", "parameter_file.xsd");
        // iterate over all parameters and save it in the filename
        for (auto i = myParameterDialogParent->myEditedParameters.begin(); i != myParameterDialogParent->myEditedParameters.end(); i++) {
            device.openTag(SUMO_TAG_PARAM);
            device.writeAttr(SUMO_ATTR_KEY, i->first);
            device.writeAttr(SUMO_ATTR_VALUE, i->second);
            device.closeTag();
        }
        device.close();
    }
    return 1;
}


long
GNEParametersDialog::ParametersOptions::onCmdClearParameters(FXObject*, FXSelector, void*) {
    // simply clear parameters from ParametersValues
    myParameterDialogParent->myParametersValues->clearParameters();
    return 1;
}


long
GNEParametersDialog::ParametersOptions::onCmdSortParameters(FXObject*, FXSelector, void*) {
    std::vector<std::pair<std::string, std::string> > ParametersNoEmpty;
    std::vector<std::string> valuesEmpty;
    // first extract empty values
    for (auto i = myParameterDialogParent->myEditedParameters.begin(); i != myParameterDialogParent->myEditedParameters.end(); i++) {
        if (!i->first.empty()) {
            ParametersNoEmpty.push_back(*i);
        } else if (i->first.empty() && !i->second.empty()) {
            valuesEmpty.push_back(i->second);
        }
    }
    // now sort non-empty parameters
    std::sort(ParametersNoEmpty.begin(), ParametersNoEmpty.end());
    // add values without key
    for (auto i : valuesEmpty) {
        ParametersNoEmpty.push_back(std::make_pair("", i));
    }
    // fill ParametersNoEmpty with empty values
    while (ParametersNoEmpty.size() < myParameterDialogParent->myEditedParameters.size()) {
        ParametersNoEmpty.push_back(std::make_pair("", ""));
    }
    // finally replace parameters in myParametersValues with ParametersNoEmpty
    myParameterDialogParent->myParametersValues->setParameters(ParametersNoEmpty);
    // update values
    myParameterDialogParent->myParametersValues->updateValues();
    return 1;
}


long
GNEParametersDialog::ParametersOptions::onCmdHelpParameter(FXObject*, FXSelector, void*) {
    // Create dialog box
    FXDialogBox* ParameterHelpDialog = new FXDialogBox(this, " Parameters Help", GUIDesignDialogBox);
    ParameterHelpDialog->setIcon(GUIIconSubSys::getIcon(GUIIcon::APP_TABLE));
    // set help text
    std::ostringstream help;
    help
            << "- Parameters are defined by a Key and a Value.\n"
            << "- In Netedit can be defined using format key1=parameter1|key2=parameter2|...\n"
            << " - Duplicated and empty Keys aren't valid.\n"
            << " - Certain characters aren't allowed (\t\n\r@$%^&/|\\....)\n";
    // Create label with the help text
    new FXLabel(ParameterHelpDialog, help.str().c_str(), nullptr, GUIDesignLabelFrameInformation);
    // Create horizontal separator
    new FXHorizontalSeparator(ParameterHelpDialog, GUIDesignHorizontalSeparator);
    // Create frame for OK Button
    FXHorizontalFrame* myHorizontalFrameOKButton = new FXHorizontalFrame(ParameterHelpDialog, GUIDesignAuxiliarHorizontalFrame);
    // Create Button Close (And two more horizontal frames to center it)
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    new FXButton(myHorizontalFrameOKButton, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), ParameterHelpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Opening Parameter help dialog");
    // create Dialog
    ParameterHelpDialog->create();
    // show in the given position
    ParameterHelpDialog->show(PLACEMENT_CURSOR);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    getApp()->runModalFor(ParameterHelpDialog);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Closing Parameter help dialog");
    return 1;
}


GNEParametersDialog::ParametersOptions::GNEParameterHandler::GNEParameterHandler(ParametersOptions* ParametersOptionsParent, const std::string& file) :
    SUMOSAXHandler(file),
    myParametersOptionsParent(ParametersOptionsParent) {
}


GNEParametersDialog::ParametersOptions::GNEParameterHandler::~GNEParameterHandler() {}


void
GNEParametersDialog::ParametersOptions::GNEParameterHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    // Obtain tag of element
    SumoXMLTag tag = static_cast<SumoXMLTag>(element);
    // only continue if tag is valid
    if (tag != SUMO_TAG_NOTHING) {
        // Call parse and build depending of tag
        switch (tag) {
            case SUMO_TAG_PARAM:
                // Check that format of Parameter is correct
                if (!attrs.hasAttribute(SUMO_ATTR_KEY)) {
                    WRITE_WARNING("Key of Parameter not defined");
                } else if (!attrs.hasAttribute(SUMO_ATTR_VALUE)) {
                    WRITE_WARNING("Value of Parameter not defined");
                } else {
                    // obtain Key and value
                    std::string key = attrs.getString(SUMO_ATTR_KEY);
                    std::string value = attrs.getString(SUMO_ATTR_VALUE);
                    // check that parsed values are correct
                    if (!SUMOXMLDefinitions::isValidParameterKey(key)) {
                        if (key.size() == 0) {
                            WRITE_WARNING("Key of Parameter cannot be empty");
                        } else {
                            WRITE_WARNING("Key '" + key + "' of Parameter contains invalid characters");
                        }
                    } else {
                        // add parameter to vector of myParameterDialogParent
                        myParametersOptionsParent->myParameterDialogParent->myParametersValues->addParameter(std::make_pair(key, value));
                    }
                }
                break;
            default:
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// GNEParametersDialog - methods
// ---------------------------------------------------------------------------

GNEParametersDialog::GNEParametersDialog(GNEFrameAttributesModuls::ParametersEditor* ParametersEditor) :
    FXDialogBox(ParametersEditor->getFrameParent()->getViewNet()->getApp(), "Edit parameters", GUIDesignDialogBoxExplicitStretchable(400, 300)),
    myParametersEditor(ParametersEditor),
    myEditedParameters(ParametersEditor->getParametersVectorStr()),
    myCopyOfParameters(ParametersEditor->getParametersVectorStr()) {
    // set vehicle icon for this dialog
    setIcon(GUIIconSubSys::getIcon(GUIIcon::APP_TABLE));
    // create main frame
    FXVerticalFrame* mainFrame = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);
    // create frame for Parameters and options
    FXHorizontalFrame* horizontalFrameParametersAndOptions = new FXHorizontalFrame(mainFrame, GUIDesignAuxiliarFrame);
    // create parameters values
    myParametersValues = new ParametersValues(horizontalFrameParametersAndOptions, this);
    // create parameters options
    myParametersOptions = new ParametersOptions(horizontalFrameParametersAndOptions, this);
    // add separator
    new FXHorizontalSeparator(mainFrame, GUIDesignHorizontalSeparator);
    // create dialog buttons bot centered
    FXHorizontalFrame* buttonsFrame = new FXHorizontalFrame(mainFrame, GUIDesignHorizontalFrame);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
    myAcceptButton = new FXButton(buttonsFrame, "accept\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), this, MID_GNE_BUTTON_ACCEPT, GUIDesignButtonAccept);
    myCancelButton = new FXButton(buttonsFrame, "cancel\t\tclose", GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, MID_GNE_BUTTON_CANCEL, GUIDesignButtonCancel);
    myResetButton = new FXButton(buttonsFrame,  "reset\t\tclose",  GUIIconSubSys::getIcon(GUIIcon::RESET), this, MID_GNE_BUTTON_RESET,  GUIDesignButtonReset);
    new FXHorizontalFrame(buttonsFrame, GUIDesignAuxiliarHorizontalFrame);
}


GNEParametersDialog::~GNEParametersDialog() {}


long
GNEParametersDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    // check if all edited parameters are valid
    for (const auto& parameter : myEditedParameters) {
        if (parameter.first.empty()) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
            // open warning Box
            FXMessageBox::warning(getApp(), MBOX_OK, "Empty Parameter key", "%s", "Parameters with empty keys aren't allowed");
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
            return 1;
        } else if (!SUMOXMLDefinitions::isValidParameterKey(parameter.first)) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
            // open warning Box
            FXMessageBox::warning(getApp(), MBOX_OK, "Invalid Parameter key", "%s", "There are keys with invalid characters");
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
            return 1;
        } else if ((myParametersEditor->getAttrType() == Parameterised::ParameterisedAttrType::DOUBLE) && !GNEAttributeCarrier::canParse<double>(parameter.second)) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
            // open warning Box
            FXMessageBox::warning(getApp(), MBOX_OK, "Invalid double Parameter value", "%s", "There are values that cannot be parsed to floats");
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
            return 1;
        }
    }
    // now check if there is duplicates parameters
    std::vector<std::pair<std::string, std::string> > sortedParameters = myEditedParameters;
    std::sort(sortedParameters.begin(), sortedParameters.end());
    for (auto i = sortedParameters.begin(); i != sortedParameters.end(); i++) {
        if (((i + 1) != sortedParameters.end()) && (i->first) == (i + 1)->first) {
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
            // open warning Box
            FXMessageBox::warning(getApp(), MBOX_OK, "Duplicated Parameters", "%s", "Parameters with the same Key aren't allowed");
            // write warning if netedit is running in testing mode
            WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
            return 1;
        }
    }
    // set parameters in Parameters editor parents
    myParametersEditor->setParameters(myEditedParameters);
    // all ok, then close dialog
    getApp()->stopModal(this, TRUE);
    return 1;
}


long
GNEParametersDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // restore copy of parameters
    myParametersValues->setParameters(myCopyOfParameters);
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNEParametersDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // simply restore copy of parameters and continue editing
    myParametersValues->setParameters(myCopyOfParameters);
    return 1;
}


/****************************************************************************/
