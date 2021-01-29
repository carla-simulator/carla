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
/// @file    GNEVehicleTypeDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// Dialog for edit vehicleTypes
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOVehicleClass.h>

#include "GNEDemandElementDialog.h"

// ===========================================================================
// class declarations
// ===========================================================================

class GNEVehicleType;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEVehicleTypeDialog
 * @brief Dialog for edit rerouter intervals
 */

class GNEVehicleTypeDialog : public GNEDemandElementDialog {

public:
    /// @brief class for VClasses
    class VTypeAtributes : protected FXVerticalFrame {
        /// @brief FOX-declaration
        FXDECLARE(GNEVehicleTypeDialog::VTypeAtributes)

    public:
        /// @brief class for VClasses
        class VClassRow : protected FXHorizontalFrame {

        public:
            /// @brief constructor
            VClassRow(VTypeAtributes* VTypeAtributesParent, FXVerticalFrame* column);

            /// @brief set vehicle class
            SUMOVehicleClass setVariable();

            /// @brief update values
            SUMOVehicleClass updateValue();

        private:
            /// @brief pointer to VTypeAtributes parent
            VTypeAtributes* myVTypeAtributesParent;

            /// @brief FXComboBox for VClass
            FXComboBox* myComboBoxVClass;

            /// @brief label with image of VClass
            FXLabel* myComboBoxVClassLabelImage;

            /// @brief set VClass texture
            void setVClassLabelImage();
        };

        /// @brief class for VShapeRow
        class VShapeRow : protected FXHorizontalFrame {

        public:
            /// @brief constructor
            VShapeRow(VTypeAtributes* VTypeAtributesParent, FXVerticalFrame* column);

            /// @brief set variables
            void setVariable();

            /// @brief update values
            void updateValues();

        private:
            /// @brief pointer to VTypeAtributes parent
            VTypeAtributes* myVTypeAtributesParent;

            /// @brief FXComboBox for Shape
            FXComboBox* myComboBoxShape;

            /// @brief label with image of Shape
            FXLabel* myComboBoxShapeLabelImage;

            /// @brief set VShape texture
            void setVShapeLabelImage();

        };

        /// @brief class used for represent rows with Vehicle Type parameters
        class VTypeAttributeRow : private FXHorizontalFrame {
        public:

            /// @brief Attribute type
            enum RowAttrType {
                ROWTYPE_STRING,
                ROWTYPE_COLOR,
                ROWTYPE_FILENAME,
                ROWTYPE_COMBOBOX
            };

            /// @brief constructor
            VTypeAttributeRow(VTypeAtributes* VTypeAtributesParent, FXVerticalFrame* verticalFrame, const SumoXMLAttr attr, const RowAttrType rowAttrType, const std::vector<std::string>& values = {});

            /// @brief set Variablen in VehicleType (using default value obtained from GNEAttributeCarrier)
            void setVariable();

            /// @brief set Variablen in VehicleType (Specifying default value)
            void setVariable(const std::string& defaultValue);

            /// @brief update value of Vehicle Type (using default value obtained from GNEAttributeCarrier)
            void updateValue();

            /// @brief update value of Vehicle Type (Specifying default value)
            void updateValue(const std::string& defaultValue);

            /// @brief get button
            const FXButton* getButton() const;

            /// @brief open color dialog
            void openColorDialog();

            /// @brief open image file dialog
            void openImageFileDialog();

            /// @brief open OSG file dialog
            void openOSGFileDialog();

        private:
            /// @brief pointer to VTypeAttributeParameters parent
            VTypeAtributes* myVTypeAtributesParent;

            /// @brief edited attribute
            const SumoXMLAttr myAttr;

            /// @brief RowAttrType
            const RowAttrType myRowAttrType;

            /// @brief button
            FXButton* myButton;

            /// @brief text field
            FXTextField* myTextField;

            /// @brief ComboBox for attributes with limited values
            FXComboBox* myComboBox;

            /// @brief filter attribute name
            FXString filterAttributeName(const SumoXMLAttr attr) const;
        };

        /// @brief constructor
        VTypeAtributes(GNEVehicleTypeDialog* vehicleTypeDialog, FXHorizontalFrame* column);

        /// @brief build commmon attributes (A)
        void buildAttributesA(FXVerticalFrame* column);

        /// @brief build commmon attributes (B)
        void buildAttributesB(FXVerticalFrame* column);

        /// @brief build JunctionModel attributes (A)
        void buildJunctionModelAttributesA(FXVerticalFrame* column);

        /// @brief build JunctionModel attributes (B)
        void buildJunctionModelAttributesB(FXVerticalFrame* column);

        /// @brief build LaneChangeModel attributes
        void buildLaneChangeModelAttributes(FXVerticalFrame* column);

        /// @brief update values
        void updateValues();

        /// @name FOX-callbacks
        /// @{
        /// @event called after change a Vehicle Type parameter
        long onCmdSetAttribute(FXObject*, FXSelector, void*);

        /// @event called after press a button dialog
        long onCmdSetAttributeDialog(FXObject* obj, FXSelector, void*);

        /// @}

    protected:
        FOX_CONSTRUCTOR(VTypeAtributes)

        /// @brief FXTextfield for vehicleTypeID
        FXTextField* myTextFieldVehicleTypeID;

        /// @name Common Attributes
        /// @{

        /// @brief vehicle class row
        VClassRow* myVClassRow;

        /// @brief VTypeAttributeRow for color
        VTypeAttributeRow* myColor;

        /// @brief VTypeAttributeRow for Length
        VTypeAttributeRow* myLength;

        /// @brief VTypeAttributeRow for MinGap
        VTypeAttributeRow* myMinGap;

        /// @brief VTypeAttributeRow for MaxSpeed
        VTypeAttributeRow* myMaxSpeed;

        /// @brief VTypeAttributeRow for SpeedFactor
        VTypeAttributeRow* mySpeedFactor;

        /// @brief VTypeAttributeRow for SpeedDev
        VTypeAttributeRow* mySpeedDev;

        /// @brief VTypeAttributeRow for EmissionClass
        VTypeAttributeRow* myEmissionClass;

        /// @brief vehicle shape row
        VShapeRow* myVShapeRow;

        /// @brief VTypeAttributeRow for Width
        VTypeAttributeRow* myWidth;

        /// @brief VTypeAttributeRow for Height
        VTypeAttributeRow* myHeight;

        /// @brief VTypeAttributeRow for Filename
        VTypeAttributeRow* myFilename;

        /// @brief VTypeAttributeRow for OSG
        VTypeAttributeRow* myOSGFile;

        /// @brief VTypeAttributeRow for carriage length
        VTypeAttributeRow* myCarriageLength;

        /// @brief VTypeAttributeRow for locomotive length
        VTypeAttributeRow* myLocomotiveLength;

        /// @brief VTypeAttributeRow for carriage GAP
        VTypeAttributeRow* myCarriageGap;

        /// @brief VTypeAttributeRow for LaneChangeModel
        VTypeAttributeRow* myLaneChangeModel;

        /// @brief VTypeAttributeRow for PersonCapacity
        VTypeAttributeRow* myPersonCapacity;

        /// @brief VTypeAttributeRow for ContainerCapacity
        VTypeAttributeRow* myContainerCapacity;

        /// @brief VTypeAttributeRow for BoardingDuration
        VTypeAttributeRow* myBoardingDuration;

        /// @brief VTypeAttributeRow for LoadingDuration
        VTypeAttributeRow* myLoadingDuration;

        /// @brief VTypeAttributeRow for LatAlignment
        VTypeAttributeRow* myLatAlignment;

        /// @brief VTypeAttributeRow for MinGapLat
        VTypeAttributeRow* myMinGapLat;

        /// @brief VTypeAttributeRow for MaxSpeedLat
        VTypeAttributeRow* myMaxSpeedLat;

        /// @brief VTypeAttributeRow for ActionStepLength
        VTypeAttributeRow* myActionStepLength;

        /// @brief VTypeAttributeRow for Probability
        VTypeAttributeRow* myProbability;

        /// @}

        /// @name JM Attributes
        /// @{

        /// @brief VTypeAttributeRow for Crossing Gap
        VTypeAttributeRow* myJMCrossingGap;

        /// @brief VTypeAttributeRow for Ignore Keep Clear Time
        VTypeAttributeRow* myJMIgnoreKeepclearTime;

        /// @brief VTypeAttributeRow for Drive After Yellow Time
        VTypeAttributeRow* myJMDriveAfterYellowTime;

        /// @brief VTypeAttributeRow for Drive After Red Time
        VTypeAttributeRow* myJMDriveAfterRedTime;

        /// @brief VTypeAttributeRow for Drive Red Speed
        VTypeAttributeRow* myJMDriveRedSpeed;

        /// @brief VTypeAttributeRow for Ignore FOE Probability
        VTypeAttributeRow* myJMIgnoreFoeProb;

        /// @brief VTypeAttributeRow for Ignore FOE Speed
        VTypeAttributeRow* myJMIgnoreFoeSpeed;

        /// @brief VTypeAttributeRow for Sigma Minor
        VTypeAttributeRow* myJMSigmaMinor;

        /// @brief VTypeAttributeRow for Time GAP Minor
        VTypeAttributeRow* myJMTimeGapMinor;

        /// @brief VTypeAttributeRow for Impatience
        VTypeAttributeRow* myJMImpatience;

        /// @}


        /// @name LCM Attributes
        /// @{

        /// @brief VTypeAttributeRow for strategic param
        VTypeAttributeRow* myLCAStrategicParam;

        /// @brief VTypeAttributeRow for cooperative param
        VTypeAttributeRow* myLCACooperativeParam;

        /// @brief VTypeAttributeRow for speed gain param
        VTypeAttributeRow* myLCASpeedgainParam;

        /// @brief VTypeAttributeRow for keep right param
        VTypeAttributeRow* myLCAKeeprightParam;

        /// @brief VTypeAttributeRow for sublane param
        VTypeAttributeRow* myLCASublaneParam;

        /// @brief VTypeAttributeRow for opposite param
        VTypeAttributeRow* myLCAOppositeParam;

        /// @brief VTypeAttributeRow for pushy
        VTypeAttributeRow* myLCAPushy;

        /// @brief VTypeAttributeRow for pushy gap
        VTypeAttributeRow* myLCAPushygap;

        /// @brief VTypeAttributeRow for assertive
        VTypeAttributeRow* myLCAAssertive;

        /// @brief VTypeAttributeRow for impatience
        VTypeAttributeRow* myLCAImpatience;

        /// @brief VTypeAttributeRow for time to impatience
        VTypeAttributeRow* myLCATimeToImpatience;

        /// @brief VTypeAttributeRow for accel lat
        VTypeAttributeRow* myLCAAccelLat;

        /// @brief VTypeAttributeRow for loock ahead left
        VTypeAttributeRow* myLCALookAheadLeft;

        /// @brief VTypeAttributeRow for speed gain right
        VTypeAttributeRow* myLCASpeedGainRight;

        /// @brief VTypeAttributeRow for max speed lat standing
        VTypeAttributeRow* myLCAMaxSpeedLatStanding;

        /// @brief VTypeAttributeRow for max speed lat factor
        VTypeAttributeRow* myLCAMaxSpeedLatFactor;

        /// @brief VTypeAttributeRow for turn alignment distance
        VTypeAttributeRow* myLCATurnAlignmentDistance;

        /// @brief VTypeAttributeRow for overtake right
        VTypeAttributeRow* myLCAOvertakeRight;

        /// @brief VTypeAttributeRow for experimental
        /* VTypeAttributeRow* myLCAExperimental; */

        /// @}

    private:
        /// @brief pointer to Vehicle Type dialog parent
        GNEVehicleTypeDialog* myVehicleTypeDialog;
    };

    /// @brief class for CarFollowingModel
    class CarFollowingModelParameters : public FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEVehicleTypeDialog::CarFollowingModelParameters)

    public:
        /// @brief constructor
        CarFollowingModelParameters(GNEVehicleTypeDialog* vehicleTypeDialog, FXHorizontalFrame* column);

        /// @brief refresh Car Following Model Fields
        void refreshCFMFields();

        /// @brief update values
        void updateValues();

        /// @name FOX-callbacks
        /// @{
        /// @event called after change a CFM variable
        long onCmdSetVariable(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(CarFollowingModelParameters)

        /// @brief class used for represent rows with Car Following Model parameters
        class CarFollowingModelRow : public FXHorizontalFrame {
        public:
            /// @brief constructor
            CarFollowingModelRow(CarFollowingModelParameters* carFollowingModelParametersParent, FXVerticalFrame* verticalFrame, SumoXMLAttr attr);

            /// @brief set Variablen in VehicleType
            void setVariable();

            /// @brief update value of Vehicle Type
            void updateValue();

        private:
            /// @brief pointer to CarFollowingModelParameters parent
            CarFollowingModelParameters* myCarFollowingModelParametersParent;

            /// @brief edited attribute
            SumoXMLAttr myAttr;

            /// @brief text field
            FXTextField* myTextField;
        };

    private:
        /// @brief pointer to Vehicle Type dialog parent
        GNEVehicleTypeDialog* myVehicleTypeDialog;

        /// @brief vector with the Car Following Model Row
        std::vector<CarFollowingModelRow*> myRows;

        /// @brief Row for CarFollowModel
        FXComboBox* myComboBoxCarFollowModel;

        /// @brief Vertical Frame for CarFollowingModelRow
        FXVerticalFrame* myVerticalFrameRows;

        /// @brief Row for Accel
        CarFollowingModelRow* myAccelRow;

        /// @brief Row for Decel
        CarFollowingModelRow* myDecelRow;

        /// @brief Row for aparent Decel
        CarFollowingModelRow* myApparentDecelRow;

        /// @brief Row for emergency Decel
        CarFollowingModelRow* myEmergencyDecelRow;

        /// @brief Row for Sigma
        CarFollowingModelRow* mySigmaRow;

        /// @brief Row for Tau
        CarFollowingModelRow* myTauRow;

        /// @brief Row for MinGapFactor
        CarFollowingModelRow* myMinGapFactorRow;

        /// @brief Row for MinGap (only for Kerner)
        CarFollowingModelRow* myKRow;

        /// @brief Row for MinGap (only for Kerner)
        CarFollowingModelRow* myPhiRow;

        /// @brief Row for MinGap (only for IDM)
        CarFollowingModelRow* myDeltaRow;

        /// @brief Row for MinGap(only for IDM)
        CarFollowingModelRow* mySteppingRow;

        /// @brief Row for Security (only for Wiedemann)
        CarFollowingModelRow* mySecurityRow;

        /// @brief Row for Estimation (only for Wiedemann)
        CarFollowingModelRow* myEstimationRow;

        /// @brief Row for TMP1
        CarFollowingModelRow* myTmp1Row;

        /// @brief Row for TMP2
        CarFollowingModelRow* myTmp2Row;

        /// @brief Row for TMP3
        CarFollowingModelRow* myTmp3Row;

        /// @brief Row for TMP4
        CarFollowingModelRow* myTmp4Row;

        /// @brief Row for TMP5
        CarFollowingModelRow* myTmp5Row;

        /// @brief Row for TrainType
        CarFollowingModelRow* myTrainTypeRow;

        /// @brief Row for TauLast
        CarFollowingModelRow* myTrauLastRow;

        /// @brief Row for Aprob
        CarFollowingModelRow* myAprobRow;

        /// @brief Row for Adapt Factor
        CarFollowingModelRow* myAdaptFactorRow;

        /// @brief Row for Adapt Time
        CarFollowingModelRow* myAdaptTimeRow;

        /// @brief temporal label for incomplete attributes
        FXLabel* myLabelIncompleteAttribute;
    };

    /// @brief constructor
    GNEVehicleTypeDialog(GNEDemandElement* editedVehicleType, bool updatingElement);

    /// @brief destructor
    ~GNEVehicleTypeDialog();

    /// @name FOX-callbacks
    /// @{
    /// @brief event after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief event after press reset button
    long onCmdReset(FXObject*, FXSelector, void*);

    /// @event after change a variable of vehicle type
    long onCmdSetVariable(FXObject*, FXSelector, void*);
    /// @}

private:
    /// @brief flag to check if current vehicleType is valid
    bool myVehicleTypeValid;

    /// @brief current sumo attribute invalid
    SumoXMLAttr myInvalidAttr;

    /// @brief Vehicle Type Common Attributes
    VTypeAtributes* myVTypeAtributes;

    /// @brief Car Following model parameters
    CarFollowingModelParameters* myCarFollowingModelParameters;

    /// @brief Invalidated copy constructor.
    GNEVehicleTypeDialog(const GNEVehicleTypeDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNEVehicleTypeDialog& operator=(const GNEVehicleTypeDialog&) = delete;
};

