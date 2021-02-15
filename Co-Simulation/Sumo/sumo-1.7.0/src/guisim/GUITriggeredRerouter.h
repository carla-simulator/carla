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
/// @file    GUITriggeredRerouter.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 25.07.2005
///
// Reroutes vehicles passing an edge (gui-version)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <foreign/rtree/SUMORTree.h>
#include <microsim/trigger/MSTriggeredRerouter.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <gui/GUIManipulator.h>

// ===========================================================================
// class declarations
// ===========================================================================
class GUIEdge;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUITriggeredRerouter
 * @brief Reroutes vehicles passing an edge
 * One rerouter can be active on multiple edges. To reduce drawing load we
 * instantiate GUIGlObjects for every edge
 * XXX multiple rerouters active on the same edge are problematic
 */
class GUITriggeredRerouter
    : public MSTriggeredRerouter,
      public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
     * @param[in] id The id of the rerouter
     * @param[in] edges The edges the rerouter is placed at
     * @param[in] prob The probability to reroute vehicles
     * @param[in] aXMLFilename The file to read further definitions from
     * @param[in] off Whether the rerouter is off (not working) initially
     */
    GUITriggeredRerouter(const std::string& id,
                         const MSEdgeVector& edges, double prob,
                         const std::string& aXMLFilename, bool off,
                         SUMOTime timeThreshold,
                         const std::string& vTypes,
                         SUMORTree& rtree);


    /// @brief Destructor
    ~GUITriggeredRerouter();


    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);

    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app,
                                       GUISUMOAbstractView& parent);


    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app,
            GUISUMOAbstractView& parent);


    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;


    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    //@}



    GUIManipulator* openManipulator(GUIMainWindow& app,
                                    GUISUMOAbstractView& parent);

    /// @brief shit route probabilities
    void shiftProbs();

public:

    enum RerouterEdgeType {
        REROUTER_TRIGGER_EDGE,
        REROUTER_CLOSED_EDGE,
        REROUTER_SWITCH_EDGE
    };

    class GUITriggeredRerouterEdge : public GUIGlObject {

    public:
        GUITriggeredRerouterEdge(GUIEdge* edge, GUITriggeredRerouter* parent, RerouterEdgeType edgeType, int distIndex = -1);

        virtual ~GUITriggeredRerouterEdge();

        /// @name inherited from GUIGlObject
        //@{

        /** @brief Returns an own popup-menu
         *
         * @param[in] app The application needed to build the popup-menu
         * @param[in] parent The parent window needed to build the popup-menu
         * @return The built popup-menu
         * @see GUIGlObject::getPopUpMenu
         */
        GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app,
                                           GUISUMOAbstractView& parent);


        /** @brief Returns an own parameter window
         *
         * @param[in] app The application needed to build the parameter window
         * @param[in] parent The parent window needed to build the parameter window
         * @return The built parameter window
         * @see GUIGlObject::getParameterWindow
         */
        GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app,
                GUISUMOAbstractView& parent);


        /** @brief Returns the boundary to which the view shall be centered in order to show the object
         *
         * @return The boundary the object is within
         * @see GUIGlObject::getCenteringBoundary
         */
        Boundary getCenteringBoundary() const;


        /** @brief Draws the object
         * @param[in] s The settings for the current view (may influence drawing)
         * @see GUIGlObject::drawGL
         */
        void drawGL(const GUIVisualizationSettings& s) const;

        void onLeftBtnPress(void* data);

        RerouterEdgeType getRerouterEdgeType() const {
            return myEdgeType;
        }

        const MSEdge* getEdge() const {
            return myEdge;
        }
        //@}

    private:
        /// Definition of a positions container
        typedef std::vector<Position> PosCont;

        /// Definition of a rotation container
        typedef std::vector<double> RotCont;

    private:
        /// The parent rerouter to which this edge instance belongs
        GUITriggeredRerouter* myParent;

        /// The edge for which this visualization applies
        MSEdge* myEdge;

        /// whether this edge instance visualizes a closed edge
        const RerouterEdgeType myEdgeType;

        /// The positions in full-geometry mode
        PosCont myFGPositions;

        /// The rotations in full-geometry mode
        RotCont myFGRotations;

        /// The boundary of this rerouter
        Boundary myBoundary;

        /// @brief the index for this in edge in routeProbs
        int myDistIndex;
    };

public:
    class GUITriggeredRerouterPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUITriggeredRerouterPopupMenu)
    public:

        GUITriggeredRerouterPopupMenu(GUIMainWindow& app,
                                      GUISUMOAbstractView& parent, GUIGlObject& o);

        ~GUITriggeredRerouterPopupMenu();

        /** @brief Called if the object's manipulator shall be shown */
        long onCmdOpenManip(FXObject*, FXSelector, void*);

    protected:
        GUITriggeredRerouterPopupMenu() { }

    };


    class GUIManip_TriggeredRerouter : public GUIManipulator {
        FXDECLARE(GUIManip_TriggeredRerouter)
    public:
        enum {
            MID_USER_DEF = FXDialogBox::ID_LAST,
            MID_PRE_DEF,
            MID_OPTION,
            MID_CLOSE,
            MID_SHIFT_PROBS,
            ID_LAST
        };
        /// Constructor
        GUIManip_TriggeredRerouter(GUIMainWindow& app,
                                   const std::string& name, GUITriggeredRerouter& o,
                                   int xpos, int ypos);

        /// Destructor
        virtual ~GUIManip_TriggeredRerouter();

        long onCmdOverride(FXObject*, FXSelector, void*);
        long onCmdClose(FXObject*, FXSelector, void*);
        long onCmdUserDef(FXObject*, FXSelector, void*);
        long onUpdUserDef(FXObject*, FXSelector, void*);
        long onCmdChangeOption(FXObject*, FXSelector, void*);
        long onCmdShiftProbs(FXObject*, FXSelector, void*);

    private:
        GUIMainWindow* myParent;

        FXint myChosenValue;

        FXDataTarget myChosenTarget;

        double myUsageProbability;

        FXRealSpinner* myUsageProbabilityDial;

        FXDataTarget myUsageProbabilityTarget;

        GUITriggeredRerouter* myObject;

    protected:
        GUIManip_TriggeredRerouter() { }

    };


private:
    /// The boundary of this rerouter
    Boundary myBoundary;

    std::vector<GUITriggeredRerouterEdge*> myEdgeVisualizations;

    int myShiftProbDistIndex;
};
