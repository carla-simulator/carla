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
/// @file    GUIParameterTableItem.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date
///
// A single line in a parameter window
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <fx.h>
#include <utils/common/ValueSource.h>
#include <utils/common/ToString.h>
#include <utils/gui/div/GUIParam_PopupMenu.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/windows/GUIAppEnum.h>


// ===========================================================================
// class definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// GUIParameterTableItemInterface
// ---------------------------------------------------------------------------
/**
 * @class GUIParameterTableItemInterface
 * @brief Interface to a single line in a parameter window
 *
 * Because a single line in a parameter window may display different kinds of
 *  information (different value types, dynamic or static information), an
 *  interface is needed in order to allow a common access to the functions of
 *  a line - basically the possibility to open a tracker (GUIParameterTracker)
 *  for the stored, dynamic value.
 *
 * The implementation is done by GUIParameterTableItem.
 *
 * @see GUIParameterTracker
 * @see GUIParameterTableItem
 */
class GUIParameterTableItemInterface {
public:
    /// @brief Destructor
    virtual ~GUIParameterTableItemInterface()  {}


    /// @name Methods to be implemented by derived classes
    /// @{

    /// @brief Returns the information whether the value changes over simulation time
    virtual bool dynamic() const = 0;

    /// @brief Forces an update of the value
    virtual void update() = 0;

    /// @brief Returns a double-typed copy of the value-source
    virtual ValueSource<double>* getdoubleSourceCopy() const = 0;

    /// @brief Returns the name of the value
    virtual const std::string& getName() const = 0;
    /// @}
};


// ---------------------------------------------------------------------------
// GUIParameterTableItem
// ---------------------------------------------------------------------------
/**
 * @class GUIParameterTableItem
 * @brief Instance of a single line in a parameter window
 *
 * This class represents a single item of a parameter table and is an
 *  implementation of the GUIParameterTableItemInterface that allows different
 *  value-types.
 *
 * As some values may change over the simulation, this class holds the
 * information whether they change and how to ask for new values if they do.
 *
 * @see GUIParameterTracker
 * @see GUIParameterTableItemInterface
 */
template<class T>
class GUIParameterTableItem : public GUIParameterTableItemInterface {
public:
    /** @brief Constructor for changing (dynamic) values
     *
     * @param[in] table The table this item belongs to
     * @param[in] pos The row of the table this item fills
     * @param[in] name The name of the represented value
     * @param[in] dynamic Information whether this value changes over time
     * @param[in] src The value source
     * @todo Consider using a reference to the table
     * @todo Check whether the name should be stored in GUIParameterTableItemInterface
     */
    GUIParameterTableItem(FXTable* table, unsigned pos, const std::string& name,
                          bool dynamic, ValueSource<T>* src) :
        myAmDynamic(dynamic), myName(name), myTablePosition((FXint) pos), mySource(src),
        myValue(src->getValue()), myTable(table) {
        init(dynamic, toString<T>(src->getValue()));
    }

    /** @brief Constructor for non-changing (static) values
     *
     * @param[in] table The table this item belongs to
     * @param[in] pos The row of the table this item fills
     * @param[in] name The name of the represented value
     * @param[in] dynamic Information whether this value changes over time
     * @param[in] value The value
     * @todo Consider using a reference to the table
     * @todo Check whether the name should be stored in GUIParameterTableItemInterface
     * @todo Should never be dynamic!?
     */
    GUIParameterTableItem(FXTable* table, unsigned pos, const std::string& name,
                          bool dynamic, T value) :
        myAmDynamic(dynamic), myName(name), myTablePosition((FXint) pos), mySource(0),
        myValue(value), myTable(table) {
        init(dynamic, toString<T>(value));
    }

    /// @brief Destructor
    ~GUIParameterTableItem() {
        delete mySource;
    }

    /** @brief Initialises the line
     *
     * Fills the line using the name, the current value, and the information
     *  whether the value changes over time.
     *
     * @param[in] dynamic Information whether this value changes over time
     * @param[in] value The current (initial) value
     */
    void init(bool dynamic, std::string value) {
        myTable->setItemText(myTablePosition, 0, myName.c_str());
        myTable->setItemText(myTablePosition, 1, value.c_str());
        if (dynamic) {
            if (getdoubleSourceCopy() == nullptr) {
                myTable->setItemIcon(myTablePosition, 2, GUIIconSubSys::getIcon(GUIIcon::YES));
            } else {
                myTable->setItemIcon(myTablePosition, 2, GUIIconSubSys::getIcon(GUIIcon::TRACKER));
            }
        } else {
            myTable->setItemIcon(myTablePosition, 2, GUIIconSubSys::getIcon(GUIIcon::NO));
        }
        myTable->setItemJustify(myTablePosition, 2, FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
    }

    /// @brief Returns the information whether this item may change over time
    bool dynamic() const {
        return myAmDynamic;
    }

    /// @brief Returns the name of this value
    const std::string& getName() const {
        return myName;
    }

    /** @brief Resets the value if it's dynamic
     *
     * If the value is dynamic, the current value is retrieved from the value
     *  source. If it is different from the previous one (stored in myValue),
     *  it is stored in myValue and set as the current value text within the
     *  according table cell.
     */
    void update() {
        if (!dynamic() || mySource == 0) {
            return;
        }
        T value = mySource->getValue();
        if (value != myValue) {
            myValue = value;
            myTable->setItemText(myTablePosition, 1, toString<T>(myValue).c_str());
        }
    }

    /// @brief Returns a copy of the source if the value is dynamic
    ValueSource<T>* getSourceCopy() const {
        if (mySource == 0) {
            return 0;
        }
        return mySource->copy();
    }

    /// @brief Returns a double-typed copy of the source if the value is dynamic
    ValueSource<double>* getdoubleSourceCopy() const {
        if (mySource == 0) {
            return 0;
        }
        return mySource->makedoubleReturningCopy();
    }

private:
    /// @brief Information whether the value may change
    bool myAmDynamic;

    /// @brief The name of this value
    std::string myName;

    /// @brief The position within the table
    FXint myTablePosition;

    /// @brief The source to gain new values from; this source is==0 if the values are not dynamic
    ValueSource<T>* mySource;

    /// @brief A backup of the value to avoid the redrawing when nothing has changed
    T myValue;

    /// @brief The table this entry belongs to
    FXTable* myTable;
};
