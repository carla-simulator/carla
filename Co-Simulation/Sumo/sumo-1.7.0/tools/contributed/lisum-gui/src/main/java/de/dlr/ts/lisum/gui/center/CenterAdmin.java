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
/// @file    CenterAdmin.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.center;

import de.dlr.ts.lisum.gui.debug.DebugConsole;
import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.logger.LogLevel;
import de.dlr.ts.lisum.gui.GlobalConfig;
import de.dlr.ts.lisum.gui.Icons;
import de.dlr.ts.lisum.gui.SystemProperties;
import javafx.event.ActionEvent;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextArea;
import javafx.scene.control.ToolBar;
import javafx.scene.control.Tooltip;
import javafx.scene.layout.AnchorPane;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class CenterAdmin {
    private final AnchorPane root = new AnchorPane();
    private TextArea textArea = new TextArea();
    private final ToolBar toolbar = new ToolBar();


    /**
     *
     */
    private final Button clearbutton = new Button("", Icons.getInstance().getIconImageView("Cancel", 16));
    {
        clearbutton.setOnAction((event) -> textArea.clear());
        clearbutton.setTooltip(new Tooltip("Clear logging area"));
    }

    /**
     *
     */
    private ComboBox<String> logLevelComboBox = new ComboBox<>();
    {
        for (LogLevel value : LogLevel.values()) {
            logLevelComboBox.getItems().add(value.name());
        }

        logLevelComboBox.setEditable(false);

        logLevelComboBox.setOnAction((ActionEvent event) -> {
            DLRLogger.setLevel(LogLevel.ALL);
            String selectedItem = logLevelComboBox.getSelectionModel().getSelectedItem();
            DLRLogger.info(this, "Setting Log level to " + selectedItem);

            GlobalConfig.getInstance().setLoggingLevel(selectedItem);
            GlobalConfig.getInstance().saveProps();

            DLRLogger.setLevel(LogLevel.parse(selectedItem.toUpperCase()));

            if (selectedItem.equals("OFF"))
                DebugConsole.getInstance().setEmptyPrintOutput();
            else if (selectedItem.equals("ALL") || selectedItem.equals("FINEST") || selectedItem.equals("FINER")) {
                System.out.println("***************************************************************************************************");
                System.out.println("***************************************************************************************************");
                System.out.println("****                             >>>  Logging level set to " + selectedItem + " <<<              ");
                System.out.println("****                                                                                    ");
                System.out.println("****                         Use this logging level only for debugging purposes!");
                System.out.println("****                  If you run SUMO at full speed, you may experience some problems");
                System.out.println("****                       like interruptions and/or a delayed execution of LiSuM");
                System.out.println("***************************************************************************************************");
                System.out.println("***************************************************************************************************");
            } else {
                DebugConsole.getInstance().setTextAreaPrintOutput();
            }
        });

        logLevelComboBox.setTooltip(new Tooltip("Logging level"));
    }

    /**
     *
     */
    public CenterAdmin() {
        clearbutton.setPrefWidth(55.);

        //textArea.setDisable(true);
        textArea.setEditable(false);
        textArea.setWrapText(false);

        AnchorPane.setTopAnchor(textArea, 42.);
        AnchorPane.setLeftAnchor(textArea, 3.);
        AnchorPane.setRightAnchor(textArea, 3.);
        AnchorPane.setBottomAnchor(textArea, 3.);

        toolbar.getItems().addAll(logLevelComboBox, clearbutton);
        AnchorPane.setTopAnchor(toolbar, 3.);
        AnchorPane.setLeftAnchor(toolbar, 3.);
        AnchorPane.setRightAnchor(toolbar, 3.);

        root.getChildren().addAll(textArea, toolbar);

        DebugConsole.getInstance().setTextArea(textArea);
        DebugConsole.getInstance().setTextAreaPrintOutput();

        //logLevelComboBox.setValue(DLRLogger.getCurrentLevel().name());
        logLevelComboBox.setValue(GlobalConfig.getInstance().getLoggingLevel());

        DLRLogger.info(this, "" + SystemProperties.getInstance().getSystemName() + " started");
    }

    /**
     *
     * @return
     */
    public Node getNode() {
        return root;
    }

}
