/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    MFXIconComboBox.h
/// @author  Jakob Erdmann
/// @date    2018-12-19
///
//
/****************************************************************************/

#ifndef MFXIconComboBox_h
#define MFXIconComboBox_h
#include <config.h>

#include <fx.h>


/** A list item which allows for custom coloring
*/

class MFXListItem : public FXListItem {
    FXDECLARE(MFXListItem)

public:
    /// Construct new item with given text, icon, and user-data
    MFXListItem(const FXString& text, FXIcon* ic, FXColor _bgColor, void* ptr = NULL):
        FXListItem(text, ic, ptr),
        bgColor(_bgColor) {}
    FXColor bgColor;


    void draw(const FXList* list, FXDC& dc, FXint x, FXint y, FXint w, FXint h);

protected:
    MFXListItem(): FXListItem("", nullptr) {}

};

/** A combobox which allows for icons and background color
* This class behaves behaves just like FXComboBox but allows appending items with an icon
*/
class MFXIconComboBox : public FXComboBox {
protected:
    // FOX stuff
    FXDECLARE(MFXIconComboBox)
    MFXIconComboBox() {}
public:

    /// same constructor as parent
    MFXIconComboBox(FXComposite* p, FXint cols, FXObject* tgt = nullptr, FXSelector sel = 0, FXuint opts = COMBOBOX_NORMAL, FXint x = 0, FXint y = 0, FXint w = 0, FXint h = 0, FXint pl = DEFAULT_PAD, FXint pr = DEFAULT_PAD, FXint pt = DEFAULT_PAD, FXint pb = DEFAULT_PAD);

    /// Destructor.
    virtual ~MFXIconComboBox() {};

    FXint appendIconItem(const FXString& text, FXIcon* icon, FXColor bgColor = FXRGBA(0, 0, 0, 0), void* ptr = NULL);

};




#endif
