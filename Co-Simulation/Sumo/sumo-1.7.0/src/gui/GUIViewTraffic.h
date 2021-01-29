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
/// @file    GUIViewTraffic.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Andreas Gaubatz
/// @date    Sept 2002
///
// A view on the simulation; this view is a microscopic one
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/common/RGBColor.h>
#include <utils/geom/PositionVector.h>
#include "GUISUMOViewParent.h"
#include <utils/gui/windows/GUISUMOAbstractView.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUINet;
class GUISUMOViewParent;
class GUIVehicle;
class GUIVideoEncoder;
class MSRoute;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIViewTraffic
 * Microsocopic view at the simulation
 */
class GUIViewTraffic : public GUISUMOAbstractView {
    FXDECLARE(GUIViewTraffic)
public:
    /// @brief constructor
    GUIViewTraffic(FXComposite* p, GUIMainWindow& app,
                   GUISUMOViewParent* parent, GUINet& net, FXGLVisual* glVis,
                   FXGLCanvas* share);
    /// @brief destructor
    virtual ~GUIViewTraffic();

    /// @brief builds the view toolbars
    virtual void buildViewToolBars(GUIGlChildWindow*);


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

    /// @brief recalibrate color scheme according to the current value range
    void buildColorRainbow(const GUIVisualizationSettings& s, GUIColorScheme& scheme, int active, GUIGlObjectType objectType,
                           bool hide = false, double hideThreshold = 0);

    /// @brief return list of loaded edgeData attributes
    std::vector<std::string> getEdgeDataAttrs() const;

    /// @brief return list of available edge parameters
    std::vector<std::string> getEdgeLaneParamKeys(bool edgeKeys) const;

    /// @brief return list of available vehicle parameters
    std::vector<std::string> getVehicleParamKeys(bool vTypeKeys) const;

    /// @brief return list of available POI parameters
    std::vector<std::string> getPOIParamKeys() const;

    /// @brief handle mouse click in gaming mode
    void onGamingClick(Position pos);
    void onGamingRightClick(Position pos);

    /// @brief get the current simulation time
    SUMOTime getCurrentTimeStep() const;

    /// @brief interaction with the simulation
    long onCmdCloseLane(FXObject*, FXSelector, void*);
    long onCmdCloseEdge(FXObject*, FXSelector, void*);
    long onCmdAddRerouter(FXObject*, FXSelector, void*);

    /// @brief highlight edges according to reachability
    long onCmdShowReachability(FXObject*, FXSelector, void*);

    long onDoubleClicked(FXObject*, FXSelector, void*);

    /** @brief Adds a frame to a video snapshot which will be initialized if neccessary
     */
    void saveFrame(const std::string& destFile, FXColor* buf);

    /** @brief Ends a video snapshot
     */
    void endSnapshot();

    /** @brief Checks whether it is time for a snapshot
     */
    void checkSnapshots();

    /// @brief retrieve breakpoints from the current runThread
    const std::vector<SUMOTime> retrieveBreakpoints() const;

protected:
    int doPaintGL(int mode, const Boundary& bound);

    GUILane* getLaneUnderCursor();

private:
    GUIGlID myTrackedID;

    /// @brief whether game mode was set to 'tls'
    bool myTLSGame;

#ifdef HAVE_FFMPEG
    GUIVideoEncoder* myCurrentVideo;
#endif

protected:
    GUIViewTraffic() { }

};
