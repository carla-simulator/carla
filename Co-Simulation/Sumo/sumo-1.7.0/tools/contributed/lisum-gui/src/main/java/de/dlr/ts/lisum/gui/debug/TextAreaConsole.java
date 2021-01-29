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
/// @file    TextAreaConsole.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.debug;

import java.io.IOException;
import java.io.OutputStream;
import javafx.application.Platform;
import javafx.scene.control.TextArea;

/**
 *
 */
class TextAreaConsole extends OutputStream {
    private final TextArea txtArea;
    private final StringBuilder buffer = new StringBuilder(128);


    /**
     *
     * @param txtArea
     */
    public TextAreaConsole(TextArea txtArea) {
        this.txtArea = txtArea;
    }

    /**
     *
     * @param b
     * @throws IOException
     */
    @Override
    public synchronized void write(int b) throws IOException {
        buffer.append((char) b);

        if (b == 10) {
            String aa = buffer.toString();
            buffer.delete(0, buffer.length());
            Platform.runLater(() -> { txtArea.appendText(aa); });
        }
    }
}
