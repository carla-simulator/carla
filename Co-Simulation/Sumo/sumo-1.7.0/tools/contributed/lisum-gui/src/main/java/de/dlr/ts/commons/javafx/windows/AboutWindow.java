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
/// @file    AboutWindow.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.windows;

import de.dlr.ts.commons.javafx.buttonspanels.OneButtonsPanel;
import javafx.event.ActionEvent;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.image.ImageView;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.StackPane;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;
import javafx.stage.Window;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public final class AboutWindow {
    private Scene scene;
    private Stage stage;
    private AnchorPane root = new AnchorPane();
    private AnchorPane bodyPane = new AnchorPane();

    private OneButtonsPanel okButtonPanel = new OneButtonsPanel();
    private ImageView icon;
    private String systemNameString;
    private String copyright;

    private TextArea textArea = new TextArea();
    private String license = new String();


    /**
     *
     * @param owner
     */
    public AboutWindow(Window owner) {
        scene = new Scene(root, 450, 600);

        stage = new Stage();
        stage.initStyle(StageStyle.UTILITY);
        stage.setScene(scene);
        stage.initModality(Modality.WINDOW_MODAL);
        stage.initOwner(owner);
        stage.setResizable(false);

        okButtonPanel.getButton().setText("OK");
        AnchorPane.setLeftAnchor(okButtonPanel.getNode(), 5.);
        AnchorPane.setRightAnchor(okButtonPanel.getNode(), 5.);
        AnchorPane.setBottomAnchor(okButtonPanel.getNode(), 5.);
        root.getChildren().add(okButtonPanel.getNode());
        okButtonPanel.setAlignment(Pos.CENTER);

        AnchorPane.setTopAnchor(bodyPane, 0.);
        AnchorPane.setLeftAnchor(bodyPane, 0.);
        AnchorPane.setRightAnchor(bodyPane, 0.);
        AnchorPane.setBottomAnchor(bodyPane, 45.);
        root.getChildren().add(bodyPane);

        okButtonPanel.getButton().setOnAction((ActionEvent event) -> {
            stage.close();
        });

        //ScrollBar scrollBarv = (ScrollBar)textArea.lookup(".scroll-bar:horizontal");
        //scrollBarv.setDisable(true);

        AnchorPane.setLeftAnchor(textArea, 15.);
        AnchorPane.setRightAnchor(textArea, 15.);
        AnchorPane.setBottomAnchor(textArea, 12.);
        textArea.setPrefHeight(140.);
        textArea.setWrapText(true);
        textArea.setEditable(false);
        bodyPane.getChildren().add(textArea);
    }

    /**
     *
     */
    private void build() {
        StackPane stackPane = new StackPane(icon);
        AnchorPane.setTopAnchor(stackPane, 0.);
        AnchorPane.setLeftAnchor(stackPane, 0.);
        AnchorPane.setRightAnchor(stackPane, 0.);
        bodyPane.getChildren().add(stackPane);

        /**
         *
         */
        Label systemName = new Label(systemNameString);
        //label.setFont(Font.font(20));
        systemName.setFont(Font.font("Arial", FontWeight.BOLD, 20));
        systemName.setAlignment(Pos.CENTER);
        AnchorPane.setLeftAnchor(systemName, 0.);
        AnchorPane.setRightAnchor(systemName, 0.);
        AnchorPane.setBottomAnchor(systemName, 210.);
        bodyPane.getChildren().add(systemName);

        Label first = new Label(copyright);
        first.setAlignment(Pos.CENTER);
        AnchorPane.setLeftAnchor(first, 0.);
        AnchorPane.setRightAnchor(first, 0.);
        AnchorPane.setBottomAnchor(first, 188.);
        bodyPane.getChildren().add(first);

        textArea.setText(license);
    }

    /**
     *
     * @return
     */
    public AnchorPane getBodyPane() {
        return bodyPane;
    }

    /**
     *
     */
    public void show() {
        build();
        stage.show();
    }

    public void setIcon(ImageView icon) {
        this.icon = icon;
    }

    public void setCopyright(String copyright) {
        this.copyright = copyright;
    }

    public void setSystemNameString(String systemNameString) {
        this.systemNameString = systemNameString;
        stage.setTitle("About " + systemNameString);
    }

    public void setLicense(String license) {
        this.license = license;
    }

}
