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
/// @file    MFXAddEditTypedTable.h
/// @author  Daniel Krajzewicz
/// @date    2004-07-02
///
// missing_desc
/****************************************************************************/
#ifndef MFXAddEditTypedTable_h
#define MFXAddEditTypedTable_h
#include <config.h>

#include "MFXEditableTable.h"
#include <vector>

enum CellType {
    CT_UNDEFINED = -1,
    CT_REAL = 0,
    CT_STRING = 1,
    CT_INT = 2,
    CT_BOOL = 3,
    CT_ENUM = 4,
    CT_MAX
};



class MFXAddEditTypedTable : public FXTable {
    FXDECLARE(MFXAddEditTypedTable)
public:
    MFXAddEditTypedTable(FXComposite* p, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = 0, FXint x = 0, FXint y = 0, FXint w = 0, FXint h = 0, FXint pl = DEFAULT_MARGIN, FXint pr = DEFAULT_MARGIN, FXint pt = DEFAULT_MARGIN, FXint pb = DEFAULT_MARGIN);
    ~MFXAddEditTypedTable();

public:
    struct NumberCellParams {
        int pos;
        double min;
        double max;
        double steps1;
        double steps2;
        double steps3;
        std::string format;
    };

    struct EditedTableItem {
        FXTableItem* item;
        int row;
        int col;
        bool updateOnly;
    };


    CellType getCellType(int pos) const;
    void setCellType(int pos, CellType t);
    void setNumberCellParams(int pos, double min, double max,
                             double steps1, double steps2, double steps3,
                             const std::string& format);
    NumberCellParams getNumberCellParams(int pos) const;
    void setEnums(int pos, const std::vector<std::string>& params);
    void addEnum(int pos, const std::string& e);
    const std::vector<std::string>& getEnums(int pos) const;
    /*
        class FXTableItem_Int : public FXTableItem {
        public:
            FXTableItem_Int(const FXString& text,FXIcon* ic=NULL,void* ptr=NULL);
            ~FXTableItem_Int();
        protected:
            /// Create input control for editing this item
            virtual FXWindow *getControlFor(FXTable* table);

            /// Set value from input control
            virtual void setFromControl(FXWindow *control);

        };

        class FXTableItem_Real : public FXTableItem {
        public:
            FXTableItem_Real(const FXString& text,FXIcon* ic=NULL,void* ptr=NULL);
            ~FXTableItem_Real();
        protected:
            /// Create input control for editing this item
            virtual FXWindow *getControlFor(FXTable* table);

            /// Set value from input control
            virtual void setFromControl(FXWindow *control);

        };

        class FXTableItem_Enum : public FXTableItem {
        public:
            FXTableItem_Enum(const FXString& text,FXIcon* ic=NULL,void* ptr=NULL);
            ~FXTableItem_Enum();
        protected:
            /// Create input control for editing this item
            virtual FXWindow *getControlFor(FXTable* table);

            /// Set value from input control
            virtual void setFromControl(FXWindow *control);

        };

        class FXTableItem_Bool : public FXTableItem {
        public:
            FXTableItem_Bool(const FXString& text,FXIcon* ic=NULL,void* ptr=NULL);
            ~FXTableItem_Bool();
        protected:
            /// Create input control for editing this item
            virtual FXWindow *getControlFor(FXTable* table);

            /// Set value from input control
            virtual void setFromControl(FXWindow *control);

        };
    */

    enum {
        ID_TEXT_CHANGED = FXTable::ID_LAST,
        ID_LAST
    };

    void cancelInput();
    long onClicked(FXObject*, FXSelector, void* ptr);
    long onDoubleClicked(FXObject*, FXSelector, void* ptr);
    long onLeftBtnRelease(FXObject*, FXSelector, void* ptr);
    long onLeftBtnPress(FXObject*, FXSelector, void* ptr);

protected:
    virtual FXWindow* getControlForItem(FXint r, FXint c);
    virtual void setItemFromControl(FXint r, FXint c, FXWindow* control);
    void acceptInput(FXbool notify);
    void setItemFromControl_NoRelease(FXint r, FXint c, FXWindow* control);

protected:
    std::vector<CellType> myCellTypes;
    std::vector<NumberCellParams> myNumberCellParams;
    std::vector<std::vector<std::string> > myEnums;

protected:
    MFXAddEditTypedTable() { }

};


#endif
