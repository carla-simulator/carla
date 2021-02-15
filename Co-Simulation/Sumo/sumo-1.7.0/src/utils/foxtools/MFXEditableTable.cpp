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
/// @file    MFXEditableTable.cpp
/// @author  Daniel Krajzewicz
/// @date    2004-07-02
///
// missing_desc
/****************************************************************************/
#include <config.h>

#include <fx.h>
#include <fxkeys.h>
#include "MFXEditableTable.h"

FXDEFMAP(MFXEditableTable) MFXEditableTableMap[] = {
    FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, MFXEditableTable::onLeftBtnPress),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE, 0, MFXEditableTable::onLeftBtnRelease),
    FXMAPFUNC(SEL_CLICKED, 0, MFXEditableTable::onLeftBtnClicked),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, 0, MFXEditableTable::onRightBtnRelease),
    FXMAPFUNC(SEL_KEYRELEASE, 0, MFXEditableTable::onKeyRelease),
    FXMAPFUNC(SEL_KEYPRESS, MFXEditableTable::ID_EDITOR, MFXEditableTable::onEditKeyPress),
    //    FXMAPFUNC(SEL_TIMEOUT,FXTable::ID_TIPTIMER, MFXEditableTable::onTipTimer),
    FXMAPFUNC(SEL_FOCUSOUT, MFXEditableTable::ID_EDITOR, MFXEditableTable::onEditFocusOut),
    FXMAPFUNC(SEL_COMMAND, MFXEditableTable::ID_EDITEND, MFXEditableTable::onEditEnd),

    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_HSCROLLED, MFXEditableTable::onHScrollerChanged),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_VSCROLLED, MFXEditableTable::onVScrollerChanged),
    FXMAPFUNC(SEL_CHANGED, FXWindow::ID_HSCROLLED, MFXEditableTable::onHScrollerDragged),
    FXMAPFUNC(SEL_CHANGED, FXWindow::ID_VSCROLLED, MFXEditableTable::onVScrollerDragged),
    /*
      FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,MFXEditableTable::onAutoScroll),
      FXMAPFUNC(SEL_UNGRABBED,0,MFXEditableTable::onUngrabbed),
      FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,MFXEditableTable::onRightBtnPress),
      FXMAPFUNC(SEL_KEYPRESS,0,MFXEditableTable::onTabKeyPress),
      FXMAPFUNC(SEL_KEYRELEASE,0,MFXEditableTable::onTabKeyRelease),
      FXMAPFUNC(SEL_CLIPBOARD_LOST,0,MFXEditableTable::onClipboardLost),
      FXMAPFUNC(SEL_CLIPBOARD_GAINED,0,MFXEditableTable::onClipboardGained),
      FXMAPFUNC(SEL_CLIPBOARD_REQUEST,0,MFXEditableTable::onClipboardRequest),
      FXMAPFUNC(SEL_DOUBLECLICKED,0,MFXEditableTable::onDoubleClicked),
      FXMAPFUNC(SEL_TRIPLECLICKED,0,MFXEditableTable::onTripleClicked),
      FXMAPFUNC(SEL_COMMAND,0,MFXEditableTable::onCommand),
      */
    /*
      FXMAPFUNC(SEL_FOCUSIN,0,MFXEditableTable::onFocusIn),
      FXMAPFUNC(SEL_FOCUSOUT,0,MFXEditableTable::onFocusOut),
      FXMAPFUNC(SEL_SELECTION_LOST,0,MFXEditableTable::onSelectionLost),
      FXMAPFUNC(SEL_SELECTION_GAINED,0,MFXEditableTable::onSelectionGained),
      FXMAPFUNC(SEL_SELECTION_REQUEST,0,MFXEditableTable::onSelectionRequest),
    */

    FXMAPFUNC(SEL_UPDATE, FXTable::ID_HORZ_GRID, MFXEditableTable::onUpdHorzGrid),
    FXMAPFUNC(SEL_UPDATE, FXTable::ID_VERT_GRID, MFXEditableTable::onUpdVertGrid),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_HORZ_GRID, MFXEditableTable::onCmdHorzGrid),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_VERT_GRID, MFXEditableTable::onCmdVertGrid),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_DELETE_COLUMN, MFXEditableTable::onCmdDeleteColumn),
    FXMAPFUNC(SEL_UPDATE, FXTable::ID_DELETE_COLUMN, MFXEditableTable::onUpdDeleteColumn),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_DELETE_ROW, MFXEditableTable::onCmdDeleteRow),
    FXMAPFUNC(SEL_UPDATE, FXTable::ID_DELETE_ROW, MFXEditableTable::onUpdDeleteRow),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_INSERT_COLUMN, MFXEditableTable::onCmdInsertColumn),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_INSERT_ROW, MFXEditableTable::onCmdInsertRow),

    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_LEFT, MFXEditableTable::onCmdMoveLeft),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_RIGHT, MFXEditableTable::onCmdMoveRight),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_UP, MFXEditableTable::onCmdMoveUp),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_DOWN, MFXEditableTable::onCmdMoveDown),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_HOME, MFXEditableTable::onCmdMoveHome),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_END, MFXEditableTable::onCmdMoveEnd),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_TOP, MFXEditableTable::onCmdMoveTop),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_BOTTOM, MFXEditableTable::onCmdMoveBottom),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_PAGEDOWN, MFXEditableTable::onCmdMovePageDown),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MOVE_PAGEUP, MFXEditableTable::onCmdMovePageUp),

    FXMAPFUNC(SEL_COMMAND, FXTable::ID_SELECT_ROW_INDEX, MFXEditableTable::onCmdSelectRowIndex),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_SELECT_COLUMN_INDEX, MFXEditableTable::onCmdSelectColumnIndex),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_SELECT_COLUMN, MFXEditableTable::onCmdSelectColumn),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_SELECT_ROW, MFXEditableTable::onCmdSelectRow),
    //  FXMAPFUNC(SEL_COMMAND,FXTable::ID_SELECT_CELL,MFXEditableTable::onCmdSelectCell),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_SELECT_ALL, MFXEditableTable::onCmdSelectAll),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_DESELECT_ALL, MFXEditableTable::onCmdDeselectAll),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_MARK, MFXEditableTable::onCmdMark),
    FXMAPFUNC(SEL_COMMAND, FXTable::ID_EXTEND, MFXEditableTable::onCmdExtend),
};


// Object implementation
FXIMPLEMENT(MFXEditableTable, FXTable, MFXEditableTableMap, ARRAYNUMBER(MFXEditableTableMap))


MFXEditableTable::MFXEditableTable(FXComposite* p, FXObject* tgt,
                                   FXSelector sel, FXuint opts,
                                   FXint x, FXint y, FXint w, FXint h,
                                   FXint pl, FXint pr, FXint pt, FXint pb)
    : FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb),
      myEditedItem(nullptr) {
    myEditor =
        new FXTextField((FXComposite*)getParent(), 1, this,
                        ID_EDITOR, FRAME_NORMAL | LAYOUT_EXPLICIT);
    myEditor->hide();
    flags |= FLAG_ENABLED | FLAG_DROPTARGET;
}


MFXEditableTable::~MFXEditableTable() {}


long
MFXEditableTable::onLeftBtnPress(FXObject* sender, FXSelector sel, void* ptr) {
    editEnd();
    FXTable::onLeftBtnPress(sender, sel, ptr);
    return 1;
}


long
MFXEditableTable::onLeftBtnClicked(FXObject*, FXSelector, void* ptr) {
    FXTablePos* e = (FXTablePos*)ptr;
    int r = e->row;
    int c = e->col;
    if (c == -1 || r == -1) {
        return 1;
    }
    killSelection(false);
    myEditedItem = getItem(r, c);
    myEditedCol = c;
    myEditedRow = r;
    if (c >= 0 && r >= 0) {
        editItem(myEditedItem, 0);
    } else {
        editEnd();
    }
    return 1;
}


long
MFXEditableTable::onLeftBtnRelease(FXObject* sender, FXSelector sel, void* ptr) {
    FXTable::onLeftBtnRelease(sender, sel, ptr);
    /*    FXEvent* e= (FXEvent*)ptr;
        if(flgEditTimer) {
            flgEditTimer = FALSE;
            if(!e->moved) getApp()->addTimeout(this,ID_EDITTIMER,600);
        }*/
    return 1;
}


void
MFXEditableTable::editItem(FXTableItem* item, FXint how) {
    if (item == nullptr) {
        editEnd();
        return;
    }
    if (myWriteProtectedCols.find(myEditedCol) != myWriteProtectedCols.end()) {
        editEnd();
        return;
    }
    FXTableItem* it = item;
    myPreviousText = item->getText();
    FXint x = getColumnX(myEditedCol) + getRowHeader()->getWidth() + xpos;
    FXint y = getRowY(myEditedRow) + getColumnHeader()->getHeight() + ypos;
    FXIcon* icon = item->getIcon();
    if (icon) {
        x += icon->getWidth() + 4;
    }
    FXint vw = getViewportWidth();
    if (vertical->shown()) {
        vw -= vertical->getWidth();
    }
    if (vw > getColumnWidth(myEditedCol)) {
        vw = getColumnWidth(myEditedCol) + x;
    }
    myEditor->setText(it->getText());
    myEditor->move(x, y);
    myEditor->resize(vw - x + 1, getRowHeight(myEditedRow) + 1);
    myEditor->show();
    myEditor->raise();
    myEditor->setFocus();
    if (how == 'I') {
        myEditor->killSelection();
        myEditor->setCursorPos(0);
    } else if (how == 'A') {
        myEditor->killSelection();
        myEditor->setCursorPos(myEditor->getText().length());
    } else {
        myEditor->selectAll();
    }
    myEditedItem = it;
}


void
MFXEditableTable::editEnd() {
    if (!myEditedItem) {
        return;
    }
    FXTableItem* item = myEditedItem;
    myEditedItem = nullptr;
    myEditor->hide();
    setFocus();
    FXString text = myEditor->getText();
    myEditedItem = nullptr;
    if (item->getText() == text) {
        return;
    }
    if (handle(item, FXSEL(SEL_COMMAND, ID_EDITEND), &text)) {
        item->setText(text);
        handle(this, FXSEL(SEL_CHANGED, 0), item);
    }
    killSelection(true);
    if (target) {
        MFXEditedTableItem edited;
        edited.item = item;
        edited.row = myEditedRow;
        edited.col = myEditedCol;
        if (!target->handle(this, FXSEL(SEL_CHANGED, ID_TEXT_CHANGED), (void*) &edited)) {
            item->setText(myPreviousText);
        }
    }
}


long
MFXEditableTable::onEditEnd(FXObject*, FXSelector, void*) {
    delete myEditedItem;
    myEditedItem = nullptr;
    return 1;
}


void
MFXEditableTable::editCancel() {
    if (!myEditedItem) {
        return;
    }
    myEditedItem = nullptr;
    myEditor->hide();
    setFocus();
    killSelection(true);
}


long
MFXEditableTable::onEditKeyPress(FXObject*, FXSelector, void* ptr) {
    FXEvent* e = (FXEvent*)ptr;
    switch (e->code) {
        case KEY_Escape:
            editCancel();
            return 1;
        case KEY_Return:
        case KEY_KP_Enter:
            editEnd();
//        flgEnter = TRUE;
//        if(e->state & SHIFTMASK) handle(this, FXSEL(SEL_COMMAND,ID_NEXTLINE), NULL);
            return 1;
        case KEY_Up:
        case KEY_Down:
//        editEnd();
            //      handle(this, FXSEL(SEL_KEYPRESS,0), ptr);
            return 1;
    }
    return 0;
}


long
MFXEditableTable::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr) {
    FXint key = ((FXEvent*)ptr)->code;
    if (/*flgEnter &&*/ key == KEY_Return || key == KEY_KP_Enter) {
//        flgEnter = FALSE;
        return 1;
    }
//    flgEnter = FALSE;
    return FXTable::onKeyRelease(sender, sel, ptr);
}


long
MFXEditableTable::onEditFocusOut(FXObject*, FXSelector, void*) {
    editEnd();
    return 0;
}

/*
long
MFXEditableTable::onTipTimer(FXObject* sender, FXSelector sel, void* ptr)
{
    if(myEditedItem) return 0;
    return FXTable::onTipTimer(sender, sel, ptr);
}
*/

long
MFXEditableTable::onRightBtnRelease(FXObject* sender, FXSelector sel, void* ptr) {
    /*  if(dragItem) {
        flags &= ~(FLAG_PRESSED | FLAG_TRYDRAG | FLAG_DODRAG);
        endDrag(FALSE);
        setDragCursor(getDefaultCursor());
        dragItem = NULL;
        }*/
    return FXTable::onRightBtnRelease(sender, sel, ptr);
}



long
MFXEditableTable::onHScrollerChanged(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXScrollArea::onHScrollerChanged(o, s, d);
}


long
MFXEditableTable::onVScrollerChanged(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXScrollArea::onVScrollerChanged(o, s, d);
}


long
MFXEditableTable::onHScrollerDragged(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXScrollArea::onHScrollerDragged(o, s, d);
}


long
MFXEditableTable::onVScrollerDragged(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXScrollArea::onVScrollerDragged(o, s, d);
}


long
MFXEditableTable::onAutoScroll(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onAutoScroll(o, s, d);
}


long
MFXEditableTable::onUngrabbed(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onRightBtnPress(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onRightBtnPress(o, s, d);
}


long
MFXEditableTable::onTabKeyPress(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onKeyPress(o, s, d);
}


long
MFXEditableTable::onTabKeyRelease(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onKeyRelease(o, s, d);
}


long
MFXEditableTable::onFocusIn(FXObject* o, FXSelector s, void* d) {
//    editEnd();
    return FXTable::onFocusIn(o, s, d);
}


long
MFXEditableTable::onFocusOut(FXObject* o, FXSelector s, void* d) {
//    editEnd();
    return FXTable::onFocusOut(o, s, d);
}


long
MFXEditableTable::onSelectionLost(FXObject* o, FXSelector s, void* d) {
//    editEnd();
    return FXTable::onSelectionLost(o, s, d);
}


long
MFXEditableTable::onSelectionGained(FXObject* o, FXSelector s, void* d) {
//    editEnd();
    return FXTable::onSelectionGained(o, s, d);
}


long
MFXEditableTable::onSelectionRequest(FXObject* o, FXSelector s, void* d) {
//    editEnd();
    return FXTable::onSelectionRequest(o, s, d);
}


long
MFXEditableTable::onClipboardLost(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onClipboardLost(o, s, d);
}


long
MFXEditableTable::onClipboardGained(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onClipboardGained(o, s, d);
}


long
MFXEditableTable::onClipboardRequest(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onClipboardRequest(o, s, d);
}


long
MFXEditableTable::onDoubleClicked(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onTripleClicked(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCommand(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCommand(o, s, d);
}


long
MFXEditableTable::onUpdHorzGrid(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onUpdHorzGrid(o, s, d);
}


long
MFXEditableTable::onUpdVertGrid(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onUpdVertGrid(o, s, d);
}


long
MFXEditableTable::onCmdHorzGrid(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdHorzGrid(o, s, d);
}


long
MFXEditableTable::onCmdVertGrid(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdVertGrid(o, s, d);
}


long
MFXEditableTable::onCmdDeleteColumn(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onUpdDeleteColumn(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdDeleteRow(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onUpdDeleteRow(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdInsertColumn(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdInsertRow(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}



long
MFXEditableTable::onCmdMoveLeft(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveLeft(o, s, d);
}


long
MFXEditableTable::onCmdMoveRight(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveRight(o, s, d);
}


long
MFXEditableTable::onCmdMoveUp(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveUp(o, s, d);
}


long
MFXEditableTable::onCmdMoveDown(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveDown(o, s, d);
}


long
MFXEditableTable::onCmdMoveHome(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveHome(o, s, d);
}


long
MFXEditableTable::onCmdMoveEnd(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveEnd(o, s, d);
}


long
MFXEditableTable::onCmdMoveTop(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveTop(o, s, d);
}


long
MFXEditableTable::onCmdMoveBottom(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMoveBottom(o, s, d);
}


long
MFXEditableTable::onCmdMovePageDown(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMovePageDown(o, s, d);
}


long
MFXEditableTable::onCmdMovePageUp(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMovePageUp(o, s, d);
}



long
MFXEditableTable::onCmdSelectRowIndex(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdSelectRowIndex(o, s, d);
}


long
MFXEditableTable::onCmdSelectColumnIndex(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdSelectColumn(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdSelectRow(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdSelectRow(o, s, d);
}


long
MFXEditableTable::onCmdSelectCell(FXObject*, FXSelector, void*) {
//    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdSelectAll(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdDeselectAll(FXObject*, FXSelector, void*) {
    editEnd();
    return 1;
//    return FXTable::onUngrabbed(o, s, d);
}


long
MFXEditableTable::onCmdMark(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdMark(o, s, d);
}


long
MFXEditableTable::onCmdExtend(FXObject* o, FXSelector s, void* d) {
    editEnd();
    return FXTable::onCmdExtend(o, s, d);
}


void
MFXEditableTable::writeProtectCol(int col, bool val) {
    if (val) {
        myWriteProtectedCols.insert(col);
    } else {
        std::set<int>::iterator i = myWriteProtectedCols.find(col);
        if (i != myWriteProtectedCols.end()) {
            myWriteProtectedCols.erase(i);
        }
    }
}


/****************************************************************************/
