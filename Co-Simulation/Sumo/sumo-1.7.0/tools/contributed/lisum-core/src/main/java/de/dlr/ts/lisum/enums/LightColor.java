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
/// @file    LightColor.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.enums;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public enum LightColor {
    OFF(0, 'O'),
    RED(3, 'r'),
    YELLOW_BLINKING(8, 'o'),
    YELLOW(12, 'y'),
    RED_YELLOW(15, 'u'),
    GREEN(48, 'G'),
    DARK_GREEN(-1 /*invalid*/, 'g');

    private final int ocitCode;
    private final char sumoCode;

    /**
     *
     * @param ocitCode
     */
    private LightColor(int ocitCode, char sumoCode) {
        this.ocitCode = ocitCode;
        this.sumoCode = sumoCode;
    }

    /**
     *
     * @return
     */
    public int getOcitCode() {
        return ocitCode;
    }

    public boolean isGreen() {
        return ocitCode == 48;
    }

    /**
     *
     * @return
     */
    public char getSumoCode() {
        return sumoCode;
    }

    /**
     *
     * @param code
     * @return
     */
    public static LightColor getPerOcitCode(int code) {
        for (LightColor sc : values())
            if (sc.ocitCode == code) {
                return sc;
            }

        return null;
    }

    /**
     *
     * @param code
     * @return
     */
    public static LightColor getPerSumoCode(char code) {
        for (LightColor sc : values())
            if (sc.sumoCode == code) {
                return sc;
            }

        return null;
    }
}
