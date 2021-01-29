/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    BottomArea.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.bottom;

import javafx.scene.Node;
import javafx.scene.layout.AnchorPane;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class BottomArea {
    //private DebugScreen debugScreen;
    private StatusBar statusBar = new StatusBar();
    private AnchorPane anchorPane = new AnchorPane();


    /**
     *
     */
    public BottomArea() {
        /*
        debugScreen = new DebugScreen();
        debugScreen.init();
        debugScreen.setStyle("-fx-background-color: #989898; -fx-background-color: #989898;");

        //centerAnchorPane.getChildren().add(debugScreen);
        //AnchorPane.setTopAnchor(debugScreen, 500.);
        debugScreen.setPrefHeight(100);

        AnchorPane.setRightAnchor(debugScreen, 0.);
        AnchorPane.setLeftAnchor(debugScreen, 0.);
        AnchorPane.setBottomAnchor(debugScreen, 27.);
        */

        /**
         *
         */
        anchorPane.getChildren().add(statusBar.getNode());

        //AnchorPane.setTopAnchor(bottomBar.getNode(), 0.);
        AnchorPane.setRightAnchor(statusBar.getNode(), 0.);
        AnchorPane.setLeftAnchor(statusBar.getNode(), 0.);
        AnchorPane.setBottomAnchor(statusBar.getNode(), 0.);
    }

    /**
     *
     * @return
     */
    public Node getNode() {
        return anchorPane;
    }

    /**
     *
     * @return
     */
    public StatusBar getStatusBar() {
        return statusBar;
    }

}
