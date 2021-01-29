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
/// @file    ToString.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.logger;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class ToString {

    private String className;
    private List<Add> adds = new ArrayList<Add>();

    /**
     *
     * @param name
     */
    public ToString(String name) {
        this.className = name;
    }

    /**
     *
     * @param name
     * @param value
     * @return
     */
    public ToString add(String name, String value) {
        adds.add(new Add(name, value));
        return this;
    }

    /**
     *
     * @param name
     * @param value
     * @return
     */
    public ToString add(String name, int value) {
        adds.add(new Add(name, value));
        return this;
    }

    /**
     *
     * @param name
     * @param value
     * @return
     */
    public ToString add(String name, double value) {
        adds.add(new Add(name, value));
        return this;
    }

    /**
     *
     * @param name
     * @param value
     * @return
     */
    public ToString add(String name, long value) {
        adds.add(new Add(name, value));
        return this;
    }

    /**
     *
     * @param name
     * @param value
     * @return
     */
    public ToString add(String name, byte value) {
        adds.add(new Add(name, value));
        return this;
    }

    /**
     *
     * @param name
     * @param value
     * @return
     */
    public ToString add(String name, boolean value) {
        adds.add(new Add(name, String.valueOf(value)));
        return this;
    }

    @Override
    public String toString() {
        String tmp = className + "{";

        for (Add add : adds) {
            tmp += add.toString() + " ";
        }

        return tmp.trim() + "}";
    }


    /**
     *
     */
    private static class Add {
        String name;
        Object value;

        /**
         *
         * @param name
         * @param value
         */
        public Add(String name, Object value) {
            this.name = name;
            this.value = value;
        }

        @Override
        public String toString() {
            String tmp  = name + "=";

            if (value instanceof String) {
                tmp += (String)value;
            } else if (value instanceof Integer) {
                tmp += (Integer)value;
            } else if (value instanceof Double) {
                tmp += (Double)value;
            } else if (value instanceof Long) {
                tmp += (Long)value;
            } else if (value instanceof Byte) {
                tmp += (Byte)value;
            }

            return tmp;
        }

    }

}
