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
/// @file    MainProgram.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui;

import de.dlr.ts.lisum.gui.center.CenterAdmin;
import de.dlr.ts.lisum.gui.menus.MenuAdmin;
import de.dlr.ts.lisum.gui.toolbars.SimulationToolBar;
import de.dlr.ts.commons.javafx.mainwindow.MainWindow;
import de.dlr.ts.commons.javafx.messages.Message;
import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.logger.LogLevel;
import de.dlr.ts.lisum.gui.options.WorkspaceWindow;
import de.dlr.ts.lisum.gui.toolbars.FileToolBar;
import javafx.application.Platform;
import javafx.beans.value.ObservableValue;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MainProgram {
    private static final MainProgram INSTANCE = new MainProgram();

    private Stage stage;
    private Scene scene;
    private MenuAdmin menuAdmin;
    private MainWindow mainWindow;
    private final FileToolBar fileToolBar = new FileToolBar();
    private final SimulationToolBar simulationToolBar = new SimulationToolBar();
    private boolean simulationStarted = false;

    //private CenterAdmin centerAdmin;

    /**
     *
     */
    private final ScrollPane scrollPane = new ScrollPane();
    {
        scrollPane.setHbarPolicy(ScrollPane.ScrollBarPolicy.NEVER);
        scrollPane.setVbarPolicy(ScrollPane.ScrollBarPolicy.NEVER);
    }

    /**
     *
     */
    public void init() {
    }

    /**
     *
     * @param stage
     */
    public void start(Stage stage) {
        this.stage = stage;

        /**
         * Checking Workspace
         */
        WorkspaceWindow workspaceWindow = new WorkspaceWindow();
        workspaceWindow.readFromFile();

        if (WorkspaceWindow.getWorkspace() == null) {
            workspaceWindow.show(null);
        }

        if (WorkspaceWindow.getWorkspace() == null) {
            return;
        }

        GlobalConfig.getInstance().setWorkspace(WorkspaceWindow.getWorkspace().getAbsolutePath());

        /**
         *
         */
        stage.setTitle(SystemProperties.getInstance().getSystemName());

        mainWindow = new MainWindow(stage,
                                    SystemProperties.getInstance().getWidth(),
                                    SystemProperties.getInstance().getHeight());
        mainWindow.getStage().setMaximized(true);
        scene = mainWindow.getScene();

        //ImageView iv = new ImageView(new Image("file:images/LisaSumoIcon.png"));
        //mainWindow.getCenterAnchorPane().getChildren().add(iv);

        stage.setOnCloseRequest((WindowEvent event) -> { Actions.getInstance().exitSystem(event, stage); });

        DLRLogger.setLevel(LogLevel.parse(GlobalConfig.getInstance().getLoggingLevel()));

        /**
         * Menu
         */
        initMainMenu();

        /**
         * Setting icon
         */
        stage.getIcons().add(SystemProperties.getInstance().getMainIcon());

        /**
         *
         */
        Message.setProgramName(SystemProperties.getInstance().getSystemName());
        Message.setIcon(SystemProperties.getInstance().getMainIcon());

        mainWindow.getCenterArea().set(new CenterAdmin().getNode());

        /**
         *
         */
        mainWindow.getTopArea().getMainToolBar().addToolBar(fileToolBar);
        mainWindow.getTopArea().getMainToolBar().addToolBar(simulationToolBar);

        bottomBarUpdate();
        changeApplicationTitleListener();

        mainWindow.show();
    }


    /**
     *
     */
    private void changeApplicationTitleListener() {
        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            if (newValue)
                stage.setTitle(SystemProperties.getInstance().getSystemName() + " - " +
                               SystemProperties.getInstance().getCurrentSimulation().getSimulationFiles().getLisumConfigFile());
            else {
                stage.setTitle(SystemProperties.getInstance().getSystemName());
            }
        });
    }

    /**
     *
     */
    private void bottomBarUpdate() {
        SystemProperties.getInstance().simulationStartedProperty().addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            simulationStarted = newValue;

            if (newValue) {
                new Thread(() -> {
                    Label rightLabel = mainWindow.getBottomArea().getStatusBar().getRightLabel();
                    rightLabel.setPrefWidth(550d);
                    while (simulationStarted) {
                        long stepsPerSec = SystemProperties.getInstance().getCurrentSimulation().getSumo().getStepsPerSecond();
                        long step = SystemProperties.getInstance().getCurrentSimulation().getCurrentSimulationStep();
                        long vehiclesCount = SystemProperties.getInstance().getCurrentSimulation().getSumo().getVehiclesCount();

                        String formatted = "Vehicles count: " + String.format("%03d", vehiclesCount);
                        formatted = formatted + " | Steps per second: " + String.format("%04d", stepsPerSec);
                        String string = formatted + " |  Simulation step: " + String.format("%07d", step);

                        Platform.runLater(() -> rightLabel.setText(string));

                        Tools.sleepi(200L);
                    }

                    Platform.runLater(() -> rightLabel.setText(""));
                }).start();
            } else {
                simulationStarted = false;
            }
        });
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
     */
    private void initMainMenu() {
        menuAdmin = new MenuAdmin();
        menuAdmin.setMainMenu(mainWindow.getTopArea().getMainMenu());
        menuAdmin.init();
    }

    /**
     *
     * @return
     */
    public MainWindow getMainWindow() {
        return mainWindow;
    }

    /**
     *
     * @return
     */
    public MenuAdmin getMenuAdmin() {
        return menuAdmin;
    }

    /**
     *
     * @return
     */
    public static MainProgram getInstance() {
        return INSTANCE;
    }

    /**
     *
     * @return
     */
    public Stage getStage() {
        return stage;
    }

}
