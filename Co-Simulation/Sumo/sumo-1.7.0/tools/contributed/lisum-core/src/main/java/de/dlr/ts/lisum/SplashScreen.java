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
/// @file    SplashScreen.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
/*
 * Copyright (C) 2014
 * Deutsches Zentrum fuer Luft- und Raumfahrt e.V.
 * Institut fuer Verkehrssystemtechnik
 *
 * German Aerospace Center
 * Institute of Transportation Systems
 *
 */
package de.dlr.ts.lisum;

import de.dlr.ts.commons.tools.StringTools;
import de.dlr.ts.commons.utils.print.Color;
import de.dlr.ts.commons.utils.print.ColorString;
import de.dlr.ts.commons.utils.print.Effect;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class SplashScreen {

    private String programName = "";
    private String author = "";
    private String email = "";
    private String organization = "DLR";
    private int year = 2015;
    private List<String> comments = new ArrayList<String>();
    private List<Property> props = new ArrayList<Property>();
    private List<RuntimeOption> options = new ArrayList<RuntimeOption>();

    private long waitingTime = 0l;
    private int splashWidth = 80;
    private final String STARS_LINE = StringTools.repeatChar("*", splashWidth);
    private final String STARS_LINE_SHORT = "**";

    private boolean showCopyright = false;

    public void setShowCopyright(boolean showCopyright) {
        this.showCopyright = showCopyright;
    }

    /**
     *
     * @param email
     */
    public void setEmail(String email) {
        this.email = email;
    }

    /**
     *
     * @param organization
     */
    public void setOrganization(String organization) {
        this.organization = organization;
    }

    public void setYear(int year) {
        this.year = year;
    }

    /**
     *
     * @param programName
     */
    public SplashScreen(String programName) {
        this.programName = programName;
    }

    public void setAuthor(String author) {
        this.author = author;
    }

    /**
     *
     * @param modificator
     * @param variable
     * @param defaultValue
     */
    public void addRuntimeOption(String modificator, String variable, String defaultValue) {
        RuntimeOption r = new RuntimeOption();

        r.modificator = modificator;

        if (variable == null) {
            r.variable = "";
        } else {
            r.variable = variable;
        }

        r.defaultValue = defaultValue;

        options.add(r);
    }

    /**
     *
     */
    private class RuntimeOption {

        String modificator;
        String variable = "";
        String defaultValue = "";

        @Override
        public String toString() {
            ColorString modif = new ColorString(modificator, Color.WHITE, Effect.BOLD);
            String var = "";
            String defa = "";
            int width = splashWidth + modif.getOverheadLength();

            if (variable != null && !variable.isEmpty()) {
                var = " <" + variable.toUpperCase().replace(" ", "_") + "> ";
            }

            if (defaultValue != null && !defaultValue.isEmpty()) {
                defa = " : default " + defaultValue;
            }

            return StringTools.alignLeft(modif.toString() + var + defa, width, STARS_LINE_SHORT);
        }
    }

    /**
     *
     */
    private class Property {

        public Property(String name, String value) {
            this.name = name;
            this.value = value;
        }

        String name;
        String value;

        @Override
        public String toString() {
            ColorString _name = new ColorString(name, Color.WHITE, Effect.BOLD, Effect.UNDERLINE);
            return StringTools.alignLeft(_name.toString() + ": " + value, splashWidth + _name.getOverheadLength(), STARS_LINE_SHORT);
        }
    }

    /**
     *
     * @param name
     * @param value
     */
    public void addProperty(String name, String value) {
        props.add(new Property(name, value));
    }

    /**
     *
     * @param name
     * @param value
     */
    public void addProperty(String name, double value) {
        props.add(new Property(name, String.valueOf(value)));
    }

    /**
     *
     * @param name
     * @param value
     */
    public void addProperty(String name, long value) {
        props.add(new Property(name, String.valueOf(value)));
    }

    /**
     *
     * @param name
     * @param value
     */
    public void addProperty(String name, int value) {
        props.add(new Property(name, String.valueOf(value)));
    }

    /**
     *
     * @param name
     * @param value
     */
    public void addProperty(String name, boolean value) {
        props.add(new Property(name, String.valueOf(value)));
    }

    /**
     *
     * @param title
     * @return
     */
    private String getFormattedTitle(String title, Color color) {
        ColorString cs = new ColorString(title, color, Effect.BOLD, Effect.UNDERLINE);

        return StringTools.centerText(cs.toString(), splashWidth + cs.getOverheadLength(), "**");
    }

    /**
     *
     * @param author
     * @param color
     * @return
     */
    private String getFormattedAuthor(String author, Color color) {
        ColorString label = new ColorString("Author: ", Color.WHITE, Effect.BOLD);

        return StringTools.alignLeft(label.toString() + author + " <" + email + ">", splashWidth + label.getOverheadLength(), "**");
    }

    /**
     *
     * @param millis
     */
    public void setSplashScreenTime(long millis) {
        this.waitingTime = millis;
    }

    /**
     *
     * @param comment
     */
    public void addComment(String comment) {
        comments.add(StringTools.alignLeft(comment, splashWidth, "**"));
    }

    /**
     *
     */
    public void showSplashScreen() {
        System.out.println(getSplashScreen());

        if (waitingTime > 0) {
            waitForEnter();
        }
    }

    /**
     *
     */
    private void animation() {
        char[] animationChars = new char[] {'|', '/', '-', '\\'};

        for (int i = (int)(waitingTime / 1000); i >= 0; i--) {
            //System.out.print("||");

            System.out.print("Starting in " + i + " seconds " + animationChars[i % 4] + "\r");

            try {
                Thread.sleep(1000);
            } catch (InterruptedException ex) {
            }
        }
        System.out.println("Starting " + programName + "!");
    }

    /**
     *
     */
    private void waitForEnter() {
        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
        long startTime = System.currentTimeMillis();

        try {
            while ((System.currentTimeMillis() - startTime) < waitingTime && !in.ready()) {
                System.out.print("Press enter to start or wait " + (waitingTime / 1000 - ((System.currentTimeMillis() - startTime) / 1000) - 1) + " seconds" + "\r");
                Thread.sleep(500L);
            }
        } catch (InterruptedException ex) {
        } catch (IOException ex) {
            ex.printStackTrace();
        }

        System.out.println("\r");
        System.out.println("Program started!");
    }

    /**
     *
     */
    public void newBlankLine() {
        this.comments.add(System.lineSeparator());
    }

    /**
     *
     */
    public void newCommentsLine() {
        this.comments.add(STARS_LINE_SHORT);
    }

    /**
     *
     */
    public void newLineWithStars() {
        comments.add(STARS_LINE);
    }

    private void addCopyRight(StringBuilder sb) {
        sb.append(STARS_LINE).append(System.lineSeparator());
        sb.append(STARS_LINE).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("Copyright (C) " + year, splashWidth, "**")).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("Deutsches Zentrum fuer Luft- und Raumfahrt e.V.", splashWidth, "**")).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("Institut fuer Verkehrssystemtechnik", splashWidth, "**")).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("", splashWidth, "**")).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("German Aerospace Center", splashWidth, "**")).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("Institute of Transportation Systems", splashWidth, "**")).append(System.lineSeparator());
        sb.append(StringTools.alignLeft("http://www.dlr.de", splashWidth, "**")).append(System.lineSeparator());

        sb.append(STARS_LINE).append(System.lineSeparator());
    }

    /**
     *
     * @return
     */
    public String getSplashScreen() {
        StringBuilder sb = new StringBuilder();
        sb.append(System.lineSeparator());
        sb.append(System.lineSeparator());

        if (showCopyright) {
            addCopyRight(sb);
        }

        sb.append(STARS_LINE).append(System.lineSeparator());
        sb.append(StringTools.centerText("", splashWidth, "**")).append(System.lineSeparator());

        ColorString program_name = new ColorString(programName, Color.GREEN, Effect.BOLD, Effect.UNDERLINE);
        sb.append(StringTools.centerText(program_name.toString(), splashWidth + program_name.getOverheadLength(), "**")).append(System.lineSeparator());

        sb.append(StringTools.centerText("", splashWidth, "**")).append(System.lineSeparator());

        if (!author.isEmpty()) {
            sb.append(getFormattedAuthor(author, Color.NONE)).append(System.lineSeparator());
        }

        //sb.append(StringUtils.centerText("", splashWidth, "**")).append(System.lineSeparator());
        sb.append(STARS_LINE).append(System.lineSeparator());

        if (options.size() > 0) {
            sb.append(STARS_LINE).append(System.lineSeparator());
            sb.append(getFormattedTitle("Runtime options", Color.CYAN)).append(System.lineSeparator());
            sb.append(StringTools.centerText("", splashWidth, "**")).append(System.lineSeparator());

            for (RuntimeOption ro : options) {
                sb.append(ro).append(System.lineSeparator());
            }
            sb.append(STARS_LINE).append(System.lineSeparator());
        }

        if (props.size() > 0) {
            sb.append(STARS_LINE).append(System.lineSeparator());
            sb.append(getFormattedTitle("Current configuration", Color.CYAN)).append(System.lineSeparator());
            sb.append(StringTools.centerText("", splashWidth, "**")).append(System.lineSeparator());

            for (Property s : props) {
                sb.append(s).append(System.lineSeparator());
            }
            sb.append(STARS_LINE).append(System.lineSeparator());
        }

        if (comments.size() > 0) {
            sb.append(STARS_LINE).append(System.lineSeparator());
            for (String string : comments) {
                sb.append(string).append(System.lineSeparator());
            }
            sb.append(STARS_LINE).append(System.lineSeparator());
        }

        sb.append(STARS_LINE).append(System.lineSeparator());

        sb.append(System.lineSeparator());
        //sb.append(System.lineSeparator());

        return sb.toString();
    }

}
