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
/// @file    GNEPersonTypeFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// The Widget for edit person type (VTypes with vclass='pedestrian) elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>

// ===========================================================================
// class declarations
// ===========================================================================

class GNEPerson;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEPersonTypeFrame
 */
class GNEPersonTypeFrame : public GNEFrame {

public:

    // ===========================================================================
    // class PersonTypeSelector
    // ===========================================================================

    class PersonTypeSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEPersonTypeFrame::PersonTypeSelector)

    public:
        /// @brief constructor
        PersonTypeSelector(GNEPersonTypeFrame* personTypeFrameParent);

        /// @brief destructor
        ~PersonTypeSelector();

        /// @brief get current Vehicle Type
        GNEDemandElement* getCurrentPersonType() const;

        /// @brief set current Vehicle Type
        void setCurrentPersonType(GNEDemandElement* vType);

        /// @brief refresh person type
        void refreshPersonTypeSelector();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user select another element in ComboBox
        long onCmdSelectItem(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(PersonTypeSelector)

    private:
        /// @brief pointer to Frame Parent
        GNEPersonTypeFrame* myPersonTypeFrameParent;

        /// @brief pointer to current person type
        GNEDemandElement* myCurrentPersonType;

        /// @brief comboBox with the list of elements type
        FXComboBox* myTypeMatchBox;
    };

    // ===========================================================================
    // class PersonTypeEditor
    // ===========================================================================

    class PersonTypeEditor : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEPersonTypeFrame::PersonTypeEditor)

    public:
        /// @brief constructor
        PersonTypeEditor(GNEPersonTypeFrame* personTypeFrameParent);

        /// @brief destructor
        ~PersonTypeEditor();

        /// @brief show PersonTypeEditor modul
        void showPersonTypeEditorModul();

        /// @brief hide PersonTypeEditor box
        void hidePersonTypeEditorModul();

        /// @brief update PersonTypeEditor modul
        void refreshPersonTypeEditorModul();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when "Vreate Vehicle Type" button is clicked
        long onCmdCreatePersonType(FXObject*, FXSelector, void*);

        /// @brief Called when "Delete Vehicle Type" button is clicked
        long onCmdDeletePersonType(FXObject*, FXSelector, void*);

        /// @brief Called when "Delete Vehicle Type" button is clicked
        long onCmdResetPersonType(FXObject*, FXSelector, void*);

        /// @brief Called when "Copy Vehicle Type" button is clicked
        long onCmdCopyPersonType(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(PersonTypeEditor)

    private:
        /// @brief pointer to person type Frame Parent
        GNEPersonTypeFrame* myPersonTypeFrameParent;

        /// @brief "create person type" button
        FXButton* myCreatePersonTypeButton;

        /// @brief "delete person type" button
        FXButton* myDeletePersonTypeButton;

        /// @brief "delete default person type" button
        FXButton* myResetDefaultPersonTypeButton;

        /// @brief "copy person type"
        FXButton* myCopyPersonTypeButton;
    };

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEPersonTypeFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEPersonTypeFrame();

    /// @brief show Frame
    void show();

    /// @brief get person type selector
    PersonTypeSelector* getPersonTypeSelector() const;

protected:
    /// @brief function called after set a valid attribute in AttributeCreator/AttributeEditor/ParametersEditor/...
    void attributeUpdated();

private:
    /// @brief person type selector
    PersonTypeSelector* myPersonTypeSelector;

    /// @brief editorinternal person type attributes
    GNEFrameAttributesModuls::AttributesEditor* myPersonTypeAttributesEditor;

    /// @brief Vehicle Type editor (Create, copy, etc.)
    PersonTypeEditor* myPersonTypeEditor;
};
