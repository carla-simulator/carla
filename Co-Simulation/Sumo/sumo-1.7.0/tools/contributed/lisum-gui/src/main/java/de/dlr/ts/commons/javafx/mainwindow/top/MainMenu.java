/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MainMenu.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.top;

import java.util.ArrayList;
import java.util.List;
import javafx.scene.Node;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuBar;
import javafx.scene.input.KeyCombination;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MainMenu {
    private List<Menu> menus = new ArrayList<>();
    private MenuBar menuBar = new MenuBar();        //default height 25

    private FileMenu fileMenu = new FileMenu();
    private EditMenu editMenu = new EditMenu();
    private Menu windowMenu = new Menu("_Window");
    private Menu helpMenu = new Menu("_Help");



    /**
     *
     */
    public MainMenu() {
        menus.add(fileMenu);
        menus.add(editMenu);
        menus.add(windowMenu);
        menus.add(helpMenu);

        windowMenu.setAccelerator(KeyCombination.keyCombination("SHORTCUT+W"));
        helpMenu.setAccelerator(KeyCombination.keyCombination("SHORTCUT+H"));

        loadMenus();
    }

    /**
     *
     * @return
     */
    public FileMenu getFileMenu() {
        return fileMenu;
    }

    /**
     *
     * @return
     */
    public EditMenu getEditMenu() {
        return editMenu;
    }

    /**
     *
     * @return
     */
    public Menu getWindowMenu() {
        return windowMenu;
    }

    /**
     *
     * @return
     */
    public Menu getHelpMenu() {
        return helpMenu;
    }

    /**
     *
     * @param name
     * @return
     */
    public Menu getMenu(String name) {
        for (Menu menu : menus)
            if (menu.getText().equalsIgnoreCase(name)) {
                return menu;
            }

        return null;
    }

    /**
     *
     * @return
     */
    public Node getNode() {
        return menuBar;
    }

    /**
     *
     */
    private void loadMenus() {
        menuBar.getMenus().clear();

        for (Menu menu : menus) {
            menuBar.getMenus().add(menu);
        }
    }

    /**
     *
     * @param index
     * @param menuName
     */
    public Menu addMenu(int index, String menuName) {
        Menu menu = new Menu(menuName);
        menus.add(index, menu);
        loadMenus();

        return menu;
    }

    /**
     *
     * @param menuName
     */
    public void removeMenu(String menuName) {
        for (int i = 0; i < menus.size(); i++) {
            if (menus.get(i).getText().equalsIgnoreCase(menuName)) {
                menus.remove(i);
                break;
            }
        }
    }

}
