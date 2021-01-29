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
/// @file    Color.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.utils.print;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public enum Color {
    NONE("", ""),
    BLANK("30", "40"),
    RED("31", "41"),
    GREEN("32", "42"),
    YELLOW("33", "43"),
    BLUE("34", "44"),
    MAGENTA("35", "45"),
    CYAN("36", "46"),
    WHITE("37", "47"),
    ORANGE("38;5;214", "48;5;208"),
    REDYELLOW("38;5;214", "48;5;208"),
    OFF("37", "47");

    private final String fore;
    private final String back;

    private Color(String fore, String back) {
        this.fore = fore;
        this.back = back;
    }

    public String getForegroundCode() {
        return fore;
    }

    public String getBackgroundCode() {
        return back;
    }
}
