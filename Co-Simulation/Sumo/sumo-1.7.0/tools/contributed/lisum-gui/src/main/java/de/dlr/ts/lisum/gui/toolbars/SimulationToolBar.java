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
/// @file    SimulationToolBar.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.toolbars;

import de.dlr.ts.commons.javafx.messages.ErrorMessage;
import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.gui.GlobalConfig;
import de.dlr.ts.lisum.simulation.LisumSimulation;
import de.dlr.ts.lisum.gui.Actions;
import de.dlr.ts.lisum.gui.Icons;
import de.dlr.ts.lisum.gui.SystemProperties;
import java.io.File;
import java.io.IOException;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventType;
import javafx.scene.control.Button;
import javafx.scene.control.MenuItem;
import javafx.scene.control.Separator;
import javafx.scene.control.SplitMenuButton;
import javafx.scene.control.ToolBar;
import javafx.scene.control.Tooltip;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public final class SimulationToolBar extends ToolBar implements ToolBarInterface {
    private String lastFile = null;

    private final SplitMenuButton split = new SplitMenuButton();
    {
        split.setPrefWidth(150);
        split.setPrefHeight(32);

        //split.setText("Edit simulation files");
        split.setTooltip(new Tooltip("Edit simulation project files"));

        split.setOnAction((ActionEvent event) -> {
            if (lastFile != null)
                editTextFile(lastFile);
        });
    }

    private final Button playButton = new Button("", Icons.getInstance().getIconImageView("Start", 24));
    {
        playButton.setOnAction((ActionEvent event) -> Actions.getInstance().execCurrentSimulation());
        playButton.setTooltip(new Tooltip("Open SUMO"));
    }

    private final Button reloadProjectButton = new Button("", Icons.getInstance().getIconImageView("Refresh", 24));
    {
        reloadProjectButton.setOnAction((ActionEvent event) -> {
            try {
                SystemProperties.getInstance().getCurrentSimulation().load();
            } catch (Exception ex) {
                DLRLogger.severe(this, ex);
            }
        });
        reloadProjectButton.setTooltip(new Tooltip("Reload simulation project files"));
    }

    private final Button exploreProjectFolderButton = new Button("", Icons.getInstance().getIconImageView("Folder3", 24));
    {
        exploreProjectFolderButton.setOnAction((ActionEvent event) -> {
            try {
                openSimulationFolder();
            } catch (Exception ex) {
                DLRLogger.severe(this, ex);
            }
        });
        exploreProjectFolderButton.setTooltip(new Tooltip("Explore simulation project folder"));
    }

    private final Button controlUnitsPreferencesButton = new Button("", Icons.getInstance().getIconImageView("Dots", 24));
    {
        controlUnitsPreferencesButton.setOnAction((ActionEvent event) -> {
            try {
                Actions.getInstance().openControlUnitPreferences();
            } catch (Exception ex) {
                DLRLogger.severe(this, ex);
            }
        });
        controlUnitsPreferencesButton.setTooltip(new Tooltip("Control Units Management"));
    }

    /**
     *
     */
    public SimulationToolBar() {
        this.getItems().addAll(new Separator(), playButton, reloadProjectButton, split, exploreProjectFolderButton, controlUnitsPreferencesButton);

        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) ->
                     this.setEnabled(SystemProperties.getInstance().simulationOpenedProperty().get())
                    );

        SystemProperties.getInstance().simulationStartedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) ->
                     this.setSimulationStarted(oldValue));

        this.setEnabled(false);
    }

    /**
     *
     * @param enabled
     */
    @Override
    public void setEnabled(boolean enabled) {
        playButton.setDisable(!enabled);
        reloadProjectButton.setDisable(!enabled);
        split.setDisable(!enabled);
        exploreProjectFolderButton.setDisable(!enabled);
        controlUnitsPreferencesButton.setDisable(!enabled);

        split.getItems().clear();

        if (enabled) {
            LisumSimulation cp = SystemProperties.getInstance().getCurrentSimulation();
            for (String s : cp.getSimulationFiles().getSimulationDirectory().list()) {
                if (s.endsWith(".xml") || s.endsWith(".sumocfg")) {
                    MenuItem mi = new MenuItem(s);
                    split.getItems().add(mi);
                    mi.addEventHandler(EventType.ROOT, (Event event) -> {
                        editTextFile(s);
                        split.setText(s);
                    });
                }
            }

            if (!split.getItems().isEmpty() && lastFile == null) {
                split.setText(split.getItems().get(0).getText());
                lastFile = split.getItems().get(0).getText();
            }
        }
    }

    /**
     *
     * @param enabled
     */
    @Override
    public void setSimulationStarted(boolean enabled) {
        setEnabled(enabled);
        controlUnitsPreferencesButton.setDisable(false);
    }

    /**
     *
     */
    private void openSimulationFolder() {
        String ff = SystemProperties.getInstance().getCurrentSimulation().getSimulationFiles()
                    .getSimulationDirectory().getAbsolutePath();

        String program = GlobalConfig.getInstance().getFilesExplorer();

        if (program.isEmpty()) {
            if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
                program = "explorer.exe";
            } else {
                ErrorMessage em = new ErrorMessage("Please set a files explorer program in the System Preferences");
                em.show();
                return;
            }
        } else {
            if (!System.getProperty("os.name").toUpperCase().contains("WIN"))
                if (program.endsWith(".exe")) {
                    ErrorMessage em = new ErrorMessage("Please set a suitable files explorer program in the System Preferences");
                    em.show();
                    return;
                }
        }

        try {
            new ProcessBuilder(program, ff).start();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    /**
     *
     * @param name
     */
    private void editTextFile(String name) {
        DLRLogger.finest(this, "Opening " + name);

        File dd = new File(name);
        if (!dd.isAbsolute()) {
            String ff = SystemProperties.getInstance().getCurrentSimulation().getSimulationFiles()
                        .getSimulationDirectory().getAbsolutePath();
            ff += File.separator + name;

            dd = new File(ff);
        }

        lastFile = dd.getAbsolutePath();

        //String program = "C:/Program Files (x86)/Notepad++/notepad++.exe";
        String program = GlobalConfig.getInstance().getTextEditor();

        DLRLogger.finest(this, "Text editor " + program);

        if (program.isEmpty()) { // || !new File(program).exists()
            if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
                program = "notepad.exe";
            } else {
                program = "gedit";
            }
        }

        try {
            new ProcessBuilder(program, dd.getAbsolutePath()).start();
        } catch (IOException ex) {
            DLRLogger.severe(this, "Error opening text editor: " + program);
            //ex.printStackTrace();
        }
    }

}
