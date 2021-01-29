/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    FXLCDLabel.cpp
/// @author  Mathew Robertson
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004-03-19
///
//
/****************************************************************************/


/* =========================================================================
 * included modules
 * ======================================================================= */
#include <config.h>

#include <fxver.h>
#define NOMINMAX
#include <xincs.h>
#undef NOMINMAX
#include <fxdefs.h>
#include <fx.h>
/*
#include <FXStream.h>
#include <FXString.h>
#include <FXSize.h>
#include <FXPoint.h>
#include <FXRectangle.h>
#include <FXRegistry.h>
#include <FXHash.h>
#include <FXApp.h>
#include <FXDCWindow.h>
#include <FXLabel.h>
#include <FXFrame.h>
*/
using namespace FX;
#include "FXSevenSegment.h"
#include "FXLCDLabel.h"
#include "FXBaseObject.h"

using namespace FXEX;
namespace FXEX {

FXDEFMAP(FXLCDLabel) FXLCDLabelMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, FXLCDLabel::onPaint),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETVALUE, FXLCDLabel::onCmdSetValue),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETINTVALUE, FXLCDLabel::onCmdSetIntValue),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETREALVALUE, FXLCDLabel::onCmdSetRealValue),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETSTRINGVALUE, FXLCDLabel::onCmdSetStringValue),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_GETINTVALUE, FXLCDLabel::onCmdGetIntValue),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_GETREALVALUE, FXLCDLabel::onCmdGetRealValue),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_GETSTRINGVALUE, FXLCDLabel::onCmdGetStringValue),
    //  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXLCDLabel::onQueryTip),
    //  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXLCDLabel::onQueryHelp),
    FXMAPKEY(FXLCDLabel::ID_SEVENSEGMENT, FXLCDLabel::onRedirectEvent),
};
FXIMPLEMENT(FXLCDLabel, FXHorizontalFrame, FXLCDLabelMap, ARRAYNUMBER(FXLCDLabelMap))

FXLCDLabel::FXLCDLabel(FXComposite* p, FXuint nfig, FXObject* tgt, FXSelector sel, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs) : FXHorizontalFrame(p, opts, 0, 0, 0, 0, pl, pr, pt, pb, hs, 0) {
    if (nfig == 0) {
        fxerror("%s: must have at least one figure.\n", getClassName());
    }
    nfigures = nfig;
    setTarget(tgt);
    setSelector(sel);
    enable();
    for (FXint i = 0; i < nfigures; i++) {
        new FXSevenSegment(this, this, ID_SEVENSEGMENT, 0, 0, 0, 0);
    }
}

FXLCDLabel::~FXLCDLabel() {
    /*
      for (FXSevenSegment *child=(FXSevenSegment*)getFirst(); child; child=(FXSevenSegment*)child->getNext()){
        delete child;
        }
    */
}

// create resources
void FXLCDLabel::create() {
    FXHorizontalFrame::create();
    for (FXWindow* child = getFirst(); child; child = child->getNext()) {
        child->create();
    }
}

// detach resources
void FXLCDLabel::detach() {
    for (FXWindow* child = getFirst(); child; child = child->getNext()) {
        child->detach();
    }
    FXHorizontalFrame::detach();
}

// destroy resources
void FXLCDLabel::destroy() {
    for (FXWindow* child = getFirst(); child; child = child->getNext()) {
        child->destroy();
    }
    FXHorizontalFrame::destroy();
}

// get the foreground color
FXColor FXLCDLabel::getFgColor() const {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    return child->getFgColor();
}

// set the foreground color
void FXLCDLabel::setFgColor(FXColor clr) {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (clr != child->getFgColor()) {
        for (child = (FXSevenSegment*)getFirst(); child; child = (FXSevenSegment*)child->getNext()) {
            child->setFgColor(clr);
        }
    }
}

// get the background color
FXColor FXLCDLabel::getBgColor() const {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    return child->getBgColor();
}

// set the background color
void FXLCDLabel::setBgColor(FXColor clr) {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (clr != child->getBgColor()) {
        for (child = (FXSevenSegment*)getFirst(); child; child = (FXSevenSegment*)child->getNext()) {
            child->setBgColor(clr);
        }
    }
}

// set the text
void FXLCDLabel::setText(FXString lbl) {
    if (lbl != label) {
        label = lbl;
        recalc();
        update();
    }
}

// get the length of the horizontal segments
FXint FXLCDLabel::getHorizontal() const {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    return child->getHorizontal();
}

// set the length of the horizontal segments
void FXLCDLabel::setHorizontal(const FXint len) {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (len != child->getHorizontal()) {
        for (child = (FXSevenSegment*)getFirst(); child; child = (FXSevenSegment*)child->getNext()) {
            child->setHorizontal(len);
        }
        recalc();
        update();
    }
}

// get the length of the vertical segments
FXint FXLCDLabel::getVertical() const {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    return child->getVertical();
}

// set the length of the vertical segments
void FXLCDLabel::setVertical(const FXint len) {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (len != child->getVertical()) {
        for (child = (FXSevenSegment*)getFirst(); child; child = (FXSevenSegment*)child->getNext()) {
            child->setVertical(len);
        }
        recalc();
        update();
    }
}

// get the width of the segments
FXint FXLCDLabel::getThickness() const {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    return child->getThickness();
}

// set the width of the segments
void FXLCDLabel::setThickness(const FXint width) {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (width != child->getThickness()) {
        for (child = (FXSevenSegment*)getFirst(); child; child = (FXSevenSegment*)child->getNext()) {
            child->setThickness(width);
        }
        recalc();
        update();
    }
}

// get the width of the segments
FXint FXLCDLabel::getGroove() const {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    return child->getGroove();
}

// set the groove width
void FXLCDLabel::setGroove(const FXint width) {
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (width != child->getGroove()) {
        for (child = (FXSevenSegment*)getFirst(); child; child = (FXSevenSegment*)child->getNext()) {
            child->setGroove(width);
        }
        recalc();
        update();
    }
}

// Update value from a message
long FXLCDLabel::onCmdSetValue(FXObject*, FXSelector, void* ptr) {
    setText((const FXchar*)ptr);
    return 1;
}

// Update value from a message
long FXLCDLabel::onCmdSetIntValue(FXObject*, FXSelector, void* ptr) {
    setText(FXStringVal(*((FXint*)ptr)));
    return 1;
}

// Update value from a message
long FXLCDLabel::onCmdSetRealValue(FXObject*, FXSelector, void* ptr) {
    setText(FXStringVal(*((FXdouble*)ptr)));
    return 1;
}

// Update value from a message
long FXLCDLabel::onCmdSetStringValue(FXObject*, FXSelector, void* ptr) {
    setText(*((FXString*)ptr));
    return 1;
}

// Obtain value from text field
long FXLCDLabel::onCmdGetIntValue(FXObject*, FXSelector, void* ptr) {
    *((FXint*)ptr) = FXIntVal(getText());
    return 1;
}

// Obtain value from text field
long FXLCDLabel::onCmdGetRealValue(FXObject*, FXSelector, void* ptr) {
    *((FXdouble*)ptr) = FXDoubleVal(getText());
    return 1;
}

// Obtain value from text field
long FXLCDLabel::onCmdGetStringValue(FXObject*, FXSelector, void* ptr) {
    *((FXString*)ptr) = getText();
    return 1;
}

// handle paint event
long FXLCDLabel::onPaint(FXObject*, FXSelector, void* ptr) {
    FXEvent* event = (FXEvent*) ptr;
    FXDCWindow dc(this, event);
    drawFrame(dc, 0, 0, width, height);
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    // Fill the background
    dc.setForeground(child->getBgColor());
    dc.fillRectangle(border, border, width - (border << 1), height - (border << 1));
    // Draw the current string
    dc.setForeground(child->getFgColor());
    drawString(label);
    return 1;
}

// draw a specified string/label
void FXLCDLabel::drawString(const FXString& lbl) {
    FXint i = 0;
    FXString displayString(' ', nfigures);
    if (options & LCDLABEL_LEADING_ZEROS && (FXIntVal(lbl) || lbl == "0")) {
        FXString txt = lbl;
        if (txt[0] == '-') {
            displayString.replace(0, '-');
            txt.erase(0);
            i = 1;
        }
        for (; (i + txt.length()) < nfigures; i++) {
            displayString.replace(i, '0');
        }
        displayString.insert(i, txt);
    } else if (options & JUSTIFY_RIGHT) {
        for (; (i + lbl.length()) < nfigures; i++) {}
        displayString.insert(i, lbl);
    } else {
        displayString.insert(0, lbl);
    }
    displayString.trunc(nfigures);
    i = 0;

    // FIXME: at the moment, if we resize the parent widget, we must use integer multiples
    //        of the SevenSegment width.  The problem is that it makes the padding on the
    //        RHS look wrong.  What we need to do is to extend the horizontal segment width
    //        for the last sevensegment, so as to fill the remaining space.
    FXSevenSegment* child = (FXSevenSegment*)getFirst();
    if (options & LAYOUT_FILL) {
        FXint width = this->width - padleft - padright - (border << 1);
        FXint height = this->height - padtop - padbottom - (border << 1);
        hspacing = FXMAX(width, height) / 50;
        if (hspacing < 1) {
            hspacing = 1;
        }
        FXint hsl = (width - (nfigures - 1) * hspacing) / nfigures;
        if (hsl < 5) {
            hsl = 5;
        }
        FXint vsl = height >> 1;
        if (vsl < 5) {
            vsl = 5;
        }
        FXint st = FXMIN(hsl, vsl) / 4;
        if (st < 1) {
            st = 1;
        }
        FXint groove = st / 4;
        if (groove < 1) {
            groove = 1;
        }
        if (options & LAYOUT_FILL_X) {
            hsl -= groove << 1;
            for (; child; child = (FXSevenSegment*)child->getNext()) {
                child->setHorizontal(hsl);
            }
            child = (FXSevenSegment*)getFirst();
        }
        if (options & LAYOUT_FILL_Y) {
            vsl -= groove << 1;
            for (; child; child = (FXSevenSegment*)child->getNext()) {
                child->setVertical(vsl);
            }
            child = (FXSevenSegment*)getFirst();
        }
        for (; child; child = (FXSevenSegment*)child->getNext()) {
            child->setText(displayString[i++]);
            child->setGroove(groove);
            child->setThickness(st);
        }
    } else {
        for (; child; child = (FXSevenSegment*)child->getNext()) {
            child->setText(displayString[i++]);
        }
    }
}

// redirect events to main window
long FXLCDLabel::onRedirectEvent(FXObject*, FXSelector sel, void* ptr) {
    FXuint seltype = FXSELTYPE(sel);
    if (isEnabled()) {
        if (target) {
            target->handle(this, FXSEL(seltype, message), ptr);
        }
    }
    return 1;
}

// return minimum width
FXint FXLCDLabel::getDefaultWidth() {
    return padleft + getFirst()->getDefaultWidth() * nfigures + hspacing * (nfigures - 1) + padright + (border << 1);
}

// return minimum height
FXint FXLCDLabel::getDefaultHeight() {
    return padtop + getFirst()->getDefaultHeight() + padbottom + (border << 1);
}

// save resources
void FXLCDLabel::save(FXStream& store) const {
    FXHorizontalFrame::save(store);
    store << label;
    store << nfigures;
}

// load resources
void FXLCDLabel::load(FXStream& store) {
    FXHorizontalFrame::load(store);
    store >> label;
    store >> nfigures;
}

// let parent show tip if appropriate
long FXLCDLabel::onQueryTip(FXObject* sender, FXSelector sel, void* ptr) {
    if (getParent()) {
        return getParent()->handle(sender, sel, ptr);
    }
    return 0;
}

// let parent show help if appropriate
long FXLCDLabel::onQueryHelp(FXObject* sender, FXSelector sel, void* ptr) {
    if (getParent()) {
        return getParent()->handle(sender, sel, ptr);
    }
    return 0;
}

}

