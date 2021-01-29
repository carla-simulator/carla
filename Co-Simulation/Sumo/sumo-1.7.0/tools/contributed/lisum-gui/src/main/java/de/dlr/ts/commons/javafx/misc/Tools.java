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
package de.dlr.ts.commons.javafx.misc;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class Tools {
    /**
     *
     * @param webColor
     * @return
     */
    public static String webToKMLColor(String webColor) {
        String opa = webColor.substring(6);
        String first = webColor.substring(0, 2);
        String second = webColor.substring(2, 4);
        String third = webColor.substring(4, 6);

        return opa + third + second + first;
    }

    /**
     *
     * @param kmlColor
     * @return
     */
    public static String kmlToWebColor(String kmlColor) {
        String opa = kmlColor.substring(0, 2);
        String second = kmlColor.substring(2, 4);
        String third = kmlColor.substring(4, 6);
        String fourth = kmlColor.substring(6, 8);

        return fourth + third + second + opa;
    }
}
