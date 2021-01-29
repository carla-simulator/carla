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
/// @file    MyString.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.logger;

import de.dlr.ts.commons.utils.print.Color;
import de.dlr.ts.commons.utils.print.ColorString;
import de.dlr.ts.commons.utils.print.Effect;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MyString {
    private List<Component> components = new ArrayList<Component>();


    /**
     *
     * @return
     */
    public static MyString create() {
        return new MyString();
    }

    /**
     *
     * @param text
     * @return
     */
    public static MyString create(String text) {
        return new MyString(text);
    }

    /**
     *
     * @param text
     * @param color
     * @return
     */
    public static MyString create(String text, Color color) {
        return new MyString(text, color);
    }

    /**
     *
     */
    public MyString() {
    }

    /**
     *
     * @param text
     */
    public MyString(final String text) {
        components.add(new Component(text));
    }

    /**
     *
     * @param text
     * @param color
     */
    public MyString(final String text, Color color) {
        if (color == Color.NONE || color == null) {
            components.add(new Component(text));
        } else {
            components.add(new Component(text, color));
        }
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString string(final String text) {
        components.add(new Component(text));

        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString red(final String text) {
        components.add(new Component(text, Color.RED));
        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString green(final String text) {
        components.add(new Component(text, Color.GREEN));
        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString yellow(final String text) {
        components.add(new Component(text, Color.YELLOW));
        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString cyan(final String text) {
        components.add(new Component(text, Color.CYAN));
        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString magenta(final String text) {
        components.add(new Component(text, Color.MAGENTA));
        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString orange(final String text) {
        components.add(new Component(text, Color.ORANGE));
        return this;
    }

    /**
     *
     * @param text
     * @return
     */
    public MyString blue(final String text) {
        components.add(new Component(text, Color.BLUE));
        return this;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        for (Component c : components)
            if (c.color != null) {
                sb.append(ColorString.string(c.text, c.color, Effect.BOLD));
            } else {
                sb.append(c.text);
            }

        return sb.toString();
    }

    /**
     *
     * @return
     */
    public String getPlainString() {
        StringBuilder sb = new StringBuilder();

        for (Component c : components) {
            sb.append(c.text);
        }

        return sb.toString();
    }

    /**
     *
     */
    private class Component {
        String text;
        Color color = null;

        /**
         *
         * @param text
         */
        public Component(String text) {
            this.text = text;
        }

        /**
         *
         * @param text
         * @param color
         */
        public Component(String text, Color color) {
            this(text);
            this.color = color;
        }
    }
}
