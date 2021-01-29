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
/// @file    GNEInspectorFrame.cpp
/// @author  Jakob Erdmann
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2011
///
// The Widget for modifying network-element attributes (i.e. lane speed)
/****************************************************************************/
#include <config.h>

#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewParent.h>
#include <netedit/frames/common/GNESelectorFrame.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNELane.h>

#include "GNEInspectorFrame.h"
#include "GNEDeleteFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEInspectorFrame) GNEInspectorFrameMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_INSPECTORFRAME_GOBACK,  GNEInspectorFrame::onCmdGoBack)
};

FXDEFMAP(GNEInspectorFrame::NeteditAttributesEditor) NeteditAttributesEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,      GNEInspectorFrame::NeteditAttributesEditor::onCmdSetNeteditAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_MARKFRONTELEMENT,   GNEInspectorFrame::NeteditAttributesEditor::onCmdMarkFrontElement),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,                   GNEInspectorFrame::NeteditAttributesEditor::onCmdNeteditAttributeHelp)
};

FXDEFMAP(GNEInspectorFrame::GEOAttributesEditor) GEOAttributesEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,  GNEInspectorFrame::GEOAttributesEditor::onCmdSetGEOAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_HELP,               GNEInspectorFrame::GEOAttributesEditor::onCmdGEOAttributeHelp)
};

FXDEFMAP(GNEInspectorFrame::TemplateEditor) TemplateEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F1_TEMPLATE_SET,   GNEInspectorFrame::TemplateEditor::onCmdSetTemplate),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F2_TEMPLATE_COPY,  GNEInspectorFrame::TemplateEditor::onCmdCopyTemplate),
    FXMAPFUNC(SEL_COMMAND,  MID_HOTKEY_SHIFT_F3_TEMPLATE_CLEAR, GNEInspectorFrame::TemplateEditor::onCmdCopyTemplate),
};

// Object implementation
FXIMPLEMENT(GNEInspectorFrame,                              FXVerticalFrame,    GNEInspectorFrameMap,       ARRAYNUMBER(GNEInspectorFrameMap))
FXIMPLEMENT(GNEInspectorFrame::NeteditAttributesEditor,     FXGroupBox,         NeteditAttributesEditorMap, ARRAYNUMBER(NeteditAttributesEditorMap))
FXIMPLEMENT(GNEInspectorFrame::GEOAttributesEditor,         FXGroupBox,         GEOAttributesEditorMap,     ARRAYNUMBER(GEOAttributesEditorMap))
FXIMPLEMENT(GNEInspectorFrame::TemplateEditor,              FXGroupBox,         TemplateEditorMap,          ARRAYNUMBER(TemplateEditorMap))


// ===========================================================================
// method definitions
// ===========================================================================

GNEInspectorFrame::GNEInspectorFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet):
    GNEFrame(horizontalFrameParent, viewNet, "Inspector"),
    myPreviousElementInspect(nullptr),
    myPreviousElementDelete(nullptr),
    myPreviousElementDeleteWasMarked(false) {

    // Create back button
    myBackButton = new FXButton(myHeaderLeftFrame, "", GUIIconSubSys::getIcon(GUIIcon::BIGARROWLEFT), this, MID_GNE_INSPECTORFRAME_GOBACK, GUIDesignButtonIconRectangular);
    myHeaderLeftFrame->hide();
    myBackButton->hide();

    // Create Overlapped Inspection modul
    myOverlappedInspection = new GNEFrameModuls::OverlappedInspection(this);

    // Create Attributes Editor modul
    myAttributesEditor = new GNEFrameAttributesModuls::AttributesEditor(this);

    // Create GEO Parameters Editor modul
    myGEOAttributesEditor = new GEOAttributesEditor(this);

    // create parameters Editor modul
    myParametersEditor = new GNEFrameAttributesModuls::ParametersEditor(this, "Parameters");

    // Create Netedit Attributes Editor modul
    myNeteditAttributesEditor = new NeteditAttributesEditor(this);

    // Create Template editor modul
    myTemplateEditor = new TemplateEditor(this);

    // Create HierarchicalElementTree modul
    myHierarchicalElementTree = new GNEFrameModuls::HierarchicalElementTree(this);
}


GNEInspectorFrame::~GNEInspectorFrame() {}


void
GNEInspectorFrame::show() {
    // inspect a null element to reset inspector frame
    inspectSingleElement(nullptr);
    GNEFrame::show();
}


void
GNEInspectorFrame::hide() {
    myViewNet->setInspectedAttributeCarrier(nullptr);
    GNEFrame::hide();
}


bool
GNEInspectorFrame::processNetworkSupermodeClick(const Position& clickedPosition, GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // first check if we have clicked over an Attribute Carrier
    if (objectsUnderCursor.getAttributeCarrierFront()) {
        // if Control key is Pressed, select instead inspect element
        if (myViewNet->getKeyPressed().controlKeyPressed()) {
            // Check if this GLobject type is locked
            if (!myViewNet->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->IsObjectTypeLocked(objectsUnderCursor.getGlTypeFront())) {
                // toogle networkElement selection
                if (objectsUnderCursor.getAttributeCarrierFront()->isAttributeCarrierSelected()) {
                    objectsUnderCursor.getAttributeCarrierFront()->unselectAttributeCarrier();
                } else {
                    objectsUnderCursor.getAttributeCarrierFront()->selectAttributeCarrier();
                }
            }
        } else {
            // first check if we clicked over a OverlappedInspection point
            if (myViewNet->getKeyPressed().shiftKeyPressed()) {
                if (!myOverlappedInspection->previousElement(clickedPosition)) {
                    // inspect attribute carrier, (or multiselection if AC is selected)
                    inspectClickedElement(objectsUnderCursor, clickedPosition);
                }
            } else  if (!myOverlappedInspection->nextElement(clickedPosition)) {
                // inspect attribute carrier, (or multiselection if AC is selected)
                inspectClickedElement(objectsUnderCursor, clickedPosition);
            }
            // focus upper element of inspector frame
            focusUpperElement();
        }
        return true;
    } else {
        return false;
    }
}


bool
GNEInspectorFrame::processDemandSupermodeClick(const Position& clickedPosition, GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // first check if we have clicked over a demand element
    if (objectsUnderCursor.getDemandElementFront()) {
        // if Control key is Pressed, select instead inspect element
        if (myViewNet->getKeyPressed().controlKeyPressed()) {
            // Check if this GLobject type is locked
            if (!myViewNet->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->IsObjectTypeLocked(objectsUnderCursor.getGlTypeFront())) {
                // toogle networkElement selection
                if (objectsUnderCursor.getAttributeCarrierFront()->isAttributeCarrierSelected()) {
                    objectsUnderCursor.getAttributeCarrierFront()->unselectAttributeCarrier();
                } else {
                    objectsUnderCursor.getAttributeCarrierFront()->selectAttributeCarrier();
                }
            }
        } else {
            // first check if we clicked over a OverlappedInspection point
            if (myViewNet->getKeyPressed().shiftKeyPressed()) {
                if (!myOverlappedInspection->previousElement(clickedPosition)) {
                    // inspect attribute carrier, (or multiselection if AC is selected)
                    inspectClickedElement(objectsUnderCursor, clickedPosition);
                }
            } else  if (!myOverlappedInspection->nextElement(clickedPosition)) {
                // inspect attribute carrier, (or multiselection if AC is selected)
                inspectClickedElement(objectsUnderCursor, clickedPosition);
            }
            // focus upper element of inspector frame
            focusUpperElement();
        }
        return true;
    } else {
        return false;
    }
}


bool
GNEInspectorFrame::processDataSupermodeClick(const Position& clickedPosition, GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // first check if we have clicked over a data element
    if (objectsUnderCursor.getGenericDataElementFront()) {
        // if Control key is Pressed, select instead inspect element
        if (myViewNet->getKeyPressed().controlKeyPressed()) {
            // Check if this GLobject type is locked
            if (!myViewNet->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->IsObjectTypeLocked(objectsUnderCursor.getGlTypeFront())) {
                // toogle networkElement selection
                if (objectsUnderCursor.getAttributeCarrierFront()->isAttributeCarrierSelected()) {
                    objectsUnderCursor.getAttributeCarrierFront()->unselectAttributeCarrier();
                } else {
                    objectsUnderCursor.getAttributeCarrierFront()->selectAttributeCarrier();
                }
            }
        } else {
            // first check if we clicked over a OverlappedInspection point
            if (myViewNet->getKeyPressed().shiftKeyPressed()) {
                if (!myOverlappedInspection->previousElement(clickedPosition)) {
                    // inspect attribute carrier, (or multiselection if AC is selected)
                    inspectClickedElement(objectsUnderCursor, clickedPosition);
                }
            } else  if (!myOverlappedInspection->nextElement(clickedPosition)) {
                // inspect attribute carrier, (or multiselection if AC is selected)
                inspectClickedElement(objectsUnderCursor, clickedPosition);
            }
            // focus upper element of inspector frame
            focusUpperElement();
        }
        return true;
    } else {
        return false;
    }
}


void
GNEInspectorFrame::inspectSingleElement(GNEAttributeCarrier* AC) {
    // Use the implementation of inspect for multiple AttributeCarriers to avoid repetition of code
    std::vector<GNEAttributeCarrier*> itemsToInspect;
    if (AC != nullptr) {
        myViewNet->setInspectedAttributeCarrier(AC);
        if (AC->isAttributeCarrierSelected()) {
            // obtain selected ACs depending of current supermode
            std::vector<GNEAttributeCarrier*> selectedACs = myViewNet->getNet()->getSelectedAttributeCarriers(false);
            // iterate over selected ACs
            for (const auto& i : selectedACs) {
                // filter ACs to inspect using Tag as criterium
                if (i->getTagProperty().getTag() == AC->getTagProperty().getTag()) {
                    itemsToInspect.push_back(i);
                }
            }
        } else {
            itemsToInspect.push_back(AC);
        }
    }
    inspectMultisection(itemsToInspect);
}


void
GNEInspectorFrame::inspectMultisection(const std::vector<GNEAttributeCarrier*>& ACs) {
    // hide back button
    myHeaderLeftFrame->hide();
    myBackButton->hide();
    // Hide all elements
    myAttributesEditor->hideAttributesEditorModul();
    myNeteditAttributesEditor->hideNeteditAttributesEditor();
    myGEOAttributesEditor->hideGEOAttributesEditor();
    myParametersEditor->hideParametersEditor();
    myTemplateEditor->hideTemplateEditor();
    myHierarchicalElementTree->hideHierarchicalElementTree();
    myOverlappedInspection->hideOverlappedInspection();
    // If vector of attribute Carriers contain data
    if (ACs.size() > 0) {
        // Set header
        std::string headerString;
        if (ACs.front()->getTagProperty().isNetworkElement()) {
            headerString = "Net: ";
        } else if (ACs.front()->getTagProperty().isAdditionalElement()) {
            headerString = "Additional: ";
        } else if (ACs.front()->getTagProperty().isShape()) {
            headerString = "Shape: ";
        } else if (ACs.front()->getTagProperty().isTAZElement()) {
            headerString = "TAZ: ";
        } else if (ACs.front()->getTagProperty().isVehicle()) {
            headerString = "Vehicle: ";
        } else if (ACs.front()->getTagProperty().isRoute()) {
            headerString = "Route: ";
        } else if (ACs.front()->getTagProperty().isPerson()) {
            headerString = "Person: ";
        } else if (ACs.front()->getTagProperty().isPersonPlan()) {
            headerString = "PersonPlan: ";
        } else if (ACs.front()->getTagProperty().isStop()) {
            headerString = "Stop: ";
        } else if (ACs.front()->getTagProperty().isDataElement()) {
            headerString = "Data: ";
        }
        if (ACs.size() > 1) {
            headerString += toString(ACs.size()) + " ";
        }
        headerString += ACs.front()->getTagStr();
        if (ACs.size() > 1) {
            headerString += "s";
        }
        // Set headerString into header label
        getFrameHeaderLabel()->setText(headerString.c_str());

        // Show attributes editor
        myAttributesEditor->showAttributeEditorModul(ACs, true, false);

        // show netedit attributes editor if  we're inspecting elements with Netedit Attributes
        myNeteditAttributesEditor->showNeteditAttributesEditor();

        // Show GEO Attributes Editor if we're inspecting elements with GEO Attributes
        myGEOAttributesEditor->showGEOAttributesEditor();

        // show parameters editor
        if (ACs.size() == 1) {
            myParametersEditor->showParametersEditor(ACs.front(), "parameters");
        } else {
            myParametersEditor->showParametersEditor(ACs, "parameters");
        }

        // If attributes correspond to an Edge and we aren't in demand mode, show template editor
        myTemplateEditor->showTemplateEditor();

        // if we inspect a single Attribute carrier vector, show their children
        if (ACs.size() == 1) {
            myHierarchicalElementTree->showHierarchicalElementTree(ACs.front());
        }
    } else {
        getFrameHeaderLabel()->setText("Inspect");
        myContentFrame->recalc();
    }
}


void
GNEInspectorFrame::inspectChild(GNEAttributeCarrier* AC, GNEAttributeCarrier* previousElement) {
    // Show back button if myPreviousElementInspect was defined
    myPreviousElementInspect = previousElement;
    if (myPreviousElementInspect != nullptr) {
        // disable myPreviousElementDelete to avoid inconsistences
        myPreviousElementDelete = nullptr;
        inspectSingleElement(AC);
        myHeaderLeftFrame->show();
        myBackButton->show();
    }
}


void
GNEInspectorFrame::inspectFromDeleteFrame(GNEAttributeCarrier* AC, GNEAttributeCarrier* previousElement, bool previousElementWasMarked) {
    myPreviousElementDelete = previousElement;
    myPreviousElementDeleteWasMarked = previousElementWasMarked;
    // Show back button if myPreviousElementDelete is valid
    if (myPreviousElementDelete != nullptr) {
        // disable myPreviousElementInspect to avoid inconsistences
        myPreviousElementInspect = nullptr;
        inspectSingleElement(AC);
        myHeaderLeftFrame->show();
        myBackButton->show();
    }
}


void
GNEInspectorFrame::clearInspectedAC() {
    // Only remove if there is inspected ACs
    if (myAttributesEditor->getEditedACs().size() > 0) {
        myViewNet->setInspectedAttributeCarrier(nullptr);
        // Inspect empty selection (to hide all Editors)
        inspectMultisection({});
    }
}


GNEFrameAttributesModuls::AttributesEditor*
GNEInspectorFrame::getAttributesEditor() const {
    return myAttributesEditor;
}


GNEInspectorFrame::NeteditAttributesEditor*
GNEInspectorFrame::getNeteditAttributesEditor() const {
    return myNeteditAttributesEditor;
}


GNEInspectorFrame::TemplateEditor*
GNEInspectorFrame::getTemplateEditor() const {
    return myTemplateEditor;
}


GNEFrameModuls::OverlappedInspection*
GNEInspectorFrame::getOverlappedInspection() const {
    return myOverlappedInspection;
}


GNEFrameModuls::HierarchicalElementTree*
GNEInspectorFrame::getHierarchicalElementTree() const {
    return myHierarchicalElementTree;
}


long
GNEInspectorFrame::onCmdGoBack(FXObject*, FXSelector, void*) {
    // Inspect previous element or go back to Delete Frame
    if (myPreviousElementInspect) {
        inspectSingleElement(myPreviousElementInspect);
        myPreviousElementInspect = nullptr;
    } else if (myPreviousElementDelete != nullptr) {
        myPreviousElementDelete = nullptr;
        // Hide inspect frame and show delete frame
        hide();
        myViewNet->getViewParent()->getDeleteFrame()->show();
    }
    return 1;
}


void
GNEInspectorFrame::updateFrameAfterUndoRedo() {
    // refresh Attribute Editor
    myAttributesEditor->refreshAttributeEditor(false, false);
    // refresh parametersEditor
    myParametersEditor->refreshParametersEditor();
    // refresh AC Hierarchy
    myHierarchicalElementTree->refreshHierarchicalElementTree();
}


void
GNEInspectorFrame::selectedOverlappedElement(GNEAttributeCarrier* AC) {
    // if AC is a lane but selectEdges checkBox is enabled, then inspect their parent edge
    if (AC->getTagProperty().getTag() == SUMO_TAG_LANE && myViewNet->getNetworkViewOptions().selectEdges()) {
        inspectSingleElement(dynamic_cast<GNELane*>(AC)->getParentEdge());
    } else {
        inspectSingleElement(AC);
    }
    // update view (due dotted contour)
    myViewNet->updateViewNet();
}


void
GNEInspectorFrame::inspectClickedElement(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor, const Position& clickedPosition) {
    if (objectsUnderCursor.getAttributeCarrierFront()) {
        // inspect front element
        inspectSingleElement(objectsUnderCursor.getAttributeCarrierFront());
        // if element has overlapped elements, show Overlapped Inspection modul
        if (objectsUnderCursor.getClickedAttributeCarriers().size() > 1) {
            myOverlappedInspection->showOverlappedInspection(objectsUnderCursor, clickedPosition);
        } else {
            myOverlappedInspection->hideOverlappedInspection();
        }
    }
}


void
GNEInspectorFrame::attributeUpdated() {
    myAttributesEditor->refreshAttributeEditor(false, false);
    myNeteditAttributesEditor->refreshNeteditAttributesEditor(true);
    myGEOAttributesEditor->refreshGEOAttributesEditor(true);
}

// ---------------------------------------------------------------------------
// GNEInspectorFrame::NeteditAttributesEditor - methods
// ---------------------------------------------------------------------------

GNEInspectorFrame::NeteditAttributesEditor::NeteditAttributesEditor(GNEInspectorFrame* inspectorFrameParent) :
    FXGroupBox(inspectorFrameParent->myContentFrame, "Netedit attributes", GUIDesignGroupBoxFrame),
    myInspectorFrameParent(inspectorFrameParent) {

    // Create mark as front element button
    myMarkFrontElementButton = new FXButton(this, "Mark as front element", GUIIconSubSys::getIcon(GUIIcon::FRONTELEMENT), this, MID_GNE_MARKFRONTELEMENT, GUIDesignButton);

    // Create elements for parent additional
    myHorizontalFrameParentAdditional = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myLabelParentAdditional = new FXLabel(myHorizontalFrameParentAdditional, "Block move", nullptr, GUIDesignLabelAttribute);
    myTextFieldParentAdditional = new FXTextField(myHorizontalFrameParentAdditional, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);

    // Create elements for block movement
    myHorizontalFrameBlockMovement = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myLabelBlockMovement = new FXLabel(myHorizontalFrameBlockMovement, "Block move", nullptr, GUIDesignLabelAttribute);
    myCheckBoxBlockMovement = new FXCheckButton(myHorizontalFrameBlockMovement, "", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);

    // Create elements for block shape
    myHorizontalFrameBlockShape = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myLabelBlockShape = new FXLabel(myHorizontalFrameBlockShape, "Block shape", nullptr, GUIDesignLabelAttribute);
    myCheckBoxBlockShape = new FXCheckButton(myHorizontalFrameBlockShape, "", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);

    // Create elements for close shape
    myHorizontalFrameCloseShape = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myLabelCloseShape = new FXLabel(myHorizontalFrameCloseShape, "Close shape", nullptr, GUIDesignLabelAttribute);
    myCheckBoxCloseShape = new FXCheckButton(myHorizontalFrameCloseShape, "", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);

    // Create help button
    myHelpButton = new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
}


GNEInspectorFrame::NeteditAttributesEditor::~NeteditAttributesEditor() {}


void
GNEInspectorFrame::NeteditAttributesEditor::showNeteditAttributesEditor() {
    // get edited ACS
    const auto& editedACs = myInspectorFrameParent->myAttributesEditor->getEditedACs();
    // continue if there is edited ACs
    if (editedACs.size() > 0) {
        // enable all editable elements
        myTextFieldParentAdditional->enable();
        myCheckBoxBlockMovement->enable();
        myCheckBoxBlockShape->enable();
        myCheckBoxCloseShape->enable();
        // obtain tag property (only for improve code legibility)
        const auto& tagValue = editedACs.front()->getTagProperty();
        // check if item can be mark as front elmenet
        if (editedACs.size() == 1) {
            // show NeteditAttributesEditor
            show();
            // show button
            myMarkFrontElementButton->show();
            // enable or disable
            if (myInspectorFrameParent->getViewNet()->getFrontAttributeCarrier() == editedACs.front()) {
                myMarkFrontElementButton->disable();
            } else {
                myMarkFrontElementButton->enable();
            }
        }
        // Check if item can be moved
        if (tagValue.canBlockMovement()) {
            // show NeteditAttributesEditor
            show();
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : editedACs) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(GNE_ATTR_BLOCK_MOVEMENT));
            }
            // show block movement frame
            myHorizontalFrameBlockMovement->show();
            // show help button
            myHelpButton->show();
            // set check box value and update label
            if (value) {
                myCheckBoxBlockMovement->setCheck(true);
                myCheckBoxBlockMovement->setText("true");
            } else {
                myCheckBoxBlockMovement->setCheck(false);
                myCheckBoxBlockMovement->setText("false");
            }
        }
        // check if item can block their shape
        if (tagValue.canBlockShape()) {
            // show NeteditAttributesEditor
            show();
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : editedACs) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(GNE_ATTR_BLOCK_SHAPE));
            }
            // show block shape frame
            myHorizontalFrameBlockShape->show();
            // show help button
            myHelpButton->show();
            // set check box value and update label
            if (value) {
                myCheckBoxBlockShape->setCheck(true);
                myCheckBoxBlockShape->setText("true");
            } else {
                myCheckBoxBlockShape->setCheck(false);
                myCheckBoxBlockShape->setText("false");
            }
        }
        // check if item can block their shape
        if (tagValue.canCloseShape()) {
            // show NeteditAttributesEditor
            show();
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : editedACs) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(GNE_ATTR_CLOSE_SHAPE));
            }
            // show close shape frame
            myHorizontalFrameCloseShape->show();
            // show help button
            myHelpButton->show();
            // set check box value and update label
            if (value) {
                myCheckBoxCloseShape->setCheck(true);
                myCheckBoxCloseShape->setText("true");
            } else {
                myCheckBoxCloseShape->setCheck(false);
                myCheckBoxCloseShape->setText("false");
            }
        }
        // Check if item has another item as parent and can be reparemt
        if (tagValue.isSlave() && tagValue.canBeReparent()) {
            // show NeteditAttributesEditor
            show();
            // obtain additional Parent
            std::set<std::string> parents;
            for (const auto& i : editedACs) {
                parents.insert(i->getAttribute(GNE_ATTR_PARENT));
            }
            // show parent additional frame
            myHorizontalFrameParentAdditional->show();
            // show help button
            myHelpButton->show();
            // set Label and TextField with the Tag and ID of parent
            myLabelParentAdditional->setText((toString(editedACs.front()->getTagProperty().isSlave()) + " parent").c_str());
            myTextFieldParentAdditional->setText(toString(parents).c_str());
        }
        // disable all editable elements if we're in demand mode and inspected AC isn't a demand element
        if (GNEFrameAttributesModuls::isSupermodeValid(myInspectorFrameParent->getViewNet(), editedACs.front()) == false) {
            myTextFieldParentAdditional->disable();
            myCheckBoxBlockMovement->disable();
            myCheckBoxBlockShape->disable();
            myCheckBoxCloseShape->disable();
        }
    }
}


void
GNEInspectorFrame::NeteditAttributesEditor::hideNeteditAttributesEditor() {
    // hide all elements of GroupBox
    myHorizontalFrameParentAdditional->hide();
    myHorizontalFrameBlockMovement->hide();
    myHorizontalFrameBlockShape->hide();
    myHorizontalFrameCloseShape->hide();
    myMarkFrontElementButton->hide();
    myHelpButton->hide();
    // hide groupbox
    hide();
}


void
GNEInspectorFrame::NeteditAttributesEditor::refreshNeteditAttributesEditor(bool forceRefresh) {
    if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() > 0) {
        // enable or disable mark front element button
        if (myInspectorFrameParent->getViewNet()->getFrontAttributeCarrier() == myInspectorFrameParent->myAttributesEditor->getEditedACs().front()) {
            myMarkFrontElementButton->disable();
        } else {
            myMarkFrontElementButton->enable();
        }
        // refresh block movement
        if (myHorizontalFrameBlockMovement->shown()) {
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(GNE_ATTR_BLOCK_MOVEMENT));
            }
            // set check box value and update label
            if (value) {
                myCheckBoxBlockMovement->setCheck(true);
                myCheckBoxBlockMovement->setText("true");
            } else {
                myCheckBoxBlockMovement->setCheck(false);
                myCheckBoxBlockMovement->setText("false");
            }
        }
        // refresh block shape
        if (myHorizontalFrameBlockShape->shown()) {
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(GNE_ATTR_BLOCK_SHAPE));
            }
            // set check box value and update label
            if (value) {
                myCheckBoxBlockShape->setCheck(true);
                myCheckBoxBlockShape->setText("true");
            } else {
                myCheckBoxBlockShape->setCheck(false);
                myCheckBoxBlockShape->setText("false");
            }
        }
        // refresh close shape
        if (myHorizontalFrameCloseShape->shown()) {
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(GNE_ATTR_CLOSE_SHAPE));
            }
            // set check box value and update label
            if (value) {
                myCheckBoxCloseShape->setCheck(true);
                myCheckBoxCloseShape->setText("true");
            } else {
                myCheckBoxCloseShape->setCheck(false);
                myCheckBoxCloseShape->setText("false");
            }
        }
        // Check if item has another item as parent (Currently only for single Additionals)
        if (myHorizontalFrameParentAdditional->shown() && ((myTextFieldParentAdditional->getTextColor() == FXRGB(0, 0, 0)) || forceRefresh)) {
            // set Label and TextField with the Tag and ID of parent
            myLabelParentAdditional->setText((toString(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getTagProperty().getMasterTags().front()) + " parent").c_str());
            myTextFieldParentAdditional->setText(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getAttribute(GNE_ATTR_PARENT).c_str());
        }
    }
}


long
GNEInspectorFrame::NeteditAttributesEditor::onCmdSetNeteditAttribute(FXObject* obj, FXSelector, void*) {
    // make sure that ACs has elements
    if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() > 0) {
        // check if we're changing multiple attributes
        if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() > 1) {
            myInspectorFrameParent->myViewNet->getUndoList()->p_begin("Change multiple attributes");
        }
        if (obj == myCheckBoxBlockMovement) {
            // set new values in all inspected Attribute Carriers
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                if (myCheckBoxBlockMovement->getCheck() == 1) {
                    i->setAttribute(GNE_ATTR_BLOCK_MOVEMENT, "true", myInspectorFrameParent->myViewNet->getUndoList());
                    myCheckBoxBlockMovement->setText("true");
                } else {
                    i->setAttribute(GNE_ATTR_BLOCK_MOVEMENT, "false", myInspectorFrameParent->myViewNet->getUndoList());
                    myCheckBoxBlockMovement->setText("false");
                }
            }
        } else if (obj == myCheckBoxBlockShape) {
            // set new values in all inspected Attribute Carriers
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                if (myCheckBoxBlockShape->getCheck() == 1) {
                    i->setAttribute(GNE_ATTR_BLOCK_SHAPE, "true", myInspectorFrameParent->myViewNet->getUndoList());
                    myCheckBoxBlockShape->setText("true");
                } else {
                    i->setAttribute(GNE_ATTR_BLOCK_SHAPE, "false", myInspectorFrameParent->myViewNet->getUndoList());
                    myCheckBoxBlockShape->setText("false");
                }
            }
        } else if (obj == myCheckBoxCloseShape) {
            // set new values in all inspected Attribute Carriers
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                if (myCheckBoxCloseShape->getCheck() == 1) {
                    i->setAttribute(GNE_ATTR_CLOSE_SHAPE, "true", myInspectorFrameParent->myViewNet->getUndoList());
                    myCheckBoxCloseShape->setText("true");
                } else {
                    i->setAttribute(GNE_ATTR_CLOSE_SHAPE, "false", myInspectorFrameParent->myViewNet->getUndoList());
                    myCheckBoxCloseShape->setText("false");
                }
            }
        } else if (obj == myTextFieldParentAdditional) {
            if (myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->isValid(GNE_ATTR_PARENT, myTextFieldParentAdditional->getText().text())) {
                // replace the parent of all inspected elements
                for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                    i->setAttribute(GNE_ATTR_PARENT, myTextFieldParentAdditional->getText().text(), myInspectorFrameParent->myViewNet->getUndoList());
                }
                myTextFieldParentAdditional->setTextColor(FXRGB(0, 0, 0));
                myTextFieldParentAdditional->killFocus();
            } else {
                myTextFieldParentAdditional->setTextColor(FXRGB(255, 0, 0));
            }
        }
        // finish change multiple attributes
        if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() > 1) {
            myInspectorFrameParent->myViewNet->getUndoList()->p_end();
        }
        // force refresh values of AttributesEditor and GEOAttributesEditor
        myInspectorFrameParent->myAttributesEditor->refreshAttributeEditor(true, true);
        myInspectorFrameParent->myGEOAttributesEditor->refreshGEOAttributesEditor(true);
    }
    return 1;
}


long
GNEInspectorFrame::NeteditAttributesEditor::onCmdMarkFrontElement(FXObject*, FXSelector, void*) {
    // check number of elements
    if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() == 1) {
        // mark AC as front elemnet
        myInspectorFrameParent->getViewNet()->setFrontAttributeCarrier(myInspectorFrameParent->myAttributesEditor->getEditedACs().front());
        // disable button
        myMarkFrontElementButton->disable();
    }
    return 1;
}


long
GNEInspectorFrame::NeteditAttributesEditor::onCmdNeteditAttributeHelp(FXObject*, FXSelector, void*) {
    // Create dialog box
    FXDialogBox* additionalNeteditAttributesHelpDialog = new FXDialogBox(this, "Netedit Attributes Help", GUIDesignDialogBox);
    additionalNeteditAttributesHelpDialog->setIcon(GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL));
    // set help text
    std::ostringstream help;
    help
            << "- Mark as front element: Mark element as front element (Will be drawn over all other elements)\n"
            << "- Block movement: disable movement in move mode\n"
            << "- Block shape: Disable moving of shape's vertices and edges. Entire shape can be moved'.\n"
            << "- Close shape: Add or remove the last vertex with the same position of first edge'.";
    // Create label with the help text
    new FXLabel(additionalNeteditAttributesHelpDialog, help.str().c_str(), nullptr, GUIDesignLabelFrameInformation);
    // Create horizontal separator
    new FXHorizontalSeparator(additionalNeteditAttributesHelpDialog, GUIDesignHorizontalSeparator);
    // Create frame for OK Button
    FXHorizontalFrame* myHorizontalFrameOKButton = new FXHorizontalFrame(additionalNeteditAttributesHelpDialog, GUIDesignAuxiliarHorizontalFrame);
    // Create Button Close (And two more horizontal frames to center it)
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    new FXButton(myHorizontalFrameOKButton, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), additionalNeteditAttributesHelpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    new FXHorizontalFrame(myHorizontalFrameOKButton, GUIDesignAuxiliarHorizontalFrame);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Opening NeteditAttributesEditor help dialog");
    // create Dialog
    additionalNeteditAttributesHelpDialog->create();
    // show in the given position
    additionalNeteditAttributesHelpDialog->show(PLACEMENT_CURSOR);
    // refresh APP
    getApp()->refresh();
    // open as modal dialog (will block all windows until stop() or stopModal() is called)
    getApp()->runModalFor(additionalNeteditAttributesHelpDialog);
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Closing NeteditAttributesEditor help dialog");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEInspectorFrame::GEOAttributesEditor - methods
// ---------------------------------------------------------------------------

GNEInspectorFrame::GEOAttributesEditor::GEOAttributesEditor(GNEInspectorFrame* inspectorFrameParent) :
    FXGroupBox(inspectorFrameParent->myContentFrame, "GEO Attributes", GUIDesignGroupBoxFrame),
    myInspectorFrameParent(inspectorFrameParent) {

    // Create Frame for GEOAttribute
    myGEOAttributeFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myGEOAttributeLabel = new FXLabel(myGEOAttributeFrame, "Undefined GEO Attribute", nullptr, GUIDesignLabelAttribute);
    myGEOAttributeTextField = new FXTextField(myGEOAttributeFrame, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);

    // Create Frame for use GEO
    myUseGEOFrame = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);
    myUseGEOLabel = new FXLabel(myUseGEOFrame, toString(SUMO_ATTR_GEO).c_str(), nullptr, GUIDesignLabelAttribute);
    myUseGEOCheckButton = new FXCheckButton(myUseGEOFrame, "false", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);

    // Create help button
    myHelpButton = new FXButton(this, "Help", nullptr, this, MID_HELP, GUIDesignButtonRectangular);
}


GNEInspectorFrame::GEOAttributesEditor::~GEOAttributesEditor() {}


void
GNEInspectorFrame::GEOAttributesEditor::showGEOAttributesEditor() {
    // make sure that ACs has elements
    if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() > 0) {
        // enable all editable elements
        myGEOAttributeTextField->enable();
        myUseGEOCheckButton->enable();
        // obtain tag property (only for improve code legibility)
        const auto& tagProperty = myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getTagProperty();
        // check if item can use a geo position
        if (tagProperty.hasGEOPosition() || tagProperty.hasGEOShape()) {
            // show GEOAttributesEditor
            show();
            // Iterate over AC to obtain values
            bool value = true;
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                value &= GNEAttributeCarrier::parse<bool>(i->getAttribute(SUMO_ATTR_GEO));
            }
            // show use geo frame
            myUseGEOFrame->show();
            // set UseGEOCheckButton value of and update label (only if geo conversion is defined)
            if (GeoConvHelper::getFinal().getProjString() != "!") {
                myUseGEOCheckButton->enable();
                if (value) {
                    myUseGEOCheckButton->setCheck(true);
                    myUseGEOCheckButton->setText("true");
                } else {
                    myUseGEOCheckButton->setCheck(false);
                    myUseGEOCheckButton->setText("false");
                }
            } else {
                myUseGEOCheckButton->disable();
            }
            // now specify if a single position or an entire shape must be shown (note: cannot be shown both at the same time, and GEO Shape/Position only works for single selections)
            if (tagProperty.hasGEOPosition() && myInspectorFrameParent->myAttributesEditor->getEditedACs().size() == 1) {
                myGEOAttributeFrame->show();
                myGEOAttributeLabel->setText(toString(SUMO_ATTR_GEOPOSITION).c_str());
                myGEOAttributeTextField->setTextColor(FXRGB(0, 0, 0));
                // only allow edit if geo conversion is defined
                if (GeoConvHelper::getFinal().getProjString() != "!") {
                    myGEOAttributeTextField->enable();
                    myGEOAttributeTextField->setText(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getAttribute(SUMO_ATTR_GEOPOSITION).c_str());
                } else {
                    myGEOAttributeTextField->disable();
                    myGEOAttributeTextField->setText("No geo-conversion defined");
                }
            } else if (tagProperty.hasGEOShape() && myInspectorFrameParent->myAttributesEditor->getEditedACs().size() == 1) {
                myGEOAttributeFrame->show();
                myGEOAttributeLabel->setText(toString(SUMO_ATTR_GEOSHAPE).c_str());
                myGEOAttributeTextField->setTextColor(FXRGB(0, 0, 0));
                // only allow edit if geo conversion is defined
                if (GeoConvHelper::getFinal().getProjString() != "!") {
                    myGEOAttributeTextField->enable();
                    myGEOAttributeTextField->setText(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getAttribute(SUMO_ATTR_GEOSHAPE).c_str());
                } else {
                    myGEOAttributeTextField->disable();
                    myGEOAttributeTextField->setText("No geo-conversion defined");
                }
            }
        }
        // disable all editable elements if we're in demand mode and inspected AC isn't a demand element
        if (GNEFrameAttributesModuls::isSupermodeValid(myInspectorFrameParent->getViewNet(), myInspectorFrameParent->myAttributesEditor->getEditedACs().front()) == false) {
            myGEOAttributeTextField->disable();
            myUseGEOCheckButton->disable();
        }
    }
}


void
GNEInspectorFrame::GEOAttributesEditor::hideGEOAttributesEditor() {
    // hide all elements of GroupBox
    myGEOAttributeFrame->hide();
    myUseGEOFrame->hide();
    // hide groupbox
    hide();
}


void
GNEInspectorFrame::GEOAttributesEditor::refreshGEOAttributesEditor(bool forceRefresh) {
    // obtain tag property (only for improve code legibility)
    const auto& tagProperty = myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getTagProperty();
    // Check that myGEOAttributeFrame is shown
    if ((GeoConvHelper::getFinal().getProjString() != "!") && myGEOAttributeFrame->shown() && ((myGEOAttributeTextField->getTextColor() == FXRGB(0, 0, 0)) || forceRefresh)) {
        if (tagProperty.hasGEOPosition()) {
            myGEOAttributeTextField->setText(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getAttribute(SUMO_ATTR_GEOPOSITION).c_str());
        } else if (tagProperty.hasGEOShape()) {
            myGEOAttributeTextField->setText(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getAttribute(SUMO_ATTR_GEOSHAPE).c_str());
        }
        myGEOAttributeTextField->setTextColor(FXRGB(0, 0, 0));
    }
}


long
GNEInspectorFrame::GEOAttributesEditor::onCmdSetGEOAttribute(FXObject* obj, FXSelector, void*) {
    // make sure that ACs has elements
    if ((GeoConvHelper::getFinal().getProjString() != "!") && (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() > 0)) {
        if (obj == myGEOAttributeTextField) {
            // obtain tag property (only for improve code legibility)
            const auto& tagProperty = myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getTagProperty();
            // Change GEO Attribute depending of type (Position or shape)
            if (tagProperty.hasGEOPosition()) {
                if (myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->isValid(SUMO_ATTR_GEOPOSITION, myGEOAttributeTextField->getText().text())) {
                    myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->setAttribute(SUMO_ATTR_GEOPOSITION, myGEOAttributeTextField->getText().text(), myInspectorFrameParent->myViewNet->getUndoList());
                    myGEOAttributeTextField->setTextColor(FXRGB(0, 0, 0));
                    myGEOAttributeTextField->killFocus();
                } else {
                    myGEOAttributeTextField->setTextColor(FXRGB(255, 0, 0));
                }
            } else if (tagProperty.hasGEOShape()) {
                if (myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->isValid(SUMO_ATTR_GEOSHAPE, myGEOAttributeTextField->getText().text())) {
                    myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->setAttribute(SUMO_ATTR_GEOSHAPE, myGEOAttributeTextField->getText().text(), myInspectorFrameParent->myViewNet->getUndoList());
                    myGEOAttributeTextField->setTextColor(FXRGB(0, 0, 0));
                    myGEOAttributeTextField->killFocus();
                } else {
                    myGEOAttributeTextField->setTextColor(FXRGB(255, 0, 0));
                }
            } else {
                throw ProcessError("myGEOAttributeTextField must be hidden becaurse there isn't GEO Attribute to edit");
            }
        } else if (obj == myUseGEOCheckButton) {
            // update GEO Attribute of entire selection
            for (const auto& i : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
                if (myUseGEOCheckButton->getCheck() == 1) {
                    i->setAttribute(SUMO_ATTR_GEO, "true", myInspectorFrameParent->myViewNet->getUndoList());
                    myUseGEOCheckButton->setText("true");
                } else {
                    i->setAttribute(SUMO_ATTR_GEO, "false", myInspectorFrameParent->myViewNet->getUndoList());
                    myUseGEOCheckButton->setText("false");
                }
            }
        }
        // force refresh values of Attributes editor and NeteditAttributesEditor
        myInspectorFrameParent->myAttributesEditor->refreshAttributeEditor(true, true);
        myInspectorFrameParent->myNeteditAttributesEditor->refreshNeteditAttributesEditor(true);
    }
    return 1;
}


long
GNEInspectorFrame::GEOAttributesEditor::onCmdGEOAttributeHelp(FXObject*, FXSelector, void*) {
    FXDialogBox* helpDialog = new FXDialogBox(this, "GEO attributes Help", GUIDesignDialogBox);
    std::ostringstream help;
    help
            << " SUMO uses the World Geodetic System 84 (WGS84/UTM).\n"
            << " For a GEO-referenced network, geo coordinates are represented as pairs of Longitude and Latitude\n"
            << " in decimal degrees without extra symbols. (N,W..)\n"
            << " - Longitude: East-west position of a point on the Earth's surface.\n"
            << " - Latitude: North-south position of a point on the Earth's surface.\n"
            << " - CheckBox 'geo' enables or disables saving position in GEO coordinates\n";
    new FXLabel(helpDialog, help.str().c_str(), nullptr, GUIDesignLabelFrameInformation);
    // "OK"
    new FXButton(helpDialog, "OK\t\tclose", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), helpDialog, FXDialogBox::ID_ACCEPT, GUIDesignButtonOK);
    helpDialog->create();
    helpDialog->show();
    return 1;
}

// ---------------------------------------------------------------------------
// GNEInspectorFrame::TemplateEditor - methods
// ---------------------------------------------------------------------------

GNEInspectorFrame::TemplateEditor::TemplateEditor(GNEInspectorFrame* inspectorFrameParent) :
    FXGroupBox(inspectorFrameParent->myContentFrame, "Templates", GUIDesignGroupBoxFrame),
    myInspectorFrameParent(inspectorFrameParent),
    myEdgeTemplate(nullptr) {
    // Create set template button
    mySetTemplateButton = new FXButton(this, "Set as Template\t\t", nullptr, this, MID_HOTKEY_SHIFT_F1_TEMPLATE_SET, GUIDesignButton);
    // Create copy template button
    myCopyTemplateButton = new FXButton(this, "", nullptr, this, MID_HOTKEY_SHIFT_F2_TEMPLATE_COPY, GUIDesignButton);
    // Create copy template button
    myClearTemplateButton = new FXButton(this, "clear Edge Template", nullptr, this, MID_HOTKEY_SHIFT_F3_TEMPLATE_CLEAR, GUIDesignButton);
}


GNEInspectorFrame::TemplateEditor::~TemplateEditor() {
    // before destroy template editor, we need to check if there is an active edge template
    if (myEdgeTemplate) {
        // decrease reference
        myEdgeTemplate->decRef("GNEInspectorFrame::~GNEInspectorFrame");
        // delete edge template if is unreferenced
        if (myEdgeTemplate->unreferenced()) {
            delete myEdgeTemplate;
        }
    }
}


void
GNEInspectorFrame::TemplateEditor::showTemplateEditor() {
    // show template editor only if we're editing an edge in Network mode
    if ((myInspectorFrameParent->myViewNet->getEditModes().isCurrentSupermodeNetwork()) &&
            (myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getTagProperty().getTag() == SUMO_TAG_EDGE)) {
        // show "Set As Template"
        if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() == 1) {
            mySetTemplateButton->show();
            mySetTemplateButton->setText(("Set edge '" + myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getID() + "' as Template").c_str());
        }
        // update buttons
        updateButtons();
        // show modul
        show();
    }
}


void
GNEInspectorFrame::TemplateEditor::hideTemplateEditor() {
    // hide template editor
    hide();
}


GNEEdge*
GNEInspectorFrame::TemplateEditor::getEdgeTemplate() const {
    return myEdgeTemplate;
}


void
GNEInspectorFrame::TemplateEditor::setTemplate() {
    // check if template editor AND mySetTemplateButton is enabled
    if (shown() && mySetTemplateButton->isEnabled()) {
        onCmdSetTemplate(nullptr, 0, nullptr);
    }
}


void
GNEInspectorFrame::TemplateEditor::copyTemplate() {
    // check if template editor AND myCopyTemplateButton is enabled
    if (shown() && myCopyTemplateButton->isEnabled()) {
        onCmdCopyTemplate(nullptr, 0, nullptr);
    }
}


void
GNEInspectorFrame::TemplateEditor::clearTemplate() {
    // check if template editor AND myClearTemplateButton is enabled
    if (shown() && myClearTemplateButton->isEnabled()) {
        onCmdClearTemplate(nullptr, 0, nullptr);
    }
}


long
GNEInspectorFrame::TemplateEditor::onCmdSetTemplate(FXObject*, FXSelector, void*) {
    // first check that there is exactly an inspected edge
    if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() != 1) {
        throw ProcessError("Only one edge must be inspected");
    }
    // retrieve edge ID (and throw exception if edge doesn't exist)
    GNEEdge* edge = myInspectorFrameParent->myViewNet->getNet()->retrieveEdge(myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getID());
    // set template
    setEdgeTemplate(edge);
    // update buttons
    updateButtons();
    return 1;
}


long
GNEInspectorFrame::TemplateEditor::onCmdCopyTemplate(FXObject*, FXSelector, void*) {
    for (const auto& it : myInspectorFrameParent->myAttributesEditor->getEditedACs()) {
        // retrieve edge ID (and throw exception if edge doesn't exist)
        GNEEdge* edge = myInspectorFrameParent->myViewNet->getNet()->retrieveEdge(it->getID());
        // copy template
        edge->copyTemplate(myEdgeTemplate, myInspectorFrameParent->myViewNet->getUndoList());
        // refresh inspector parent
        myInspectorFrameParent->myAttributesEditor->refreshAttributeEditor(true, true);
    }
    // update view (to see visual changes)
    myInspectorFrameParent->myViewNet->updateViewNet();
    return 1;
}


long
GNEInspectorFrame::TemplateEditor::onCmdClearTemplate(FXObject*, FXSelector, void*) {
    setEdgeTemplate(nullptr);
    // update buttons
    updateButtons();
    return 1;
}


void
GNEInspectorFrame::TemplateEditor::setEdgeTemplate(GNEEdge* tpl) {
    // before change edge template, we need to check if there is another active edge template
    if (myEdgeTemplate) {
        // decrease reference
        myEdgeTemplate->decRef("GNEInspectorFrame::setEdgeTemplate");
        // delete edge template if is unreferenced
        if (myEdgeTemplate->unreferenced()) {
            delete myEdgeTemplate;
        }
    }
    // check if we're setting a new edge template or removing it
    if (tpl) {
        // set new edge template
        myEdgeTemplate = tpl;
        // increase reference
        myEdgeTemplate->incRef("GNEInspectorFrame::setEdgeTemplate");
    } else {
        // clear edge template
        myEdgeTemplate = nullptr;
    }
}


void
GNEInspectorFrame::TemplateEditor::updateButtons() {
    // enable or disable clear buttons depending of myEdgeTemplate
    if (myEdgeTemplate) {
        // update caption of copy button
        if (myInspectorFrameParent->myAttributesEditor->getEditedACs().size() == 1) {
            myCopyTemplateButton->setText(("Copy '" + myEdgeTemplate->getMicrosimID() + "' into edge '" + myInspectorFrameParent->myAttributesEditor->getEditedACs().front()->getID() + "'").c_str());
        } else {
            myCopyTemplateButton->setText(("Copy '" + myEdgeTemplate->getMicrosimID() + "' into " + toString(myInspectorFrameParent->myAttributesEditor->getEditedACs().size()) + " selected edges").c_str());
        }
        // enable set and clear buttons
        myCopyTemplateButton->enable();
        myClearTemplateButton->enable();
    } else {
        // update caption of copy button
        myCopyTemplateButton->setText("No edge Template Set");
        // disable set and clear buttons
        myCopyTemplateButton->disable();
        myClearTemplateButton->disable();
    }
}


/****************************************************************************/
