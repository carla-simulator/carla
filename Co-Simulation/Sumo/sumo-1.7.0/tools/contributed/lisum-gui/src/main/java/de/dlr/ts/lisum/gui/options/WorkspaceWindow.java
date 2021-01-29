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
/// @file    WorkspaceWindow.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.options;

import de.dlr.ts.commons.javafx.buttons.tools.FileChooserCombo;
import de.dlr.ts.commons.javafx.buttonspanels.OkCancelButtonsPanel;
import de.dlr.ts.commons.javafx.messages.ConfirmationMessage;
import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.gui.SystemProperties;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Optional;
import java.util.Properties;
import javafx.event.ActionEvent;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Label;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;
import javafx.stage.Window;
import org.apache.commons.io.FileUtils;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class WorkspaceWindow {
    private Scene scene;
    private Stage stage;

    private final AnchorPane root = new AnchorPane();
    private final OkCancelButtonsPanel okCancelButtons = new OkCancelButtonsPanel();

    private final String initialDirName = System.getProperty("user.home") + File.separator + "LiSuMWorkspace";
    private FileChooserCombo workspaceChooser;

    private static File workspace = null;
    private String storedWorkspace;

    private final Properties props = new Properties();
    private static final String PERSISTENCE_FILE = "LiSuM_Workspace.properties";
    private static final String WORKSPACE_KEY = "workspace";
    private File metadata;



    /**
     *
     */
    public WorkspaceWindow() {
    }

    /**
     *
     */
    public void readFromFile() {
        String tmpdir = System.getProperty("java.io.tmpdir") + File.separator;
        File persFile = new File(tmpdir + PERSISTENCE_FILE);

        DLRLogger.finest(this, "Reading workspace path from " + persFile.getAbsolutePath());

        if (persFile.exists()) {
            try {
                props.load(new FileInputStream(persFile));
                storedWorkspace = props.getProperty(WORKSPACE_KEY);

                if (storedWorkspace != null && !storedWorkspace.isEmpty()) {
                    workspace = new File(storedWorkspace);
                    if (!workspace.exists()) {
                        workspace = null;
                    }
                } else {
                    storedWorkspace = initialDirName;
                }
            } catch (IOException ex) {
                ex.printStackTrace(System.out);
            }
        }
    }

    /**
     *
     */
    private void saveAndClose() {

        if (workspaceChooser.getTextField().getText().isEmpty()) {
            workspace = null;
        } else {
            workspace = new File(workspaceChooser.getTextField().getText());
            boolean createPath = createPath();

            if (!createPath) {
                return;
            }
        }

        try {
            if (workspace == null) {
                props.remove(WORKSPACE_KEY);
            } else {
                props.put(WORKSPACE_KEY, workspace.getAbsolutePath());
            }

            String tmpdir = System.getProperty("java.io.tmpdir") + File.separator;

            props.store(new FileOutputStream(tmpdir + PERSISTENCE_FILE), "");

            stage.close();
        } catch (IOException ex) {
            ex.printStackTrace(System.out);
        }
    }

    /**
     *
     */
    private boolean createPath() {
        boolean ok = true;

        if (!workspace.exists()) {
            ConfirmationMessage cm = new ConfirmationMessage("Proceed?");
            cm.setHeader("The selected workspace does not exist and will be created.");
            Optional<ButtonType> result = cm.showAndWait();

            if (result.get() == ButtonType.OK) {
                workspace.mkdirs();
            } else {
                ok = false;
            }
        }

        if (ok) {
            metadata = new File(workspace.getAbsolutePath() + File.separator + ".metadata");

            if (!metadata.exists()) {
                metadata.mkdirs();

                /**
                 * Copy sample simulation to new workspace
                 */
                File demoOrigin = new File("simulations" + File.separator + "sampleSimulation");
                File demoDest = new File(workspace.getAbsolutePath() + File.separator + "sampleSimulation");

                try {
                    if (!demoDest.exists()) {
                        FileUtils.copyDirectory(demoOrigin, demoDest);
                    }
                } catch (IOException ex) {
                    //ex.printStackTrace();
                }
            }
        }

        return ok;
    }

    /**
     *
     * @param owner
     */
    public void show(Window owner) {
        okCancelButtons.getOkButton().setOnAction((ActionEvent event) -> saveAndClose());
        okCancelButtons.getCancelButton().setOnAction((ActionEvent event) -> stage.close());
        //okButton.setDisable(true);

        scene = new Scene(root, 670, 180);
        stage = new Stage();
        stage.initStyle(StageStyle.UTILITY);
        stage.setTitle(SystemProperties.getInstance().getSystemName());
        stage.setScene(scene);
        stage.initModality(Modality.WINDOW_MODAL);
        //stage.initOwner(MainProgram.getInstance().getScene().getWindow());
        stage.initOwner(owner);
        stage.getIcons().add(SystemProperties.getInstance().getMainIcon());
        stage.setResizable(false);

        stage.addEventHandler(KeyEvent.KEY_PRESSED, (KeyEvent event) -> {
            if (event.getCode() == KeyCode.ESCAPE)
                stage.close();
        });

        /**
         *
         */
        AnchorPane pane = new AnchorPane();
        AnchorPane.setTopAnchor(pane, 0.);
        AnchorPane.setLeftAnchor(pane, 0.);
        AnchorPane.setRightAnchor(pane, 0.);
        pane.setPrefHeight(60.);
        pane.setStyle("-fx-background-color: #FFFFFF;");


        Label label = new Label("Select a directory as workspace");
        label.setFont(Font.font("Arial", FontWeight.BOLD, 14));
        AnchorPane.setTopAnchor(label, 9.);
        AnchorPane.setLeftAnchor(label, 8.);

        Label label2 = new Label("This is where you store your simulations and where LiSuM saves the system preferences.");
        label2.setFont(Font.font(12));
        AnchorPane.setTopAnchor(label2, 32.);
        AnchorPane.setLeftAnchor(label2, 16.);

        pane.getChildren().addAll(label, label2);

        /**
         *
         */
        File init;
        if (workspace == null) {
            init = new File(".");
        } else {
            String parent = workspace.getParent();
            if (parent != null && !parent.isEmpty()) {
                init = new File(parent);
            } else {
                init = workspace;
            }
        }

        workspaceChooser = new FileChooserCombo("Workspace:", new File(System.getProperty("user.home")), stage);
        workspaceChooser.enableDefaultButton(false);
        workspaceChooser.setLabelWidth(80);
        workspaceChooser.setTextFieldWidth(440.);
        workspaceChooser.getBrowseButton().setPrefWidth(90.);
        workspaceChooser.setDirectoryChooser(true);

        if (workspace == null) {
            workspaceChooser.getTextField().setText(initialDirName);
        } else {
            workspaceChooser.getTextField().setText(workspace.getAbsolutePath());
        }

        Node workspaceChooserNode = workspaceChooser.getNode();
        AnchorPane.setTopAnchor(workspaceChooserNode, 90.);
        AnchorPane.setLeftAnchor(workspaceChooserNode, 14.);

        /**
         *
         */
        Node okCancelNode = okCancelButtons.getNode();
        AnchorPane.setBottomAnchor(okCancelNode, 0.);
        AnchorPane.setLeftAnchor(okCancelNode, 0.);
        AnchorPane.setRightAnchor(okCancelNode, 0.);

        root.getChildren().addAll(okCancelNode, workspaceChooserNode, pane);

        stage.showAndWait();
    }

    /**
     *
     * @return
     */
    public static File getWorkspace() {
        return workspace;
    }
}
