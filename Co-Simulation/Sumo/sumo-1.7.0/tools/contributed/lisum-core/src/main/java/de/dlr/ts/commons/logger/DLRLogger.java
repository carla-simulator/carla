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
/// @file    DLRLogger.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.logger;

//import static de.dlr.ts.commons.network.tools.DateTools.getDateStringForLogger;
import static de.dlr.ts.commons.logger.DateTools.getDateStringForLogger;
import de.dlr.ts.commons.tools.StringTools;
import de.dlr.ts.commons.utils.print.Color;
import de.dlr.ts.commons.utils.print.ColorString;
import de.dlr.ts.commons.utils.print.Effect;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.StringWriter;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class DLRLogger {
    private static LogLevel currentLevel = LogLevel.INFO;
    private static WriteToDisk wtd = null;


    /**
     * Enables or disables writing log messages to disc and
     * sets the maximun size of the log directory.
     *
     * @param writeToDisk True to enable, false to disable.
     * @param logFolderSizeLimitInMB the maximum size of the log folder in megabytes.
     */
    @Deprecated
    public static void setWriteToDisk(boolean writeToDisk, double logFolderSizeLimitInMB) {
        if (writeToDisk) {
            wtd = new WriteToDisk(logFolderSizeLimitInMB);
            wtd.start();
        }
    }

    /**
     *
     * @return
     */
    public static PrintStream getSeverePrintStream() {
        return new PrintStream(new SevereOutputStream(), true);
    }

    /**
     *
     */
    private static class SevereOutputStream extends OutputStream {
        private StringBuilder string = new StringBuilder();

        @Override
        public void write(int b) throws IOException {
            //this.string.append((char) b );

            if (b != 13 && b != 10) {
                this.string.append((char) b);
            }

            /*
            else if(b == 10) {
                severe(string.toString());
                string = new StringBuilder();
            } */

        }

        @Override
        public void flush() throws IOException {
            severe(string.toString());
            //System.out.println(string);
            string = new StringBuilder();
        }
    }

    /**
     * Enables or disables writing log messages to disc and
     * sets the maximun size of the log directory.
     *
     * @param logFolderSizeLimitInMB the maximum size of the log folder in megabytes. Unabled if set to zero.
     */
    public static void setWriteToDisk(int logFolderSizeLimitInMB) {
        if (logFolderSizeLimitInMB != 0) {
            wtd = new WriteToDisk(logFolderSizeLimitInMB);
            wtd.start();
        }
    }

    /**
     * Enables or disables writing log messages to disc.
     *
     * @param writeToDisk True to enable, false to disable.
     */
    @Deprecated
    public static void setWriteToDisk(boolean writeToDisk) {
        setWriteToDisk(writeToDisk, 0);
    }

    /**
     *
     * @param object
     * @return
     */
    public static ToString toString(Object object) {
        if (object instanceof String) {
            return new ToString((String) object);
        }

        return new ToString(object.getClass().getSimpleName());
    }

    /**
     *
     * @return
     */
    public static LogLevel getCurrentLevel() {
        return currentLevel;
    }

    /**
     *
     * @param level
     */
    private DLRLogger(LogLevel level) {
        DLRLogger.currentLevel = level;
    }

    /**
     *
     * @param level
     */
    public static void setLevel(LogLevel level) {
        if (level != null) {
            DLRLogger.currentLevel = level;
        }
    }

    /**
     *
     * @param level
     * @return
     */
    private static ColorString getBarrita(LogLevel level) {
        switch (level) {
            case FINEST:
                return new ColorString(" | ", Color.BLUE, Effect.BOLD);
            case FINER:
                return new ColorString(" | ", Color.CYAN, Effect.BOLD);
            case FINE:
                return new ColorString(" | ", Color.CYAN, Effect.BOLD);
            case CONFIG:
                return new ColorString(" | ", Color.MAGENTA, Effect.BOLD);
            case INFO:
                return new ColorString(" | ", Color.GREEN, Effect.BOLD);
            case WARNING:
                return new ColorString(" | ", Color.YELLOW, Effect.BOLD);
            case SEVERE:
                return new ColorString(" | ", Color.RED, Effect.BOLD);
            default:
                break;
        }

        return new ColorString(" | ", Color.BLANK, Effect.BOLD);
    }

    /**
     *
     * @param text
     * @return
     */
    public static String config(String text) {
        return config(null, text);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String config(Object module, String text) {
        return config(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String config(Object module, MyString myString) {
        return print(module, myString, LogLevel.CONFIG);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String config(MyString myString) {
        return config(null, myString);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String config(Object module, Color color, String text) {
        return config(module, new MyString(text, color));
    }

    /**
     *
     * @param text
     * @param color
     */
    private static String colorIt(String text, Color color) {
        if (color != null) {
            return ColorString.string(text, color, Effect.BOLD);
        }

        return text;
    }

    /**
     *
     * @param text
     * @return
     */
    public static String finer(String text) {
        return finer(null, text);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String finer(Object module, String text) {
        return finer(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String finer(Object module, MyString myString) {
        return print(module, myString, LogLevel.FINER);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String finer(MyString myString) {
        return finer(null, myString);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String finer(Object module, Color color, String text) {
        return finer(module, new MyString(text, color));
    }

    /**
     *
     * @param text
     * @return
     */
    public static String fine(String text) {
        return fine(null, text);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String fine(Object module, String text) {
        return fine(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String fine(Object module, Color color, String text) {
        return fine(module, new MyString(text, color));
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String fine(Object module, MyString myString) {
        return print(module, myString, LogLevel.FINE);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String fine(MyString myString) {
        return fine(null, myString);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    private static String module(Object module, String text) {
        if (module != null) {
            if (module instanceof String) {
                text = "[" + ColorString.string((String) module, Color.WHITE, Effect.BOLD) + "] " + text;
            } else {
                text = "[" + ColorString.string(module.getClass().getSimpleName(), Color.WHITE, Effect.BOLD) + "] " + text;
            }
        }

        return text;
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    private static String modulePlain(Object module, String text) {
        if (module != null) {
            if (module instanceof String) {
                text = "[" + (String) module + "] " + text;
            } else {
                text = "[" + module.getClass().getSimpleName() + "] " + text;
            }
        }

        return text;
    }


    /**
     *
     * @param text
     * @return
     */
    public static String finest(String text) {
        return finest(null, text);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String finest(Object module, String text) {
        return finest(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String finest(Object module, Color color, String text) {
        return finest(module, new MyString(text, color));
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String finest(Object module, MyString myString) {
        return print(module, myString, LogLevel.FINEST);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String finest(MyString myString) {
        return finest(null, myString);
    }

    /**
     *
     * @param text
     * @return
     */
    public static String warning(String text) {
        return warning(null, text);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String warning(Object module, String text) {
        return warning(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String warning(Object module, Color color, String text) {
        return warning(module, new MyString(text, color));
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String warning(Object module, MyString myString) {
        return print(module, myString, LogLevel.WARNING);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String warning(MyString myString) {
        return warning(null, myString);
    }

    /**
     *
     * @param text
     * @param separator
     * @return
     */
    public static String getLine(String text, String separator) {
        return getDateStringForLogger(System.currentTimeMillis()) + separator + text;
    }

    /**
     *
     * @param text
     * @return
     */
    public static String severe(String text) {
        return severe(null, text);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String severe(Object module, String text) {
        return severe(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param throwable
     * @return
     */
    public static String severe(Object module, Throwable throwable) {
        StringWriter sw = new StringWriter();
        throwable.printStackTrace(new PrintWriter(sw));
        String exceptionAsString = sw.toString();

        return severe(module, Color.NONE, exceptionAsString);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String severe(Object module, Color color, String text) {
        return severe(module, new MyString(text, color));
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String severe(Object module, MyString myString) {
        return print(module, myString, LogLevel.SEVERE);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String severe(MyString myString) {
        return severe(null, myString);
    }

    /**
     *
     * @param text
     * @return
     */
    public static String info(String text) {
        return info(null, text);
    }

    /**
     *
     * @param myString
     * @return
     */
    public static String info(MyString myString) {
        return info(null, myString);
    }

    /**
     *
     * @param module
     * @param text
     * @return
     */
    public static String info(Object module, String text) {
        return info(module, Color.NONE, text);
    }

    /**
     *
     * @param module
     * @param color
     * @param text
     * @return
     */
    public static String info(Object module, Color color, String text) {
        return info(module, new MyString(text, color));
    }

    /**
     *
     * @param module
     * @param myString
     * @return
     */
    public static String info(Object module, MyString myString) {
        return print(module, myString, LogLevel.INFO);
    }

    /**
     *
     * @param level
     * @param text
     * @return
     */
    @Deprecated
    public static String log(LogLevel level, String text) {
        if (wtd != null && currentLevel.ordinal() >= level.ordinal()) {
            wtd.addLogLine(
                getDateStringForLogger(System.currentTimeMillis()) + " | "
                + StringTools.centerText(level.toString(), 7) + " | "
                + text);
        }

        if (level.ordinal() <= currentLevel.ordinal()) {
            ColorString barrita = getBarrita(level);
            String line = getLine(text, barrita.toString());
            System.out.println(line);

            return line;
        }

        return "";
    }

    /**
     *
     * @param module
     * @param myString
     * @param logLevel
     */
    private static String print(Object module, MyString myString, LogLevel logLevel) {
        if (wtd != null && currentLevel.ordinal() >= logLevel.ordinal()) {
            wtd.addLogLine(
                getDateStringForLogger(System.currentTimeMillis()) + " | "
                + StringTools.centerText(logLevel.toString(), 7) + " | "
                + modulePlain(module, myString.getPlainString()));
        }

        String text = "";

        if (currentLevel.ordinal() >= logLevel.ordinal()) {
            ColorString barrita = getBarrita(logLevel);
            //text = colorIt(text, color);
            text = myString.toString();
            text = module(module, text);

            String line = getLine(text, barrita.toString());

            if (logLevel != LogLevel.SEVERE) {
                System.out.println(line);
            } else {
                System.err.println(line);
            }

            return line;
        }

        return text;
    }
}
