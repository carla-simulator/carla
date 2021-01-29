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
/// @file    MainToolBar.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.top;

import javafx.scene.Node;
import javafx.scene.control.ToolBar;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.HBox;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MainToolBar {
    private final AnchorPane anchorPane = new AnchorPane();
    //private final FlowPane flowPane = new FlowPane(Orientation.HORIZONTAL); //For differents toolbars
    private final HBox hbox = new HBox();
    private final int TOOLBAR_HEIGHT = 35; //33 para botones normales

    /**
     *
     */
    public MainToolBar() {
        ToolBar base = new ToolBar(); //used to paint the whole bar
        //base.setPrefHeight(TOOLBAR_HEIGHT);
        //base.setStyle("-fx-background-color: red;");

        anchorPane.getChildren().addAll(base, hbox);
        anchorPane.setPrefHeight(TOOLBAR_HEIGHT);

        //hbox.setStyle("-fx-background-color: green;");

        AnchorPane.setTopAnchor(base, 0.);
        AnchorPane.setLeftAnchor(base, 0.);
        AnchorPane.setRightAnchor(base, 0.);
        AnchorPane.setBottomAnchor(base, 0.);

        AnchorPane.setTopAnchor(hbox, 0.);
        AnchorPane.setLeftAnchor(hbox, 0.);
        AnchorPane.setRightAnchor(hbox, 0.);
        AnchorPane.setBottomAnchor(hbox, 0.);
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
     * @param node
     */
    public void addToolBar(ToolBar node) {
        hbox.getChildren().add(node);
    }

    /**
     *
     * @param toolbar
     */
    public void removeToolBar(ToolBar toolbar) {
        hbox.getChildren().remove(toolbar);
    }
}
