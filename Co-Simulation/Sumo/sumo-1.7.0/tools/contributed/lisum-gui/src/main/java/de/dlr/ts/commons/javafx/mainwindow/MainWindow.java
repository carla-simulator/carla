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
/// @file    MainWindow.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow;

import de.dlr.ts.commons.javafx.mainwindow.bottom.BottomArea;
import de.dlr.ts.commons.javafx.mainwindow.center.CenterArea;
import de.dlr.ts.commons.javafx.mainwindow.right.RightArea;
import de.dlr.ts.commons.javafx.mainwindow.top.TopArea;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MainWindow {
    private final BorderPane mainBorderPane = new BorderPane();

    private Stage stage;
    private Scene scene;

    private RightArea rightArea;
    private TopArea topArea = new TopArea();
    private CenterArea centerArea = new CenterArea();
    private BottomArea bottomArea = new BottomArea();

    private int width;
    private int height;



    /**
     *
     * @param stage
     * @param width
     * @param height
     */
    public MainWindow(Stage stage, int width, int height) {
        this.stage = stage;

        scene = new Scene(mainBorderPane, width, height);
        scene.getStylesheets().add("/styles/Styles.css");
        stage.setScene(scene);

        this.width = width;
        this.height = height;

        /**
         *
         */
        rightArea = new RightArea();
        //mainBorderPane.setRight(rightPanel);
        rightArea.setWidth(200);
        BorderPane.setMargin(rightArea.getNode(), new Insets(3.));

        /**
         *
         */
        mainBorderPane.setTop(topArea.getNode());
        mainBorderPane.setCenter(centerArea.getNode());
        mainBorderPane.setBottom(bottomArea.getNode());
    }


    /**
     *
     * @return
     */
    public CenterArea getCenterArea() {
        return centerArea;
    }

    /**
     *
     * @return
     */
    public RightArea getRightPane() {
        return rightArea;
    }

    /**
     *
     * @return
     */
    public Scene getScene() {
        return scene;
    }

    /**
     *
     * @return
     */
    public Stage getStage() {
        return stage;
    }

    /**
     *
     */
    public void show() {
        stage.show();
    }

    /**
     *
     * @return
     */
    public TopArea getTopArea() {
        return topArea;
    }

    /**
     *
     * @return
     */
    public BottomArea getBottomArea() {
        return bottomArea;
    }

}
