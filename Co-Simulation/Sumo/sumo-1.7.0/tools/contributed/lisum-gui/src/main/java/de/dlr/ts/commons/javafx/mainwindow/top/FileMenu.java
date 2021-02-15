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
/// @file    FileMenu.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.top;

import java.io.File;
import javafx.event.ActionEvent;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.control.SeparatorMenuItem;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCodeCombination;
import javafx.scene.input.KeyCombination;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class FileMenu extends Menu {
    private MenuItem openMenuItem = new MenuItem("Open...");
    private MenuItem closeMenuItem = new MenuItem("Close");
    private MenuItem emptyRecentFilesListMenuItem = new MenuItem("Empty Recent Files List");
    private MenuItem exitSystemMenuItem = new MenuItem("Exit");
    private RecentFiles recentFiles = new RecentFiles();


    /**
     *
     * @param file
     */
    public final static void setRecentFilesPersistenceFile(File file) {
        RecentFiles.setLofsFile(file);
    }

    /**
     *
     */
    public FileMenu() {
        this.setText("_File");
        this.setAccelerator(KeyCombination.keyCombination("SHORTCUT+F"));

        openMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.O, KeyCombination.CONTROL_DOWN));
        closeMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.W, KeyCombination.CONTROL_DOWN));
        exitSystemMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.F4, KeyCombination.ALT_DOWN));

        emptyRecentFilesListMenuItem.setOnAction((ActionEvent event) -> {
            recentFiles.clear();
            putItems();
        });

        putItems();
    }

    /**
     *
     * @return
     */
    public MenuItem[] getLastOpenFiles() {
        return recentFiles.getAll();
    }

    /**
     *
     */
    public void close() {
        recentFiles.close();
    }

    /**
     *
     * @param listener
     */
    public void addLastOpenedFilesListener(RecentFilesListener listener) {
        recentFiles.addListener(listener);
    }

    /**
     *
     * @param enabled
     */
    public void setLastOpenFilesEnabled(boolean enabled) {
        recentFiles.setEnabled(enabled);
    }

    /**
     *
     * @param filePath
     */
    public void addLastOpen(String filePath) {
        recentFiles.add(filePath);
        putItems();
    }

    /**
     *
     */
    private void putItems() {
        this.getItems().clear();

        if (recentFiles.size() == 0) {
            this.getItems().addAll(openMenuItem, closeMenuItem, new SeparatorMenuItem(), exitSystemMenuItem);
        } else {
            this.getItems().addAll(openMenuItem, closeMenuItem, new SeparatorMenuItem());

            for (int i = 0; i < recentFiles.size(); i++) {
                this.getItems().add(recentFiles.get(i));
            }

            this.getItems().add(new SeparatorMenuItem());
            this.getItems().add(emptyRecentFilesListMenuItem);
            this.getItems().add(new SeparatorMenuItem());
            this.getItems().add(exitSystemMenuItem);
        }
    }

    /**
     *
     * @return
     */
    public MenuItem getOpenMenuItem() {
        return openMenuItem;
    }

    /**
     *
     * @return
     */
    public MenuItem getExitSystemMenuItem() {
        return exitSystemMenuItem;
    }

    /**
     *
     * @return
     */
    public MenuItem getCloseMenuItem() {
        return closeMenuItem;
    }

    public static interface RecentFilesListener {
        void event(String filePath);
    }
}
