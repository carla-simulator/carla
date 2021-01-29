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
/// @file    SystemPreferencesWindow.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.options;

import de.dlr.ts.commons.javafx.buttons.tools.FileChooserCombo;
import de.dlr.ts.commons.javafx.buttonspanels.OkCancelButtonsPanel;
import de.dlr.ts.commons.javafx.messages.ConfirmationMessage;
import de.dlr.ts.commons.logger.LogLevel;
import de.dlr.ts.lisum.Constants;
import de.dlr.ts.lisum.gui.GlobalConfig;
import de.dlr.ts.lisum.gui.Actions;
import de.dlr.ts.lisum.gui.MainProgram;
import de.dlr.ts.lisum.gui.SystemProperties;
import java.io.File;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.Tooltip;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class SystemPreferencesWindow {
    private final AnchorPane root = new AnchorPane();
    private final OkCancelButtonsPanel okCancelButtons = new OkCancelButtonsPanel();
    private final Button okButton = okCancelButtons.getOkButton();
    private final ChangeListener<String> activateOkButton = (ObservableValue<? extends String> observable, String oldValue, String newValue) -> {
        okButton.setDisable(false);
    };

    private final Scene scene;
    private Stage stage;

    private final TabPane mainTabPane = new TabPane();
    private FileChooserCombo sumoExecFileChooser;
    private FileChooserCombo lisaServerFileChooser;
    private FileChooserCombo textEditorFileChooserCombo;
    private FileChooserCombo filesExplorerFileChooserCombo;
    private FileChooserCombo workspaceFileChooserCombo;

    private final Tab sumoTab = new Tab("Sumo");
    private final Tab lisaTab = new Tab("Lisa+");
    private final Tab generalTab = new Tab("General");
    //private FileChooser.ExtensionFilter filter = new FileChooser.ExtensionFilter("Exe files (*.exe)", "*.exe");


    /**
     *
     */
    public SystemPreferencesWindow() {
        okCancelButtons.getOkButton().setOnAction((ActionEvent event) -> saveAndClose());
        okCancelButtons.getCancelButton().setOnAction((ActionEvent event) -> stage.close());
        okButton.setDisable(true);

        scene = new Scene(root, 600, 450);

        /**
         *
         */
        stage = new Stage();
        stage.initStyle(StageStyle.UTILITY);
        stage.setTitle("System Preferencies");
        stage.setScene(scene);
        stage.initModality(Modality.WINDOW_MODAL);
        stage.initOwner(MainProgram.getInstance().getScene().getWindow());
        stage.getIcons().add(SystemProperties.getInstance().getMainIcon());
        stage.setResizable(false);

        stage.addEventHandler(KeyEvent.KEY_PRESSED, (KeyEvent event) -> {
            if (event.getCode() == KeyCode.ESCAPE)
                stage.close();
        });

        /**
         *
         */
        AnchorPane.setTopAnchor(mainTabPane, 5.);
        AnchorPane.setLeftAnchor(mainTabPane, 5.);
        AnchorPane.setRightAnchor(mainTabPane, 5.);
        AnchorPane.setBottomAnchor(mainTabPane, okCancelButtons.getHeight() + 10.);

        generalTab();
        sumoTab();
        lisaTab();
    }

    /**
     *
     */
    private void save() {
        GlobalConfig.getInstance().setTextEditor(textEditorFileChooserCombo.getTextField().getText());
        GlobalConfig.getInstance().setSumoExec(sumoExecFileChooser.getTextField().getText());
        GlobalConfig.getInstance().setLoggingLevel(logLevelComboBox.getValue());
        GlobalConfig.getInstance().setLisaRestFulServerDir(lisaServerFileChooser.getTextField().getText());
        GlobalConfig.getInstance().setWorkspace(workspaceFileChooserCombo.getTextField().getText());
        GlobalConfig.getInstance().setFilesExplorer(filesExplorerFileChooserCombo.getTextField().getText());
        GlobalConfig.getInstance().saveProps();

        okButton.setDisable(true);
    }

    /**
     *
     */
    private void saveAndClose() {
        save();
        stage.close();
    }

    /**
     *
     */
    private void generalTab() {
        AnchorPane generalAnchorPane = new AnchorPane();
        generalTab.setContent(generalAnchorPane);
        generalTab.setClosable(false);


        Label logLevelLabel = new Label("Default log level:");
        AnchorPane.setTopAnchor(logLevelLabel, 24.);
        AnchorPane.setLeftAnchor(logLevelLabel, 5.);
        logLevelLabel.setFont(Font.font(10));

        AnchorPane.setTopAnchor(logLevelComboBox, 20.);
        AnchorPane.setLeftAnchor(logLevelComboBox, 105.);
        logLevelComboBox.setValue(GlobalConfig.getInstance().getLoggingLevel());
        logLevelComboBox.valueProperty().addListener((ObservableValue<? extends String> observable, String oldValue, String newValue) -> {
            okButton.setDisable(false);
        });

        /**
         *
         */
        textEditorFileChooserCombo = new FileChooserCombo("Text Editor:", new File("."), stage.getOwner());
        Node textEditorNode = textEditorFileChooserCombo.getNode();
        AnchorPane.setTopAnchor(textEditorNode, 55.);
        AnchorPane.setLeftAnchor(textEditorNode, 5.);
        textEditorFileChooserCombo.setDefaultValue(GlobalConfig.getInstance().getDefaultTextEditor());
        textEditorFileChooserCombo.getTextField().setText(GlobalConfig.getInstance().getTextEditor());
        textEditorFileChooserCombo.getTextField().textProperty().addListener(activateOkButton);
        //textEditorFileChooserCombo.getFileChooser().setSelectedExtensionFilter(filter);
        textEditorFileChooserCombo.setTextFieldWidth(330.);
        textEditorFileChooserCombo.setFontSize(10);


        /**
         *
         */
        filesExplorerFileChooserCombo = new FileChooserCombo("Files explorer: ", new File("."), stage.getOwner());
        Node filesExplorerNode = filesExplorerFileChooserCombo.getNode();
        AnchorPane.setTopAnchor(filesExplorerNode, 88.);
        AnchorPane.setLeftAnchor(filesExplorerNode, 5.);
        filesExplorerFileChooserCombo.setDefaultValue(GlobalConfig.getInstance().getDefaultFilesExplorer());
        filesExplorerFileChooserCombo.getTextField().setText(GlobalConfig.getInstance().getFilesExplorer());
        filesExplorerFileChooserCombo.getTextField().textProperty().addListener(activateOkButton);
        filesExplorerFileChooserCombo.setTextFieldWidth(330.);
        filesExplorerFileChooserCombo.setFontSize(10);


        /**
         *
         */
        workspaceFileChooserCombo = new FileChooserCombo("Workspace:",
                new File(GlobalConfig.getInstance().getWorkspace()), stage.getOwner());
        Node simuNode = workspaceFileChooserCombo.getNode();
        AnchorPane.setTopAnchor(simuNode, 121.);
        AnchorPane.setLeftAnchor(simuNode, 5.);
        workspaceFileChooserCombo.getTextField().setText(WorkspaceWindow.getWorkspace().getAbsolutePath());
        workspaceFileChooserCombo.getTextField().setDisable(true);
        workspaceFileChooserCombo.getTextField().setStyle("-fx-opacity: 0.8;");
        workspaceFileChooserCombo.setTextFieldWidth(330.);
        workspaceFileChooserCombo.enableDefaultButton(false);
        workspaceFileChooserCombo.getBrowseButton().setText("Switch");
        workspaceFileChooserCombo.setFontSize(10);
        workspaceFileChooserCombo.getBrowseButton().setOnAction((ActionEvent event) -> {
            if (!okButton.isDisabled()) {
                ConfirmationMessage cm = new ConfirmationMessage("");
                cm.setHeader("Save changes before proceeding?");
                if (cm.showAndWait().get() == ButtonType.OK) {
                    save();
                }
            }

            boolean switchWorkspace = Actions.getInstance().switchWorkspace();

            if (switchWorkspace)
                stage.close();
        });


        /**
         *
         */
        Button switchWorkspaceButton = new Button("Switch workspace");
        AnchorPane.setBottomAnchor(switchWorkspaceButton, 5.);
        AnchorPane.setRightAnchor(switchWorkspaceButton, 5.);

        switchWorkspaceButton.setOnAction((ActionEvent event) -> {
            boolean switchWorkspace = Actions.getInstance().switchWorkspace();

            if (switchWorkspace)
                stage.close();
        });

        generalAnchorPane.getChildren().addAll(/*logLevelLabel, logLevelComboBox,*/ textEditorNode,
                filesExplorerNode, simuNode);
        mainTabPane.getTabs().addAll(generalTab);
    }

    /**
     *
     */
    private void sumoTab() {
        AnchorPane sumoAnchorPane = new AnchorPane();
        sumoTab.setContent(sumoAnchorPane);
        sumoTab.setClosable(false);

        /**
         *
         */
        sumoExecFileChooser = new FileChooserCombo("SumoGUI exec:", new File("."), stage.getOwner());
        Node sumoExecNode = sumoExecFileChooser.getNode();
        AnchorPane.setTopAnchor(sumoExecNode, 20.);
        AnchorPane.setLeftAnchor(sumoExecNode, 5.);
        sumoExecFileChooser.setFontSize(10);

        sumoAnchorPane.getChildren().add(sumoExecNode);
        sumoExecFileChooser.setDefaultValue("");
        sumoExecFileChooser.getTextField().setText(GlobalConfig.getInstance().getSumoExec());
        sumoExecFileChooser.getTextField().textProperty().addListener(activateOkButton);
        //sumoExecFileChooser.getFileChooser().setSelectedExtensionFilter(filter);
        sumoExecFileChooser.enableDefaultButton(false);
        sumoExecFileChooser.setTextFieldWidth(330.);

        mainTabPane.getTabs().addAll(sumoTab);
    }

    /**
         *
         */
    private void lisaTab() {
        AnchorPane lisaAnchorPane = new AnchorPane();
        lisaTab.setContent(lisaAnchorPane);
        lisaTab.setClosable(false);

        lisaServerFileChooser = new FileChooserCombo("LISA+ RestFUL Server folder:", new File("."), stage.getOwner());
        Node node = lisaServerFileChooser.getNode();
        AnchorPane.setTopAnchor(node, 20.);
        AnchorPane.setLeftAnchor(node, 5.);
        lisaServerFileChooser.setDefaultValue(GlobalConfig.getInstance().getDefaultLisaRestFulServerDir());
        lisaServerFileChooser.getTextField().setText(GlobalConfig.getInstance().getLisaRESTfulServerPath());
        lisaServerFileChooser.getTextField().textProperty().addListener(activateOkButton);
        lisaServerFileChooser.setLabelWidth(165);
        lisaServerFileChooser.setDirectoryChooser(true);
        lisaServerFileChooser.setFontSize(10);

        Label warningLabel = new Label("System restart required!");
        warningLabel.setTextFill(Color.RED);
        AnchorPane.setTopAnchor(warningLabel, 50.);
        AnchorPane.setLeftAnchor(warningLabel, 180.);
        warningLabel.setVisible(false);

        lisaServerFileChooser.getTextField().textProperty().addListener((ObservableValue<? extends String> observable, String oldValue, String newValue) -> {
            String lisaRestFulServerDir = GlobalConfig.getInstance().getLisaRESTfulServerPath();
            if (!lisaServerFileChooser.getTextField().getText().equals(lisaRestFulServerDir))
                warningLabel.setVisible(true);
            else {
                warningLabel.setVisible(false);
            }
        });

        lisaAnchorPane.getChildren().addAll(node, warningLabel);

        mainTabPane.getTabs().addAll(lisaTab);
    }

    /**
     *
     * @param tabIndex
     */
    public void show(int tabIndex) {
        Node okCancelNode = okCancelButtons.getNode();
        AnchorPane.setBottomAnchor(okCancelNode, 0.);
        AnchorPane.setLeftAnchor(okCancelNode, 0.);
        AnchorPane.setRightAnchor(okCancelNode, 0.);

        root.getChildren().addAll(okCancelNode, mainTabPane);

        mainTabPane.getSelectionModel().select(tabIndex);

        stage.show();
    }

    /**
     *
     */
    private final ComboBox<String> logLevelComboBox = new ComboBox<>();
    {
        for (LogLevel value : LogLevel.values()) {
            logLevelComboBox.getItems().add(value.name());
        }

        logLevelComboBox.setEditable(false);
        logLevelComboBox.setValue(Constants.DEFAULT_LOG_LEVEL);

        logLevelComboBox.setTooltip(new Tooltip("Log level"));
    }
}
