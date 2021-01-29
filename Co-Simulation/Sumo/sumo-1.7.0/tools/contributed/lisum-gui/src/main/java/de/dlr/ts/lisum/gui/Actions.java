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
/// @file    Actions.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui;

import de.dlr.ts.commons.javafx.messages.ConfirmationMessage;
import de.dlr.ts.commons.javafx.messages.ExceptionMessage;
import de.dlr.ts.commons.javafx.messages.InformationMessage;
import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.simulation.LisumSimulation;
import de.dlr.ts.lisum.exceptions.LisumException;
import de.dlr.ts.lisum.gui.options.LisaControlUnitPreferences;
import de.dlr.ts.lisum.gui.options.SystemPreferencesWindow;
import de.dlr.ts.lisum.gui.options.WorkspaceWindow;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import javafx.event.EventHandler;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.MenuItem;
import javafx.stage.FileChooser;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class Actions {

    private static final Actions INSTANCE = new Actions();
    private final List<EventHandler> onCloseListeners = new ArrayList<>();

    /**
     *
     * @return
     */
    public static Actions getInstance() {
        return INSTANCE;
    }

    /**
     *
     * @param eventHandler
     */
    public void addOnCloseListener(EventHandler eventHandler) {
        onCloseListeners.add(eventHandler);
    }

    /**
     *
     */
    public void openLastSimulation() {
        SystemProperties.getInstance().setCurrentSimulation(null);

        MenuItem[] lastOpenFiles = MainProgram.getInstance().getMenuAdmin().getMainMenuBar().getFileMenu().getLastOpenFiles();

        if (lastOpenFiles != null && lastOpenFiles.length > 0) {
            File selectedFile = new File(lastOpenFiles[0].getText());
            openSimulation(selectedFile);
        } else {
            InformationMessage im = new InformationMessage("List of recent opened files is empty.");
            im.show();
        }
    }

    /**
     *
     * @return
     */
    public File openSimulationSelectingFile() {
        SystemProperties.getInstance().setCurrentSimulation(null);

        //DirectoryChooser dch = new DirectoryChooser();
        FileChooser fch = new FileChooser();
        fch.setTitle("Select simulation file");
        FileChooser.ExtensionFilter extf = new FileChooser.ExtensionFilter("LiSuM Config File", "lisum.xml");
        fch.getExtensionFilters().add(extf);

        File file = new File(GlobalConfig.getInstance().getWorkspace());
        if (!file.exists()) {
            file.mkdirs();
        }

        fch.setInitialDirectory(file);

        File selectedFile = fch.showOpenDialog(MainProgram.getInstance().getScene().getWindow());
        openSimulation(selectedFile);

        return selectedFile;
    }

    /**
     *
     * @param selectedFile
     */
    public void openSimulation(File selectedFile) {
        if (selectedFile != null && selectedFile.exists()) {
            try {
                DLRLogger.info(this, "Opening simulation " + selectedFile.getAbsolutePath());

                GlobalConfig g = GlobalConfig.getInstance();

                LisumSimulation simulation = new LisumSimulation(g.getSumoExec(), g.getSumoPort(),
                        g.getLisaServerAddress(), g.getLisaServerPort());
                simulation.load(selectedFile);

                SystemProperties.getInstance().setCurrentSimulation(simulation);
            } catch (LisumException ex) {
                ExceptionMessage em = new ExceptionMessage(null, "Error reading configuration file", ex);
                em.show();
                //DLRLogger.severe(this, ex);
            } catch (Exception ex) {
                DLRLogger.severe(this, ex);
                //Actions.getInstance().lisaServerNotFoundMessage();
            }
        }
    }

    /**
     *
     */
    public void openControlUnitPreferences() {
        LisaControlUnitPreferences.justDoIt();
    }

    /**
     *
     */
    public void closeCurrentproject() {
        LisumSimulation cs = SystemProperties.getInstance().getCurrentSimulation();
        if (cs != null) {
            DLRLogger.info("Actions", "Closing simulation " + cs.getSimulationFiles().getSimulationDirectory());
        }

        SystemProperties.getInstance().setCurrentSimulation(null);
    }

    /**
     *
     */
    private void lisaServerNotFoundMessage() {
        GlobalConfig g = GlobalConfig.getInstance();
        String dd = g.getLisaServerAddress() + ":" + g.getLisaServerPort();

        ConfirmationMessage cm = new ConfirmationMessage("Do you want to start a new instance?");
        cm.setHeader("No Lisa+ RESTful Server could be found in " + dd);
        Optional<ButtonType> result = cm.showAndWait();

        if (result.get() == ButtonType.OK) {
            Tools.startLisaRESTfulServer(g.getLisaRESTfulServerPath(), g.getLisaServerPort());
        }
    }

    /**
     *
     */
    private void noSumoExecMessage() {
        ConfirmationMessage cm = new ConfirmationMessage("Do you want to fix this now?");
        cm.setHeader("No SumoGUI executable could be found.");
        Optional<ButtonType> result = cm.showAndWait();

        if (result.get() == ButtonType.OK) {
            new SystemPreferencesWindow().show(1);
        }
    }

    /**
     *
     */
    public void execCurrentSimulation() {

        String sumoExec = GlobalConfig.getInstance().getSumoExec();

        if (sumoExec.isEmpty()) {
            noSumoExecMessage();
            return;
        }

        /**
         * Init before play
         */
        LisumSimulation.InitBeforePlayResponse initBeforePlay = SystemProperties.getInstance().getCurrentSimulation().initBeforePlay();

        if (initBeforePlay == LisumSimulation.InitBeforePlayResponse.LisaRESTfulServerNotFound) {
            Actions.getInstance().lisaServerNotFoundMessage();
        } else {
            Runnable r = SystemProperties.getInstance().getCurrentSimulation().getRunnable();
            Thread worker = new Thread(r);
            worker.start();

            Thread thread = new Thread(()
            -> {
                try {
                    worker.join();

                    SystemProperties.getInstance().simulationStartedProperty().set(false);
                } catch (InterruptedException ex) {
                    ex.printStackTrace(System.out);
                }
            });
            thread.start();

            SystemProperties.getInstance().simulationStartedProperty().set(true);
        }
    }

    /**
     *
     * @return True if workspace changed
     */
    public boolean switchWorkspace() {
        File workspace = WorkspaceWindow.getWorkspace();
        new WorkspaceWindow().show(MainProgram.getInstance().getScene().getWindow());

        boolean aa = WorkspaceWindow.getWorkspace().compareTo(workspace) != 0;

        if (aa) {
            GlobalConfig.getInstance().setWorkspace(WorkspaceWindow.getWorkspace().getAbsolutePath());
        }

        return aa;
    }

    /**
     *
     * @param event
     * @param stage
     */
    public void exitSystem(WindowEvent event, Stage stage) {
        if (SystemProperties.getInstance().simulationStartedProperty().get()) {
            StringBuilder sb = new StringBuilder("A simulation is currently running.").append("\n");
            sb.append("Do you really want to exit?");

            ConfirmationMessage cm = new ConfirmationMessage(sb.toString());

            Optional<ButtonType> showAndWait = cm.showAndWait();

            if (event == null) {
                if (showAndWait.get() == ButtonType.OK) {
                    for (EventHandler onCloseListener : onCloseListeners) {
                        onCloseListener.handle(null);
                    }

                    closeCurrentproject();
                    GlobalConfig.getInstance().saveProps();
                    event.consume();
                }
            } else {
                if (showAndWait.get() == ButtonType.CANCEL) {
                    event.consume();
                } else {
                    closeCurrentproject();
                    GlobalConfig.getInstance().saveProps();
                }
            }
        } else {
            Alert closeConfirmation = new Alert(Alert.AlertType.CONFIRMATION, "");
            closeConfirmation.setTitle(SystemProperties.getInstance().getSystemName());

            Button exitButton = (Button) closeConfirmation.getDialogPane().lookupButton(ButtonType.OK);
            exitButton.setText("Exit");

            Button cancelButton = (Button) closeConfirmation.getDialogPane().lookupButton(ButtonType.CANCEL);
            cancelButton.setText("Cancel");

            closeConfirmation.setHeaderText("Are you sure you want to exit?");
            closeConfirmation.initModality(Modality.APPLICATION_MODAL);
            closeConfirmation.initOwner(stage);

            Optional<ButtonType> closeResponse = closeConfirmation.showAndWait();
            if (!ButtonType.OK.equals(closeResponse.get())) {
                event.consume();
            } else {
                closeCurrentproject();
                GlobalConfig.getInstance().saveProps();
            }
        }
    }
}
