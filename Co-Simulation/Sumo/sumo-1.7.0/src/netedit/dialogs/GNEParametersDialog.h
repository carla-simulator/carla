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
/// @file    GNEParametersDialog.h
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2018
///
// Dialog for edit parameters
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOVehicleClass.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <netedit/frames/GNEFrameAttributesModuls.h>

// ===========================================================================
// class definitions
// ===========================================================================

class GNEAttributeCarrier;
class GNEViewNet;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEParametersDialog
 * @brief Dialog for edit  parameters
 */
class GNEParametersDialog : public FXDialogBox {
    /// @brief FOX-declaration
    FXDECLARE(GNEParametersDialog)

public:

    // ===========================================================================
    // class ParametersValues
    // ===========================================================================

    class ParametersValues : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEParametersDialog::ParametersValues)

    public:
        /// @brief constructor
        ParametersValues(FXHorizontalFrame* frame, GNEParametersDialog* ParameterDialogParent);

        /// @brief destructor
        ~ParametersValues();

        /// @brief update values
        void updateValues();

        /// @brief set  parameters
        void setParameters(const std::vector<std::pair<std::string, std::string> >& newParameters);

        /// @brief add a single  parameter
        void addParameter(std::pair<std::string, std::string> newParameter);

        /// @brief clear all  parameters
        void clearParameters();

        /// @name FOX-callbacks
        /// @{
        /// @brief on paint function (reimplemented from FXGroupBox)
        long onPaint(FXObject* o, FXSelector f, void* p);

        /// @brief event when user change an attribute
        long onCmdSetAttribute(FXObject*, FXSelector, void*);

        /// @brief event when user press a remove (or add) button
        long onCmdButtonPress(FXObject*, FXSelector, void*);

        /// @}

    protected:
        FOX_CONSTRUCTOR(ParametersValues)

    private:
        /// @brief class for  parameters Row
        class ParameterRow {

        public:
            /// @brief constructor
            ParameterRow(ParametersValues* ParametersValues, FXVerticalFrame* verticalFrameParent);

            /// @brief destructor
            ~ParameterRow();

            /// @brief disable row
            void disableRow();

            /// @brief enable rlow
            void enableRow(const std::string& parameter, const std::string& value) const;

            /// @brief toogle add button
            void toogleAddButton();

            /// @brief check if remove button is in mode "add"
            bool isButtonInAddMode() const;

            /// @brief copy values of other parameter Row
            void copyValues(const ParameterRow& other);

            /// @brief TextField for parameter
            FXTextField* keyField;

            /// @brief TextField for value
            FXTextField* valueField;

            /// @brief Button for add or remove row
            FXButton* button;
        private:
            /// @brief frame in which elements of ParameterRow are placed
            FXHorizontalFrame* horizontalFrame;
        };

        /// @brief label for key (its neccesary because has to be resized in every onPaint() iteration)
        FXLabel* myKeyLabel;

        /// @brief vertical frame in which rows are placed
        FXVerticalFrame* myVerticalFrameRow;

        /// @brief vector with the ParameterRows
        std::vector<ParameterRow*> myParameterRows;

        /// @brief pointer to ParameterDialog parent
        GNEParametersDialog* myParameterDialogParent;
    };

    // ===========================================================================
    // class ParametersOptions
    // ===========================================================================

    class ParametersOptions : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEParametersDialog::ParametersOptions)

    public:
        /// @brief constructor
        ParametersOptions(FXHorizontalFrame* frame, GNEParametersDialog* ParameterDialogParent);

        /// @brief destructor
        ~ParametersOptions();

        /// @name FOX-callbacks
        /// @{
        /// @brief event when user press load  parameters button
        long onCmdLoadParameters(FXObject*, FXSelector, void*);

        /// @brief event when user press save  parameters button
        long onCmdSaveParameters(FXObject*, FXSelector, void*);

        /// @brief event when user press clear  parameters button
        long onCmdClearParameters(FXObject*, FXSelector, void*);

        /// @brief event when user press sort  parameters button
        long onCmdSortParameters(FXObject*, FXSelector, void*);

        /// @brief event when user press help  parameters button
        long onCmdHelpParameter(FXObject*, FXSelector, void*);

        /// @}

    protected:
        FOX_CONSTRUCTOR(ParametersOptions)

    private:

        /// @class GNEParameterHandler
        /// @brief load  parameters from a filename
        class GNEParameterHandler : public SUMOSAXHandler {
        public:
            /// @brief Constructor
            GNEParameterHandler(ParametersOptions* ParametersOptionsParent, const std::string& file);

            /// @brief Destructor
            ~GNEParameterHandler();

            /// @name inherited from SAXHandler
            /// @{
            /**@brief Called on the opening of a tag;
             * @param[in] element ID of the currently opened element
             * @param[in] attrs Attributes within the currently opened element
             * @exception ProcessError If something fails
             * @see SAXHandler::myStartElement
             */
            void myStartElement(int element, const SUMOSAXAttributes& attrs);

        private:
            /// @brief pointer to ParametersOptions parent
            ParametersOptions* myParametersOptionsParent;
        };

        /// @brief pointer to Shape Frame Parent
        GNEParametersDialog* myParameterDialogParent;

        /// @brief sort button
        FXButton* mySortButton;

        /// @brief clear button
        FXButton* myClearButton;

        /// @brief load button
        FXButton* myLoadButton;

        /// @brief save button
        FXButton* mySaveButton;

        /// @brief help button
        FXButton* myHelpButton;
    };

    /// @brief Constructor
    GNEParametersDialog(GNEFrameAttributesModuls::ParametersEditor* ParametersEditor);

    /// @brief destructor
    ~GNEParametersDialog();

    /// @name FOX-callbacks
    /// @{
    /// @brief event after press accept button
    long onCmdAccept(FXObject*, FXSelector, void*);

    /// @brief event after press cancel button
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief event after press reset button
    long onCmdReset(FXObject*, FXSelector, void*);
    /// @}

protected:
    FOX_CONSTRUCTOR(GNEParametersDialog)

    /// @brief pointer to ParametersEditor
    GNEFrameAttributesModuls::ParametersEditor* myParametersEditor;

    /// @brief pointer to  parameters values
    ParametersValues* myParametersValues;

    /// @brief pointer to  parameters options
    ParametersOptions* myParametersOptions;

    /// @brief accept button
    FXButton* myAcceptButton;

    /// @brief cancel button
    FXButton* myCancelButton;

    /// @brief cancel button
    FXButton* myResetButton;

    /// @brief current edited  parameters
    std::vector<std::pair<std::string, std::string> > myEditedParameters;

    // @brief copy of current edited  Parameters (used for reset)
    const std::vector<std::pair<std::string, std::string> > myCopyOfParameters;

private:
    /// @brief Invalidated copy constructor.
    GNEParametersDialog(const GNEParametersDialog&) = delete;

    /// @brief Invalidated assignment operator.
    GNEParametersDialog& operator=(const GNEParametersDialog&) = delete;
};

