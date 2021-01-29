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
/// @file    LisaControlUnitPreferencesBody.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.options;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.Constants;
import de.dlr.ts.lisum.interfaces.ControlUnitInterface;
import de.dlr.ts.lisum.interfaces.SignalProgramInterface;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ComboBox;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.VBox;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class LisaControlUnitPreferencesBody {
    private final ComboBox<String> signalProgramsComboBox = new ComboBox<>();
    private final ComboBox<String> koordiniertComboBox = new ComboBox<>();
    private final ComboBox<String> ebeneComboBox = new ComboBox<>();
    private final CheckBox vaCheckbox = new CheckBox("VA");
    private final CheckBox ivCheckbox = new CheckBox("IV");
    private final CheckBox ovCheckbox = new CheckBox("ÖV");

    private final ObservableList<String> signalPrograms = FXCollections.observableArrayList();
    private final ObservableList<String> ebeneList = FXCollections.observableArrayList();

    private ControlUnitInterface currentControlUnit;
    private Button applyButton;


    /**
     *
     * @param enabled
     */
    public void setEnabled(boolean enabled) {
        signalProgramsComboBox.setDisable(!enabled);
        koordiniertComboBox.setDisable(!enabled);
        ebeneComboBox.setDisable(!enabled);
        vaCheckbox.setDisable(!enabled);
        ivCheckbox.setDisable(!enabled);
        ovCheckbox.setDisable(!enabled);
    }

    /**
     *
     * @param button
     */
    public LisaControlUnitPreferencesBody(Button button) {
        this.applyButton = button;

        EventHandler ev = (EventHandler)(Event event) -> applyButton.setDisable(false);

        vaCheckbox.setOnAction(ev);
        ivCheckbox.setOnAction(ev);
        ovCheckbox.setOnAction(ev);

        koordiniertComboBox.setOnAction((ActionEvent event) -> {
            applyButton.setDisable(false);
        });

        ebeneComboBox.setOnAction((ActionEvent event) -> {
            applyButton.setDisable(false);
        });

        signalProgramsComboBox.getSelectionModel().selectedItemProperty()
        .addListener((ObservableValue<? extends String> observable, String oldValue, String newValue) -> {
            updateCheckBoxesValues();
            enableDisableCheckBoxes();
            applyButton.setDisable(false);
        });

        /**
         *
         */
        for (String ebeneItem : Constants.ebeneItems) {
            ebeneList.add(ebeneItem);
        }

        /*
        ebeneList.add("Serviceeingriff");
        ebeneList.add("Bediener, lokal");
        ebeneList.add("Programm fix, lokal");
        ebeneList.add("Handrastbetrieb, lokal");
        ebeneList.add("Schaltuhr, lokal");
        ebeneList.add("Zentrale");
        ebeneList.add("Zentrale 2");
        ebeneList.add("Sondereingriff-Zentrale");
        ebeneList.add("Sondereingriff");
        ebeneList.add("Logik");
        ebeneList.add("Fehler Gerät");
        ebeneList.add("Signalsicherung");
        ebeneList.add("System");
        */
    }

    /**
     *
     * @param controlunit
     */
    public void update(ControlUnitInterface controlunit) {
        this.currentControlUnit = controlunit;

        updateComboBoxsValues();
        updateCheckBoxesValues();
        enableDisableCheckBoxes();
    }

    /**
     *
     */
    public void save() {
        int __selectedIndex = signalProgramsComboBox.getSelectionModel().getSelectedIndex();
        currentControlUnit.setCurrentSignalProgram(__selectedIndex);

        currentControlUnit.setVA(vaCheckbox.isSelected());
        currentControlUnit.setIV(ivCheckbox.isSelected());
        currentControlUnit.setOV(ovCheckbox.isSelected());

        currentControlUnit.setEbene(ebeneComboBox.getSelectionModel().selectedIndexProperty().get());
        currentControlUnit.storePersistent();


        StringBuilder sb = new StringBuilder();
        int selectedIndex = signalProgramsComboBox.getSelectionModel().getSelectedIndex();
        sb.append("Signal program=").append(signalProgramsComboBox.getValue()).append(" (").append(selectedIndex).append("), ");
        int selectedIndex1 = ebeneComboBox.getSelectionModel().getSelectedIndex();
        sb.append("Ebene=").append(ebeneComboBox.getValue()).append(" (").append(selectedIndex1).append("), ");
        sb.append("VA=").append(vaCheckbox.isSelected()).append(", ");
        sb.append("IV=").append(ivCheckbox.isSelected()).append(", ");
        sb.append("OV=").append(ovCheckbox.isSelected());
        DLRLogger.fine(this, sb.toString());
    }

    /**
     *
     */
    private void updateComboBoxsValues() {
        /**
         * Signal programs
         */
        signalPrograms.clear();

        //signalPrograms.add("Off");
        for (SignalProgramInterface signalProgram : currentControlUnit.getSignalPrograms()) {
            signalPrograms.add(signalProgram.getName());
        }

        if (currentControlUnit.getCurrentSignalProgram() == null) {
            signalProgramsComboBox.setValue("Off");
        } else {
            signalProgramsComboBox.setValue(currentControlUnit.getCurrentSignalProgram().getName());
        }

        /**
         *
         */
        ebeneComboBox.getSelectionModel().select(currentControlUnit.getEbene());
    }

    /**
     *
     */
    private void enableDisableCheckBoxes() {

        if (signalProgramsComboBox.getValue() != null &&
                signalProgramsComboBox.getValue().equalsIgnoreCase("Off")) {
            this.koordiniertComboBox.setDisable(true);
            this.ebeneComboBox.setDisable(true);
            this.vaCheckbox.setDisable(true);
            this.ivCheckbox.setDisable(true);
            this.ovCheckbox.setDisable(true);
        } else {
            this.koordiniertComboBox.setDisable(true);
            this.ebeneComboBox.setDisable(false);

            this.vaCheckbox.setDisable(false);
            this.ivCheckbox.setDisable(false);
            this.ovCheckbox.setDisable(false);
        }
    }

    /**
     *
     */
    private void updateCheckBoxesValues() {

        if (signalProgramsComboBox.getValue() != null)
            //&& signalProgramsComboBox.getValue().equalsIgnoreCase("Off"))
        {
            boolean va = currentControlUnit.isVA();
            this.vaCheckbox.setSelected(va);

            boolean iv = currentControlUnit.isIV();
            this.ivCheckbox.setSelected(iv);

            boolean ov = currentControlUnit.isOV();
            this.ovCheckbox.setSelected(ov);
        }
    }

    /**
     *
     * @return
     */
    public Node getNode() {
        AnchorPane bodyAnchorPane = new AnchorPane();
        bodyAnchorPane.setStyle("-fx-background-color:white; -fx-border-color: DimGrey; -fx-border-radius: 2;");

        signalProgramsComboBox.setItems(signalPrograms);
        AnchorPane.setTopAnchor(signalProgramsComboBox, 10.);
        AnchorPane.setLeftAnchor(signalProgramsComboBox, 5.);
        AnchorPane.setRightAnchor(signalProgramsComboBox, 5.);
        bodyAnchorPane.getChildren().add(signalProgramsComboBox);

        ObservableList<String> koordiniertList = FXCollections.observableArrayList("Coordinated", "Uncoordinated");
        koordiniertComboBox.setItems(koordiniertList);
        koordiniertComboBox.setPrefWidth(150.);
        koordiniertComboBox.setDisable(true);
        AnchorPane.setTopAnchor(koordiniertComboBox, 43.);
        AnchorPane.setLeftAnchor(koordiniertComboBox, 5.);
        //AnchorPane.setRightAnchor(koordiniertComboBox, 50.);
        bodyAnchorPane.getChildren().add(koordiniertComboBox);
        koordiniertComboBox.getSelectionModel().select(0);

        ebeneComboBox.setItems(ebeneList);
        ebeneComboBox.setPrefWidth(150.);
        AnchorPane.setTopAnchor(ebeneComboBox, 43.);
        //AnchorPane.setLeftAnchor(zentraleComboBox, 50.);
        AnchorPane.setRightAnchor(ebeneComboBox, 5.);
        bodyAnchorPane.getChildren().add(ebeneComboBox);
        ebeneComboBox.getSelectionModel().select(0);

        VBox vbox = new VBox(vaCheckbox, ivCheckbox, ovCheckbox);
        vbox.setSpacing(10.);
        AnchorPane.setTopAnchor(vbox, 115.);
        AnchorPane.setLeftAnchor(vbox, 20.);
        AnchorPane.setRightAnchor(vbox, 0.);
        bodyAnchorPane.getChildren().add(vbox);

        return bodyAnchorPane;
    }
}
