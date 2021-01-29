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
/// @file    GNEFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2016
///
// Abstract class for lateral frames in NetEdit
/****************************************************************************/
#pragma once
#include "GNEFrameModuls.h"
#include "GNEFrameAttributesModuls.h"

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEFrame
 * Abstract class for lateral frames in NetEdit
 */
class GNEFrame : public FXVerticalFrame {

    /// @brief friend class
    friend class GNEFrameModuls;
    friend class GNEFrameAttributesModuls;

public:
    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     * @brief frameLabel label of the frame
     */
    GNEFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet, const std::string& frameLabel);

    /// @brief destructor
    ~GNEFrame();

    /// @brief focus upper element of frame
    void focusUpperElement();

    /**@brief show Frame
     * @note some GNEFrames needs a re-implementation
     */
    virtual void show();

    /**@brief hide Frame
     * @note some GNEFrames needs a re-implementation
     */
    virtual void hide();

    /// @brief set width of GNEFrame
    void setFrameWidth(int newWidth);

    /// @brief get view net
    GNEViewNet* getViewNet() const;

    /// @brief get the label for the frame's header
    FXLabel* getFrameHeaderLabel() const;

    /// @brief get font of the header's frame
    FXFont* getFrameHeaderFont() const;

    /// @brief function called after undo/redo in the current frame (can be reimplemented in frame children)
    virtual void updateFrameAfterUndoRedo();

protected:
    FOX_CONSTRUCTOR(GNEFrame)

    /// @name functions called by moduls that can be reimplemented in frame children
    /// @{

    /// @brief Tag selected in TagSelector
    virtual void tagSelected();

    /// @brief selected demand element in DemandElementSelector
    virtual void demandElementSelected();

    /// @brief build a shaped element using the drawed shape (can be reimplemented in frame children)
    virtual bool shapeDrawed();

    /// @brief function called after set a valid attribute in AttributeCreator/AttributeEditor/ParametersEditor/...
    virtual void attributeUpdated();

    /// @brief open AttributesCreator extended dialog (can be reimplemented in frame children)
    virtual void attributesEditorExtendedDialogOpened();

    /// @brief open AttributesCreator extended dialog (can be reimplemented in frame children)
    virtual void selectedOverlappedElement(GNEAttributeCarrier* AC);

    /// @brief create path (can be reimplemented in frame children)
    virtual void createPath();

    /// @}

    /// @brief Open help attributes dialog
    void openHelpAttributesDialog(const GNETagProperties& tagProperties) const;

    /// @brief get predefinedTagsMML
    const std::vector<std::string>& getPredefinedTagsMML() const;

    /// @brief View Net
    GNEViewNet* myViewNet = nullptr;

    /// @brief Vertical frame that holds all widgets of frame
    FXVerticalFrame* myContentFrame = nullptr;

    /// @brief fame for header elements
    FXHorizontalFrame* myHeaderFrame = nullptr;

    /// @brief fame for left header elements
    FXHorizontalFrame* myHeaderLeftFrame = nullptr;

    /// @brief fame for right header elements
    FXHorizontalFrame* myHeaderRightFrame = nullptr;

private:
    /// @brief scroll windows that holds the content frame
    FXScrollWindow* myScrollWindowsContents = nullptr;

    /// @brief static Font for the Header (it's common for all headers, then create only one time)
    static FXFont* myFrameHeaderFont;

    /// @brief the label for the frame's header
    FXLabel* myFrameHeaderLabel = nullptr;

    /// @brief Map of attribute ids to their (readable) string-representation (needed for SUMOSAXAttributesImpl_Cached)
    std::vector<std::string> myPredefinedTagsMML;

    /// @brief Invalidated copy constructor.
    GNEFrame(const GNEFrame&) = delete;

    /// @brief Invalidated assignment operator.
    GNEFrame& operator=(const GNEFrame&) = delete;
};
