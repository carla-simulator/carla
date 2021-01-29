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
/// @file    FileToolBar.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.toolbars;

import de.dlr.ts.lisum.gui.Actions;
import de.dlr.ts.lisum.gui.Icons;
import de.dlr.ts.lisum.gui.SystemProperties;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.scene.control.Button;
import javafx.scene.control.Separator;
import javafx.scene.control.ToolBar;
import javafx.scene.control.Tooltip;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class FileToolBar extends ToolBar implements ToolBarInterface {

    private Button openSimulationButton = new Button("", Icons.getInstance().getIconImageView("Folder2", 24));
    {
        openSimulationButton.setOnAction((ActionEvent event) -> {
            openSimulationButton.setDisable(true);
            Actions.getInstance().openSimulationSelectingFile();

            if (SystemProperties.getInstance().getCurrentSimulation() == null)
                openSimulationButton.setDisable(false);
        });

        openSimulationButton.setTooltip(new Tooltip("Open simulation"));
    }

    private Button openLastSimulationButton = new Button("", Icons.getInstance().getIconImageView("Footprint", 24));
    {

        openLastSimulationButton.setOnAction((ActionEvent event) -> {
            openLastSimulationButton.setDisable(true);

            Actions.getInstance().openLastSimulation();

            if (SystemProperties.getInstance().getCurrentSimulation() == null)
                openLastSimulationButton.setDisable(false);
        });

        openLastSimulationButton.setTooltip(new Tooltip("Open last simulation"));
    }

    private Button closeSimulationButton = new Button("", Icons.getInstance().getIconImageView("Player Eject", 24));
    {
        closeSimulationButton.setOnAction((ActionEvent event) -> Actions.getInstance().closeCurrentproject());
        closeSimulationButton.setDisable(true);
        closeSimulationButton.setTooltip(new Tooltip("Close simulation"));
    }

    {
        SystemProperties.getInstance().simulationOpenedProperty().addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            openSimulationButton.setDisable(newValue);
            openLastSimulationButton.setDisable(newValue);
            closeSimulationButton.setDisable(!newValue);
        });

        SystemProperties.getInstance().simulationStartedProperty().addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            closeSimulationButton.setDisable(newValue);
        });
    }

    /**
     *
     */
    public FileToolBar() {
        this.getItems().addAll(new Separator(), openSimulationButton, openLastSimulationButton, closeSimulationButton);

        //openLastSimulationButton,
    }

    @Override
    public void setEnabled(boolean enabled) {

    }

    @Override
    public void setSimulationStarted(boolean enabled) {

    }

}
