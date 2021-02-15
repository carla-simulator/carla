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
/// @file    Tools.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui;

import de.dlr.ts.commons.logger.DLRLogger;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.io.File;
import java.io.IOException;
import java.util.Optional;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonBar.ButtonData;
import javafx.scene.control.ButtonType;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class Tools {


    /**
     *
     * @param lisaRESTfulServerPath
     * @param lisaPort
     * @return
     */
    public static boolean startLisaRESTfulServer(String lisaRESTfulServerPath, int lisaPort) {
        try {
            DLRLogger.info("Tools", "Starting Lisa+ RESTful Server");

            //String lsd = GlobalConfig.getInstance().getLisaRestFulServerDir();

            if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
                ProcessBuilder pb = new ProcessBuilder("cmd.exe", "/c", "start", "java", "-jar", lisaRESTfulServerPath + File.separator + "OmlFgServer.jar");
                Process p = pb.start();
                p.waitFor();
            } else {
                //String ss = GlobalConfig.getInstance().getDefaultLisaRestFulServerDir();
                File file = new File(lisaRESTfulServerPath);
                String text = "java -jar " + file.getAbsolutePath() + File.separator + "OmlFgServer.jar\n";

                Alert alert = new Alert(AlertType.INFORMATION);
                alert.setTitle("Start Lisa+ RESTful Server");
                alert.setHeaderText("To start a Lisa+ RESTful Server copy the text below, open a new terminal and paste it there.");
                alert.setContentText(text);

                ButtonType buttonCopy = new ButtonType("Copy");
                ButtonType buttonCopyAndOpen = new ButtonType("Copy & Open terminal");
                ButtonType buttonClose = new ButtonType("Close", ButtonData.OK_DONE);

                alert.getButtonTypes().setAll(buttonCopy, buttonCopyAndOpen, buttonClose);

                Optional<ButtonType> result = alert.showAndWait();
                if (result.get() == buttonCopy) {
                    StringSelection selection = new StringSelection(text);
                    Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
                    clipboard.setContents(selection, selection);
                } else if (result.get() == buttonCopyAndOpen) {
                    StringSelection selection = new StringSelection(text);
                    Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
                    clipboard.setContents(selection, selection);

                    ProcessBuilder pb = new ProcessBuilder("gnome-terminal");
                    Process p = pb.start();
                    p.waitFor();
                }
            }
        } catch (IOException | InterruptedException ex) {
            return false;
        }

        return true;
    }

    /**
     *
     * @param time
     */
    public static void sleepi(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException ex) {
            DLRLogger.severe("Config", ex);
        }
    }

}
