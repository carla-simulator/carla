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
/// @file    GUISUMOAbstractView.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Andreas Gaubatz
/// @date    Sept 2002
///
// The base class for a view
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <map>
#include <fx.h>
// fx3d includes windows.h so we need to guard against macro pollution
#ifdef WIN32
#define NOMINMAX
#endif
#include <fx3d.h>
#ifdef WIN32
#undef NOMINMAX
#endif

#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/common/RGBColor.h>
#include <utils/common/SUMOTime.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <foreign/rtree/SUMORTree.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUIGlChildWindow;
class GUIVehicle;
class GUIPerspectiveChanger;
class GUIMainWindow;
class GUIGLObjectPopupMenu;
class GUIGlObject;
class GUIDialog_EditViewport;
class GUIDialog_ViewSettings;
class GUIVisualizationSettings;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUISUMOAbstractView
 * This class is meant to be pure virtual later;
 * It shall be the main class to inherit views of the simulation (micro-
 * or macroscopic ones) from it.
 */
class GUISUMOAbstractView : public FXGLCanvas {
    FXDECLARE(GUISUMOAbstractView)

public:
    /// @brief constructor
    GUISUMOAbstractView(FXComposite* p, GUIMainWindow& app, GUIGlChildWindow* parent, const SUMORTree& grid, FXGLVisual* glVis, FXGLCanvas* share);

    /// @brief destructor
    virtual ~GUISUMOAbstractView();

    /// @brief builds the view toolbars
    virtual void buildViewToolBars(GUIGlChildWindow*) { }

    /// @brief recenters the view
    virtual void recenterView();

    /** @brief centers to the chosen artifact
     * @param[in] id The id of the artifact to center to
     * @param[in] applyZoom Whether to zoom in
     * @param[in] zoomDist The distance in m to use for the zoom, values < 0 means: use the centeringBoundary
     * @note caller is responsible for calling update
     */
    virtual void centerTo(GUIGlID id, bool applyZoom, double zoomDist = 20);

    /** @brief centers to the chosen position
     * @param[in] pos Position to center view
     * @param[in] applyZoom Whether to zoom in
     * @param[in] zoomDist The distance in m to use for the zoom, values < 0 means: use the centeringBoundary
     * @note caller is responsible for calling update
     */
    virtual void centerTo(const Position& pos, bool applyZoom, double zoomDist = 20);

    /// @brief centers to the chosen artifact
    void centerTo(const Boundary& bound);

    /// @brief applies the given viewport settings
    virtual void setViewportFromToRot(const Position& lookFrom, const Position& lookAt, double rotation);

    /// @brief copy the viewport to the given view
    virtual void copyViewportTo(GUISUMOAbstractView* view);

    /// @brief meter-to-pixels conversion method
    double m2p(double meter) const;

    /// @brief pixels-to-meters conversion method
    double p2m(double pixel) const;

    /// @brief Returns the information whether rotation is allowd
    ///@note disabled
    //bool allowRotation() const;

    /// @brief return windows cursor position
    Position getWindowCursorPosition() const;

    /// @brief Returns the gl-id of the object under the given coordinates
    void setWindowCursorPosition(FXint x, FXint y);

    /// @brief A reimplementation due to some internal reasons
    FXbool makeCurrent();

    /// @brief returns true, if the edit button was pressed
    bool isInEditMode();

    /// @brief get changer
    GUIPerspectiveChanger& getChanger() const;

    /// @brief get visible boundary
    Boundary getVisibleBoundary() const;

    /// @brief mouse functions
    //@{
    virtual long onConfigure(FXObject*, FXSelector, void*);
    virtual long onPaint(FXObject*, FXSelector, void*);
    virtual long onLeftBtnPress(FXObject*, FXSelector, void*);
    virtual long onLeftBtnRelease(FXObject*, FXSelector, void*);
    virtual long onMiddleBtnPress(FXObject*, FXSelector, void*);
    virtual long onMiddleBtnRelease(FXObject*, FXSelector, void*);
    virtual long onRightBtnPress(FXObject*, FXSelector, void*);
    virtual long onRightBtnRelease(FXObject*, FXSelector, void*);
    virtual long onDoubleClicked(FXObject*, FXSelector, void*);
    virtual long onMouseWheel(FXObject*, FXSelector, void*);
    virtual long onMouseMove(FXObject*, FXSelector, void*);
    virtual long onMouseLeft(FXObject*, FXSelector, void*);
    //@}

    /// @brief keyboard functions
    //@{
    virtual long onKeyPress(FXObject* o, FXSelector sel, void* data);
    virtual long onKeyRelease(FXObject* o, FXSelector sel, void* data);
    //@}

    //@brief open object dialog
    virtual void openObjectDialog();

    /// @brief A method that updates the tooltip
    void updateToolTip();

    /// @brief @name Dealing with snapshots
    ///@{
    /** @brief Sets the snapshot time to file map
     * @param[in] snaps The snapshots to take at certain times
     * @param[in] w The snapshot image width
     * @param[in] w The snapshot image height
     */
    void addSnapshot(SUMOTime time, const std::string& file, const int w = -1, const int h = -1);

    /** @brief Takes a snapshots and writes it into the given file
     *
     * The format to use is determined from the extension.
     * If compiled with ffmpeg and a video format is requested it will instantiate a video encoder.
     * @param[in] destFile The name of the file to write the snapshot into
     * @param[in] w The snapshot image width
     * @param[in] w The snapshot image height
     * @return The error message, if an error occcured; "" otherwise
     */
    std::string makeSnapshot(const std::string& destFile, const int w = -1, const int h = -1);

    /// @brief Adds a frame to a video snapshot which will be initialized if neccessary
    virtual void saveFrame(const std::string& destFile, FXColor* buf);

    /// @brief Ends a video snapshot
    virtual void endSnapshot() {}

    /// @brief Checks whether it is time for a snapshot
    virtual void checkSnapshots();

    void waitForSnapshots(const SUMOTime snapshotTime);

    /// @brief get the current simulation time
    virtual SUMOTime getCurrentTimeStep() const;
    ///@}

    /// @brief get the viewport and create it on first access
    GUIDialog_EditViewport* getViewportEditor();

    /// @brief show viewport editor
    virtual void showViewportEditor();

    /// @brief show viewsscheme editor
    void showViewschemeEditor();

    /// @brief show tool tips
    void showToolTips(bool val);

    /// @brief set color scheme
    virtual bool setColorScheme(const std::string&);

    /// @brief get visualization settings
    GUIVisualizationSettings& getVisualisationSettings() const;

    /// @brief recalibrate color scheme according to the current value range
    virtual void buildColorRainbow(const GUIVisualizationSettings& /*s*/, GUIColorScheme& /*scheme*/, int /*active*/, GUIGlObjectType /*objectType*/,
                                   bool hide = false, double hideThreshold = 0) {
        UNUSED_PARAMETER(hide);
        UNUSED_PARAMETER(hideThreshold);
    }

    /// @brief return list of loaded edgeData attributes
    virtual std::vector<std::string> getEdgeDataAttrs() const {
        return std::vector<std::string>();
    }

    /// @brief return list of available edge parameters
    virtual std::vector<std::string> getEdgeLaneParamKeys(bool /*edgeKeys*/) const {
        return std::vector<std::string>();
    }

    /// @brief return list of available vehicle parameters
    virtual std::vector<std::string> getVehicleParamKeys(bool /*vTypeKeys*/) const {
        return std::vector<std::string>();
    }

    /// @brief return list of available vehicle parameters
    virtual std::vector<std::string> getPOIParamKeys() const {
        return std::vector<std::string>();
    }

    /// @brief remove viewport
    void remove(GUIDialog_EditViewport*);

    /// @brief remove view settings
    void remove(GUIDialog_ViewSettings*);

    /// @brief get grid width
    // @todo: check why this is here
    double getGridWidth() const;

    /// @brief get grid height
    // @todo: check why this is here
    double getGridHeight() const;

    /// @brief star track
    virtual void startTrack(int /*id*/);

    /// @brief stop track
    virtual void stopTrack();

    /// @brief get tracked id
    virtual GUIGlID getTrackedID() const;

    /// @brief on gaming click
    virtual void onGamingClick(Position /*pos*/);
    virtual void onGamingRightClick(Position /*pos*/);

    /// @brief @name Additional visualisations
    ///@{
    /** @brief Adds an object to call its additional visualisation method
     * @param[in] which The object to add
     * @return Always true
     * @see GUIGlObject::drawGLAdditional
     */
    bool addAdditionalGLVisualisation(GUIGlObject* const which);

    /** @brief Removes an object from the list of objects that show additional things
     * @param[in] which The object to remoe
     * @return True if the object was known, false otherwise
     * @see GUIGlObject::drawGLAdditional
     */
    bool removeAdditionalGLVisualisation(GUIGlObject* const which);

    /** @brief Check if an object is added in the additional GL visualitation
     * @param[in] which The object to check
     * @see GUIGlObject::drawGLAdditional
     */
    bool isAdditionalGLVisualisationEnabled(GUIGlObject* const which) const;
    ///@}

    /// @brief get position of current popup
    const Position& getPopupPosition() const;

    /// @brief destoys the popup
    void destroyPopup();

public:
    ///@struct Decal
    /// @brief A decal (an image) that can be shown
    struct Decal {
        /// @brief Constructor
        Decal();

        /// @brief The path to the file the image is located at
        std::string filename;
        /// @brief The center of the image in x-direction (net coordinates, in m)
        double centerX;
        /// @brief The center of the image in y-direction (net coordinates, in m)
        double centerY;
        /// @brief The center of the image in z-direction (net coordinates, in m)
        double centerZ;
        /// @brief The width of the image (net coordinates in x-direction, in m)
        double width;
        /// @brief The height of the image (net coordinates in y-direction, in m)
        double height;
        /// @brief The altitude of the image (net coordinates in z-direction, in m)
        double altitude;
        /// @brief The rotation of the image in the ground plane (in degrees)
        double rot;
        /// @brief The tilt of the image to the ground plane (in degrees)
        double tilt;
        /// @brief The roll of the image to the ground plane (in degrees)
        double roll;
        /// @brief The layer of the image
        double layer;
        /// @brief Whether this image was initialised (inserted as a texture)
        bool initialised;
        /// @brief Whether this image should be skipped in 2D-views
        bool skip2D;
        /// @brief Whether this image should be skipped in 2D-views
        bool screenRelative;
        /// @brief whether the decal shall be drawn in screen coordinates, rather than network coordinates
        int glID;
        /// @brief The image pointer for later cleanup
        FXImage* image;
    };

public:
    /// @brief get coloring schemes combo
    FXComboBox* getColoringSchemesCombo();

    /// @brief Returns the cursor's x/y position within the network
    Position getPositionInformation() const;

    /**@brief Returns a position that is mapped to the closest grid point if the grid is active
     * @brief note: formats are pos(x,y,0) por pos(0,0,z)
     */
    Position snapToActiveGrid(const Position& pos, bool snapXY = true) const;

    /// @brief Translate screen position to network position
    Position screenPos2NetPos(int x, int y) const;

    /// @brief add decals
    void addDecals(const std::vector<Decal>& decals);

    /// @brief Returns the delay of the parent application
    double getDelay() const;

    /// @brief Sets the delay of the parent application
    void setDelay(double delay);

    /** @brief Sets the breakpoints of the parent application */
    void setBreakpoints(const std::vector<SUMOTime>& breakpoints);

    /// @brief retrieve breakpoints if provided by the application
    virtual const std::vector<SUMOTime> retrieveBreakpoints() const {
        return std::vector<SUMOTime>();
    }

    /// @brief retrieve FPS
    double getFPS() const;

protected:
    /// @brief performs the painting of the simulation
    void paintGL();

    /// @brief update position information
    void updatePositionInformation() const;

    /// @brief paint GL
    virtual int doPaintGL(int /*mode*/, const Boundary& /*boundary*/);

    /// @brief doInit
    virtual void doInit();

    /// @brief paints a grid
    void paintGLGrid();

    /// @brief Draws a line with ticks, and the length information.
    void displayLegend();

    /// @brief Draws the configured legends
    void displayLegends();

    /// @brief Draws a legend for the given scheme
    void displayColorLegend(const GUIColorScheme& scheme, bool leftSide);

    /// @brief Draws frames-per-second indicator
    void drawFPS();

    /// @brief returns the id of the front object under the cursor using GL_SELECT
    GUIGlID getObjectUnderCursor();

    /// @brief returns the id of the objects under the cursor using GL_SELECT (including overlapped objects)
    std::vector<GUIGlID> getObjectsUnderCursor();

    /// @brief returns the GUIGlObject under the cursor using GL_SELECT (including overlapped objects)
    std::vector<GUIGlObject*> getGUIGlObjectsUnderCursor();

    /// @brief returns the GUIGlObject under the gripped cursor using GL_SELECT (including overlapped objects)
    std::vector<GUIGlObject*> getGUIGlObjectsUnderSnappedCursor();

    /// @brief returns the id of the object at position using GL_SELECT
    GUIGlID getObjectAtPosition(Position pos);

    /// @brief returns the ids of the object at position within the given (rectangular) radius using GL_SELECT
    std::vector<GUIGlID> getObjectsAtPosition(Position pos, double radius);

    /// @brief returns the GUIGlObjects at position within the given (rectangular) radius using GL_SELECT
    std::vector<GUIGlObject*> getGUIGlObjectsAtPosition(Position pos, double radius);

    /// @brief returns the ids of all objects in the given boundary
    std::vector<GUIGlID> getObjectsInBoundary(Boundary bound, bool singlePosition);

    /// @brief invokes the tooltip for the given object
    void showToolTipFor(const GUIGlID id);

protected:
    FOX_CONSTRUCTOR(GUISUMOAbstractView)

    /// @brief check whether we can read image data or position with gdal
    FXImage* checkGDALImage(Decal& d);

    /// @brief Draws the stored decals
    void drawDecals();

    /// @brief applies gl-transformations to fit the Boundary given by myChanger onto the canvas.
    /// If fixRatio is true, this boundary will be enlarged to prevent anisotropic stretching.
    /// (this should be set to false when doing selections)
    Boundary applyGLTransform(bool fixRatio = true);

protected:
    /// @brief The application
    GUIMainWindow* myApp;

    /// @brief The parent window
    GUIGlChildWindow* myParent;

    /// @brief The visualization speed-up
    const SUMORTree* myGrid;

    /// @brief The perspective changer
    GUIPerspectiveChanger* myChanger;

    /// @brief Information whether too-tip informations shall be generated
    bool myInEditMode;

    /// @brief Offset to the mouse-hotspot from the mouse position
    int myMouseHotspotX, myMouseHotspotY;

    /// @brief The current popup-menu
    GUIGLObjectPopupMenu* myPopup;

    /// @brief The current popup-menu position
    Position myPopupPosition;

    /// @brief visualization settings
    GUIVisualizationSettings* myVisualizationSettings;

    /// @brief use tool tips
    bool myUseToolTips;

    /// @brief Internal information whether doInit() was called
    bool myAmInitialised;

    /// @brief viewport chooser
    GUIDialog_EditViewport* myViewportChooser;

    /// @brief Position of the cursor relative to the window
    FXint myWindowCursorPositionX, myWindowCursorPositionY;

    /// @brief Visualization changer
    GUIDialog_ViewSettings* myVisualizationChanger;

    /// @brief @name Optionally shown decals
    ///@{
    /// @brief The list of decals to show
    std::vector<Decal> myDecals;

    /// @brief The mutex to use before accessing the decals list in order to avoid thread conflicts
    FXMutex myDecalsLock;
    ///@}

    /// @brief Snapshots
    std::map<SUMOTime, std::vector<std::tuple<std::string, int, int> > > mySnapshots;

    /// @brief The mutex to use before accessing the decals list in order to avoid thread conflicts
    FXMutex mySnapshotsMutex;

    /// @brief the semaphore when waiting for snapshots to finish
    FXCondition mySnapshotCondition;

    /// @brief poly draw lock
    mutable FXMutex myPolyDrawLock;

    /// @brief List of objects for which GUIGlObject::drawGLAdditional is called
    std::map<GUIGlObject*, int> myAdditionallyDrawn;

    /// @brief counter for measuring rendering time
    long myFrameDrawTime;

private:
    // @brief sensitivity for "<>AtPosition(...) functions
    static const double SENSITIVITY;
};
