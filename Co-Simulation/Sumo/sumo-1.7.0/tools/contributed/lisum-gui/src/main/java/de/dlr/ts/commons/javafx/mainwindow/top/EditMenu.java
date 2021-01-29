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
/// @file    EditMenu.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.top;

import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCodeCombination;
import javafx.scene.input.KeyCombination;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class EditMenu extends Menu {
    private MenuItem cutMenuItem;
    private MenuItem copyMenuItem;
    private MenuItem pasteMenuItem;


    /**
     *
     */
    EditMenu() {
        this.setText("_Edit");
        this.setAccelerator(KeyCombination.keyCombination("SHORTCUT+E"));

        cutMenuItem = new MenuItem("Cut");
        cutMenuItem.setDisable(true);
        cutMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.X, KeyCombination.CONTROL_DOWN));

        copyMenuItem = new MenuItem("Copy");
        copyMenuItem.setDisable(true);
        copyMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.C, KeyCombination.CONTROL_DOWN));

        pasteMenuItem = new MenuItem("Paste");
        pasteMenuItem.setDisable(true);
        pasteMenuItem.setAccelerator(new KeyCodeCombination(KeyCode.V, KeyCombination.CONTROL_DOWN));

        getItems().addAll(cutMenuItem, copyMenuItem, pasteMenuItem);
    }

    public MenuItem getCopyMenuItem() {
        return copyMenuItem;
    }

    public MenuItem getCutMenuItem() {
        return cutMenuItem;
    }

    public MenuItem getPasteMenuItem() {
        return pasteMenuItem;
    }

}
