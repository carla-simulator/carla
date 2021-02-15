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
/// @file    GUIEdge.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A road/street connecting two junctions (gui-version)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <fx.h>
#include <microsim/MSEdge.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/settings/GUIPropertySchemeStorage.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MESegment;
class MSBaseVehicle;
class GUILane;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIEdge
 * @brief A road/street connecting two junctions (gui-version)
 *
 * @see MSEdge
 */
class GUIEdge : public MSEdge, public GUIGlObject {
public:
    /** @brief Constructor.
     * @param[in] id The id of the edge
     * @param[in] numericalID The numerical id (index) of the edge
     * @see MSEdge
     */
    GUIEdge(const std::string& id, int numericalID,
            const SumoXMLEdgeFunc function,
            const std::string& streetName, const std::string& edgeType, int priority,
            double distance);


    /// @brief Destructor.
    ~GUIEdge();

    /// Has to be called after all edges were built and all connections were set
    virtual void closeBuilding();

    /* @brief Returns the gl-ids of all known edges
     * @param[in] includeInternal Whether to include ids of internal edges
     */
    static std::vector<GUIGlID> getIDs(bool includeInternal);

    /* @brief Returns the combined length of all edges
     * @param[in] includeInternal Whether to include lengths of internal edges
     * @param[in] eachLane Whether to count each lane separately
     */
    static double getTotalLength(bool includeInternal, bool eachLane);

    /// Returns the street's geometry
    Boundary getBoundary() const;

    /// returns the enumerated lane (!!! why not private with a friend?)
    MSLane& getLane(int laneNo);



    /** returns the position on the line given by the coordinates where "prev"
        is the length of the line and "wanted" the distance from the begin
        !!! should be within another class */
    static std::pair<double, double> getLaneOffsets(double x1, double y1,
            double x2, double y2, double prev, double wanted);


    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    virtual GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app,
            GUISUMOAbstractView& parent);


    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    virtual GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app,
            GUISUMOAbstractView& parent);


    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;

    /// @brief Returns the street name
    const std::string getOptionalName() const;

    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    //@}


    void addPerson(MSTransportable* p) const {
        FXMutexLock locker(myLock);
        MSEdge::addPerson(p);
    }

    void removePerson(MSTransportable* p) const {
        FXMutexLock locker(myLock);
        MSEdge::removePerson(p);
    }


    void addContainer(MSTransportable* c) const {
        FXMutexLock locker(myLock);
        MSEdge::addContainer(c);
    }

    void removeContainer(MSTransportable* c) const {
        FXMutexLock locker(myLock);
        MSEdge::removeContainer(c);
    }

    double getAllowedSpeed() const;
    /// @brief return meanSpead divided by allowedSpeed
    double getRelativeSpeed() const;

    /// @brief sets the color according to the currente settings
    void setColor(const GUIVisualizationSettings& s) const;

    /// @brief sets the color according to the current scheme index and some edge function
    bool setFunctionalColor(const GUIColorer& c) const;

    /// @brief sets multiple colors according to the current scheme index and edge function
    bool setMultiColor(const GUIColorer& c) const;

    /// @brief gets the color value according to the current scheme index
    double getColorValue(const GUIVisualizationSettings& s, int activeScheme) const;

    /// @brief gets the scaling value according to the current scheme index
    double getScaleValue(int activeScheme) const;

    /// @brief returns the segment closest to the given position
    MESegment* getSegmentAtPosition(const Position& pos);

    void drawMesoVehicles(const GUIVisualizationSettings& s) const;

    /// @brief grant exclusive access to the mesoscopic state
    void lock() const {
        myLock.lock();
    }

    /// @brief release exclusive access to the mesoscopic state
    void unlock() const {
        myLock.unlock();
    }

    /// @brief close this edge for traffic
    void closeTraffic(const GUILane* lane);

    /// @brief add a rerouter
    void addRerouter();

    /// @brief return segment colors (meso)
    const std::vector<RGBColor>& getSegmentColors() const {
        return mySegmentColors;
    }

    RGBColor getMesoColor() const {
        return myMesoColor;
    }

    bool showDeadEnd() const {
        return myShowDeadEnd;
    }

    /// @brief whether this lane is selected in the GUI
    bool isSelected() const;

    /// The color of the segments (cached)
    mutable std::vector<RGBColor> mySegmentColors;

    /// @brief whether to highlight this edge as a dead-end edge
    bool myShowDeadEnd;


private:
    /// @brief invalidated copy constructor
    GUIEdge(const GUIEdge& s);

    /// @brief invalidated assignment operator
    GUIEdge& operator=(const GUIEdge& s);


private:
    /// The mutex used to avoid concurrent updates of myPersons/ myContainers
    mutable FXMutex myLock;

    mutable RGBColor myMesoColor;

};
