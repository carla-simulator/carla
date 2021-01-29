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
/// @file    GUIDialog_ViewSettings.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 21. Dec 2005
///
// The dialog to change the view (gui) settings.
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/foxtools/MFXAddEditTypedTable.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MFXIconComboBox;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIDialog_ViewSettings
 * @brief The dialog to change the view (gui) settings.
 *
 * @todo Check whether saving/loading settings should be done via XML
 */
class GUIDialog_ViewSettings : public FXDialogBox {
    // is a FOX-object with an own mapping
    FXDECLARE(GUIDialog_ViewSettings)
public:

    class NamePanel {
    public:
        NamePanel(FXMatrix* parent, GUIDialog_ViewSettings* target,
                  const std::string& title,
                  const GUIVisualizationTextSettings& settings);

        GUIVisualizationTextSettings getSettings();
        void update(const GUIVisualizationTextSettings& settings);

        FXCheckButton* myCheck;
        FXRealSpinner* mySizeDial;
        FXColorWell* myColorWell;
        FXColorWell* myBGColorWell;
        FXCheckButton* myConstSizeCheck;
        FXMatrix* myMatrix0;
    };

    class SizePanel {
    public:
        SizePanel(FXMatrix* parent, GUIDialog_ViewSettings* target,
                  const GUIVisualizationSizeSettings& settings);

        GUIVisualizationSizeSettings getSettings();
        void update(const GUIVisualizationSizeSettings& settings);

        FXRealSpinner* myMinSizeDial;
        FXRealSpinner* myExaggerateDial;
        FXCheckButton* myCheck;
        FXCheckButton* myCheckSelected;
    };

    /** @brief Constructor
     * @param[in] parent The view to report changed settings to
     * @param[in, out] settings The current settings that can be changed
     * @param[in] laneEdgeModeSource Class storing known lane coloring schemes
     * @param[in] vehicleModeSource Class storing known vehicle coloring schemes
     * @param[in] decals Decals used
     * @param[in] decalsLock A lock to set if the decals are changed
     */
    GUIDialog_ViewSettings(GUISUMOAbstractView* parent,
                           GUIVisualizationSettings* settings,
                           std::vector<GUISUMOAbstractView::Decal>* decals,
                           FXMutex* decalsLock);


    /// @brief Destructor
    ~GUIDialog_ViewSettings();

    /// @brief show view settings dialog
    void show();

    /** @brief Sets current settings (called if reopened)
     * @param[in, out] settings The current settings that can be changed
     */
    void setCurrent(GUIVisualizationSettings* settings);



    /// @name FOX-callbacks
    /// @{

    /// @brief Called if the OK-button was pressed
    long onCmdOk(FXObject*, FXSelector, void*);

    /// @brief Called if the Cancel-button was pressed
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// @brief Called if something (color, width, etc.) has been changed
    long onCmdColorChange(FXObject*, FXSelector, void*);

    /// @brief Called if the decals-table was changed
    long onCmdEditTable(FXObject*, FXSelector, void* data);

    /// @brief Called if the name of the scheme was changed
    long onCmdNameChange(FXObject*, FXSelector, void*);

    /// @brief Called if the settings shall be saved into the registry
    long onCmdSaveSetting(FXObject*, FXSelector, void* data);
    /// @brief Called when updating the button that allows to save the settings into the registry
    long onUpdSaveSetting(FXObject*, FXSelector, void* data);

    /// @brief Called if the settings shall be deleted
    long onCmdDeleteSetting(FXObject*, FXSelector, void* data);
    /// @brief Called when updating the button that allows to delete settings
    long onUpdDeleteSetting(FXObject*, FXSelector, void* data);

    /// @brief Called if the settings shall be exported into a file
    long onCmdExportSetting(FXObject*, FXSelector, void* data);
    /// @brief Called when updating the button that allows to export settings into a file
    long onUpdExportSetting(FXObject*, FXSelector, void* data);

    /// @brief Called if the settings shall be read from a file
    long onCmdImportSetting(FXObject*, FXSelector, void* data);
    /// @brief Called when updating the button that allows to read settings from a file
    long onUpdImportSetting(FXObject*, FXSelector, void* data);

    /// @brief Called if the decals shall be loaded from a file
    long onCmdLoadDecals(FXObject*, FXSelector, void* data);
    /// @brief Called if the decals shall be saved to a file
    long onCmdSaveDecals(FXObject*, FXSelector, void* data);
    /// @}



    /** @brief Returns the name of the currently chosen scheme
     * @return The name of the edited (chosen) scheme)
     */
    std::string getCurrentScheme() const;


    /** @brief Sets the named scheme as the current
     * @param[in] The name of the scheme that shall be set as current
     */
    void setCurrentScheme(const std::string&);


private:
    bool updateColorRanges(FXObject* sender, std::vector<FXColorWell*>::const_iterator colIt,
                           std::vector<FXColorWell*>::const_iterator colEnd,
                           std::vector<FXRealSpinner*>::const_iterator threshIt,
                           std::vector<FXRealSpinner*>::const_iterator threshEnd,
                           std::vector<FXButton*>::const_iterator buttonIt,
                           GUIColorScheme& scheme);

    bool updateScaleRanges(FXObject* sender, std::vector<FXRealSpinner*>::const_iterator colIt,
                           std::vector<FXRealSpinner*>::const_iterator colEnd,
                           std::vector<FXRealSpinner*>::const_iterator threshIt,
                           std::vector<FXRealSpinner*>::const_iterator threshEnd,
                           std::vector<FXButton*>::const_iterator buttonIt,
                           GUIScaleScheme& scheme);

    /** @brief Rebuilds manipulators for the current coloring scheme
     */
    FXMatrix* rebuildColorMatrix(FXVerticalFrame* frame,
                                 std::vector<FXColorWell*>& colors,
                                 std::vector<FXRealSpinner*>& thresholds,
                                 std::vector<FXButton*>& buttons,
                                 FXCheckButton* interpolation,
                                 GUIColorScheme& scheme);

    /** @brief Rebuilds manipulators for the current scaling scheme
     */
    FXMatrix* rebuildScaleMatrix(FXVerticalFrame* frame,
                                 std::vector<FXRealSpinner*>& scales,
                                 std::vector<FXRealSpinner*>& thresholds,
                                 std::vector<FXButton*>& buttons,
                                 FXCheckButton* interpolation,
                                 GUIScaleScheme& scheme);


    /** @brief Rebuilds color changing dialogs after choosing another coloring scheme
     * @param[in] doCreate Whether "create" shall be called (only if built the first time)
     */
    void rebuildColorMatrices(bool doCreate = false);


    /** @brief Rebuilds the decals table */
    void rebuildList();


    /** @brief Loads a scheme from a file
     * @param[in] file The name of the file to read the settings from
     */
    void loadSettings(const std::string& file);


    /** @brief Writes the currently used decals into a file
     * @param[in] file The name of the file to write the decals into
     */
    void saveDecals(OutputDevice& dev) const;


    /** @brief Loads decals from a file
     * @param[in] file The name of the file to read the decals from
     */
    void loadDecals(const std::string& file);

    /// @brief save window position and size to the registry
    void saveWindowSize();

    /// @brief load window position and size from the registry
    void loadWindowSize();

    /// @brief reload known vehicle parameters
    void updateVehicleParams();

    /// @brief reload known POI parameters
    void updatePOIParams();

private:
    /// @brief The parent view (which settings are changed)
    GUISUMOAbstractView* myParent;

    /// @brief The current settings
    GUIVisualizationSettings* mySettings;

    /// @brief A backup of the settings (used if the "Cancel" button is pressed)
    GUIVisualizationSettings myBackup;

    /// @brief The parent's decals
    std::vector<GUISUMOAbstractView::Decal>* myDecals;

    /// @brief Lock used when changing the decals
    FXMutex* myDecalsLock;

    /// @name Dialog elements
    /// @{
    FXComboBox* mySchemeName;
    FXCheckButton* myShowGrid;
    FXRealSpinner* myGridXSizeDialer, *myGridYSizeDialer;

    FXColorWell* myBackgroundColor;
    FXVerticalFrame* myDecalsFrame;
    MFXAddEditTypedTable* myDecalsTable;

    /// @brief selection colors
    FXColorWell* mySelectionColor;
    FXColorWell* mySelectedEdgeColor;
    FXColorWell* mySelectedLaneColor;
    FXColorWell* mySelectedConnectionColor;
    FXColorWell* mySelectedProhibitionColor;
    FXColorWell* mySelectedCrossingColor;
    FXColorWell* mySelectedAdditionalColor;
    FXColorWell* mySelectedRouteColor;
    FXColorWell* mySelectedVehicleColor;
    FXColorWell* mySelectedPersonColor;
    FXColorWell* mySelectedPersonPlanColor;
    FXColorWell* mySelectedEdgeDataColor;

    /// ... lane colorer
    MFXIconComboBox* myLaneEdgeColorMode;
    FXVerticalFrame* myLaneColorSettingFrame;
    std::vector<FXColorWell*> myLaneColors;
    std::vector<FXRealSpinner*> myLaneThresholds;
    std::vector<FXButton*> myLaneButtons;
    FXCheckButton* myLaneColorInterpolation;
    FXButton* myLaneColorRainbow;
    FXCheckButton* myLaneColorRainbowCheck;
    FXRealSpinner* myLaneColorRainbowThreshold;
    FXButton* myJunctionColorRainbow;
    FXComboBox* myParamKey;

    /// ... lane scaler
    MFXIconComboBox* myLaneEdgeScaleMode;
    FXVerticalFrame* myLaneScaleSettingFrame;
    std::vector<FXRealSpinner*> myLaneScales;
    std::vector<FXRealSpinner*> myLaneScaleThresholds;
    std::vector<FXButton*> myLaneScaleButtons;
    FXCheckButton* myLaneScaleInterpolation;

    FXCheckButton* myShowLaneBorders, *myShowBikeMarkings, *myShowLaneDecals, *myShowLinkRules, *myShowRails,
                   *myHideMacroConnectors, *myShowLaneDirection, *myShowSublanes, *mySpreadSuperposed;
    FXRealSpinner* myLaneWidthUpscaleDialer;
    FXRealSpinner* myLaneMinWidthDialer;

    // Vehicles
    MFXIconComboBox* myVehicleColorMode, *myVehicleShapeDetail;
    FXVerticalFrame* myVehicleColorSettingFrame;
    std::vector<FXColorWell*> myVehicleColors;
    std::vector<FXRealSpinner*> myVehicleThresholds;
    std::vector<FXButton*> myVehicleButtons;
    FXCheckButton* myVehicleColorInterpolation;
    FXCheckButton* myShowBlinker, *myShowMinGap, *myShowBrakeGap, *myShowBTRange, *myShowRouteIndex; /* *myShowLaneChangePreference,*/
    FXComboBox* myVehicleParamKey;
    FXComboBox* myVehicleTextParamKey;

    // Persons
    MFXIconComboBox* myPersonColorMode, *myPersonShapeDetail;
    FXVerticalFrame* myPersonColorSettingFrame;
    std::vector<FXColorWell*> myPersonColors;
    std::vector<FXRealSpinner*> myPersonThresholds;
    std::vector<FXButton*> myPersonButtons;
    FXCheckButton* myPersonColorInterpolation;

    // Containers
    MFXIconComboBox* myContainerColorMode, *myContainerShapeDetail;
    FXVerticalFrame* myContainerColorSettingFrame;
    std::vector<FXColorWell*> myContainerColors;
    std::vector<FXRealSpinner*> myContainerThresholds;
    std::vector<FXButton*> myContainerButtons;
    FXCheckButton* myContainerColorInterpolation;
    FXRealSpinner* myContainerMinSizeDialer, *myContainerUpscaleDialer;

    // junctions
    MFXIconComboBox* myJunctionColorMode;
    FXVerticalFrame* myJunctionColorSettingFrame;
    std::vector<FXColorWell*> myJunctionColors;
    std::vector<FXRealSpinner*> myJunctionThresholds;
    std::vector<FXButton*> myJunctionButtons;
    FXCheckButton* myJunctionColorInterpolation;

    // POIs
    MFXIconComboBox* myPOIColorMode, *myPOIShapeDetail;
    FXVerticalFrame* myPOIColorSettingFrame;
    std::vector<FXColorWell*> myPOIColors;
    std::vector<FXRealSpinner*> myPOIThresholds;
    std::vector<FXButton*> myPOIButtons;
    FXCheckButton* myPOIColorInterpolation;
    FXComboBox* myPOITextParamKey;

    // Polygons
    MFXIconComboBox* myPolyColorMode, *myPolyShapeDetail;
    FXVerticalFrame* myPolyColorSettingFrame;
    std::vector<FXColorWell*> myPolyColors;
    std::vector<FXRealSpinner*> myPolyThresholds;
    std::vector<FXButton*> myPolyButtons;
    FXCheckButton* myPolyColorInterpolation;

    FXCheckButton* myShowLane2Lane;
    FXCheckButton* myDrawJunctionShape;
    FXCheckButton* myDrawCrossingsAndWalkingAreas;
    FXCheckButton* myDither;
    FXCheckButton* myFPS;
    FXCheckButton* myDrawBoundaries;
    FXCheckButton* myForceDrawForPositionSelection;
    FXCheckButton* myForceDrawForRectangleSelection;
    FXCheckButton* myShowSizeLegend;
    FXCheckButton* myShowColorLegend;
    FXCheckButton* myShowVehicleColorLegend;

    NamePanel* myEdgeNamePanel, *myInternalEdgeNamePanel, *myCwaEdgeNamePanel, *myStreetNamePanel, *myEdgeValuePanel,
               *myJunctionIndexPanel, *myTLIndexPanel,
               *myJunctionIDPanel, *myJunctionNamePanel, *myInternalJunctionNamePanel,
               *myTLSPhaseIndexPanel, *myTLSPhaseNamePanel,
               *myVehicleNamePanel, *myVehicleValuePanel, *myVehicleTextPanel,
               *myPersonNamePanel, *myPersonValuePanel,
               *myContainerNamePanel,
               *myAddNamePanel, *myAddFullNamePanel,
               *myPOINamePanel, *myPOITypePanel, *myPOITextPanel,
               *myPolyNamePanel, *myPolyTypePanel;

    SizePanel* myVehicleSizePanel, *myPersonSizePanel, *myContainerSizePanel, *myPOISizePanel, *myPolySizePanel, *myAddSizePanel, *myJunctionSizePanel;


    // load/save-menu
    FXCheckButton* mySaveViewPort, *mySaveDelay, *mySaveDecals, *mySaveBreakpoints;
    /// @}


protected:
    FOX_CONSTRUCTOR(GUIDialog_ViewSettings)


private:
    /// @brief invalidated copy constructor
    GUIDialog_ViewSettings(const GUIDialog_ViewSettings& s);

    /// @brief invalidated assignment operator
    GUIDialog_ViewSettings& operator=(const GUIDialog_ViewSettings& s);


};
