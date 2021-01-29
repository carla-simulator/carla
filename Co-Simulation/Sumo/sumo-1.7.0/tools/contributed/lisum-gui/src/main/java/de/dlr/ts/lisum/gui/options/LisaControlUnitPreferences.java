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
/// @file    LisaControlUnitPreferences.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.options;

import de.dlr.ts.commons.javafx.buttonspanels.OneButtonsPanel;
import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.interfaces.CityInterface;
import de.dlr.ts.lisum.interfaces.ControlUnitInterface;
import de.dlr.ts.lisum.simulation.SimulationControlUnits;
import de.dlr.ts.lisum.gui.Icons;
import de.dlr.ts.lisum.gui.SystemProperties;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.geometry.Orientation;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.Separator;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.FlowPane;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;
import javafx.stage.WindowEvent;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class LisaControlUnitPreferences {
    private static LisaControlUnitPreferences instance;

    private Scene scene;
    private Stage stage;

    AnchorPane root = new AnchorPane();
    private OneButtonsPanel applyButtonPanel = new OneButtonsPanel();

    private final Label controlUnitLabel = new Label("Control unit:");
    private ComboBox<String> controlUnitComboBox = new ComboBox<>();
    private final ObservableList<String> controlUnits = FXCollections.observableArrayList();

    private final CityInterface city = SystemProperties.getInstance().getCurrentSimulation().getCityInterface();

    protected Separator separator = new Separator(Orientation.HORIZONTAL);
    private ControlUnitInterface currentControlUnit;
    private LisaControlUnitPreferencesBody lisaOptionsBody;

    private static int lastControlUnitSelected = 0;

    private static double _x = -1;
    private static double _y = -1;

    private final CheckBox enabledCheckBox = new CheckBox();



    /**
     *
     */
    public LisaControlUnitPreferences() {
        lisaOptionsBody = new LisaControlUnitPreferencesBody(applyButtonPanel.getButton());

        enabledCheckBox.selectedProperty().
        addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            lisaOptionsBody.setEnabled(newValue);
            applyButtonPanel.getButton().setDisable(false);
        });
    }

    /**
     *
     * @param scene
     * @param stage
     * @param currentControlUnit
     * @param lisaOptionsBody
     */
    public LisaControlUnitPreferences(Scene scene, Stage stage, ControlUnitInterface currentControlUnit, LisaControlUnitPreferencesBody lisaOptionsBody) {
        this.scene = scene;
        this.stage = stage;
        this.currentControlUnit = currentControlUnit;
        this.lisaOptionsBody = lisaOptionsBody;
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
        scene = new Scene(root, 350, 350);

        stage = new Stage();
        //stage.initStyle(StageStyle.UTILITY);
        stage.initStyle(StageStyle.DECORATED);
        stage.setTitle("Control Units Management");
        stage.setScene(scene);
        stage.initModality(Modality.NONE);
        //stage.initOwner(MainProgram.instance.getScene().getWindow());
        stage.getIcons().add(SystemProperties.getInstance().getMainIcon());
        stage.setResizable(false);

        SystemProperties.getInstance().simulationOpenedProperty()
        .addListener((ObservableValue<? extends Boolean> observable,
        Boolean oldValue, Boolean newValue) -> {
            if (!newValue)
                stage.close();
        });

        if (_x != -1) {
            stage.setX(_x);
            stage.setY(_y);
        }

        stage.setOnCloseRequest((WindowEvent event) -> {
            _x = stage.getX();
            _y = stage.getY();

            instance = null;
        });

        stage.addEventHandler(KeyEvent.KEY_PRESSED, (KeyEvent event) -> {
            if (event.getCode() == KeyCode.ESCAPE)
                stage.close();

            _x = stage.getX();
            _y = stage.getY();
        });

        Node controlUnit = this.getControlUnit();
        AnchorPane.setTopAnchor(controlUnit, 20.);
        AnchorPane.setLeftAnchor(controlUnit, 20.);
        AnchorPane.setRightAnchor(controlUnit, 0.);
        root.getChildren().add(controlUnit);

        Node body = lisaOptionsBody.getNode();
        AnchorPane.setTopAnchor(body, 70.);
        AnchorPane.setLeftAnchor(body, 10.);
        AnchorPane.setRightAnchor(body, 10.);
        AnchorPane.setBottomAnchor(body, 45.);
        root.getChildren().add(body);


        /**
         *
         */
        Node node = applyButtonPanel.getNode();
        root.getChildren().add(node);
        AnchorPane.setBottomAnchor(node, 0.);
        AnchorPane.setLeftAnchor(node, 0.);
        AnchorPane.setRightAnchor(node, 0.);

        //Image mainIcon = SystemProperties.getInstance().getMainIcon();
        applyButtonPanel.getButton().setGraphic(Icons.getInstance().getIconImageView("Ok", 16));
        applyButtonPanel.getButton().setDisable(true);

        applyButtonPanel.getButton().setOnAction((ActionEvent event) -> save());

        populateControlUnitInitialValues();

        applyButtonPanel.getButton().setDisable(true);

        stage.show();
    }

    /**
     *
     */
    private void save() {
        DLRLogger.fine("Saving options of " + currentControlUnit.getFullName());
        lisaOptionsBody.save();
        applyButtonPanel.getButton().setDisable(true);
        currentControlUnit.setEnabled(this.enabledCheckBox.isSelected());
        //instance = null;
    }

    /**
     *
     */
    private void populateControlUnitInitialValues() {
        SimulationControlUnits cus = SystemProperties.getInstance()
                                     .getCurrentSimulation().getControlUnits();

        for (ControlUnitInterface _controlUnit : city.getControlUnits()) {
            String name = _controlUnit.getFullName();
            String sumoName = cus.getSumoName(name);

            if (sumoName != null) {
                name += " (" + sumoName + ")";
                _controlUnit.setEnabled(true);
            }

            controlUnits.add(name);
        }

        controlUnitComboBox.getSelectionModel().selectedItemProperty()
        .addListener((ObservableValue<? extends String> observable,
        String oldValue, String newValue) -> {
            int selectedIndex = controlUnitComboBox.getSelectionModel().getSelectedIndex();
            currentControlUnit = city.getControlUnits()[selectedIndex];
            //currentControlUnit = city.getControlUnit(newValue);
            lisaOptionsBody.update(currentControlUnit);
            applyButtonPanel.getButton().setDisable(true);
            lastControlUnitSelected = controlUnitComboBox.getSelectionModel().getSelectedIndex();

            this.lisaOptionsBody.setEnabled(currentControlUnit.isEnabled());
            this.enabledCheckBox.setSelected(currentControlUnit.isEnabled());
        });

        controlUnitComboBox.getSelectionModel().select(lastControlUnitSelected); //Initial value

        int selectedIndex = controlUnitComboBox.getSelectionModel().getSelectedIndex();
        currentControlUnit = city.getControlUnits()[selectedIndex];
        lisaOptionsBody.update(currentControlUnit);

        this.lisaOptionsBody.setEnabled(currentControlUnit.isEnabled());
        this.enabledCheckBox.setSelected(currentControlUnit.isEnabled());
    }



    /**
     *
     * @return
     */
    private Node getControlUnit() {
        controlUnitComboBox = new ComboBox<>(controlUnits);
        controlUnitComboBox.setPrefWidth(180);

        enabledCheckBox.setText("On");

        FlowPane flowPane = new FlowPane(Orientation.HORIZONTAL);
        flowPane.getChildren().addAll(controlUnitLabel, controlUnitComboBox, enabledCheckBox);
        flowPane.setHgap(10.);

        return flowPane;
    }

    /**
     *
     */
    public static void justDoIt() {
        if (instance == null) {
            LisaControlUnitPreferences lisaOption = new LisaControlUnitPreferences();
            instance = lisaOption;
            lisaOption.show();
        } else {
            instance.getStage().toFront();
        }
    }

}
