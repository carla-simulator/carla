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
/// @file    GUIOSGView.h
/// @author  Daniel Krajzewicz
/// @date    19.01.2012
///
// An OSG-based 3D view on the simulation
/****************************************************************************/
#pragma once
#include <config.h>

#ifdef HAVE_OSG

#include <string>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275) // do not warn about the DLL interface for OSG
#endif
#include <osgGA/TerrainManipulator>
#include <osgViewer/Viewer>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/ref_ptr>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/common/RGBColor.h>
#include <utils/geom/PositionVector.h>
#include <gui/GUISUMOViewParent.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUINet;
class GUISUMOViewParent;
class GUIVehicle;
class GUILaneWrapper;
class MSRoute;
namespace osgGA {
class CameraManipulator;
class NodeTrackerManipulator;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIOSGView
 * @brief An OSG-based 3D view on the simulation
 */
class GUIOSGView : public GUISUMOAbstractView {
    FXDECLARE(GUIOSGView)
public:
    /**
     * @class Command_TLSChange
     * @brief Updates scene on each tls switch
     */
    class Command_TLSChange : public MSTLLogicControl::OnSwitchAction {
    public:
        /** @brief Constructor
         *
         * @param[in] link   The link to observe
         * @param[in] root   the root of the scene
         * @param[in] green  the green light
         * @param[in] yellow the yellow light
         * @param[in] red    the red light
         */
        Command_TLSChange(const MSLink* const link, osg::Switch* switchNode);


        /// @brief Destructor
        virtual ~Command_TLSChange();


        /** @brief Executes the command
         *
         * Called when an active tls program switches.
         *  If the state of the observed linkchanged, this method removes
         *  the old traffic light and adds a new one.
         */
        void execute();


    private:
        /// @brief The link to observe
        const MSLink* const myLink;

        /// @brief The switch for the traffic light models
        osg::ref_ptr<osg::Switch> mySwitch;

        /// @brief The previous link state
        LinkState myLastState;


    private:
        /// @brief Invalidated copy constructor.
        Command_TLSChange(const Command_TLSChange&);

        /// @brief Invalidated assignment operator.
        Command_TLSChange& operator=(const Command_TLSChange&);

    };

    /// @brief struct for OSG movable elements
    struct OSGMovable {
        osg::ref_ptr<osg::PositionAttitudeTransform> pos;
        osg::ref_ptr<osg::ShapeDrawable> geom;
        osg::ref_ptr<osg::Switch> lights;
        bool active;
    };

    /// @brief constructor
    GUIOSGView(FXComposite* p, GUIMainWindow& app,
               GUISUMOViewParent* parent, GUINet& net, FXGLVisual* glVis,
               FXGLCanvas* share);

    /// @brief destructor
    virtual ~GUIOSGView();

    /// @brief builds the view toolbars
    virtual void buildViewToolBars(GUIGlChildWindow*);

    /// @brief recenters the view
    void recenterView();

    /** @brief centers to the chosen artifact
     * @param[in] id The id of the artifact to center to
     * @param[in] applyZoom Whether to zoom in
     * @param[in] zoomDist The distance in m to use for the zoom, values < 0 means: use the centeringBoundary
     * @note caller is responsible for calling update
     */
    void centerTo(GUIGlID id, bool applyZoom, double zoomDist = 20);

    /// @brief show viewport editor
    void showViewportEditor();

    /// @brief applies the given viewport settings
    void setViewportFromToRot(const Position& lookFrom, const Position& lookAt, double rotation);

    /// @brief copy the viewport to the given view
    void copyViewportTo(GUISUMOAbstractView* view);

    /** @brief Starts vehicle tracking
     * @param[in] id The glID of the vehicle to track
     */
    void startTrack(int id);


    /** @brief Stops vehicle tracking
     */
    void stopTrack();


    /** @brief Returns the id of the tracked vehicle (-1 if none)
     * @return The glID of the vehicle to track
     */
    GUIGlID getTrackedID() const;

    bool setColorScheme(const std::string& name);

    /// @brief handle mouse click in gaming mode
    void onGamingClick(Position pos);

    /// @brief get the current simulation time
    SUMOTime getCurrentTimeStep() const;

    void removeVeh(MSVehicle* veh);
    void removeTransportable(MSTransportable* t);

    // callback
    long onConfigure(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onLeftBtnPress(FXObject*, FXSelector, void*);
    long onLeftBtnRelease(FXObject*, FXSelector, void*);
    long onMiddleBtnPress(FXObject*, FXSelector, void*);
    long onMiddleBtnRelease(FXObject*, FXSelector, void*);
    long onRightBtnPress(FXObject*, FXSelector, void*);
    long onRightBtnRelease(FXObject*, FXSelector, void*);
    //long onMotion(FXObject*, FXSelector, void*);
    long onMouseMove(FXObject*, FXSelector, void*);
    long onPaint(FXObject*, FXSelector, void*);
    long OnIdle(FXObject* sender, FXSelector sel, void* ptr);

private:
    class SUMOTerrainManipulator : public osgGA::TerrainManipulator {
    public:
        SUMOTerrainManipulator() {
            setAllowThrow(false);
            setRotationMode(ELEVATION_AZIM_ROLL); // default is ELEVATION_AZIM and this prevents rotating the view around the z-axis
        }
        bool performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy) {
            return osgGA::TerrainManipulator::performMovementMiddleMouseButton(eventTimeDelta, dx, dy);
        }
        bool performMovementMiddleMouseButton(const double eventTimeDelta, const double dx, const double dy) {
            return osgGA::TerrainManipulator::performMovementLeftMouseButton(eventTimeDelta, dx, dy);
        }
        bool performMovementRightMouseButton(const double eventTimeDelta, const double dx, const double dy) {
            return osgGA::TerrainManipulator::performMovementRightMouseButton(eventTimeDelta, dx, -dy);
        }
    };

    class FXOSGAdapter : public osgViewer::GraphicsWindow {
    public:
        FXOSGAdapter(GUISUMOAbstractView* parent, FXCursor* cursor);
        void grabFocus();
        void grabFocusIfPointerInWindow() {}
        void useCursor(bool cursorOn);

        bool makeCurrentImplementation();
        bool releaseContext();
        void swapBuffersImplementation();

        // not implemented yet...just use dummy implementation to get working.
        bool valid() const {
            return true;
        }
        bool realizeImplementation() {
            return true;
        }
        bool isRealizedImplementation() const  {
            return true;
        }
        void closeImplementation() {}
        bool releaseContextImplementation() {
            return true;
        }

    protected:
        ~FXOSGAdapter();
    private:
        GUISUMOAbstractView* const myParent;
        FXCursor* const myOldCursor;
    };

protected:

    osg::ref_ptr<FXOSGAdapter> myAdapter;
    osg::ref_ptr<osgViewer::Viewer> myViewer;
    osg::ref_ptr<osg::Group> myRoot;

private:
    GUIVehicle* myTracked;
    osg::ref_ptr<osgGA::CameraManipulator> myCameraManipulator;

    SUMOTime myLastUpdate;

    std::map<MSVehicle*, OSGMovable > myVehicles;
    std::map<MSTransportable*, OSGMovable > myPersons;

    osg::ref_ptr<osg::Node> myGreenLight;
    osg::ref_ptr<osg::Node> myYellowLight;
    osg::ref_ptr<osg::Node> myRedLight;
    osg::ref_ptr<osg::Node> myRedYellowLight;

protected:
    GUIOSGView() { }

};

#endif
