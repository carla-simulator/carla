/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUIDialog_EditViewport.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Mon, 25.04.2005
///
// A dialog to change the viewport
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/geom/Position.h>
#include <utils/foxtools/MFXUtils.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/settings/GUISettingsHandler.h>
#include <utils/options/OptionsCont.h>

#include "GUISUMOAbstractView.h"
#include "GUIDialog_EditViewport.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIDialog_EditViewport) GUIDialog_EditViewportMap[] = {
    FXMAPFUNC(SEL_COMMAND, GUIDialog_EditViewport::MID_CHANGED, GUIDialog_EditViewport::onCmdChanged),
    FXMAPFUNC(SEL_COMMAND, GUIDialog_EditViewport::MID_OK,      GUIDialog_EditViewport::onCmdOk),
    FXMAPFUNC(SEL_COMMAND, GUIDialog_EditViewport::MID_CANCEL,  GUIDialog_EditViewport::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND, GUIDialog_EditViewport::MID_LOAD,    GUIDialog_EditViewport::onCmdLoad),
    FXMAPFUNC(SEL_COMMAND, GUIDialog_EditViewport::MID_SAVE,    GUIDialog_EditViewport::onCmdSave),
};

// Object implementation
FXIMPLEMENT(GUIDialog_EditViewport, FXDialogBox, GUIDialog_EditViewportMap, ARRAYNUMBER(GUIDialog_EditViewportMap))

// ===========================================================================
// method definitions
// ===========================================================================

GUIDialog_EditViewport::GUIDialog_EditViewport(GUISUMOAbstractView* parent, const char* name, int x, int y) :
    FXDialogBox(parent, name, GUIDesignDialogBox, x, y, 0, 0, 0, 0, 0, 0),
    myParent(parent) {
    // create contents frame
    FXVerticalFrame* contentsFrame = new FXVerticalFrame(this, GUIDesignContentsFrame);
    // create frame for file icons
    FXHorizontalFrame* frameFiles = new FXHorizontalFrame(contentsFrame, GUIDesignHorizontalFrameIcons);
    myLoadButton = new FXButton(frameFiles, "Load\t\tLoad viewport from file",
                                GUIIconSubSys::getIcon(GUIIcon::OPEN_CONFIG), this, GUIDialog_EditViewport::MID_LOAD, GUIDesignButtonToolbarWithText);
    mySaveButton = new FXButton(frameFiles, "Save\t\tSave viewport to file",
                                GUIIconSubSys::getIcon(GUIIcon::SAVE), this, GUIDialog_EditViewport::MID_SAVE, GUIDesignButtonToolbarWithText);
    // create horizontalframe for zoom elements and OSG
    FXHorizontalFrame* editElementsFrame = new FXHorizontalFrame(contentsFrame, GUIDesignAuxiliarHorizontalFrame);

    // create vertical frame for XYZ values
    FXVerticalFrame* lookFromFrame = new FXVerticalFrame(editElementsFrame, GUIDesignAuxiliarVerticalFrame);

    // create zoom elements
    FXHorizontalFrame* zoomFrame = new FXHorizontalFrame(lookFromFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(zoomFrame, "Zoom:", nullptr, GUIDesignLabelLeftThick);
    myZoom = new FXRealSpinner(zoomFrame, 16, this, MID_CHANGED, GUIDesignSpinDial);
    myZoom->setRange(0.0001, 100000);
    //myZoom->setNumberFormat(4);

    // create lookFromX elements
    FXHorizontalFrame* lookFromXFrame = new FXHorizontalFrame(lookFromFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(lookFromXFrame, "X:", nullptr, GUIDesignLabelLeftThick);
    myXOff = new FXRealSpinner(lookFromXFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);

    // create lookFromY elements
    FXHorizontalFrame* lookFromYFrame = new FXHorizontalFrame(lookFromFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(lookFromYFrame, "Y:", nullptr, GUIDesignLabelLeftThick);
    myYOff = new FXRealSpinner(lookFromYFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);

    // create lookFromZ elements
    FXHorizontalFrame* lookFromZFrame = new FXHorizontalFrame(lookFromFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(lookFromZFrame, "Z:", nullptr, GUIDesignLabelLeftThick);
    myZOff = new FXRealSpinner(lookFromZFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);
    myZOff->setRange(0.12, 100000000);

    // create rotation elements
    FXHorizontalFrame* rotationFrame = new FXHorizontalFrame(lookFromFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(rotationFrame, "A:", nullptr, GUIDesignLabelLeftThick);
    myRotation = new FXRealSpinner(rotationFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);

    // create vertical frame for OSG
    FXVerticalFrame* lookAtFrame = new FXVerticalFrame(editElementsFrame, GUIDesignAuxiliarVerticalFrame);
    new FXLabel(lookAtFrame, "OSG", nullptr, GUIDesignLabelCenterThick);

    // create lookAtX elements
    FXHorizontalFrame* lookAtXFrame = new FXHorizontalFrame(lookAtFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(lookAtXFrame, "LookAtX:", nullptr, GUIDesignLabelLeftThick);
    myLookAtX = new FXRealSpinner(lookAtXFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);

    // create lookAtY elements
    FXHorizontalFrame* lookAtYFrame = new FXHorizontalFrame(lookAtFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(lookAtYFrame, "LookAtY:", nullptr, GUIDesignLabelLeftThick);
    myLookAtY = new FXRealSpinner(lookAtYFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);

    // create lookAtZ elements
    FXHorizontalFrame* lookAtZFrame = new FXHorizontalFrame(lookAtFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(lookAtZFrame, "LookAtZ:", nullptr, GUIDesignLabelLeftThick);
    myLookAtZ = new FXRealSpinner(lookAtZFrame, 16, this, MID_CHANGED, GUIDesignSpinDial | SPIN_NOMIN | SPIN_NOMAX);

    // only show LookAt elements if OSG is enabled
#ifdef HAVE_OSG
    lookAtFrame->show();
#else
    lookAtFrame->hide();
#endif

    // create buttons ok/cancel
    new FXHorizontalSeparator(contentsFrame, GUIDesignHorizontalSeparator);
    FXHorizontalFrame* frameButtons = new FXHorizontalFrame(contentsFrame, GUIDesignAuxiliarHorizontalFrame);
    new FXHorizontalFrame(frameButtons, GUIDesignAuxiliarHorizontalFrame);
    myOKButton = new FXButton(frameButtons, "&OK\t\taccept", GUIIconSubSys::getIcon(GUIIcon::ACCEPT), this, GUIDialog_EditViewport::MID_OK, GUIDesignButtonOK);
    myCancelButton = new FXButton(frameButtons, "&Cancel\t\tclose", GUIIconSubSys::getIcon(GUIIcon::CANCEL), this, GUIDialog_EditViewport::MID_CANCEL, GUIDesignButtonCancel);
    new FXHorizontalFrame(frameButtons, GUIDesignAuxiliarHorizontalFrame);
    // set dialog icon
    setIcon(GUIIconSubSys::getIcon(GUIIcon::EDITVIEWPORT));
}


GUIDialog_EditViewport::~GUIDialog_EditViewport() {}


void
GUIDialog_EditViewport::show() {
    // If testing mode is enabled, we need to place focus in the Z dial
    if (OptionsCont::getOptions().getBool("gui-testing")) {
        myLoadButton->setFocus();
    } else {
        myOKButton->setFocus();
    }
    FXDialogBox::show();
}


long
GUIDialog_EditViewport::onCmdOk(FXObject*, FXSelector, void*) {
    myParent->setViewportFromToRot(Position(myXOff->getValue(), myYOff->getValue(), myZOff->getValue()),
#ifdef HAVE_OSG
                                   Position(myLookAtX->getValue(), myLookAtY->getValue(), myLookAtZ->getValue())
#else
                                   Position::INVALID
#endif
                                   , myRotation->getValue()
                                  );
    // write information of current zoom status
    WRITE_DEBUG("Current Viewport values: " + toString(myXOff->getValue()) + ", " + toString(myYOff->getValue()) + ", " + toString(myZOff->getValue()) +
                ". Zoom = '" + toString(myZoom->getValue()) + "'");
    saveWindowPos();
    hide();
    return 1;
}


long
GUIDialog_EditViewport::onCmdCancel(FXObject*, FXSelector, void*) {
    myParent->setViewportFromToRot(myOldLookFrom, myOldLookAt, myOldRotation);
    saveWindowPos();
    hide();
    return 1;
}


long
GUIDialog_EditViewport::onCmdChanged(FXObject* o, FXSelector, void*) {
    if (o == myZOff) {
        myZoom->setValue(myParent->getChanger().zPos2Zoom(myZOff->getValue()));
    } else if (o == myZoom) {
        myZOff->setValue(myParent->getChanger().zoom2ZPos(myZoom->getValue()));
    }
    myParent->setViewportFromToRot(Position(myXOff->getValue(), myYOff->getValue(), myZOff->getValue()),
#ifdef HAVE_OSG
                                   Position(myLookAtX->getValue(), myLookAtY->getValue(), myLookAtZ->getValue())
#else
                                   Position::INVALID
#endif
                                   , myRotation->getValue()
                                  );
    return 1;
}


long
GUIDialog_EditViewport::onCmdLoad(FXObject*, FXSelector, void*) {
    FXFileDialog opendialog(this, "Load Viewport");
    opendialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::EMPTY));
    opendialog.setSelectMode(SELECTFILE_ANY);
    opendialog.setPatternList("*.xml");
    if (gCurrentFolder.length() != 0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (opendialog.execute()) {
        gCurrentFolder = opendialog.getDirectory();
        GUISettingsHandler handler(opendialog.getFilename().text());
        handler.applyViewport(myParent);
        setValues(myParent->getChanger().getZoom(), myParent->getChanger().getXPos(), myParent->getChanger().getYPos(), myParent->getChanger().getRotation());
    }
    return 1;
}


long
GUIDialog_EditViewport::onCmdSave(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this, "Save Viewport", ".xml", GUIIconSubSys::getIcon(GUIIcon::EMPTY), gCurrentFolder);
    if (file == "") {
        return 1;
    }
    try {
        OutputDevice& dev = OutputDevice::getDevice(file.text());
        dev.openTag(SUMO_TAG_VIEWSETTINGS);
        writeXML(dev);
        dev.closeTag();
        dev.close();
    } catch (IOError& e) {
        FXMessageBox::error(this, MBOX_OK, "Storing failed!", "%s", e.what());
    }
    return 1;
}


void
GUIDialog_EditViewport::writeXML(OutputDevice& dev) {
    dev.openTag(SUMO_TAG_VIEWPORT);
    dev.writeAttr(SUMO_ATTR_ZOOM, myZoom->getValue());
    dev.writeAttr(SUMO_ATTR_X, myXOff->getValue());
    dev.writeAttr(SUMO_ATTR_Y, myYOff->getValue());
    dev.writeAttr(SUMO_ATTR_ANGLE, myRotation->getValue());
#ifdef HAVE_OSG
    dev.writeAttr(SUMO_ATTR_CENTER_X, myLookAtX->getValue());
    dev.writeAttr(SUMO_ATTR_CENTER_Y, myLookAtY->getValue());
    dev.writeAttr(SUMO_ATTR_CENTER_Z, myLookAtZ->getValue());
#endif
    dev.closeTag();
}


void
GUIDialog_EditViewport::setValues(double zoom, double xoff, double yoff, double rotation) {
    myZoom->setValue(zoom);
    myXOff->setValue(xoff);
    myYOff->setValue(yoff);
    myZOff->setValue(myParent->getChanger().zoom2ZPos(zoom));
    myRotation->setValue(rotation);
}


void
GUIDialog_EditViewport::setValues(const Position& lookFrom, const Position& lookAt, double rotation) {
    myXOff->setValue(lookFrom.x());
    myYOff->setValue(lookFrom.y());
    myZOff->setValue(lookFrom.z());
    myZoom->setValue(myParent->getChanger().zPos2Zoom(lookFrom.z()));
#ifdef HAVE_OSG
    myLookAtX->setValue(lookAt.x());
    myLookAtY->setValue(lookAt.y());
    myLookAtZ->setValue(lookAt.z());
#else
    UNUSED_PARAMETER(lookAt);
#endif
    myRotation->setValue(rotation);
}


void
GUIDialog_EditViewport::setOldValues(const Position& lookFrom, const Position& lookAt, double rotation) {
    setValues(lookFrom, lookAt, rotation);
    myOldLookFrom = lookFrom;
    myOldLookAt = lookAt;
    myOldRotation = rotation;
}


bool
GUIDialog_EditViewport::haveGrabbed() const {
    return false;
    //return myZoom->getDial().grabbed() || myXOff->getDial().grabbed() || myYOff->getDial().grabbed();
}

void
GUIDialog_EditViewport::saveWindowPos() {
    getApp()->reg().writeIntEntry("VIEWPORT_DIALOG_SETTINGS", "x", getX());
    getApp()->reg().writeIntEntry("VIEWPORT_DIALOG_SETTINGS", "y", getY());
}


/****************************************************************************/
