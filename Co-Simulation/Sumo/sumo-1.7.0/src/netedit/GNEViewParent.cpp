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
/// @file    GNEViewParent.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// A single child window which contains a view of the edited network (adapted
// from GUISUMOViewParent)
// While we don't actually need MDI for netedit it is easier to adapt existing
// structures than to write everything from scratch.
/****************************************************************************/

#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/dialogs/GNEDialogACChooser.h>
#include <netedit/frames/common/GNEDeleteFrame.h>
#include <netedit/frames/common/GNEInspectorFrame.h>
#include <netedit/frames/common/GNESelectorFrame.h>
#include <netedit/frames/common/GNEMoveFrame.h>
#include <netedit/frames/data/GNEEdgeDataFrame.h>
#include <netedit/frames/data/GNEEdgeRelDataFrame.h>
#include <netedit/frames/data/GNETAZRelDataFrame.h>
#include <netedit/frames/demand/GNEPersonFrame.h>
#include <netedit/frames/demand/GNEPersonPlanFrame.h>
#include <netedit/frames/demand/GNEPersonTypeFrame.h>
#include <netedit/frames/demand/GNERouteFrame.h>
#include <netedit/frames/demand/GNEStopFrame.h>
#include <netedit/frames/demand/GNEVehicleFrame.h>
#include <netedit/frames/demand/GNEVehicleTypeFrame.h>
#include <netedit/frames/network/GNEAdditionalFrame.h>
#include <netedit/frames/network/GNEConnectorFrame.h>
#include <netedit/frames/network/GNECreateEdgeFrame.h>
#include <netedit/frames/network/GNECrossingFrame.h>
#include <netedit/frames/network/GNEPolygonFrame.h>
#include <netedit/frames/network/GNEProhibitionFrame.h>
#include <netedit/frames/network/GNETAZFrame.h>
#include <netedit/frames/network/GNETLSEditorFrame.h>
#include <netedit/elements/network/GNEJunction.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEApplicationWindow.h"
#include "GNEViewNet.h"
#include "GNENet.h"
#include "GNEViewParent.h"
#include "GNEUndoList.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEViewParent) GNEViewParentMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_MAKESNAPSHOT,                       GNEViewParent::onCmdMakeSnapshot),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEJUNCTION,                     GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEEDGE,                         GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEVEHICLE,                      GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPERSON,                       GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEROUTE,                        GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATESTOP,                         GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATETLS,                          GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEADD,                          GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPOI,                          GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEPOLY,                         GNEViewParent::onCmdLocate),
    FXMAPFUNC(SEL_COMMAND,  FXMDIChild::ID_MDI_MENUCLOSE,           GNEViewParent::onCmdClose),
    FXMAPFUNC(SEL_CHANGED,  MID_GNE_VIEWPARENT_FRAMEAREAWIDTH,      GNEViewParent::onCmdUpdateFrameAreaWidth),
};

// Object implementation
FXIMPLEMENT(GNEViewParent, GUIGlChildWindow, GNEViewParentMap, ARRAYNUMBER(GNEViewParentMap))


// ===========================================================================
// member method definitions
// ===========================================================================

GNEViewParent::GNEViewParent(FXMDIClient* p, FXMDIMenu* mdimenu, const FXString& name, GNEApplicationWindow* parentWindow,
                             FXGLCanvas* share, GNENet* net, GNEUndoList* undoList, FXIcon* ic, FXuint opts, FXint x, FXint y, FXint w, FXint h) :
    GUIGlChildWindow(p, parentWindow, mdimenu, name, parentWindow->getToolbarsGrip().navigation, ic, opts, x, y, w, h),
    myGNEAppWindows(parentWindow) {
    // Add child to parent
    myParent->addGLChild(this);

    // Create Vertical separator
    new FXVerticalSeparator(myGripNavigationToolbar, GUIDesignVerticalSeparator);

    // Create undo/redo buttons
    myUndoButton = new FXButton(myGripNavigationToolbar, "\tUndo\tUndo the last Change.", GUIIconSubSys::getIcon(GUIIcon::UNDO), parentWindow, MID_HOTKEY_CTRL_Z_UNDO, GUIDesignButtonToolbar);
    myRedoButton = new FXButton(myGripNavigationToolbar, "\tRedo\tRedo the last Change.", GUIIconSubSys::getIcon(GUIIcon::REDO), parentWindow, MID_HOTKEY_CTRL_Y_REDO, GUIDesignButtonToolbar);

    // Create Frame Splitter
    myFramesSplitter = new FXSplitter(myContentFrame, this, MID_GNE_VIEWPARENT_FRAMEAREAWIDTH, GUIDesignSplitter | SPLITTER_HORIZONTAL);

    // Create frames Area
    myFramesArea = new FXHorizontalFrame(myFramesSplitter, GUIDesignFrameArea);

    // Set default width of frames area
    myFramesArea->setWidth(220);

    // Create view area
    myViewArea = new FXHorizontalFrame(myFramesSplitter, GUIDesignViewnArea);

    // Add the view to a temporary parent so that we can add items to myViewArea in the desired order
    FXComposite* tmp = new FXComposite(this);

    // Create view net
    GNEViewNet* viewNet = new GNEViewNet(tmp, myViewArea, *myParent, this, net, undoList, myParent->getGLVisual(), share);

    // show toolbar grips
    myGNEAppWindows->getToolbarsGrip().buildMenuToolbarsGrip();

    // Set pointer myView with the created view net
    myView = viewNet;

    // build frames
    myCommonFrames.buildCommonFrames(this, viewNet);
    myNetworkFrames.buildNetworkFrames(this, viewNet);
    myDemandFrames.buildDemandFrames(this, viewNet);
    myDataFrames.buildDataFrames(this, viewNet);

    // Update frame areas after creation
    onCmdUpdateFrameAreaWidth(nullptr, 0, nullptr);

    // Hidde all Frames Area
    hideFramesArea();

    // Build view toolBars
    myView->buildViewToolBars(this);

    // create windows
    GUIGlChildWindow::create();
}


GNEViewParent::~GNEViewParent() {
    // delete toolbar grips
    myGNEAppWindows->getToolbarsGrip().destroyParentToolbarsGrips();
    // Remove child before remove
    myParent->removeGLChild(this);
}


void
GNEViewParent::hideAllFrames() {
    myCommonFrames.hideCommonFrames();
    myNetworkFrames.hideNetworkFrames();
    myDemandFrames.hideDemandFrames();
    myDataFrames.hideDataFrames();
}


GNEFrame*
GNEViewParent::getCurrentShownFrame() const {
    if (myCommonFrames.isCommonFrameShown()) {
        return myCommonFrames.getCurrentShownFrame();
    } else if (myNetworkFrames.isNetworkFrameShown()) {
        return myNetworkFrames.getCurrentShownFrame();
    } else if (myDemandFrames.isDemandFrameShown()) {
        return myDemandFrames.getCurrentShownFrame();
    } else if (myDataFrames.isDataFrameShown()) {
        return myDataFrames.getCurrentShownFrame();
    } else {
        return nullptr;
    }
}


GNEInspectorFrame*
GNEViewParent::getInspectorFrame() const {
    return myCommonFrames.inspectorFrame;
}


GNEDeleteFrame*
GNEViewParent::getDeleteFrame() const {
    return myCommonFrames.deleteFrame;
}


GNESelectorFrame*
GNEViewParent::getSelectorFrame() const {
    return myCommonFrames.selectorFrame;
}


GNEMoveFrame*
GNEViewParent::getMoveFrame() const {
    return myCommonFrames.moveFrame;
}


GNEConnectorFrame*
GNEViewParent::getConnectorFrame() const {
    return myNetworkFrames.connectorFrame;
}


GNETLSEditorFrame*
GNEViewParent::getTLSEditorFrame() const {
    return myNetworkFrames.TLSEditorFrame;
}


GNEAdditionalFrame*
GNEViewParent::getAdditionalFrame() const {
    return myNetworkFrames.additionalFrame;
}


GNECrossingFrame*
GNEViewParent::getCrossingFrame() const {
    return myNetworkFrames.crossingFrame;
}


GNETAZFrame*
GNEViewParent::getTAZFrame() const {
    return myNetworkFrames.TAZFrame;
}


GNEPolygonFrame*
GNEViewParent::getPolygonFrame() const {
    return myNetworkFrames.polygonFrame;
}


GNEProhibitionFrame*
GNEViewParent::getProhibitionFrame() const {
    return myNetworkFrames.prohibitionFrame;
}


GNECreateEdgeFrame*
GNEViewParent::getCreateEdgeFrame() const {
    return myNetworkFrames.createEdgeFrame;
}


GNERouteFrame*
GNEViewParent::getRouteFrame() const {
    return myDemandFrames.routeFrame;
}


GNEVehicleFrame*
GNEViewParent::getVehicleFrame() const {
    return myDemandFrames.vehicleFrame;
}


GNEVehicleTypeFrame*
GNEViewParent::getVehicleTypeFrame() const {
    return myDemandFrames.vehicleTypeFrame;
}


GNEStopFrame*
GNEViewParent::getStopFrame() const {
    return myDemandFrames.stopFrame;
}


GNEPersonTypeFrame*
GNEViewParent::getPersonTypeFrame() const {
    return myDemandFrames.personTypeFrame;
}


GNEPersonFrame*
GNEViewParent::getPersonFrame() const {
    return myDemandFrames.personFrame;
}


GNEPersonPlanFrame*
GNEViewParent::getPersonPlanFrame() const {
    return myDemandFrames.personPlanFrame;
}


GNEEdgeDataFrame*
GNEViewParent::getEdgeDataFrame() const {
    return myDataFrames.edgeDataFrame;
}


GNEEdgeRelDataFrame*
GNEViewParent::getEdgeRelDataFrame() const {
    return myDataFrames.edgeRelDataFrame;
}


GNETAZRelDataFrame*
GNEViewParent::getTAZRelDataFrame() const {
    return myDataFrames.TAZRelDataFrame;
}


void
GNEViewParent::showFramesArea() {
    const bool frameShown = myCommonFrames.isCommonFrameShown() ||
                            myNetworkFrames.isNetworkFrameShown() ||
                            myDemandFrames.isDemandFrameShown() ||
                            myDataFrames.isDataFrameShown();
    // show and recalc framesArea if at least there is a frame shown
    if (frameShown) {
        myFramesArea->recalc();
        myFramesArea->show();
    }
}


void
GNEViewParent::hideFramesArea() {
    const bool frameShown = myCommonFrames.isCommonFrameShown() ||
                            myNetworkFrames.isNetworkFrameShown() ||
                            myDemandFrames.isDemandFrameShown() ||
                            myDataFrames.isDataFrameShown();
    // hide and recalc frames Area if all frames are hidden is enabled
    if (!frameShown) {
        myFramesArea->hide();
        myFramesArea->recalc();
    }
}


GUIMainWindow*
GNEViewParent::getGUIMainWindow() const {
    return myParent;
}


GNEApplicationWindow*
GNEViewParent::getGNEAppWindows() const {
    return myGNEAppWindows;
}


void
GNEViewParent::eraseACChooserDialog(GNEDialogACChooser* chooserDialog) {
    if (chooserDialog == nullptr) {
        throw ProcessError("ChooserDialog already deleted");
    } else if (chooserDialog == myACChoosers.ACChooserJunction) {
        myACChoosers.ACChooserJunction = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserEdges) {
        myACChoosers.ACChooserEdges = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserVehicles) {
        myACChoosers.ACChooserVehicles = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserPersons) {
        myACChoosers.ACChooserPersons = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserRoutes) {
        myACChoosers.ACChooserRoutes = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserStops) {
        myACChoosers.ACChooserStops = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserTLS) {
        myACChoosers.ACChooserTLS = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserAdditional) {
        myACChoosers.ACChooserAdditional = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserPOI) {
        myACChoosers.ACChooserPOI = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserPolygon) {
        myACChoosers.ACChooserPolygon = nullptr;
    } else if (chooserDialog == myACChoosers.ACChooserProhibition) {
        myACChoosers.ACChooserProhibition = nullptr;
    } else {
        throw ProcessError("Unregistered chooserDialog");
    }
}


void
GNEViewParent::updateUndoRedoButtons() {
    myGNEAppWindows->getUndoList()->p_onUpdUndo(myUndoButton, 0, nullptr);
    myGNEAppWindows->getUndoList()->p_onUpdRedo(myRedoButton, 0, nullptr);
}


long
GNEViewParent::onCmdMakeSnapshot(FXObject*, FXSelector, void*) {
    // get the new file name
    FXFileDialog opendialog(this, "Save Snapshot");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_ANY);
    opendialog.setPatternList("All Image Files (*.gif, *.bmp, *.xpm, *.pcx, *.ico, *.rgb, *.xbm, *.tga, *.png, *.jpg, *.jpeg, *.tif, *.tiff, *.ps, *.eps, *.pdf, *.svg, *.tex, *.pgf)\n"
                              "GIF Image (*.gif)\nBMP Image (*.bmp)\nXPM Image (*.xpm)\nPCX Image (*.pcx)\nICO Image (*.ico)\n"
                              "RGB Image (*.rgb)\nXBM Image (*.xbm)\nTARGA Image (*.tga)\nPNG Image  (*.png)\n"
                              "JPEG Image (*.jpg, *.jpeg)\nTIFF Image (*.tif, *.tiff)\n"
                              "Postscript (*.ps)\nEncapsulated Postscript (*.eps)\nPortable Document Format (*.pdf)\n"
                              "Scalable Vector Graphics (*.svg)\nLATEX text strings (*.tex)\nPortable LaTeX Graphics (*.pgf)\n"
                              "All Files (*)");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (!opendialog.execute() || !MFXUtils::userPermitsOverwritingWhenFileExists(this, opendialog.getFilename())) {
        return 1;
    }
    gCurrentFolder = opendialog.getDirectory();
    std::string file = opendialog.getFilename().text();
    std::string error = myView->makeSnapshot(file);
    if (error != "") {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox 'error saving snapshot'");
        // open message box
        FXMessageBox::error(this, MBOX_OK, "Saving failed.", "%s", error.c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox 'error saving snapshot' with 'OK'");
    }
    return 1;
}


long
GNEViewParent::onCmdClose(FXObject*, FXSelector /* sel */, void*) {
    myParent->handle(this, FXSEL(SEL_COMMAND, MID_HOTKEY_CTRL_W_CLOSESIMULATION), nullptr);
    return 1;
}


long
GNEViewParent::onCmdLocate(FXObject*, FXSelector sel, void*) {
    GNEViewNet* viewNet = dynamic_cast<GNEViewNet*>(myView);
    // check that viewNet exist
    if (viewNet) {
        // declare a vector in which save attribute carriers to locate
        std::vector<GNEAttributeCarrier*> ACsToLocate;
        switch (FXSELID(sel)) {
            case MID_LOCATEJUNCTION: {
                if (myACChoosers.ACChooserJunction) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserJunction->setFocus();
                } else {
                    // fill ACsToLocate with junctions
                    std::vector<GNEJunction*> junctions = viewNet->getNet()->retrieveJunctions();
                    ACsToLocate.reserve(junctions.size());
                    for (auto i : junctions) {
                        ACsToLocate.push_back(i);
                    }
                    myACChoosers.ACChooserJunction = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEJUNCTION), "Junction Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEEDGE: {
                if (myACChoosers.ACChooserEdges) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserEdges->setFocus();
                } else {
                    // fill ACsToLocate with edges
                    std::vector<GNEEdge*> edges = viewNet->getNet()->retrieveEdges();
                    ACsToLocate.reserve(edges.size());
                    for (auto i : edges) {
                        ACsToLocate.push_back(i);
                    }
                    myACChoosers.ACChooserEdges = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEEDGE), "Edge Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEVEHICLE: {
                if (myACChoosers.ACChooserVehicles) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserVehicles->setFocus();
                } else {
                    // reserve memory
                    ACsToLocate.reserve(viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VEHICLE).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_TRIP).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_FLOW_ROUTE).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_FLOW).size());
                    // fill ACsToLocate with vehicles
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VEHICLE)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with vehicles
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_TRIP)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with routeFlows
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(GNE_TAG_FLOW_ROUTE)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with routeFlowsFromTo
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_FLOW)) {
                        ACsToLocate.push_back(i.second);
                    }
                    myACChoosers.ACChooserVehicles = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEVEHICLE), "Vehicle Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEPERSON: {
                if (myACChoosers.ACChooserPersons) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserPersons->setFocus();
                } else {
                    // reserve memory
                    ACsToLocate.reserve(viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSON).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSONFLOW).size());
                    // fill ACsToLocate with persons
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSON)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with personFlows
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_PERSONFLOW)) {
                        ACsToLocate.push_back(i.second);
                    }
                    myACChoosers.ACChooserPersons = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEPERSON), "Person Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEROUTE: {
                if (myACChoosers.ACChooserRoutes) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserRoutes->setFocus();
                } else {
                    // reserve memory
                    ACsToLocate.reserve(viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_ROUTE).size());
                    // fill ACsToLocate with routes
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_ROUTE)) {
                        ACsToLocate.push_back(i.second);
                    }
                    myACChoosers.ACChooserRoutes = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEROUTE), "Route Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATESTOP: {
                if (myACChoosers.ACChooserStops) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserStops->setFocus();
                } else {
                    // reserve memory
                    ACsToLocate.reserve(viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_LANE).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_BUSSTOP).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_CONTAINERSTOP).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_CHARGINGSTATION).size() +
                                        viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_PARKINGAREA).size());
                    // fill ACsToLocate with stop over lanes
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_LANE)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with stop over busstops
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_BUSSTOP)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with stop over container stops
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_CONTAINERSTOP)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with stop over charging stations
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_CHARGINGSTATION)) {
                        ACsToLocate.push_back(i.second);
                    }
                    // fill ACsToLocate with stop over parking areas
                    for (const auto& i : viewNet->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_STOP_PARKINGAREA)) {
                        ACsToLocate.push_back(i.second);
                    }
                    myACChoosers.ACChooserStops = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATESTOP), "Stop Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATETLS: {
                if (myACChoosers.ACChooserTLS) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserTLS->setFocus();
                } else {
                    // fill ACsToLocate with junctions that haven TLS
                    std::vector<GNEJunction*> junctions = viewNet->getNet()->retrieveJunctions();
                    ACsToLocate.reserve(junctions.size());
                    for (auto i : junctions) {
                        if (i->getNBNode()->getControllingTLS().size() > 0) {
                            ACsToLocate.push_back(i);
                        }
                    }
                    myACChoosers.ACChooserTLS = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATETLS), "TLS Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEADD: {
                if (myACChoosers.ACChooserAdditional) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserAdditional->setFocus();
                } else {
                    // fill ACsToLocate with additionals
                    for (const auto& additionalTag : viewNet->getNet()->getAttributeCarriers()->getAdditionals()) {
                        for (const auto& additional : additionalTag.second) {
                            ACsToLocate.push_back(additional.second);
                        }
                    }
                    myACChoosers.ACChooserAdditional = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEADD), "Additional Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEPOI: {
                if (myACChoosers.ACChooserPOI) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserPOI->setFocus();
                } else {
                    // fill ACsToLocate with POIs
                    for (const auto& POI : viewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POI)) {
                        ACsToLocate.push_back(POI.second);
                    }
                    for (const auto& POILane : viewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POILANE)) {
                        ACsToLocate.push_back(POILane.second);
                    }
                    myACChoosers.ACChooserPOI = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEPOI), "POI Chooser", ACsToLocate);
                }
                break;
            }
            case MID_LOCATEPOLY: {
                if (myACChoosers.ACChooserPolygon) {
                    // set focus in the existent chooser dialog
                    myACChoosers.ACChooserPolygon->setFocus();
                } else {
                    // fill ACsToLocate with polys
                    for (const auto& polygon : viewNet->getNet()->getAttributeCarriers()->getShapes().at(SUMO_TAG_POLY)) {
                        ACsToLocate.push_back(polygon.second);
                    }
                    myACChoosers.ACChooserPolygon = new GNEDialogACChooser(this, GUIIconSubSys::getIcon(GUIIcon::LOCATEPOLY), "Poly Chooser", ACsToLocate);
                }
                break;
            }
            default:
                throw ProcessError("Unknown Message ID in onCmdLocate");
        }
        // update locator popup
        myLocatorPopup->popdown();
        myLocatorButton->killFocus();
        myLocatorPopup->update();
    }
    return 1;
}


long
GNEViewParent::onKeyPress(FXObject* o, FXSelector sel, void* eventData) {
    myView->onKeyPress(o, sel, eventData);
    return 0;
}


long
GNEViewParent::onKeyRelease(FXObject* o, FXSelector sel, void* eventData) {
    myView->onKeyRelease(o, sel, eventData);
    return 0;
}


long
GNEViewParent::onCmdUpdateFrameAreaWidth(FXObject*, FXSelector, void*) {
    const int framesAreaWidth = myFramesArea->getWidth();
    // set width of FramesArea in all frames
    myCommonFrames.setCommonFramesWidth(framesAreaWidth);
    myNetworkFrames.setNetworkFramesWidth(framesAreaWidth);
    myDemandFrames.setDemandFramesWidth(framesAreaWidth);
    myDataFrames.setDataFramesWidth(framesAreaWidth);
    return 0;
}

// ---------------------------------------------------------------------------
// GNEViewParent::CommonFrames - methods
// ---------------------------------------------------------------------------

GNEViewParent::CommonFrames::CommonFrames() :
    inspectorFrame(nullptr),
    deleteFrame(nullptr),
    selectorFrame(nullptr),
    moveFrame(nullptr) {
}


void
GNEViewParent::CommonFrames::buildCommonFrames(GNEViewParent* viewParent, GNEViewNet* viewNet) {
    inspectorFrame = new GNEInspectorFrame(viewParent->myFramesArea, viewNet);
    selectorFrame = new GNESelectorFrame(viewParent->myFramesArea, viewNet);
    moveFrame = new GNEMoveFrame(viewParent->myFramesArea, viewNet);
    deleteFrame = new GNEDeleteFrame(viewParent->myFramesArea, viewNet);
}


void
GNEViewParent::CommonFrames::hideCommonFrames() {
    inspectorFrame->hide();
    deleteFrame->hide();
    selectorFrame->hide();
    moveFrame->hide();
}


void
GNEViewParent::CommonFrames::setCommonFramesWidth(int frameWidth) {
    // set width in all frames
    inspectorFrame->setFrameWidth(frameWidth);
    deleteFrame->setFrameWidth(frameWidth);
    selectorFrame->setFrameWidth(frameWidth);
    moveFrame->setFrameWidth(frameWidth);
}


bool
GNEViewParent::CommonFrames::isCommonFrameShown() const {
    // check all frames
    if (inspectorFrame->shown()) {
        return true;
    } else if (deleteFrame->shown()) {
        return true;
    } else if (selectorFrame->shown()) {
        return true;
    } else if (moveFrame->shown()) {
        return true;
    } else {
        return false;
    }
}


GNEFrame*
GNEViewParent::CommonFrames::getCurrentShownFrame() const {
    // check all frames
    if (inspectorFrame->shown()) {
        return inspectorFrame;
    } else if (deleteFrame->shown()) {
        return deleteFrame;
    } else if (selectorFrame->shown()) {
        return selectorFrame;
    } else if (moveFrame->shown()) {
        return moveFrame;
    } else {
        return nullptr;
    }
}

// ---------------------------------------------------------------------------
// GNEViewParent::NetworkFrames - methods
// ---------------------------------------------------------------------------

GNEViewParent::NetworkFrames::NetworkFrames() :
    connectorFrame(nullptr),
    TLSEditorFrame(nullptr),
    additionalFrame(nullptr),
    crossingFrame(nullptr),
    TAZFrame(nullptr),
    polygonFrame(nullptr),
    prohibitionFrame(nullptr),
    createEdgeFrame(nullptr) {
}


void
GNEViewParent::NetworkFrames::buildNetworkFrames(GNEViewParent* viewParent, GNEViewNet* viewNet) {
    connectorFrame = new GNEConnectorFrame(viewParent->myFramesArea, viewNet);
    prohibitionFrame = new GNEProhibitionFrame(viewParent->myFramesArea, viewNet);
    TLSEditorFrame = new GNETLSEditorFrame(viewParent->myFramesArea, viewNet);
    additionalFrame = new GNEAdditionalFrame(viewParent->myFramesArea, viewNet);
    crossingFrame = new GNECrossingFrame(viewParent->myFramesArea, viewNet);
    TAZFrame = new GNETAZFrame(viewParent->myFramesArea, viewNet);
    polygonFrame = new GNEPolygonFrame(viewParent->myFramesArea, viewNet);
    createEdgeFrame = new GNECreateEdgeFrame(viewParent->myFramesArea, viewNet);
}


void
GNEViewParent::NetworkFrames::hideNetworkFrames() {
    connectorFrame->hide();
    TLSEditorFrame->hide();
    additionalFrame->hide();
    crossingFrame->hide();
    TAZFrame->hide();
    polygonFrame->hide();
    prohibitionFrame->hide();
    createEdgeFrame->hide();
}


void
GNEViewParent::NetworkFrames::setNetworkFramesWidth(int frameWidth) {
    // set width in all frames
    connectorFrame->setFrameWidth(frameWidth);
    TLSEditorFrame->setFrameWidth(frameWidth);
    additionalFrame->setFrameWidth(frameWidth);
    crossingFrame->setFrameWidth(frameWidth);
    TAZFrame->setFrameWidth(frameWidth);
    polygonFrame->setFrameWidth(frameWidth);
    prohibitionFrame->setFrameWidth(frameWidth);
    createEdgeFrame->setFrameWidth(frameWidth);
}


bool
GNEViewParent::NetworkFrames::isNetworkFrameShown() const {
    // check all frames
    if (connectorFrame->shown()) {
        return true;
    } else if (TLSEditorFrame->shown()) {
        return true;
    } else if (additionalFrame->shown()) {
        return true;
    } else if (crossingFrame->shown()) {
        return true;
    } else if (TAZFrame->shown()) {
        return true;
    } else if (polygonFrame->shown()) {
        return true;
    } else if (prohibitionFrame->shown()) {
        return true;
    } else if (createEdgeFrame->shown()) {
        return true;
    } else {
        return false;
    }
}


GNEFrame*
GNEViewParent::NetworkFrames::getCurrentShownFrame() const {
    // check all frames
    if (connectorFrame->shown()) {
        return connectorFrame;
    } else if (TLSEditorFrame->shown()) {
        return TLSEditorFrame;
    } else if (additionalFrame->shown()) {
        return additionalFrame;
    } else if (crossingFrame->shown()) {
        return crossingFrame;
    } else if (TAZFrame->shown()) {
        return TAZFrame;
    } else if (polygonFrame->shown()) {
        return polygonFrame;
    } else if (prohibitionFrame->shown()) {
        return prohibitionFrame;
    } else if (createEdgeFrame->shown()) {
        return createEdgeFrame;
    } else {
        return nullptr;
    }
}

// ---------------------------------------------------------------------------
// GNEViewParent::DemandFrames - methods
// ---------------------------------------------------------------------------

GNEViewParent::DemandFrames::DemandFrames() :
    routeFrame(nullptr),
    vehicleFrame(nullptr),
    vehicleTypeFrame(nullptr),
    stopFrame(nullptr),
    personFrame(nullptr),
    personTypeFrame(nullptr),
    personPlanFrame(nullptr) {
}


void
GNEViewParent::DemandFrames::buildDemandFrames(GNEViewParent* viewParent, GNEViewNet* viewNet) {
    routeFrame = new GNERouteFrame(viewParent->myFramesArea, viewNet);
    vehicleFrame = new GNEVehicleFrame(viewParent->myFramesArea, viewNet);
    vehicleTypeFrame = new GNEVehicleTypeFrame(viewParent->myFramesArea, viewNet);
    stopFrame = new GNEStopFrame(viewParent->myFramesArea, viewNet);
    personTypeFrame = new GNEPersonTypeFrame(viewParent->myFramesArea, viewNet);
    personFrame = new GNEPersonFrame(viewParent->myFramesArea, viewNet);
    personPlanFrame = new GNEPersonPlanFrame(viewParent->myFramesArea, viewNet);
}


void
GNEViewParent::DemandFrames::hideDemandFrames() {
    routeFrame->hide();
    vehicleFrame->hide();
    vehicleTypeFrame->hide();
    stopFrame->hide();
    personTypeFrame->hide();
    personFrame->hide();
    personPlanFrame->hide();
}


void
GNEViewParent::DemandFrames::setDemandFramesWidth(int frameWidth) {
    // set width in all frames
    routeFrame->setFrameWidth(frameWidth);
    vehicleFrame->setFrameWidth(frameWidth);
    vehicleTypeFrame->setFrameWidth(frameWidth);
    stopFrame->setFrameWidth(frameWidth);
    personTypeFrame->setFrameWidth(frameWidth);
    personFrame->setFrameWidth(frameWidth);
    personPlanFrame->setFrameWidth(frameWidth);
}


bool
GNEViewParent::DemandFrames::isDemandFrameShown() const {
    // check all frames
    if (routeFrame->shown()) {
        return true;
    } else if (vehicleFrame->shown()) {
        return true;
    } else if (vehicleTypeFrame->shown()) {
        return true;
    } else if (stopFrame->shown()) {
        return true;
    } else if (personTypeFrame->shown()) {
        return true;
    } else if (personFrame->shown()) {
        return true;
    } else if (personPlanFrame->shown()) {
        return true;
    } else {
        return false;
    }
}


GNEFrame*
GNEViewParent::DemandFrames::getCurrentShownFrame() const {
    // check all frames
    if (routeFrame->shown()) {
        return routeFrame;
    } else if (vehicleFrame->shown()) {
        return vehicleFrame;
    } else if (vehicleTypeFrame->shown()) {
        return vehicleTypeFrame;
    } else if (personTypeFrame->shown()) {
        return personTypeFrame;
    } else if (stopFrame->shown()) {
        return stopFrame;
    } else if (personFrame->shown()) {
        return personFrame;
    } else if (personPlanFrame->shown()) {
        return personPlanFrame;
    } else {
        return nullptr;
    }
}

// ---------------------------------------------------------------------------
// GNEViewParent::DataFrames - methods
// ---------------------------------------------------------------------------

GNEViewParent::DataFrames::DataFrames() :
    edgeDataFrame(nullptr),
    edgeRelDataFrame(nullptr),
    TAZRelDataFrame(nullptr) {
}


void
GNEViewParent::DataFrames::buildDataFrames(GNEViewParent* viewParent, GNEViewNet* viewNet) {
    edgeDataFrame = new GNEEdgeDataFrame(viewParent->myFramesArea, viewNet);
    edgeRelDataFrame = new GNEEdgeRelDataFrame(viewParent->myFramesArea, viewNet);
    TAZRelDataFrame = new GNETAZRelDataFrame(viewParent->myFramesArea, viewNet);
}


void
GNEViewParent::DataFrames::hideDataFrames() {
    edgeDataFrame->hide();
    edgeRelDataFrame->hide();
    TAZRelDataFrame->hide();
}


void
GNEViewParent::DataFrames::setDataFramesWidth(int frameWidth) {
    // set width in all frames
    edgeDataFrame->setFrameWidth(frameWidth);
    edgeRelDataFrame->setFrameWidth(frameWidth);
    TAZRelDataFrame->setFrameWidth(frameWidth);
}


bool
GNEViewParent::DataFrames::isDataFrameShown() const {
    // check all frames
    if (edgeDataFrame->shown()) {
        return true;
    } else if (edgeRelDataFrame->shown()) {
        return true;
    } else if (TAZRelDataFrame->shown()) {
        return true;
    } else {
        return false;
    }
}


GNEFrame*
GNEViewParent::DataFrames::getCurrentShownFrame() const {
    // check all frames
    if (edgeDataFrame->shown()) {
        return edgeDataFrame;
    } else if (edgeRelDataFrame->shown()) {
        return edgeRelDataFrame;
    } else if (TAZRelDataFrame->shown()) {
        return TAZRelDataFrame;
    } else {
        return nullptr;
    }
}

// ---------------------------------------------------------------------------
// GNEViewParent::ACChoosers - methods
// ---------------------------------------------------------------------------

GNEViewParent::ACChoosers::ACChoosers() :
    ACChooserJunction(nullptr),
    ACChooserEdges(nullptr),
    ACChooserVehicles(nullptr),
    ACChooserPersons(nullptr),
    ACChooserRoutes(nullptr),
    ACChooserStops(nullptr),
    ACChooserTLS(nullptr),
    ACChooserAdditional(nullptr),
    ACChooserPOI(nullptr),
    ACChooserPolygon(nullptr),
    ACChooserProhibition(nullptr) {
}


GNEViewParent::ACChoosers::~ACChoosers() {
    // remove all  dialogs if are active
    if (ACChooserJunction) {
        delete ACChooserJunction;
    }
    if (ACChooserEdges) {
        delete ACChooserEdges;
    }
    if (ACChooserRoutes) {
        delete ACChooserRoutes;
    }
    if (ACChooserStops) {
        delete ACChooserStops;
    }
    if (ACChooserVehicles) {
        delete ACChooserVehicles;
    }
    if (ACChooserPersons) {
        delete ACChooserPersons;
    }
    if (ACChooserTLS) {
        delete ACChooserTLS;
    }
    if (ACChooserAdditional) {
        delete ACChooserAdditional;
    }
    if (ACChooserPOI) {
        delete ACChooserPOI;
    }
    if (ACChooserPolygon) {
        delete ACChooserPolygon;
    }
    if (ACChooserProhibition) {
        delete ACChooserProhibition;
    }
}


/****************************************************************************/
