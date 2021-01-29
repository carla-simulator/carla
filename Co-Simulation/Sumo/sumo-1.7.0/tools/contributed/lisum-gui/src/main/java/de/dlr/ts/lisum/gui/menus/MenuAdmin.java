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
/// @file    MenuAdmin.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.menus;

import de.dlr.ts.commons.javafx.mainwindow.top.FileMenu;
import de.dlr.ts.commons.javafx.mainwindow.top.MainMenu;
import de.dlr.ts.lisum.simulation.LisumSimulation;
import de.dlr.ts.lisum.gui.Actions;
import de.dlr.ts.lisum.gui.GlobalConfig;
import de.dlr.ts.lisum.gui.MainProgram;
import de.dlr.ts.lisum.gui.SystemClipboard;
import de.dlr.ts.lisum.gui.SystemProperties;
import de.dlr.ts.lisum.gui.Tools;
import de.dlr.ts.lisum.gui.misc.LisumAboutWindow;
import de.dlr.ts.lisum.gui.options.SystemPreferencesWindow;
import java.awt.Desktop;
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.event.EventType;
import javafx.scene.Node;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.control.SeparatorMenuItem;
import javafx.scene.control.TextArea;
import javafx.scene.input.ClipboardContent;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCodeCombination;
import javafx.scene.input.KeyCombination;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MenuAdmin {
    private MainMenu mainMenuBar;

    /**
     *
     * @param mainMenuBar
     */
    public void setMainMenu(MainMenu mainMenuBar) {
        this.mainMenuBar = mainMenuBar;
    }

    /**
     *
     * @return
     */
    public MainMenu getMainMenuBar() {
        return mainMenuBar;
    }

    /**
     *
     */
    public void init() {
        initFileMenu();
        initSimulationMenu();
        initToolsMenu();
        initWindowMenu();
        initHelpMenu();

        MainMenu mainMenu = MainProgram.getInstance().getMainWindow().getTopArea().getMainMenu();
        MenuItem copyMenuItem = mainMenu.getEditMenu().getCopyMenuItem();
        copyMenuItem.setDisable(false);
        copyMenuItem.setOnAction((ActionEvent event) -> {
            Node focusOwner = MainProgram.getInstance().getMainWindow().getScene().getFocusOwner();

            if (focusOwner instanceof TextArea) {
                TextArea ta = (TextArea)focusOwner;
                ClipboardContent content = new ClipboardContent();
                content.putString(ta.getSelectedText());
                SystemClipboard.getInstance().getClipboard().setContent(content);
            }
        });
    }

    /**
     *
     */
    public void initDebugMenu() {
        mainMenuBar.addMenu(4, "Debug");
        MenuItem systemPropertiesMenuItem = new MenuItem("Show system properties");
        systemPropertiesMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
        });
    }


    /**
     *
     */
    public void initSimulationMenu() {
        Menu simuMenu = mainMenuBar.addMenu(2, "_Simulation");
        simuMenu.setAccelerator(KeyCombination.keyCombination("SHORTCUT+S"));

        MenuItem playMenuItem = new MenuItem("Open SUMO");
        playMenuItem.setDisable(true);
        playMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.P, KeyCodeCombination.CONTROL_DOWN));
        playMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            Actions.getInstance().execCurrentSimulation();
        });

        MenuItem reloadFilesMenuItem = new MenuItem("Reload simulation project files");
        reloadFilesMenuItem.setDisable(true);
        reloadFilesMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.P, KeyCodeCombination.CONTROL_DOWN));
        reloadFilesMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            try {
                SystemProperties.getInstance().getCurrentSimulation().load();
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        });

        simuMenu.getItems().addAll(playMenuItem, reloadFilesMenuItem);

        /**
         * When there's no opened simulation disable menu items
         */
        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            playMenuItem.setDisable(oldValue);
            reloadFilesMenuItem.setDisable(oldValue);
        });

        /**
         * When simulation starts disable play menu item
         */
        SystemProperties.getInstance().simulationStartedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            playMenuItem.setDisable(newValue);
            reloadFilesMenuItem.setDisable(newValue);
        });

    }

    /**
     *
     */
    public void hideSimulationMenu() {
        mainMenuBar.removeMenu("Simulation");
    }


    /**
     *
     */
    private void initWindowMenu() {
        Menu windowMenu = mainMenuBar.getWindowMenu();

        MenuItem fullScreenMenuItem = new MenuItem("Full Screen");
        fullScreenMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            MainProgram.getInstance().getStage().setFullScreen(true);
        });

        windowMenu.getItems().add(fullScreenMenuItem);
    }

    /**
     *
     */
    private void initFileMenu() {
        FileMenu fileMenu = mainMenuBar.getFileMenu();

        /**
         * Adding action to menu item "Open..."
         */
        MenuItem openMenuItem = fileMenu.getOpenMenuItem();
        openMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            Actions.getInstance().openSimulationSelectingFile();
        });

        /**
         * Adding listener to last opened files
         */
        fileMenu.addLastOpenedFilesListener((String filePath) -> {
            File file = new File(filePath);
            if (file.exists())
                Actions.getInstance().openSimulation(file);
        });

        /**
         * Disabling menu items when a simulation was opened
         */
        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            LisumSimulation ssimo = SystemProperties.getInstance().getCurrentSimulation();
            if (ssimo != null) {
                String simu = ssimo.getConfigurationFile().getConfigurationFile().getAbsolutePath();
                fileMenu.addLastOpen(simu);
            }

            openMenuItem.setDisable(newValue);
            fileMenu.setLastOpenFilesEnabled(!newValue);
        });


        /**
         *
         */
        MenuItem closeSimulationMenuItem = fileMenu.getCloseMenuItem();
        closeSimulationMenuItem.setDisable(true);
        closeSimulationMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            Actions.getInstance().closeCurrentproject();
        });
        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            closeSimulationMenuItem.setDisable(oldValue);
        });
        SystemProperties.getInstance().simulationStartedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            closeSimulationMenuItem.setDisable(newValue);
        });


        /**
         *
         */
        MenuItem exitSystemMenuItem = fileMenu.getExitSystemMenuItem();
        exitSystemMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            Stage stage = MainProgram.getInstance().getStage();
            stage.fireEvent(new WindowEvent(stage, WindowEvent.WINDOW_CLOSE_REQUEST));
        });

        Actions.getInstance().addOnCloseListener((EventHandler)(Event event) -> {
            ///fileMenu.close();
        });
    }


    /**
     *
     */
    public void initToolsMenu() {
        Menu addMenu = mainMenuBar.addMenu(3, "_Tools");
        addMenu.setAccelerator(KeyCombination.keyCombination("SHORTCUT+T"));


        MenuItem restFulServerMenuItem = new MenuItem("Start LISA+ Virtual Controllers");
        restFulServerMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.R, KeyCodeCombination.CONTROL_DOWN));
        restFulServerMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            Tools.startLisaRESTfulServer(GlobalConfig.getInstance().getLisaRESTfulServerPath(),
                                         GlobalConfig.getInstance().getLisaServerPort());
        });
        addMenu.getItems().add(restFulServerMenuItem);
        addMenu.getItems().add(new SeparatorMenuItem());

        /**
         *
         */
        MenuItem controlUnitPreferencesItem = new MenuItem("Control Units Management");
        controlUnitPreferencesItem.setDisable(true);
        controlUnitPreferencesItem.setAccelerator(new KeyCodeCombination(KeyCode.M, KeyCodeCombination.CONTROL_DOWN));
        controlUnitPreferencesItem.addEventHandler(EventType.ROOT, (Event event) -> Actions.getInstance().openControlUnitPreferences());
        addMenu.getItems().add(controlUnitPreferencesItem);

        /**
         * When there's no opened simulation disable control units menu item
         */
        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) ->
                     controlUnitPreferencesItem.setDisable(oldValue)
                    );

        addMenu.getItems().add(new SeparatorMenuItem());

        /**
         *
         */
        MenuItem preferenciesMenuItem = new MenuItem("System Preferences");
        preferenciesMenuItem.addEventHandler(EventType.ROOT, (Event event) -> new SystemPreferencesWindow().show(0));
        addMenu.getItems().add(preferenciesMenuItem);
    }

    /**
     *
     */
    private void initHelpMenu() {
        Menu helpMenu = mainMenuBar.getHelpMenu();

        MenuItem helpMenuItem = new MenuItem("LiSuM Online Documentation");
        helpMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.F1, KeyCodeCombination.SHIFT_DOWN));

        helpMenuItem.addEventHandler(EventType.ROOT, (Event event) -> {
            try {
                Desktop.getDesktop().browse(new URI("https://sumo.dlr.de/docs/Tools/LiSuM.html"));
            } catch (IOException | URISyntaxException ex) {
                ex.printStackTrace(System.out);
            }
        });
        helpMenu.getItems().add(helpMenuItem);

        /**
         *
         */
        MenuItem item = new MenuItem("About");
        item.addEventHandler(EventType.ROOT, (Event event) -> new LisumAboutWindow(MainProgram.getInstance().getScene().getWindow()).show());
        helpMenu.getItems().add(item);
    }

}
