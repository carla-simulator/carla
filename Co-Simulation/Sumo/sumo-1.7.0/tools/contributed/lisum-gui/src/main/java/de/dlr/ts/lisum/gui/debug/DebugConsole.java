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
/// @file    DebugConsole.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.debug;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import javafx.scene.control.TextArea;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class DebugConsole {
    private static final DebugConsole INSTANCE = new DebugConsole();
    private final PrintStream DEFAULT_PRINT_STREAM = System.out;
    private final PrintStream emptyPs;
    private PrintStream textAreaPs;


    /**
     *
     * @return
     */
    public static DebugConsole getInstance() {
        return INSTANCE;
    }

    /**
     *
     */
    public DebugConsole() {
        emptyPs = new PrintStream(new EmptyConsole(), true);
    }

    /**
     *
     */
    public void setDefaultPrintOutput() {
        System.setOut(DEFAULT_PRINT_STREAM);
        System.setErr(DEFAULT_PRINT_STREAM);
    }

    /**
     *
     * @param textArea
     */
    public void setTextArea(TextArea textArea) {
        textAreaPs = new PrintStream(new TextAreaConsole(textArea), true);
    }

    /**
     *
     */
    public void setEmptyPrintOutput() {
        System.setOut(emptyPs);
        System.setErr(emptyPs);
    }

    /**
     *
     */
    public void setTextAreaPrintOutput() {
        System.setOut(textAreaPs);
        System.setErr(textAreaPs);
    }

    /**
     *
     */
    private class EmptyConsole extends OutputStream {

        @Override
        public void write(int b) throws IOException {
        }
    }
}
