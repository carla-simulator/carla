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
/// @file    Message.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.messages;

import javafx.scene.control.Alert;
import javafx.scene.image.Image;
import javafx.stage.Stage;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public abstract class Message {
    protected static String programName = null;
    protected static Image icon = null;
    protected Alert alert;


    /**
     *
     * @param name
     */
    public static void setProgramName(String name) {
        Message.programName = name;
    }

    /**
     *
     */
    protected final void init() {
        if (icon != null) {
            Stage st = (Stage)alert.getDialogPane().getScene().getWindow();
            st.getIcons().add(Message.icon);
        }

        if (programName != null) {
            alert.setTitle(programName);
        } else {
            alert.setTitle("Information");
        }
    }

    /**
     *
     * @param icon
     */
    public static void setIcon(Image icon) {
        Message.icon = icon;
    }

    /**
     *
     */
    public final void show() {
        alert.showAndWait();
    }
}
