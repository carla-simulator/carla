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
/// @file    Splash.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.misc;

import de.dlr.ts.lisum.gui.SystemProperties;
import javafx.scene.Scene;
import javafx.scene.effect.Bloom;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class Splash {
    private final Scene scene;
    private final Stage stage;

    private final ImageView imageView = new ImageView(SystemProperties.getInstance().getMainIcon());
    private final StackPane root = new StackPane(imageView);

    /**
     *
     */
    public Splash() {
        Bloom bloom = new Bloom();
        bloom.setThreshold(0.7);
        imageView.setEffect(bloom);

        imageView.setStyle(
            "-fx-background-color: rgba(255, 255, 255, 1.0);"
            +   "-fx-effect: dropshadow(gaussian, gray, 50, 0, 0, 0);"
            +   "-fx-background-insets: 10;"
        );

        scene = new Scene(root, 300, 350);
        //scene.setFill(Color.TRANSPARENT);
        stage = new Stage();
        stage.setScene(scene);
        //stage.initStyle(StageStyle.TRANSPARENT);
        stage.centerOnScreen();

    }

}
