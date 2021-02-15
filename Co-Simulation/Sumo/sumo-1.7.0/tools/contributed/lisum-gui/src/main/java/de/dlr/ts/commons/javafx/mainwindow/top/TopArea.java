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
/// @file    TopArea.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.mainwindow.top;

import javafx.scene.Node;
import javafx.scene.layout.AnchorPane;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class TopArea {
    private final AnchorPane root = new AnchorPane();

    private double menuHeight = 25d;
    private double toolBarHeight = 35d;

    private MainToolBar mainToolBar = new MainToolBar();
    private MainMenu mainMenu = new MainMenu();

    private boolean toolBarVisible = true;
    private boolean menuBarVisible = true;

    /**
     *
     */
    public TopArea() {
        this(true, true);
    }

    /**
     *
     * @param menuBarVisible
     * @param toolBarVisible
     */
    public TopArea(boolean menuBarVisible, boolean toolBarVisible) {
        setHeight();

        if (toolBarVisible) {
            setToolBarVisible(true);
        }
        if (menuBarVisible) {
            setMenuBarVisible(true);
        }
    }


    /**
     *
     */
    public void setBigToolBar() {
        double tmp = toolBarHeight + 8d;

        if (menuBarVisible) {
            tmp += menuHeight;
        }

        root.setPrefHeight(tmp);
    }

    /**
     *
     * @return
     */
    public MainToolBar getMainToolBar() {
        return mainToolBar;
    }

    /**
     *
     * @return
     */
    public MainMenu getMainMenu() {
        return mainMenu;
    }

    /**
     *
     * @return
     */
    public Node getNode() {
        return root;
    }

    private void setHeight() {
        double tmp = 0d;

        if (menuBarVisible) {
            tmp += menuHeight;
        }
        if (toolBarVisible) {
            tmp += toolBarHeight;
        }

        root.setPrefHeight(tmp);
    }

    /**
     *
     * @param visible
     */
    public final void setMenuBarVisible(boolean visible) {
        AnchorPane.setTopAnchor(mainMenu.getNode(), 0.);
        AnchorPane.setRightAnchor(mainMenu.getNode(), 0.);
        AnchorPane.setLeftAnchor(mainMenu.getNode(), 0.);

        menuBarVisible = visible;

        if (visible) {
            if (!root.getChildren().contains(mainMenu.getNode())) {
                root.getChildren().add(mainMenu.getNode());
                setHeight();
            }
        } else {
            if (root.getChildren().contains(mainMenu.getNode())) {
                root.getChildren().remove(mainMenu.getNode());
                setHeight();
            }
        }

        setToolBarVisible(toolBarVisible);
    }

    /**
     *
     * @param visible
     */
    public final void setToolBarVisible(boolean visible) {
        double gg = 25d;
        if (!menuBarVisible) {
            gg = 0d;
        }

        AnchorPane.setTopAnchor(mainToolBar.getNode(), gg);
        AnchorPane.setRightAnchor(mainToolBar.getNode(), 0d);
        AnchorPane.setLeftAnchor(mainToolBar.getNode(), 0d);
        AnchorPane.setBottomAnchor(mainToolBar.getNode(), 0d);

        toolBarVisible = visible;

        if (visible) {
            if (!root.getChildren().contains(mainToolBar.getNode())) {
                root.getChildren().add(mainToolBar.getNode());
                setHeight();
            }
        } else {
            if (root.getChildren().contains(mainToolBar.getNode())) {
                root.getChildren().remove(mainToolBar.getNode());
                setHeight();
            }
        }
    }
}
