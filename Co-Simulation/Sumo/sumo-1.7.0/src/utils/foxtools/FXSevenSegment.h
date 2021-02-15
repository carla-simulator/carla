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
/// @file    FXSevenSegment.h
/// @author  Mathew Robertson
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004-03-19
///
//
/****************************************************************************/


#ifndef FXSEVENSEGMENT_H
#define FXSEVENSEGMENT_H
#include <config.h>

#ifndef FXFRAME_H

#include <FXFrame.h>
using namespace FX;
#endif
namespace FXEX {

/**
 * Seven-segment (eg LCD/watch style) widget
 */
class /* FXAPI // patch by Daniel Krajzewicz 24.02.2004 */
    FXSevenSegment : public FXFrame {
    FXDECLARE(FXSevenSegment)

private:
    FXchar  value;      // The currently shown character
    FXColor fgcolor;    // The color of the LCD text
    FXColor bgcolor;    // The color of the LCD background
    FXshort hsl;        // This is pixel length of a horizontal segment
    FXshort vsl;        // This is pixel length of a vertical segment
    FXshort st;         // This is segment thickness, in pixels
    FXshort groove;     // Groove between segments

private:
    void checkSize();

protected:
    FXSevenSegment() {}

    // Draws the individual segment types
    void drawTopSegment(FXDCWindow& dc, FXshort x, FXshort y);
    void drawLeftTopSegment(FXDCWindow& dc, FXshort x, FXshort y);
    void drawRightTopSegment(FXDCWindow& dc, FXshort x, FXshort y);
    void drawMiddleSegment(FXDCWindow& dc, FXshort x, FXshort y);
    void drawLeftBottomSegment(FXDCWindow& dc, FXshort x, FXshort y);
    void drawRightBottomSegment(FXDCWindow& dc, FXshort x, FXshort y);
    void drawBottomSegment(FXDCWindow& dc, FXshort x, FXshort y);

    // Draw a seven-segment unit (each segment can be set indepentantly)
    void drawSegments(FXDCWindow& dc, FXbool s1, FXbool s2, FXbool s3, FXbool s4, FXbool s5, FXbool s6, FXbool s7);

    // Draw an alphanumeric figure (consisting of seven segments)
    virtual void drawFigure(FXDCWindow& dc, FXchar figure);

public:
    long onPaint(FXObject*, FXSelector, void*);
    long onCmdSetValue(FXObject*, FXSelector, void*);
    long onCmdSetIntValue(FXObject*, FXSelector, void*);
    long onCmdGetIntValue(FXObject*, FXSelector, void*);
    long onCmdSetStringValue(FXObject*, FXSelector, void*);
    long onCmdGetStringValue(FXObject*, FXSelector, void*);
    long onQueryTip(FXObject*, FXSelector, void*);
    long onQueryHelp(FXObject*, FXSelector, void*);

public:
    /// create a seven segment display
    FXSevenSegment(FXComposite* p, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = FRAME_NONE, FXint pl = DEFAULT_PAD, FXint pr = DEFAULT_PAD, FXint pt = DEFAULT_PAD, FXint pb = DEFAULT_PAD);

    /// set the text on the display
    void setText(const FXchar val);

    /// get the text on the display
    FXchar getText() const {
        return value;
    }

    /// get/set foreground color
    void setFgColor(const FXColor clr);
    FXColor getFgColor() const {
        return fgcolor;
    }

    /// get/set background color
    void setBgColor(const FXColor clr);
    FXColor getBgColor() const {
        return bgcolor;
    }

    /// get/set horizontal segment length
    void setHorizontal(const FXint len);
    FXint getHorizontal() const {
        return hsl;
    }

    /// get/set vertical segment length
    void setVertical(const FXint len);
    FXint getVertical() const {
        return vsl;
    }

    /// get/set segment thickness
    void setThickness(const FXint width);
    FXint getThickness() const {
        return st;
    }

    /// get/set groove thickness
    void setGroove(const FXint width);
    FXint getGroove() const {
        return groove;
    }

    /// Return minimum width
    virtual FXint getDefaultWidth();

    /// Return minimum height
    virtual FXint getDefaultHeight();

    /// save resources
    virtual void save(FXStream& store) const;

    /// load resources
    virtual void load(FXStream& store);

    /// dtor
    virtual ~FXSevenSegment() {}
};

} // namespace FXEX
#endif // FXSEVENSEGMENT_H
