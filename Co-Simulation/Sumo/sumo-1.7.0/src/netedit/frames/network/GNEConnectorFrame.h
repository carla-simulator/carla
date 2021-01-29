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
/// @file    GNEConnectorFrame.h
/// @author  Jakob Erdmann
/// @date    May 2011
///
// The Widget for modifying lane-to-lane connections
/****************************************************************************/
#pragma once

#include <config.h>
#include <netbuild/NBEdge.h>
#include <netedit/frames/GNEFrame.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEConnectorFrame
 * The Widget for modifying selections of network-elements
 */
class GNEConnectorFrame : public GNEFrame {

public:

    // ===========================================================================
    // class CurrentLane
    // ===========================================================================

    class CurrentLane : protected FXGroupBox {

    public:
        /// @brief constructor
        CurrentLane(GNEConnectorFrame* connectorFrameParent);

        /// @brief destructor
        ~CurrentLane();

        /// @brief set current junction label
        void updateCurrentLaneLabel(const std::string& laneID);

    private:
        /// @brief Label for current Lane
        FXLabel* myCurrentLaneLabel;
    };

    // ===========================================================================
    // class ConnectionModifications
    // ===========================================================================

    class ConnectionModifications : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEConnectorFrame::ConnectionModifications)

    public:
        /// @brief constructor
        ConnectionModifications(GNEConnectorFrame* connectorFrameParent);

        /// @brief destructor
        ~ConnectionModifications();

        /// @name FOX-callbacks
        /// @{

        /// @brief Called when the user presses the OK-Button saves any connection modifications
        long onCmdSaveModifications(FXObject*, FXSelector, void*);

        /// @brief Called when the user presses the Cancel-button discards any connection modifications
        long onCmdCancelModifications(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(ConnectionModifications)

    private:
        /// @brief pointer to connectorFrame parent
        GNEConnectorFrame* myConnectorFrameParent;

        /// @brief "Cancel" button
        FXButton* myCancelButton;

        /// @brief "OK" button
        FXButton* mySaveButton;

        /// @brief protect routes checkbox
        FXCheckButton* myProtectRoutesCheckBox;
    };

    // ===========================================================================
    // class ConnectionOperations
    // ===========================================================================

    class ConnectionOperations : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEConnectorFrame::ConnectionOperations)

    public:
        /// @brief constructor
        ConnectionOperations(GNEConnectorFrame* connectorFrameParent);

        /// @brief destructor
        ~ConnectionOperations();

        /// @name FOX-callbacks
        /// @{

        /// @brief Called when the user presses the select dead ends button
        long onCmdSelectDeadEnds(FXObject*, FXSelector, void*);

        /// @brief Called when the user presses the select dead starts button
        long onCmdSelectDeadStarts(FXObject*, FXSelector, void*);

        /// @brief Called when the user presses the select conflicts button
        long onCmdSelectConflicts(FXObject*, FXSelector, void*);

        /// @brief Called when the user presses the select pass button
        long onCmdSelectPass(FXObject*, FXSelector, void*);

        /// @brief Called when the user presses the clear selected connections  button
        long onCmdClearSelectedConnections(FXObject*, FXSelector, void*);

        /// @brief Called when the user presses the reset selected connections button
        long onCmdResetSelectedConnections(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(ConnectionOperations)

    private:
        /// @brief pointer to connectorFrame parent
        GNEConnectorFrame* myConnectorFrameParent;

        /// @brief "Select Dead Ends" button
        FXButton* mySelectDeadEndsButton;

        /// @brief "Select Dead Starts" button
        FXButton* mySelectDeadStartsButton;

        /// @brief "Select Conflicts" button
        FXButton* mySelectConflictsButton;

        /// @brief "Select Edges which may always pass"
        FXButton* mySelectPassingButton;

        /// @brief "Clear Selected"
        FXButton* myClearSelectedButton;

        /// @brief "Reset Selected"
        FXButton* myResetSelectedButton;
    };

    // ===========================================================================
    // class ConnectionSelection
    // ===========================================================================

    class ConnectionSelection : protected FXGroupBox {

    public:
        /// @brief constructor
        ConnectionSelection(GNEConnectorFrame* connectorFrameParent);

        /// @brief destructor
        ~ConnectionSelection();

    private:
        /// @brief Selection Hint
        FXLabel* myHoldShiftLabel;

        /// @brief hold control label
        FXLabel* myHoldControlLabel;
    };

    // ===========================================================================
    // class ConnectionLegend
    // ===========================================================================

    class ConnectionLegend : protected FXGroupBox {

    public:
        /// @brief constructor
        ConnectionLegend(GNEConnectorFrame* connectorFrameParent);

        /// @brief destructor
        ~ConnectionLegend();

    private:
    };

    /**@brief Constructor
     * @brief parent FXHorizontalFrame in which this GNEFrame is placed
     * @brief viewNet viewNet that uses this GNEFrame
     */
    GNEConnectorFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEConnectorFrame();

    /**@brief either sets the current lane or toggles the connection of the
     * @param objectsUnderCursor collection of objects under cursor after click over view
     */
    void handleLaneClick(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor);

    /// @brief get pointer to ConnectionModifications modul
    ConnectionModifications* getConnectionModifications() const;

private:
    /// @brief the status of a target lane
    enum class LaneStatus {
        UNCONNECTED,
        CONNECTED,
        CONNECTED_PASS,
        CONFLICTED
    };

    /**@brief either sets the current lane or toggles the connection of the
     * current lane to this lane (if they share a junction)
     * @param[in] lane Either the lane to set as current lane, or the destination from current lane
     * @param[in] mayDefinitelyPass Whether new connections shall have the pass attribute set
     * @param[in] toggle Whether non-existing connections shall be created
     */
    void buildConnection(GNELane* lane, const bool mayDefinitelyPass, const bool allowConflict, const bool toggle);

    /// @brief init targets
    void initTargets();

    /// @brief clean up when deselecting current lane
    void cleanup();

    /// @brief remove connections
    void removeConnections(GNELane* lane);

    /// @brief return the status of toLane
    LaneStatus getLaneStatus(const std::vector<NBEdge::Connection>& connections, const GNELane* targetLane) const;

    /// @brief CurrentLane modul
    CurrentLane* myCurrentLane;

    /// @brief ConnectionModifications modul
    ConnectionModifications* myConnectionModifications;

    /// @brief ConnectionOperations modul
    ConnectionOperations* myConnectionOperations;

    /// @brief ConnectionSelection modul
    ConnectionSelection* myConnectionSelection;

    /// @brief ConnectionLegend modul
    ConnectionLegend* myConnectionLegend;

    /// @brief the lane of which connections are to be modified
    GNELane* myCurrentEditedLane;

    /// @brief the set of lanes to which the current lane may be connected
    std::set<GNELane*> myPotentialTargets;

    /// @brief number of changes
    int myNumChanges;

    /// @brief the internal lanes belonging the the current junction indexed by their tl-index
    std::map<int, GNEInternalLane*> myInternalLanes;

    /// @brief vector of connections deleted in the current editing step
    std::vector<NBEdge::Connection> myDeletedConnections;
};
