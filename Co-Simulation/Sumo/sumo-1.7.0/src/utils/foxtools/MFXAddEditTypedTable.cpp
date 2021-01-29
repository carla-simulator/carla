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
/// @file    MFXAddEditTypedTable.cpp
/// @author  Daniel Krajzewicz
/// @date    2004-07-02
///
// missing_desc
/****************************************************************************/
#include <config.h>

#include <fx.h>
#include <fxkeys.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include "MFXAddEditTypedTable.h"
#include <iostream>


// Map
FXDEFMAP(MFXAddEditTypedTable) MFXAddEditTypedTableMap[] = {
    FXMAPFUNC(SEL_CLICKED, 0, MFXAddEditTypedTable::onClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, 0, MFXAddEditTypedTable::onDoubleClicked),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE, 0, MFXAddEditTypedTable::onLeftBtnRelease),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, MFXAddEditTypedTable::onLeftBtnPress),
};
// Object implementation
FXIMPLEMENT(MFXAddEditTypedTable, FXTable, MFXAddEditTypedTableMap, ARRAYNUMBER(MFXAddEditTypedTableMap))


MFXAddEditTypedTable::MFXAddEditTypedTable(FXComposite* p, FXObject* tgt,
        FXSelector sel, FXuint opts,
        FXint x, FXint y, FXint w, FXint h,
        FXint pl, FXint pr, FXint pt, FXint pb)
    : FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb) {}


MFXAddEditTypedTable::~MFXAddEditTypedTable() {}

/*
void
MFXAddEditTypedTable::editItem(FXTableItem* item,FXint how)
{
    if(item==0) {
        editEnd();
        return;
    }
    if(myWriteProtectedCols.find(myEditedCol)!=myWriteProtectedCols.end()) {
        editEnd();
        return;
    }
    FXTableItem* it= item;
    myPreviousText = item->getText();
    FXint x = getColumnX(myEditedCol) + getRowHeader()->getWidth() + xpos;
    FXint y = getRowY(myEditedRow) + getColumnHeader()->getHeight() + ypos;
    FXIcon* icon = item->getIcon();
    if(icon) x += icon->getWidth() + 4;
    FXint vw = getViewportWidth();
    if(vertical->shown()) vw -= vertical->getWidth();
    if(vw>getColumnWidth(myEditedCol)) {
        vw = getColumnWidth(myEditedCol) + x;
    }
    switch(getCellType(myEditedCol)) {
    case CT_UNDEFINED:
    case CT_STRING:
        myEditor->setText(it->getText());
        myEditor->move(x, y);
        myEditor->resize(vw - x + 1, getRowHeight(myEditedRow) + 1);
        myEditor->show();
        myEditor->raise();
        myEditor->enable();
        myEditor->setFocus();
        myEditor->grab();
        if(how == 'I') {
            myEditor->killSelection();
            myEditor->setCursorPos(0);
        } else if(how == 'A') {
            myEditor->killSelection();
            myEditor->setCursorPos(myEditor->getText().length());
        } else myEditor->selectAll();
        break;
    case CT_REAL:
        {
            try {
                myNumberEditor->setValue(
                    TplConvert::_2double(it->getText().text()));
            } catch (NumberFormatException &) {
            } catch (EmptyData &) {
            }
            NumberCellParams p = getNumberCellParams(myEditedCol);
            if(p.format!="undefined") {
                myNumberEditor->setFormatString((char*) p.format.c_str());
                myNumberEditor->setIncrements(p.steps1, p.steps2, p.steps3);
                myNumberEditor->setRange(p.min, p.max);
            }
            myNumberEditor->move(x, y);
            myNumberEditor->resize(vw - x + 1, getRowHeight(myEditedRow) + 1);
            myNumberEditor->show();
            myNumberEditor->raise();
            myNumberEditor->setFocus();
            myNumberEditor->selectAll();
        }
        //myNumberEditor->setRange(0,1000);
        break;
    case CT_INT:
        {
            try {
                myNumberEditor->setValue(
                    TplConvert::_2int(it->getText().text()));
            } catch (NumberFormatException &) {
            } catch (EmptyData &) {
            }
            NumberCellParams p = getNumberCellParams(myEditedCol);
            if(p.format!="undefined") {
                myNumberEditor->setFormatString((char*) p.format.c_str());
                myNumberEditor->setIncrements(p.steps1, p.steps2, p.steps3);
                myNumberEditor->setRange(p.min, p.max);
            }
            myNumberEditor->move(x, y);
            myNumberEditor->resize(vw - x + 1, getRowHeight(myEditedRow) + 1);
            myNumberEditor->show();
            myNumberEditor->raise();
            myNumberEditor->setFocus();
            myNumberEditor->selectAll();
        }
        break;
    case CT_BOOL:
        try {
            myBoolEditor->setCheck(
                TplConvert::_2bool(it->getText().text())
                ? true : false);
        } catch (NumberFormatException &) {
        } catch (EmptyData &) {
        }
        myBoolEditor->move(x, y);
        myBoolEditor->resize(vw - x + 1, getRowHeight(myEditedRow) + 1);
        myBoolEditor->show();
        myBoolEditor->raise();
        myBoolEditor->setFocus();
        break;
    case CT_ENUM:
        {
            myEnumEditor->hide();
            myEnumEditor->clearItems();
            if(myEnums.size()>myEditedCol) {
                for(int i=0; i<myEnums[myEditedCol].size(); i++) {
                    myEnumEditor->appendItem(myEnums[myEditedCol][i].c_str());
                }
            }
            if(myEnumEditor->findItem(it->getText())>=0) {
                myEnumEditor->setCurrentItem(
                    myEnumEditor->findItem(it->getText()));
            } else {
                myEnumEditor->setCurrentItem(0);
            }
            myEnumEditor->setNumVisible(
                myEnums[myEditedCol].size()<10
                ? myEnums[myEditedCol].size()
                : 10);
            myEnumEditor->layout();
            y = getRowY(myEditedRow) + getColumnHeader()->getHeight() + ypos
                - getRowHeight(myEditedRow);
            myEnumEditor->move(x, y);
            myEnumEditor->resize(vw - x + 1, getRowHeight(myEditedRow) + 1);
            myEnumEditor->show();
            myEnumEditor->raise();
            myEnumEditor->setFocus();
        }
        break;
    default:
        throw 1;
    }
    myEditedItem = it;
}
*/


FXWindow*
MFXAddEditTypedTable::getControlForItem(FXint r, FXint c) {
    FXTableItem* item = cells[r * ncols + c];
    if (item == nullptr) {
        return nullptr;
//         cells[r * ncols + c] = item = createItem("", NULL, NULL);
//         if (isItemSelected(r, c)) {
//             item->setSelected(FALSE);
//         }
    }
    delete editor;
    editor = nullptr;
    switch (getCellType(c)) {
        case CT_UNDEFINED:
        case CT_STRING: {
            FXTextField* field;
            FXuint justify = 0;
            field = new FXTextField(this, 1, nullptr, 0, TEXTFIELD_ENTER_ONLY, 0, 0, 0, 0, getMarginLeft(), getMarginRight(), getMarginTop(), getMarginBottom());
            // !!! if(state&LEFT) justify|=JUSTIFY_LEFT;
            // !!! if(state&RIGHT) justify|=JUSTIFY_RIGHT;
            // !!! if(state&TOP) justify|=JUSTIFY_TOP;
            // !!! if(state&BOTTOM) justify|=JUSTIFY_BOTTOM;
            field->create();
            field->setJustify(justify);
            field->setFont(getFont());
            field->setBackColor(getBackColor());
            field->setTextColor(getTextColor());
            field->setSelBackColor(getSelBackColor());
            field->setSelTextColor(getSelTextColor());
            field->setText(item->getText());
            field->selectAll();
            return field;
        }
        case CT_REAL:
//        return myNumberEditor;
        case CT_INT: {
            FXRealSpinner* field;
            //FXuint justify=0;
            field = new FXRealSpinner(this, 1, nullptr, 0, TEXTFIELD_ENTER_ONLY, 0, 0, 0, 0, getMarginLeft(), getMarginRight(), getMarginTop(), getMarginBottom());
            // !!! if(state&LEFT) justify|=JUSTIFY_LEFT;
            // !!! if(state&RIGHT) justify|=JUSTIFY_RIGHT;
            // !!! if(state&TOP) justify|=JUSTIFY_TOP;
            // !!! if(state&BOTTOM) justify|=JUSTIFY_BOTTOM;
            field->create();
//            field->setJustify(justify);
            field->setFont(getFont());
            field->setBackColor(getBackColor());
            field->setTextColor(getTextColor());
            field->setSelBackColor(getSelBackColor());
            field->setSelTextColor(getSelTextColor());
            NumberCellParams p = getNumberCellParams(c);
            if (p.format != "undefined") {
                //field->setFormatString((char*) p.format.c_str());
                //field->setIncrements(p.steps1, p.steps2, p.steps3);
                field->setIncrement(p.steps2);
                field->setRange(p.min, p.max);
            }
            try {
                if (getCellType(c) == CT_REAL) {
                    field->setValue(StringUtils::toDouble(item->getText().text()));
                } else {
                    field->setValue(StringUtils::toInt(item->getText().text()));
                }
            } catch (NumberFormatException&) {
                field->setValue(0);
            }
            //field->selectAll();
            return field;
        }
        case CT_BOOL:
//        return myBoolEditor;
        case CT_ENUM:
//        return myEnumEditor;
        default:
            throw 1;
    }
}


// Cancel editing cell
void
MFXAddEditTypedTable::cancelInput() {
    if (editor) {
        delete editor;
        input.fm.row = -1;
        input.to.row = -1;
        input.fm.col = -1;
        input.to.col = -1;
        editor = nullptr;
    }
}

// Done with editing cell
void
MFXAddEditTypedTable::acceptInput(FXbool notify) {
    bool set = false;
    FXTableRange tablerange = input;
    if (editor) {
        FXRealSpinner* dial = dynamic_cast<FXRealSpinner*>(editor);
        if (dial != nullptr) {
            setItemFromControl_NoRelease(input.fm.row, input.fm.col, editor);
        }
        if (dynamic_cast<FXTextField*>(editor) != nullptr) {
            set = true;
        }
    }
    if (set) {
        setItemFromControl(input.fm.row, input.fm.col, editor);
        cancelInput();
        if (notify && target) {
            target->tryHandle(this, FXSEL(SEL_REPLACED, message), (void*)&tablerange);
        }
    }
}




void
MFXAddEditTypedTable::setItemFromControl(FXint r, FXint c, FXWindow* control) {
    FXTableItem* item = cells[r * ncols + c];
    if (item == nullptr) {
        cells[r * ncols + c] = item = createItem("", nullptr, nullptr);
        if (isItemSelected(r, c)) {
            item->setSelected(FALSE);
        }
    }
    switch (getCellType(c)) {
        case CT_UNDEFINED:
        case CT_STRING:
            item->setFromControl(control);
            break;
        case CT_REAL:
            item->setText(toString(static_cast<FXRealSpinner*>(control)->getValue()).c_str());
            break;
        case CT_INT:
            item->setText(toString((int) static_cast<FXRealSpinner*>(control)->getValue()).c_str());
            break;
        case CT_BOOL:
//        return myBoolEditor;
        case CT_ENUM:
//        return myEnumEditor;
        default:
            throw 1;
    }
//    current.row = -1;
//    current.col = -1;
    EditedTableItem edited;
    edited.item = item;
    edited.row = r;
    edited.col = c;
    edited.updateOnly = false;
    killSelection(true);
    bool accepted = true;
    if (target) {
        if (!target->handle(this, FXSEL(SEL_CHANGED, ID_TEXT_CHANGED), (void*) &edited)) {
            accepted = false;
            // !!! item->setText(myPreviousText);
        }
    }
    if (accepted) {
        if (edited.row == getNumRows() - 1) {
            insertRows(getNumRows(), 1, true);
            for (int i = 0; i < getNumColumns(); i++) {
                setItemText(getNumRows() - 1, i, "");
                setItemJustify(getNumRows() - 1, i, JUSTIFY_CENTER_X);
            }
        }
    }
    mode = MOUSE_NONE;
}


void
MFXAddEditTypedTable::setItemFromControl_NoRelease(FXint r, FXint c, FXWindow* control) {
    FXTableItem* item = cells[r * ncols + c];
    if (item == nullptr) {
        return;
    }
    switch (getCellType(c)) {
        case CT_UNDEFINED:
        case CT_STRING:
            item->setFromControl(control);
            break;
        case CT_REAL:
            item->setText(toString(static_cast<FXRealSpinner*>(control)->getValue()).c_str());
            break;
        case CT_INT:
            item->setText(toString((int) static_cast<FXRealSpinner*>(control)->getValue()).c_str());
            break;
        case CT_BOOL:
//        return myBoolEditor;
        case CT_ENUM:
//        return myEnumEditor;
        default:
            throw 1;
    }
    EditedTableItem edited;
    edited.item = item;
    edited.row = r;
    edited.col = c;
    edited.updateOnly = true;
    if (target) {
        if (!target->handle(this, FXSEL(SEL_CHANGED, ID_TEXT_CHANGED), (void*) &edited)) {
            // !!! item->setText(myPreviousText);
        }
    }
}


// Released button
long MFXAddEditTypedTable::onLeftBtnRelease(FXObject*, FXSelector, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    if (isEnabled()) {
        ungrab();
        flags &= ~FLAG_PRESSED;
        flags |= FLAG_UPDATE;
        mode = MOUSE_NONE;
        stopAutoScroll();
        setDragCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
        if (target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONRELEASE, message), ptr)) {
            return 1;
        }

        // Scroll to make item visibke
        makePositionVisible(current.row, current.col);

        // Update anchor
        //setAnchorItem(current.row,current.col); // FIXME look into the selection stuff

        // Generate clicked callbacks
        if (event->click_count == 1) {
            handle(this, FXSEL(SEL_CLICKED, 0), (void*)&current);
        } else if (event->click_count == 2) {
            handle(this, FXSEL(SEL_DOUBLECLICKED, 0), (void*)&current);
        } else if (event->click_count == 3) {
            handle(this, FXSEL(SEL_TRIPLECLICKED, 0), (void*)&current);
        }

        // Command callback only when clicked on item
        if (0 <= current.row && 0 <= current.col && isItemEnabled(current.row, current.col)) {
            handle(this, FXSEL(SEL_COMMAND, 0), (void*)&current);
        }
        return 1;
    }
    return 0;
}


// Pressed button
long
MFXAddEditTypedTable::onLeftBtnPress(FXObject*, FXSelector, void* ptr) {
    FXEvent* event = (FXEvent*)ptr;
    FXTablePos tablepos;
    flags &= ~FLAG_TIP;
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);
    if (isEnabled()) {
        grab();
        if (target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONPRESS, message), ptr)) {
            return 1;
        }

        // Cell being clicked on
        tablepos.row = rowAtY(event->win_y);
        tablepos.col = colAtX(event->win_x);

        // Outside table
        if (tablepos.row < 0 || tablepos.row >= nrows || tablepos.col < 0 || tablepos.col >= ncols) {
            setCurrentItem(-1, -1, TRUE);
            return 0;
        }

        // Change current item
        bool wasEdited = editor != nullptr;
        setCurrentItem(tablepos.row, tablepos.col, TRUE);
        if (!wasEdited) {

            // Select or deselect
            if (event->state & SHIFTMASK) {
                if (0 <= anchor.row && 0 <= anchor.col) {
                    if (isItemEnabled(anchor.row, anchor.col)) {
                        extendSelection(current.row, current.col, TRUE);
                    }
                } else {
                    setAnchorItem(current.row, current.col);
                    if (isItemEnabled(current.row, current.col)) {
                        extendSelection(current.row, current.col, TRUE);
                    }
                }
                mode = MOUSE_SELECT;
            } else {
                if (isItemEnabled(current.row, current.col)) {
                    killSelection(TRUE);
                    setAnchorItem(current.row, current.col);
                    extendSelection(current.row, current.col, TRUE);
                } else {
                    setAnchorItem(current.row, current.col);
                }
                mode = MOUSE_SELECT;
            }
        }
        flags &= ~FLAG_UPDATE;
        flags |= FLAG_PRESSED;
        return 1;
    }
    return 0;
}



// Clicked in list
long
MFXAddEditTypedTable::onClicked(FXObject*, FXSelector, void* ptr) {
    if (editor) {
        delete editor;
        input.fm.row = -1;
        input.to.row = -1;
        input.fm.col = -1;
        input.to.col = -1;
        editor = nullptr;
        current.row = -1;
        current.col = -1;
    }
    if (target && target->tryHandle(this, FXSEL(SEL_CLICKED, message), ptr)) {
        return 1;
    }
    handle(this, FXSEL(SEL_COMMAND, ID_START_INPUT), nullptr);
    return 1;
}


// Double clicked in list; ptr may or may not point to an item
long MFXAddEditTypedTable::onDoubleClicked(FXObject*, FXSelector, void* ptr) {
    if (editor) {
        delete editor;
        input.fm.row = -1;
        input.to.row = -1;
        input.fm.col = -1;
        input.to.col = -1;
        editor = nullptr;
    } else {
        if (target && target->tryHandle(this, FXSEL(SEL_CLICKED, message), ptr)) {
            return 1;
        }
        handle(this, FXSEL(SEL_COMMAND, ID_START_INPUT), nullptr);
    }
    return 1;
}


CellType
MFXAddEditTypedTable::getCellType(int pos) const {
    if ((int)myCellTypes.size() <= pos) {
        return CT_UNDEFINED;
    }
    return myCellTypes[pos];
}


void
MFXAddEditTypedTable::setCellType(int pos, CellType t) {
    while ((int)myCellTypes.size() < pos + 1) {
        myCellTypes.push_back(CT_UNDEFINED);
    }
    myCellTypes[pos] = t;
}

void
MFXAddEditTypedTable::setNumberCellParams(int pos, double min, double max,
        double steps1,
        double steps2,
        double steps3,
        const std::string& format) {
    while ((int)myNumberCellParams.size() <= pos) {
        NumberCellParams np;
        np.format = "undefined";
        myNumberCellParams.push_back(np);
    }
    NumberCellParams np;
    np.pos = (int)(pos);
    np.min = min;
    np.max = max;
    np.steps1 = steps1;
    np.steps2 = steps2;
    np.steps3 = steps3;
    np.format = format;
    myNumberCellParams[pos] = np;
}


MFXAddEditTypedTable::NumberCellParams
MFXAddEditTypedTable::getNumberCellParams(int pos) const {
    if ((int)myNumberCellParams.size() <= pos) {
        NumberCellParams np;
        np.format = "undefined";
        return np;
    }
    return myNumberCellParams[pos];
}



void
MFXAddEditTypedTable::setEnums(int pos,
                               const std::vector<std::string>& params) {
    while ((int)myEnums.size() <= pos) {
        myEnums.push_back(std::vector<std::string>());
    }
    myEnums[pos] = params;
}


void
MFXAddEditTypedTable::addEnum(int pos,
                              const std::string& e) {
    while ((int)myEnums.size() <= pos) {
        myEnums.push_back(std::vector<std::string>());
    }
    myEnums[pos].push_back(e);
}


const std::vector<std::string>&
MFXAddEditTypedTable::getEnums(int pos) const {
    return myEnums[pos];
}


/****************************************************************************/
