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
/// @file    DLRProperties.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.tools;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class DLRProperties {

    private List<Property> props = new ArrayList<Property>();

    /**
     *
     * @param xmlFileName
     */
    public void loadXML(String xmlFileName) {
        Properties ss = new Properties();
    }

    /**
     *
     * @param key
     * @return
     */
    public String getProperty(String key) {
        return getProperty(key, null);
    }

    /**
     *
     * @param key
     * @param defaultValue
     * @return
     */
    public String getProperty(String key, String defaultValue) {
        for (Property property : props) {
            if (property.name.equals(key)) {
                return property.value;
            }
        }

        return defaultValue;
    }

    /**
     *
     * @param key
     * @param defaultValue
     * @return
     */
    public long getProperty(String key, long defaultValue) {
        return Long.valueOf(getProperty(key, String.valueOf(defaultValue)));
    }

    /**
     *
     * @param key
     * @param defaultValue
     * @return
     */
    public boolean getProperty(String key, boolean defaultValue) {
        return Boolean.valueOf(getProperty(key, String.valueOf(defaultValue)));
    }

    /**
     *
     * @param key
     * @param defaultValue
     * @return
     */
    public int getProperty(String key, int defaultValue) {
        return Integer.valueOf(getProperty(key, String.valueOf(defaultValue)));
    }

    /**
     *
     * @param args
     */
    public void load(String... args) {
        if (args == null) {
            return;
        }

        for (int i = 0; i < args.length; i++) {
            if (args[i].startsWith("-")) {
                String value;

                if (i + 1 < args.length) {
                    value = args[i + 1];
                } else {
                    value = "true";
                }

                if (value.startsWith("-")) { //there is no value assigned for this modificator
                    value = "true";         //just store true
                }

                props.add(new Property(args[i], value));
            }
        }
    }

    /**
     *
     * @param name
     * @param value
     */
    public void setProperty(String name, String value) {
        props.add(new Property(name, value));
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        for (Property p : props) {
            sb.append(p);
            sb.append(System.lineSeparator());
        }

        return sb.toString();
    }

    /**
     *
     */
    class Property {

        String name;
        String value;

        public Property(String name, String value) {
            this.name = name;
            this.value = value;
        }

        @Override
        public String toString() {
            return name + " = " + value;
        }
    }
}
