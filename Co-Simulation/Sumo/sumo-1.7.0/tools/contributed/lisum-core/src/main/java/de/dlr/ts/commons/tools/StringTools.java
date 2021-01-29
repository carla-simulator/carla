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
/// @file    StringTools.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.tools;

import java.util.Arrays;
import java.util.List;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class StringTools {
    /**
     * Repeats <code>charToRepeat</code> <code>times</code> times.
     *
     * @param charToRepeat
     * @param times
     * @return
     */
    public static String repeatChar(final String charToRepeat, int times) {
        String ret = "";
        for (int i = 0; i < times; i++) {
            ret += charToRepeat;
        }

        return ret;
    }

    /**
     *
     * @param text
     * @param width
     * @return
     */
    public static String centerText(final String text, int width) {
        String ret;
        int head = (width - text.length()) / 2;

        ret = repeatChar(" ", head);
        ret += text;
        ret += repeatChar(" ", width - (head + text.length()));

        return ret;
    }

    /**
     *
     * For example: <code>centerText("Text", 20, "**")</code> returns <pre><b>**     text    **</b></pre>.
     *
     * @param text
     * @param width
     * @param borders
     * @return
     */
    public static String centerText(String text, int width, String borders) {
        return borders + StringTools.centerText(text, width - (borders.length() * 2)) + borders;
    }

    /**
     * For example: <code>alignLeft("Text", 20, "#")</code> returns <pre><b># Text             #</b></pre>
     *
     * @param text Text to align left.
     * @param width Total width of the returned {@link String}.
     * @param borders Leading and trailing char of the returned {@link String}.
     * @return a {@link String} of length <code>witdh</code>,
     * begining and ending with <code>borders</code> and with <code>text</code> in the middle.
     */
    public static String alignLeft(String text, int width, String borders) {
        int space = width - (borders.length() + 1 + text.length() + borders.length());
        return borders + " " + text + repeatChar(" ", space) + borders;
    }

    public static String alignLeft(String text, int width) {
        int space = width - (1 + text.length());
        return " " + text + repeatChar(" ", space);
    }

    public static String alignRight(String text, int width, String borders) {
        int space = width - (borders.length() + 1 + text.length() + borders.length());
        return borders + " " + repeatChar(" ", space) + text + borders;
    }

    /**
     *
     * @param text
     * @param width
     * @param direction L = left, R = right, C = center
     * @return
     */
    public static String align(String text, int width, String direction) {
        if (direction.equalsIgnoreCase("L")) {
            return alignLeft(text, width);
        }
        if (direction.equalsIgnoreCase("R")) {
            return alignRight(text, width);
        }

        return centerText(text, width);
    }

    public static String align(int text, int width, String direction) {
        return align("" + text, width, direction);
    }

    public static String alignRight(String text, int width) {
        int space = width - (1 + text.length());
        return " " + repeatChar(" ", space) + text;
    }

    /**
     * Centers a text and adds leading and trailing characters with the
     * <code>charToFillWith</code> character and <code>length</code> times.
     * For example: <code>centerAndFillWithChar("Text", '#', 20)</code> returns
     * <b>####### Text #######</b>
     *
     * @param text Text to center
     * @param charToFillWith Character for filling
     * @param length
     * @return
     */
    public static String centerAndFillWithChar(final String text, char charToFillWith, int length) {
        if (text.length() + 2 > length) {
            return text;
        }

        String tmp = " " + text + " ";
        int rest = length - (text.length() + 2);

        for (int i = 0; i < rest / 2; i++) {
            tmp = charToFillWith + tmp;
        }

        for (int i = 0; i < (rest / 2) + rest % 2; i++) {
            tmp = tmp + charToFillWith;
        }

        return tmp;
    }

    /**
     * Aligns a text right and adds trailing characters with the
     * <code>charToFillWith</code> character and <code>length</code> times.
     * For example: <code>centerAndFillWithChar("Text", '#', 20)</code> returns
     * <b>Text ###############</b>
     *
     *
     * @param text
     * @param charToFillWith
     * @param length
     * @return
     */
    public static String alignLeftAndFillWithChar(final String text, char charToFillWith, int length) {
        if (text.length() + 1 > length) {
            return text;
        }

        String tmp = text + " ";

        for (int i = 0; i < length - (text.length() + 1); i++) {
            tmp = tmp + charToFillWith;
        }

        return tmp;
    }

    /**
     * Aligns a text right and adds leading characters with the
     * <code>charToFillWith</code> character and <code>length</code> times.
     * For example: <code>centerAndFillWithChar("Text", '#', 20)</code> returns
     * <b>############### Text</b>
     *
     * @param text
     * @param charToFillWith
     * @param length
     * @return
     */
    public static String alignRightAndFillWithChar(final String text, char charToFillWith, int length) {
        if (text.length() + 1 > length) {
            return text;
        }

        String tmp = " " + text;

        for (int i = 0; i < length - (text.length() + 1); i++) {
            tmp = charToFillWith + tmp;
        }

        return tmp;
    }

    /**
     *
     * @param originalString
     * @param chars
     * @param i
     * @return
     */
    @Deprecated
    public static String fillStringWithChars(String originalString, String chars, int i) {
        String ret = "";

        for (int j = 0; j < i - originalString.length(); j++) {
            ret += chars;
        }

        return ret;
    }



    /**
     *
     * @param string
     * @return
     */
    public static List<String> convertStringToList(String string) {
        return Arrays.asList(string.split(System.lineSeparator()));
    }

}
