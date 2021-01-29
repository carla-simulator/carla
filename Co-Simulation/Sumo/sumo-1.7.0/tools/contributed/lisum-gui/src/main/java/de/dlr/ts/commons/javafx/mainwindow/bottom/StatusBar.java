/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    StatusBar.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.bottom;

import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.layout.AnchorPane;
import javafx.scene.text.Font;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class StatusBar {
    private AnchorPane root = new AnchorPane();
    private Label rightLabel = new Label();



    /**
     *
     */
    public StatusBar() {
        root.setPrefHeight(25);
        //mainPane.setStyle("-fx-border-color: #909090; -fx-border-insets: 2.;");
        //mainPane.setStyle("-fx-border-color: #00FF00;");
        //-fx-background-color: #FFFFFF;

        /**
         *
         */
        rightLabel.setPrefWidth(150);
        rightLabel.setAlignment(Pos.CENTER_RIGHT);
        rightLabel.setFont(new Font(14));

        AnchorPane.setTopAnchor(rightLabel, 2.);
        AnchorPane.setRightAnchor(rightLabel, 12.);
        AnchorPane.setBottomAnchor(rightLabel, 2.);
        root.getChildren().add(rightLabel);

    }

    /**
     *
     * @return
     */
    public Node getNode() {
        return root;
    }

    /**
     *
     * @return
     */
    public Label getRightLabel() {
        return rightLabel;
    }

}
