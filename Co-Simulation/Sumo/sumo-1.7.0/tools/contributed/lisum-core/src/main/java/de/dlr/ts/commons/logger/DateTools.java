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
/// @file    DateTools.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.logger;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class DateTools {

    private static final DateFormat dfmt = new SimpleDateFormat("yyyyMMdd HH:mm:ss.SSS");
    private static final DateFormat dfmt2 = new SimpleDateFormat("yyyyMMdd_HH.mm.ss.SSS");
    private static final DateFormat dfmt3 = new SimpleDateFormat("yyyyMMdd_HH.mm.ss");

    /**
     *
     * Returns date with the format <i>20150910 10:10:43.059</i>
     *
     * @param millis
     * @return
     */
    public static String getDateStringForLogger(long millis) {
        return dfmt.format(new Date(millis));
    }

    /**
     *
     * Returns date with the format <i>20150910_10.16.13.481</i>
     *
     * @param millis
     * @return
     */
    public static String getDateStringForFileNameWithMillis(long millis) {
        return dfmt2.format(new Date(millis));
    }

    /**
     *
     * Returns date with the format <i>20150910_10.16.13</i>
     *
     * @param millis
     * @return
     */
    public static String getDateStringForFileNameNoMillis(long millis) {
        return dfmt3.format(new Date(millis));
    }
}
