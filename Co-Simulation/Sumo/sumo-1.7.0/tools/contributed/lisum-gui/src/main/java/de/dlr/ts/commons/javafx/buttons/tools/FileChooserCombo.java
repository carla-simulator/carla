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
/// @file    FileChooserCombo.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.buttons.tools;

import de.dlr.ts.commons.javafx.buttons.CustomButton;
import java.io.File;
import javafx.event.ActionEvent;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.AnchorPane;
import javafx.scene.text.Font;
import javafx.stage.DirectoryChooser;
import javafx.stage.FileChooser;
import javafx.stage.Window;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class FileChooserCombo {
    private AnchorPane root = new AnchorPane();

    private Label label = new Label();
    private TextField textField = new TextField();
    private CustomButton browseButton = new CustomButton("Browse...");
    private CustomButton defaultButton = new CustomButton("Default");

    private File selectedFile = null;
    private FileChooser fileChooser = new FileChooser();
    private DirectoryChooser directoryChooser = new DirectoryChooser();

    private static File initialDirectory = null;
    private boolean defaultButtonEnabled = true;

    private double labelWidth = 95d;
    private double textFieldWidth = 260d;
    private Window ownerWindow;



    /**
     *
     * @param fontSize
     */
    public void setFontSize(double fontSize) {
        label.setFont(Font.font(fontSize));
        textField.setFont(Font.font(fontSize));
        browseButton.setFont(Font.font(fontSize));
        defaultButton.setFont(Font.font(fontSize));
    }

    /**
     *
     * @param labelText
     * @param initialDirectory
     * @param ownerWindow
     */
    public FileChooserCombo(String labelText, final File initialDirectory, Window ownerWindow) {
        FileChooserCombo.initialDirectory = initialDirectory;

        label.setText(labelText);
        setLayout();

        browseButton.setOnAction((ActionEvent event) -> {
            fileChooser.setInitialDirectory(FileChooserCombo.initialDirectory);
            selectedFile = fileChooser.showOpenDialog(ownerWindow);

            if (selectedFile != null) {
                textField.setText(selectedFile.getAbsolutePath());
                FileChooserCombo.initialDirectory = selectedFile.getParentFile();
            }
        });
    }

    /**
     *
     * @return
     */
    public double getTextFieldWidth() {
        return textFieldWidth;
    }

    /**
     *
     * @return
     */
    public double getLabelWidth() {
        return labelWidth;
    }

    /**
     *
     * @param width
     */
    public void setLabelWidth(double width) {
        this.labelWidth = width;
        setLayout();
    }

    /**
     *
     * @param textFieldWidth
     */
    public void setTextFieldWidth(double textFieldWidth) {
        this.textFieldWidth = textFieldWidth;
        setLayout();
    }

    /**
     *
     * @param enable
     */
    public void enableDefaultButton(boolean enable) {
        defaultButtonEnabled = enable;
        setLayout();
    }

    /**
     *
     */
    private void setLayout() {
        root.getChildren().clear();

        double width = labelWidth + 5d + textFieldWidth + 5d + 65d;

        root.setPrefSize(width, 25.);

        browseButton.setPrefWidth(65.);
        AnchorPane.setTopAnchor(browseButton, 0.);
        AnchorPane.setLeftAnchor(browseButton, labelWidth + 5d + textFieldWidth + 5d);
        AnchorPane.setBottomAnchor(browseButton, 0.);

        textField.setPrefWidth(textFieldWidth);
        AnchorPane.setTopAnchor(textField, 0.);
        AnchorPane.setLeftAnchor(textField, labelWidth + 5d);
        //AnchorPane.setRightAnchor(textField, 140.);
        AnchorPane.setBottomAnchor(textField, 0.);

        label.setPrefWidth(labelWidth);
        AnchorPane.setTopAnchor(label, 0.);
        AnchorPane.setLeftAnchor(label, 0.);
        AnchorPane.setBottomAnchor(label, 0.);

        root.getChildren().addAll(label, textField, browseButton);

        if (defaultButtonEnabled) {
            defaultButton.setPrefWidth(65.);
            AnchorPane.setTopAnchor(defaultButton, 0.);
            AnchorPane.setLeftAnchor(defaultButton, width + 5d);
            AnchorPane.setBottomAnchor(defaultButton, 0.);

            root.getChildren().add(defaultButton);

            width += 5d + 65d;
        }
    }

    /**
     *
     * @param directory
     */
    public void setDirectoryChooser(boolean directory) {
        if (directory) {
            browseButton.setOnAction((ActionEvent event) -> {
                fileChooser.setInitialDirectory(FileChooserCombo.initialDirectory);
                directoryChooser.setInitialDirectory(initialDirectory);
                selectedFile = directoryChooser.showDialog(ownerWindow);

                if (selectedFile != null) {
                    textField.setText(selectedFile.getAbsolutePath());
                    FileChooserCombo.initialDirectory = selectedFile.getParentFile();
                }
            });
        }
    }

    /**
     *
     * @param defaultValue
     */
    public void setDefaultValue(String defaultValue) {
        defaultButton.setOnAction((ActionEvent event) -> textField.setText(defaultValue));
    }

    /**
     *
     * @return
     */
    public Node getNode() {
        return root;
    }

    /**
     *
     * @param dir
     */
    public void setInitialDirectory(File dir) {
        initialDirectory = dir;
    }

    /**
     *
     * @return
     */
    public File getSelectedFile() {
        return selectedFile;
    }

    /**
     *
     * @return
     */
    public CustomButton getBrowseButton() {
        return browseButton;
    }

    /**
     *
     * @return
     */
    public CustomButton getDefaultButton() {
        return defaultButton;
    }

    /**
     *
     * @return
     */
    public TextField getTextField() {
        return textField;
    }

    /**
     *
     * @return
     */
    public FileChooser getFileChooser() {
        return fileChooser;
    }

}
