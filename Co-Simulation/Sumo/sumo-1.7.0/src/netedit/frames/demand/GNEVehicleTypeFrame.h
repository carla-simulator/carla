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
/// @file    GNEVehicleTypeFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// The Widget for edit Vehicle Type elements
/****************************************************************************/
#pragma once

#include <netedit/frames/GNEFrame.h>

// ===========================================================================
// class declarations
// ===========================================================================

class GNEVehicle;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEVehicleTypeFrame
 */
class GNEVehicleTypeFrame : public GNEFrame {

public:

    // ===========================================================================
    // class VehicleTypeSelector
    // ===========================================================================

    class VehicleTypeSelector : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEVehicleTypeFrame::VehicleTypeSelector)

    public:
        /// @brief constructor
        VehicleTypeSelector(GNEVehicleTypeFrame* vehicleTypeFrameParent);

        /// @brief destructor
        ~VehicleTypeSelector();

        /// @brief get current Vehicle Type
        GNEDemandElement* getCurrentVehicleType() const;

        /// @brief set current Vehicle Type
        void setCurrentVehicleType(GNEDemandElement* vType);

        /// @brief refresh vehicle type selector
        void refreshVehicleTypeSelector();

        /// @brief refresh vehicle type selector (only IDs, without refreshing attributes)
        void refreshVehicleTypeSelectorIDs();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when the user select another element in ComboBox
        long onCmdSelectItem(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(VehicleTypeSelector)

    private:
        /// @brief pointer to Frame Parent
        GNEVehicleTypeFrame* myVehicleTypeFrameParent;

        /// @brief pointer to current vehicle type
        GNEDemandElement* myCurrentVehicleType;

        /// @brief comboBox with the list of elements type
        FXComboBox* myTypeMatchBox;
    };

    // ===========================================================================
    // class VehicleTypeEditor
    // ===========================================================================

    class VehicleTypeEditor : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEVehicleTypeFrame::VehicleTypeEditor)

    public:
        /// @brief constructor
        VehicleTypeEditor(GNEVehicleTypeFrame* vehicleTypeFrameParent);

        /// @brief destructor
        ~VehicleTypeEditor();

        /// @brief show VehicleTypeEditor modul
        void showVehicleTypeEditorModul();

        /// @brief hide VehicleTypeEditor box
        void hideVehicleTypeEditorModul();

        /// @brief update VehicleTypeEditor modul
        void refreshVehicleTypeEditorModul();

        /// @name FOX-callbacks
        /// @{
        /// @brief Called when "Vreate Vehicle Type" button is clicked
        long onCmdCreateVehicleType(FXObject*, FXSelector, void*);

        /// @brief Called when "Delete Vehicle Type" button is clicked
        long onCmdDeleteVehicleType(FXObject*, FXSelector, void*);

        /// @brief Called when "Delete Vehicle Type" button is clicked
        long onCmdResetVehicleType(FXObject*, FXSelector, void*);

        /// @brief Called when "Copy Vehicle Type" button is clicked
        long onCmdCopyVehicleType(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(VehicleTypeEditor)

    private:
        /// @brief pointer to vehicle type Frame Parent
        GNEVehicleTypeFrame* myVehicleTypeFrameParent;

        /// @brief "create vehicle type" button
        FXButton* myCreateVehicleTypeButton;

        /// @brief "delete vehicle type" button
        FXButton* myDeleteVehicleTypeButton;

        /// @brief "delete default vehicle type" button
        FXButton* myResetDefaultVehicleTypeButton;

        /// @brief "copy vehicle type"
        FXButton* myCopyVehicleTypeButton;
    };

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEVehicleTypeFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEVehicleTypeFrame();

    /// @brief show Frame
    void show();

    /// @brief get vehicle type selector
    VehicleTypeSelector* getVehicleTypeSelector() const;

protected:
    /// @brief function called after set a valid attribute in AttributeCreator/AttributeEditor/ParametersEditor/...
    void attributeUpdated();

    /// @brief open AttributesCreator extended dialog (used for editing advance attributes of Vehicle Types)
    void attributesEditorExtendedDialogOpened();

private:
    /// @brief vehicle type selector
    VehicleTypeSelector* myVehicleTypeSelector;

    /// @brief editorinternal vehicle type attributes
    GNEFrameAttributesModuls::AttributesEditor* myVehicleTypeAttributesEditor;

    /// @brief modul for open extended attributes dialog
    GNEFrameAttributesModuls::AttributesEditorExtended* myAttributesEditorExtended;

    /// @brief Vehicle Type editor (Create, copy, etc.)
    VehicleTypeEditor* myVehicleTypeEditor;
};
